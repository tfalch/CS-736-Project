#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#include "internal.h"

/* comment out for production testing. */
#define VERIFY_FLG 1 
#undef VERIFY_FLG // Blocking for large page sizes. disabling.

#ifndef DEBUG
#define DEBUG 1
#endif 

/* comment/uncomment to enable/disable DEBUG_PRINT */

//#ifdef DEBUG
//#undef DEBUG
//#endif

#ifdef DEBUG
#define PRINT_FX_NAME printk(KERN_EMERG __FUNCTION__);
#define PRINT_LOCATION printk(KERN_EMERG __LINE__);
#define DEBUG_PRINT(fmt, ...) \
  printk(KERN_EMERG fmt, __VA_ARGS__); \
  printk(KERN_EMERG "\n");
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#define MAX_PAGE_RETRIEVAL_ATTEMPTS 2
#define MAX_NUM_CHAINS 5

#define MEMORY_CHAIN_IS_FULL(chain) \
  chain->attributes != NULL && chain->attributes->is_bounded ? \
    chain->nr_links >= chain->attributes->max_links : 0

/**
 * @name __new_mem_chain
 * @description creates and initializes a new memory_chain object.
 * @inparam id chain's identifier.
 * @return initialized memory_chain object.
 */
static memory_chain_t * __new_mem_chain(unsigned int id) {

    memory_chain_t * chain = kmalloc(sizeof(memory_chain_t), GFP_KERNEL); //| GPR_ATOMIC);

    chain->id = id;
    chain->attributes = NULL;
    chain->head = NULL;
    chain->tail = NULL;
    chain->anchor = NULL;
    chain->delegate = NULL;
    chain->nr_links = 0;
    atomic_set(&chain->ref_counter, 0);
   
    spin_lock_init(&chain->lock);  

    return chain;
}

static void inline __reset_page(struct page * page,int clr_flgs) {
  
    struct memory_chain * chain = page->chain;

    page->next = NULL;
    page->prev = NULL;
    page->chain = NULL;
      
    if (clr_flgs && __PageReferenced(page)) {
        __ClearPageReferenced(page);
	atomic_dec(&chain->ref_counter);
    }
}

/**
 * @name __unlink_page
 * @description unlinks the specified page from containing chain.
 * @inparam pg page object to be unlinked.
 */
void __unlink_page(struct page * pg) {
    memory_chain_t * chain = pg->chain;

    if (chain != NULL) {

        if (pg->prev == NULL) { // head of chain
	    chain->head = pg->next;
	    if (chain->head != NULL) {
	        chain->head->prev = NULL;;
	    }
	} else {
	    struct page * previous = pg->prev;
	  
	    previous->next = pg->next;
	    if (pg->next != NULL) // not tail of chain
	        pg->next->prev = previous;
	    else 
	        chain->tail = previous;
	}

	if (chain->anchor == pg) {
	    chain->anchor = NULL;
	}

	if (chain->delegate == pg) {
	    chain->delegate = NULL;
	}
	
	chain->nr_links--;
	if (!__PageReferenced(pg)) {
	    atomic_dec(&chain->ref_counter);
	}

	__reset_page(pg, 0);

	DEBUG_PRINT("unlinked %p from chain %u, #(links)=%lu, "	 \
		    "ref_counter=%d", pg, chain->id, chain->nr_links, 
		    atomic_read(&chain->ref_counter));
    }
}

static struct page * __evict_page(memory_chain_t * chain) {
    return NULL;
}

static struct page * __link_page(memory_chain_t * chain, struct page * pg) {

    struct page * victim = NULL;
    
    // determine if pages is linked to chain previously.
    if (chain == pg->chain) {
        return NULL;
    }
    
    // check if the page is in another chain, if so, unlink page.
    if (unlikely(pg->chain != NULL)) {
        spin_lock(&pg->chain->lock);
	__unlink_page(pg);
	spin_unlock(&pg->chain->lock);
    }
 
    // if the chain is full, choose a victim for evicition.
    if (unlikely(MEMORY_CHAIN_IS_FULL(chain))) {
        victim = __evict_page(chain);

	spin_lock(&victim->chain_lock);
	__unlink_page(victim);
	spin_unlock(&victim->chain_lock);
    }

    // initialize page
    pg->chain = chain;
    pg->next = pg->prev = NULL;

    // determine if first page. 
    if (unlikely(chain->head == NULL)) {
        chain->head = pg;
	chain->tail = pg;
    } else {
        chain->head->prev = pg;
	pg->next = chain->head;
	chain->head = pg;
    }
    
    chain->nr_links++;
    if (__PageReferenced(pg)) {
        atomic_inc(&chain->ref_counter);
    }

    DEBUG_PRINT("link_page(): added page: %p to chain: %d, "
		"length(chain) = %lu", pg, chain->id, 
		chain->nr_links);
    
    return victim;
}

/** 
 * @name __find_free_slot
 * @description returns the first free slot in the chain array.
 * @inparam array array of memory chain object.
 * @inparam len array size.
 * @returns first available free slot. -1 if no slots are available.
 * @constraints len % 5 == 0 for loop unrolling. 
 */
static int __find_free_slot(memory_chain_t * array[], size_t len) {

  int slot = 0;
  for (; slot < len; slot++) {
      if (!array[slot])
         return slot;
      if (!array[++slot])
         return slot;
      if (!array[++slot])
         return slot;
      if (!array[++slot])
         return slot;
      if (!array[++slot])
         return slot;
  }

  return -1;
}

static inline void __init_mcc(struct task_struct * tsk) {

    struct memory_chain_collection * mcc =
      kmalloc(sizeof(struct memory_chain_collection),
	      GFP_KERNEL);
    
    mcc->count = 0;
    mcc->capacity = MAX_NUM_CHAINS;;
    mcc->chains = kmalloc(sizeof(memory_chain_t *) * 
			  MAX_NUM_CHAINS, GFP_KERNEL);;
    /* zero out chain array */
    memset(mcc->chains, 0, 
	   sizeof(memory_chain_t *) * MAX_NUM_CHAINS); 
    spin_lock_init(&mcc->lock);
    
    tsk->mcc = mcc;
}

SYSCALL_DEFINE0(new_mem_chain) {

    int slot = -2; // error-code: Too many open chains
    memory_chains * chain_collection = NULL;

    if ((chain_collection = current->mcc) == NULL) {
        __init_mcc(current);
	chain_collection = current->mcc;
    }

    spin_lock(&chain_collection->lock);

    /* determine # of open chains */
    if (chain_collection->count < chain_collection->capacity) {
      
        slot = __find_free_slot(chain_collection->chains,
				chain_collection->capacity);
	chain_collection->chains[slot] = __new_mem_chain(slot);
	chain_collection->count++;
    }

    DEBUG_PRINT("new_mem_chain(): tsk[chains=%p, nr=%d, max=%d, slot=%d]",
		chain_collection->chains, 
		chain_collection->count,
		chain_collection->capacity,
		slot);

    spin_unlock(&chain_collection->lock);
    
    return slot;
}

asmlinkage long sys_set_mem_chain_attr(unsigned int c,
				       const memory_chain_attr_t * attr) {
    return 0;
}

static inline int stack_guard_page(struct vm_area_struct * vma, 
				   unsigned long addr) {
    return (vma->vm_flags & VM_GROWSDOWN) &&
        (vma->vm_start == addr) &&
        !vma_stack_continue(vma->vm_prev, addr);
}

static inline struct page * __get_user_page(struct vm_area_struct * vma,
					    unsigned long addr) {
  
    int counter = 0;
    int foll_flags = FOLL_TOUCH | FOLL_FORCE;
    struct page * page= NULL;

    cond_resched();
    while (!(page = follow_page(vma, addr, foll_flags))) {
        unsigned int fault_flags = FAULT_FLAG_ALLOW_RETRY;
	int ret = handle_mm_fault(vma->vm_mm, vma, addr,
				  fault_flags);

	if (ret & VM_FAULT_ERROR ||
	    ret & VM_FAULT_RETRY) {

	    DEBUG_PRINT("Failed to get page at address: %lu",
			addr);
	    return NULL;
	}

	counter++;
	if (counter >= MAX_PAGE_RETRIEVAL_ATTEMPTS) {
	  DEBUG_PRINT("Too many attempts to retrieve page at address: %lu",
		      addr);
	  break;
	}
	  
	cond_resched();
    }

    return page;
}

/**
 * @name _mlink_vma_pages_range
 * @description links pages found in the provided vma to the specified memory
 * chain object. If an anchored address is provided, the page at the 
 * specified address is anchored in the chain. 
 * @inparam chain chain object pages in provided range should be linked to.
 * @inparam vma vma object containing specified address range. 
 * @inparam start start address within the vam of the addres range. 
 * @inparam end end address within the vma of the address range.
 * @inparam anchor address of anchor page.
 * @return 0 on success; -1 otherwise. 
 */
static long __mlink_vma_pages_range(memory_chain_t * chain,
				    struct vm_area_struct * vma, 
				    unsigned long start, unsigned long end,
				    unsigned long anchor)
{
    unsigned long addr;
    struct page * page = NULL;

    DEBUG_PRINT("mlink_vma_pages_range(): vma-range[s=%lu, e=%lu]; " \
		"range[start=%lu, end=%lu]", vma->vm_start, vma->vm_end, 
		start, end);

    /* validate page ranges within vma. */
    BUG_ON(start < vma->vm_start || end > vma->vm_end);

    /* do not try to access the guard page of a stack vma */
    if (stack_guard_page(vma, start)) 
        start += PAGE_SIZE;

    for (addr = start; addr < end; addr += PAGE_SIZE) {

        page = __get_user_page(vma, addr);
	
	if (page != NULL) {
	  
	    spin_lock(&page->chain_lock);
	  
	    __link_page(chain, page);
	    
	    if (unlikely(start == anchor)) 
	        chain->anchor = page;

	    spin_unlock(&page->chain_lock);
	    
	    DEBUG_PRINT("mlink_vma_pages_range(): linked(address): %lu " \
			"in vma: %lu", start, (unsigned long)vma);
	} else {
	    DEBUG_PRINT("mlink_vma_pages_range(): no page found at %lu\n", 
			addr);
	}
    }

#ifdef VERIFY_FLG // validation check. 
    {
        int nonblocking = 0;
        int nr_pages = (end - start) / PAGE_SIZE;
	struct page ** pages = kmalloc(sizeof(struct page *) * nr_pages, 
				       GFP_KERNEL);
	int n = 0;
	int i = 0;
	
	down_read(&vma->vm_mm->mmap_sem);
	n = __get_user_pages(current, vma->vm_mm, start, nr_pages,
			     FOLL_TOUCH, pages, NULL, &nonblocking);
	up_read(&vma->vm_mm->mmap_sem);
	
	DEBUG_PRINT("nr-links=%lu, nr-pages=%d, n=%d", chain->nr_links, 
		    nr_pages, n);

	for (i = 0; i < n; i++) {
	    if (pages[i] != NULL) {
	        if (pages[i]->chain != chain) {
		  printk(KERN_EMERG "pages[%d] at %p has invalid container. " \
			 "expected %p, but found %p", i, pages[i], chain, 
			 pages[i]->chain);
		}
		
		BUG_ON(pages[i]->chain != chain);
	    }
	}

	kfree(pages);
    }
#endif
    
    return 0;
}

/**
 * @name do_mlink_pages
 * @description links pages in the provided address range with the specified 
 * memory chain object. If an anchored address is provided, the page at the 
 * specified address is anchored in the chain. 
 *
 * @inparam chain chain object pages in provided range should be linked to.
 * @inparam start start address of address range.
 * @inparam len length of address range to be linked.
 * @inparam anchor address of anchor page.
 * @return 0 on success; -1 otherwise. 
 */
static long do_mlink_pages(struct memory_chain * chain, 
			   unsigned long start, size_t len,
			   unsigned long anchor) {

    int r = -1; // return code.
    unsigned long end = start + len;
    unsigned long s = start; // current vma's start address. 
    unsigned long e = end;   // current vma's end address.

    struct mm_struct * mm = current->mm;
    struct vm_area_struct * vma = find_vma(mm, s); // retrieve first vma. 

    /* check valid vma returned. */
    if (!vma || vma->vm_start >= end)
        return -1;

    DEBUG_PRINT("do_mlink_pages(): range[start=%lu, end=%lu]", start, end);

    for (s = start; s < end; s = e) {

        /* determine if current address lies outside range 
	   of current vma; if so move to next vma. */
		if (s >= vma->vm_end) 
		    vma = vma->vm_next;

		if (!vma || vma->vm_start >= end){
		    return -1;
		}

		/* determine current vma's end address. */
		e = end < vma->vm_end ? end : vma->vm_end;
      
		/* determine current vma's start address. */
        if (s < vma->vm_start)
	    	s = vma->vm_start;

		r = __mlink_vma_pages_range(chain, vma, s, e, anchor);
    }

    return r;
}

SYSCALL_DEFINE3(link_addr_rng, unsigned int, c, unsigned long, start,
		size_t, len) {

    int error = -1; // Error code: Invalid Chain id. 

    struct memory_chain * chain = NULL;
    struct memory_chain_collection * chain_collection = current->mcc;

    if (chain_collection == NULL) {
	return -1;
    }
    
    /* acquire chain collection's lock. */
    spin_lock(&chain_collection->lock);

    /* check valid chain id provided. */
    if (c >= chain_collection->capacity ||
	(chain = chain_collection->chains[c]) == NULL) {
        spin_unlock(&chain_collection->lock);
        return error;
    }

    len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
    start &= PAGE_MASK;

    DEBUG_PRINT("link_addr_rng() chain=%u, range[start=%lu, length=%u",
		c, start, len);
 
    /* 
     * chain's lock not acquired since previously
     * linked pages which are forced eviction due
     * to memory constrains will require aquiring
     * the lock prior to modifying the linked pages.
    */
    error = do_mlink_pages(chain, start, len, 0);

    /* release chain collection's lock. */
    spin_unlock(&chain_collection->lock);
		     
    return error;
}

SYSCALL_DEFINE2(anchor, unsigned int, c, unsigned long, addr) {

    int error = -1; // Error code Invalid chain descriptor.
    unsigned long len = 0;

    struct memory_chain * chain = NULL;
    struct memory_chain_collection * chain_collection = current->mcc;

    if (chain_collection == NULL) {
		return -1;
    }
    
    /* acquire chain collection's lock. */
    spin_lock(&chain_collection->lock);

    /* check valid chain id provided. */
    if (c >= chain_collection->capacity ||
	(chain = chain_collection->chains[c]) == NULL) {
        spin_unlock(&chain_collection->lock);
        return error;
    }

    len = PAGE_ALIGN(PAGE_SIZE + (addr & ~PAGE_MASK));
    addr &= PAGE_MASK;

    /* 
     * chain's lock not acquired since previously
     * linked pages which are forced eviction due
     * to memory constrains will require aquiring
     * the lock prior to modifying the linked pages.
    */
    error = do_mlink_pages(chain, addr, len, addr);
   
    /* release chain collection's lock. */
    spin_unlock(&chain_collection->lock);

    return 0;
}

SYSCALL_DEFINE3(unlink_addr_rng, unsigned int, c, unsigned long, start, size_t, length) {
    return 0;
}

/**
 * @name __unlink_chain
 * @description unlinks all the pages contained within the speicified 
 * chain object.
 * @inparam chain chain object whose member pages are to be removed.
 */
static void __unlink_chain(memory_chain_t * chain) {

    int i = 0;
    struct page * page = chain->head;
    struct page * next = NULL;

    DEBUG_PRINT("unlinking chain: %d; nr_links=%lu", 
		chain->id, chain->nr_links);

    for (i = 0; page != NULL && i < chain->nr_links; page = next, i++) {

        spin_lock(&page->chain_lock);
        next = page->next;
	__reset_page(page, 0);
	spin_unlock(&page->chain_lock);
    }

    /* reset chain values */
    chain->head = NULL;
    chain->tail = NULL;
    chain->anchor = NULL;
    chain->delegate = NULL;
    chain->nr_links = 0;
    atomic_set(&chain->ref_counter, 0);
}

SYSCALL_DEFINE1(brk_mem_chain, unsigned int, c) {
    
    int error = -1;
    memory_chain_t * chain = NULL;
    struct memory_chain_collection * chain_collection = current->mcc;

    if (chain_collection == NULL) {
	return -1;
    }
    
    /* acquire chain collection's lock. */
    spin_lock(&chain_collection->lock);

    /* check valid chain id provided. */
    if (c >= chain_collection->capacity ||
	(chain = chain_collection->chains[c]) == NULL) {
        spin_unlock(&chain_collection->lock);
        return error;
    }

    spin_lock(&chain->lock);
    __unlink_chain(chain);
    spin_unlock(&chain->lock);

    /* release chain collection's lock */
    spin_unlock(&chain_collection->lock);

    return 0;
}

SYSCALL_DEFINE1(rls_mem_chain, unsigned int, c) {
    
    int error = -1;
    memory_chain_t * chain = NULL;
    struct memory_chain_collection * chain_collection = current->mcc; 

    if (chain_collection == NULL) {
	return -1;
    }
    
    /* acquire chain collection's lock. */
    spin_lock(&chain_collection->lock);

    /* check valid chain id provided. */
    if (c >= chain_collection->capacity ||
	(chain = chain_collection->chains[c]) == NULL) {
        spin_unlock(&chain_collection->lock);
        return error;
    }

    spin_lock(&chain->lock);

    __unlink_chain(chain);
    spin_unlock(&chain->lock);
 
    /* release attributes. */
    if (chain->attributes)
        kfree(chain->attributes);

    /* release chain object and free its slot. */
    kfree(chain);
    chain_collection->chains[c] = NULL;
    chain_collection->count--;

    /* release chain collection's lock */
    spin_unlock(&chain_collection->lock);

    DEBUG_PRINT("rls_mem_chain(): released memory chain. " \
		"#(chains)=%d", chain_collection->count);
    
    return 0; // Success
}

int __hate_vma_pages(struct vm_area_struct * vma, 
				    unsigned long start, unsigned long end){
	//Check if page is a part of a chain
	//if not, hate it
	return 0;
}

SYSCALL_DEFINE2(hate, unsigned long, start,	size_t, len) {
			
	len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
    start &= PAGE_MASK;
			
	int r = -1; // return code.
    unsigned long end = start + len;
    unsigned long s = start; // current vma's start address. 
    unsigned long e = end;   // current vma's end address.

    struct mm_struct * mm = current->mm;
    struct vm_area_struct * vma = find_vma(mm, s); // retrieve first vma. 

    /* check valid vma returned. */
    if (!vma || vma->vm_start >= end)
        return -1;

    DEBUG_PRINT("hate(): range[start=%lu, end=%lu]", start, end);

    for (s = start; s < end; s = e) {

        /* determine if current address lies outside range 
	   of current vma; if so move to next vma. */
		if (s >= vma->vm_end) 
		    vma = vma->vm_next;

		if (!vma || vma->vm_start >= end){
		    return -1;
		}

		/* determine current vma's end address. */
		e = end < vma->vm_end ? end : vma->vm_end;
      
		/* determine current vma's start address. */
        if (s < vma->vm_start)
	    	s = vma->vm_start;

		r = __hate_vma_pages(vma, s, e);
    }

    return r;
}
		

#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/mmzone.h>
#include <linux/mm_inline.h>

#include "internal.h"

/* comment out for production testing. */
#define VERIFY_FLG 1 
#undef VERIFY_FLG // Blocking for large page sizes. disabling.

#ifndef DEBUG
#define DEBUG 1
#endif 

/* comment/uncomment to enable/disable DEBUG_PRINT */
#ifdef DEBUG
#undef DEBUG
#endif

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

static inline void __init_mcc(struct task_struct * tsk) {

    struct memory_chain_collection * mcc =
      kmalloc(sizeof(struct memory_chain_collection),
	      GFP_KERNEL);
    
    mcc->count = 0;
    mcc->capacity = MAX_NUM_CHAINS;;
    mcc->chains = kmalloc(sizeof(memory_chain_t *) * 
			  MAX_NUM_CHAINS, GFP_KERNEL);
    /* zero out chain array */
    memset(mcc->chains, 0, sizeof(memory_chain_t *) * MAX_NUM_CHAINS); 
    spin_lock_init(&mcc->lock);
    
    tsk->mcc = mcc;
}

/**
 * @name __new_mem_chain
 * @description creates and initializes a new memory_chain object.
 * @inparam id chain's identifier.
 * @return initialized memory_chain object.
 */
static memory_chain_t * __new_mem_chain(unsigned int id) {

    memory_chain_t * chain = kmalloc(sizeof(memory_chain_t), GFP_KERNEL);

    chain->id = id;
    chain->attributes = NULL;

    INIT_LIST_HEAD(&chain->links);
    chain->anchor = NULL;
    chain->delegate = NULL;
    chain->nr_links = 0;
    atomic_set(&chain->ref_counter, 0);
    spin_lock_init(&chain->lock);  

    chain->evict_cnt = 0;

    return chain;
}

static void inline __reset_page(struct page * page,int clr_flgs) {
  
    struct memory_chain * chain = page->chain;

    page->chain = NULL;
    //INIT_LIST_HEAD(&page->link);
      
    if (clr_flgs && __PageReferenced(page)) {
        __ClearPageReferenced(page);
	atomic_dec(&chain->ref_counter);
    }

    /*
    if (PageLRU(page)) {
        enum lru_list lru = 0;
	enum lru_list base = 0;
        struct zone * zone = page_zone(page);
	
	spin_lock_irq(&zone->lru_lock);
	base = page_lru_base_type(page); // determine base lru FILE|ANON 
	lru = 
	spin_unlock_irq(&zone->lru_lock);
    }
    */
}

/**
 * @name __unlink_page
 * @description unlinks the specified page from containing chain.
 * @inparam pg page object to be unlinked.
 */
void __unlink_page(struct page * pg) {

    memory_chain_t * chain = NULL;
       
    if ((chain = pg->chain) != NULL) {
      
        list_del(&pg->link);

	if (chain->anchor == pg) {
	    munlock_vma_page(pg);
	    chain->anchor = NULL;
	}

	if (chain->delegate == pg) {
	    chain->delegate = NULL;
	}
	
	chain->nr_links--;
	if (__PageReferenced(pg)) {
	    atomic_dec(&chain->ref_counter);
	}

	__reset_page(pg, 0);

	DEBUG_PRINT("unlinked %p from chain %u, #(links)=%lu, ref_counter=%d",
		    pg, chain->id, chain->nr_links, 
		    atomic_read(&chain->ref_counter));
    }
}

static struct page * __evict_page(memory_chain_t * chain) {
    return NULL;
}

static struct page * __link_page(memory_chain_t * chain, struct page * pg) {

    struct page * victim = NULL;
    
    /* determine if pages is linked to chain previously. */
    if (chain == pg->chain) {
        return NULL;
    }
    
    /* check if the page is in another chain, if so, unlink page. */
    if (unlikely(pg->chain != NULL)) {
        spin_lock(&pg->chain->lock);
	__unlink_page(pg);
	spin_unlock(&pg->chain->lock);
    }
 
    /* if the chain is full, choose a victim for evicition. */
    if (unlikely(MEMORY_CHAIN_IS_FULL(chain))) {
        victim = __evict_page(chain);

	spin_lock(&victim->link_lock);
	__unlink_page(victim);
	spin_unlock(&victim->link_lock);
    }
    
    //INIT_LIST_HEAD(&pg->link);
    pg->chain = chain;
    list_add(&pg->link, &chain->links);
    
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

struct page * __get_user_page(struct vm_area_struct * vma,
					    unsigned long addr) {
  
    int counter = 0;
    int foll_flags = FOLL_TOUCH; // | FOLL_FORCE;
    struct page * page = NULL;

    while (!(page = follow_page(vma, addr, foll_flags))) {
        unsigned int fault_flags = 0; //FAULT_FLAG_ALLOW_RETRY;
	int ret = handle_mm_fault(vma->vm_mm, vma, addr, fault_flags);

	if (ret & VM_FAULT_ERROR || ret & VM_FAULT_RETRY) {
	    DEBUG_PRINT("Failed to get page at address: %lu", addr);
	    return NULL;
	}

	if (++counter >= MAX_PAGE_RETRIEVAL_ATTEMPTS) {
	    DEBUG_PRINT("Too many attempts to retrieve page at address: %lu",
			addr);
	    return NULL;
	}	  
    }

    if (page && !IS_ERR(page))
        return page;
    
    return NULL;
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
    enum lru_list lru = 0; /* current lru */
    int is_active = 0;
    int nr_linked = 0;

    DEBUG_PRINT("mlink_vma_pages_range(): vma-range[s=%lu, e=%lu]; " \
		"range[start=%lu, end=%lu]", vma->vm_start, vma->vm_end, 
		start, end);

    /* validate page ranges within vma. */
    BUG_ON(start < vma->vm_start || end > vma->vm_end);

    /* do not try to access the guard page of a stack vma */
    if (is_stack_guard_page(vma, start)) 
        start += PAGE_SIZE;

    down_read(&vma->vm_mm->mmap_sem);
    for (addr = start; addr < end; addr += PAGE_SIZE) {

        page = __get_user_page(vma, addr);
	
	if (page != NULL) {
	  
	    struct zone * z = page_zone(page);
	  
	    spin_lock(&page->link_lock);
	    spin_lock_irq(&z->lru_lock);
	    
	    /* determine current lru list. */
	    lru = page_lru_base_type(page); 
	    is_active = PageActive(page);
	    
	    /*
	    if (PageLRU(page)) 
	        / * del from current lru list * /
	        del_page_from_lru_list(z, page, lru + 
				       (is_active ? LRU_ACTIVE : LRU_BASE)); 
	    else 
	        INIT_LIST_HEAD(&page->lru);
	    */
	  	  
	    /* add page to chains link */
	    __link_page(chain, page);
	    
	    /* simulate anchoring page. */
	    if (unlikely(addr == anchor)) {
	        mlock_vma_page(page);
		chain->anchor = page;
	    }
	  
	    /* move page from linked lru list /
	    SetPageLRU(page);
	    add_page_to_lru_list(z, page, lru + LRU_LINKED +
				 (is_active ? LRU_ACTIVE : LRU_BASE)); 
	    */
	    
	    spin_unlock_irq(&z->lru_lock);
	    spin_unlock(&page->link_lock);
	    
	    mark_page_accessed(page);
	    
	    nr_linked++;
	    DEBUG_PRINT("mlink_vma_pages_range(): linked(address): %lu " \
			"in vma: %p", addr, vma);
	} else {
	    DEBUG_PRINT("mlink_vma_pages_range(): no page found at %lu\n", 
			addr);
	}
    }
    up_read(&vma->vm_mm->mmap_sem);
    
    printk(KERN_EMERG "linked %d of %d pages", nr_linked, 
	   ((end - start) / PAGE_SIZE));
		
    return nr_linked;
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
	
	if (!vma || vma->vm_start >= end) {
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
  
    struct page * page = NULL;
    struct page * next = NULL;

    DEBUG_PRINT("unlinking chain: %d; nr_links=%lu", 
		chain->id, chain->nr_links);

    list_for_each_entry_safe(page, next, &chain->links, link) {
        spin_lock(&page->link_lock);
	list_del(&page->link);
	__reset_page(page, 1);
	spin_unlock(&page->link_lock);
    }

    if (chain->anchor != NULL) {
        munlock_vma_page(chain->anchor);
    }

    /* reset chain values */
    INIT_LIST_HEAD(&chain->links);

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
    DEBUG_PRINT("chain stats: eviction-count=%lu, nr-links=%lu, "	\
		"ref_counter=%d", chain->evict_cnt,
		chain->nr_links, atomic_read(&chain->ref_counter));
	  
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

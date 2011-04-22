#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#define MAX_NUM_CHAINS 5

#define MEMORY_CHAIN_IS_FULL(chain) 0 
  /*  chain->attributes != NULL && chain->attributes->is_bounded ?	\
      atomic_read(&chain->nr_links) == chain->attributes->max_links : 0; */

/**
 * @name __new_mem_chain
 * @description creates and initializes a new memory_chain object.
 * @inparam id chain's identifier.
 * @return initialized memory_chain object.
 */
static memory_chain_t * __new_mem_chain(unsigned int id) {

    memory_chain_t * chain = kmalloc(sizeof(memory_chain_t), GFP_KERNEL); //And maybe GPR_ATOMIC?

    chain->id = id;
    chain->attributes = NULL;
    chain->head = NULL;
    chain->tail = NULL;
    chain->anchor = NULL;
    chain->delegate = NULL;
    atomic_set(&chain->nr_links, 0);
    
    atomic_set(&chain->ref_counter, 0);
    spin_lock_init(&chain->lock);  

    return chain;
}

static void __unlink_page(struct page * pg) {
    memory_chain_t * chain = pg->chain;

    if (chain != NULL) {
        if (pg->prev == NULL) { // head of chain
	    chain->head = pg->next;
	    if (chain->head != NULL) {
	        chain->head->prev = pg->prev;
	    }
	} else {
	    struct page * previous = pg->prev;
	  
	    previous->next = pg->next;
	    if (pg->next != NULL) // not tail of chain
	        pg->next->prev = previous;
	    else 
	        chain->tail = pg->prev;
	}

	if (chain->anchor == pg) {
	    chain->anchor = NULL;
	}

	if (chain->delegate == pg) {
	    chain->delegate = NULL;
	}
	
	atomic_dec(&chain->nr_links);
	if (PageReferenced(pg)) {
	    atomic_dec(&chain->ref_counter);
	}

	pg->chain = NULL;
	pg->next = NULL;
	pg->prev = NULL;
    }
}

static struct page * __evict_page(memory_chain_t * chain) {
    return NULL;
}

static struct page * __link_page(memory_chain_t * chain, struct page * pg) {

    struct page * victim = NULL;
    
    if (unlikely(chain == NULL)) {
        return NULL;
    }
    
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
	__unlink_page(victim);
    }

    //If this is the first page
    if (unlikely(chain->head == NULL)) {
        chain->head = pg;
	chain->tail = pg;
    } else {
        chain->head->prev = pg;
	pg->next = chain->head;
	chain->head = pg;
    }
    
    atomic_inc(&chain->nr_links);
    if (PageReferenced(pg)) {
        atomic_inc(&chain->ref_counter);
    }

    printk(KERN_EMERG "Added page %lu to chain %d, new size of chain is %d\n",
	   (unsigned long)pg, chain->id, atomic_read(&chain->nr_links));
    
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
    struct task_struct * tsk = current;

    printk(KERN_EMERG "new_mem_chain(): tsk[chains=%p, nr=%d, max=%d]",
	   tsk->chains, tsk->nr_chains, tsk->max_num_chains);

    if (tsk->chains != NULL) {
        /* determine # of open chains */
        if (tsk->nr_chains < MAX_NUM_CHAINS) {
	    int slot = __find_free_slot(tsk->chains, MAX_NUM_CHAINS);
	    tsk->chains[slot] = __new_mem_chain(slot);
	    tsk->nr_chains++;
	    return slot;
	} 
    } else {
      tsk->chains = kmalloc(sizeof(memory_chain_t *) * MAX_NUM_CHAINS,
			    GFP_KERNEL);
	/* zero out chain array */
	memset(tsk->chains, 0, sizeof(memory_chain_t *) * MAX_NUM_CHAINS); 

	/* assign chain to first slot */
	tsk->chains[tsk->nr_chains++] = __new_mem_chain(0);
	tsk->max_num_chains = MAX_NUM_CHAINS;
	return 0;
    }

    printk(KERN_EMERG "new_mem_chain(): Too many open memory chains");
    return -1;
}

asmlinkage long sys_set_mem_chain_attr(unsigned int c,
				       const memory_chain_attr_t * attr) {
    return 0;
}

/**
 * @name _mlink_vma_pages_range
 * @description links pages found in the provided vma to the specified memory
 * chain object. If an anchored address is provided, the page at the 
 * specified address is anchored in the chain. 
 * @inparam chain chain object pages in provided range should be linked to.
 * @inparam vma vma object containing specified address range. 
 * @inparam start start address within in the vam of the addres range. 
 * @inparam len length of address range within the vma to be linked.
 * @inparam anchor address of anchor page.
 * @return 0 on success; -1 otherwise. 
 */
static long __mlink_vma_pages_range(memory_chain_t * chain,
				    struct vm_area_struct * vma, 
				    unsigned long start, size_t len,
				    unsigned long anchor)
{

    int foll_flags = FOLL_GET | FOLL_TOUCH;
    struct page * page = NULL;
  
    for (; start < len; start += PAGE_SIZE) {

        page = follow_page(vma, start, foll_flags);
		
	if (page != NULL) {

	    __link_page(chain, page);

	    if (unlikely(start == anchor)) 
	        chain->anchor = page;
	    
	    printk(KERN_EMERG "Page: %lu VMA: %lu\n", (unsigned long)page, 
		   (unsigned long)vma);
	} else {
	    printk(KERN_EMERG "page is null\n");
	    break;
	}
    }

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
        return 0;

    for (s = start; s < end; s = e) {

        /* determine if current address lies outside range 
	   of current vma; if so move to next vma. */
	if (s >= vma->vm_end) 
	    vma = vma->vm_next;

	if (!vma || vma->vm_start >= end) 
	    break;

	/* determine current vma's end address. */
	e = min(end, vma->vm_end);
      
	/* determine current vma's start address. */
        if (s < vma->vm_start)
	    s = vma->vm_start;

	r = __mlink_vma_pages_range(chain, vma, s, e, anchor);
    }

    return r;

}

SYSCALL_DEFINE3(link_addr_rng, unsigned int, c, unsigned long, start,
		size_t, len) {

    int error = -1;
    struct memory_chain * chain = NULL;

    /* check valid chain id provided. */
    if (c >= current->max_num_chains ||
	(chain = current->chains[c]) == NULL) 
        return error;

    start &= PAGE_MASK;;
    len = PAGE_ALIGN(len + (start & ~PAGE_MASK));
 
    /* acquire chain's lock. */
    spin_lock(&chain->lock);
    error = do_mlink_pages(chain, start, len, 0);
    /* release chain's lock */
    spin_unlock(&chain->lock);
		     
    return error;
}

SYSCALL_DEFINE2(anchor, unsigned int, c, unsigned long, addr) {

    int error = -1;
    unsigned long len = 0;
    struct memory_chain * chain = NULL;

    /* check valid chain id provided. */
    if (c >= current->max_num_chains ||
	(chain = current->chains[c]) == NULL) 
        return error;

    addr &= PAGE_MASK;
    len = PAGE_ALIGN(PAGE_SIZE + (addr & ~PAGE_MASK));

    /* acquire chain's lock. */
    spin_lock(&chain->lock);
    error = do_mlink_pages(chain, addr, len, addr);
    /* release chain's lock */
    spin_unlock(&chain->lock);
    return 0;
}

SYSCALL_DEFINE2(unlink_addr_rng, unsigned long, start, size_t, length) {
    return 0;
}

SYSCALL_DEFINE1(brk_mem_chain, unsigned int, c) {
    return 0;
}

SYSCALL_DEFINE1(rls_mem_chain, unsigned int, c) {
    return 0;
}

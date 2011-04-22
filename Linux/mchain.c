#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#define MAX_NUM_CHAINS 5

#define MEMORY_CHAIN_IS_FULL(chain) 0 /* chain->attributes != NULL && chain->attributes->is_bounded ? \
					 chain->nr_links == chain->attributes->max_links ? 0; */

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

    struct page * evicted = NULL;
    
    if (unlikely(chain == NULL)) {
        return NULL;
    }
    
    //We don't want to insert a page into a chain its already in
    if (chain == pg->chain) {
        return NULL;
    }
    
    //Check if the page is in another chain, if so, we unlink it
    if (pg->chain != NULL) {
        spin_lock(&pg->chain->lock);
	__unlink_page(pg);
	spin_unlock(&pg->chain->lock);
    }
 
    //If the chain is full, we throw out a page to make room
    if (MEMORY_CHAIN_IS_FULL(chain)) {
        evicted = __evict_page(chain);
	__unlink_page(evicted);
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
    
    return evicted;
}

/**
 * constrains: len % 5 == 0 for loop unrolling. 
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

SYSCALL_DEFINE3(link_addr_rng, unsigned int, c, unsigned long, start,
		size_t, length) {
    struct page * page;
    struct vm_area_struct * vma;
    unsigned long counter = 0;
    
    
    printk(KERN_EMERG "page-size=%lu\n", PAGE_SIZE);
    
    for (counter = start; counter < start + length; counter += PAGE_SIZE) {
        //If the range is big, it might span several vma's
        //It might be possible to do this more efficiently
      
        vma = find_vma(current->mm, counter);
	page = follow_page(vma, counter, FOLL_GET | FOLL_TOUCH);
	
	
	if (page != NULL) {
	  printk(KERN_EMERG "Page: %lu VMA: %lu\n", (unsigned long)page, 
		 (unsigned long)vma);
	  __link_page(current->chains[c], page);
	}
	else{
	    printk(KERN_EMERG "page is null");
	    break;
	}
    }
    return 0;
}

SYSCALL_DEFINE2(anchor, unsigned int, c, unsigned long, addr) {
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

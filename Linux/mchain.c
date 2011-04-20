#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

#define MAX_NUM_CHAINS 2

struct memory_chain * memory_chains[MAX_NUM_CHAINS];
static int nextPos = 0;

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
    if (chain == pg->chain) {
        return NULL;
    }
    
    /* determine if page is migrating across chains. */
    if (pg->chain != NULL) {
        spin_lock(&pg->chain->lock);
	__unlink_page(pg);
	spin_unlock(&pg->chain->lock);
    }

 
    if (MEMORY_CHAIN_IS_FULL(chain)) {
        evicted = __evict_page(chain);
	__unlink_page(evicted);
    }
      
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

    return evicted;

}

SYSCALL_DEFINE0(new_mem_chain) {
    struct memory_chain * mchain = NULL;

    if (nextPos < MAX_NUM_CHAINS){
        printk(KERN_EMERG "Inserting new chain at %d\n", nextPos);
	mchain = __new_mem_chain(nextPos++);
	memory_chains[nextPos] = mchain;
	return nextPos -1;
    }
    else{
        printk(KERN_EMERG "Too many open memory chains");
	return -1;
    }

    return 0;
}

asmlinkage long sys_set_mem_chain_attr(unsigned int c,
				       const memory_chain_attr_t * attr) {
    return 0;
}

SYSCALL_DEFINE3(link_addr_rng, unsigned int, c, unsigned long, start,
		size_t, length) {
	struct page * page;
	struct vm_area_struct * vma;

	printk(KERN_EMERG "start: %lu\n", start);
	
	vma = find_vma(current->mm, start);

	printk(KERN_EMERG "vma.start: %lu\n", vma->vm_start);

	page = follow_page(vma, start, FOLL_GET | FOLL_TOUCH);

	printk(KERN_EMERG "page: %lu\n", (unsigned long)page);

	if(PageActive(page)){
		printk(KERN_EMERG "active\n");
	}
	if(PageReferenced(page)){
		printk(KERN_EMERG "referenced\n");
	}

	if (page != NULL)
	  __link_page(page);
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

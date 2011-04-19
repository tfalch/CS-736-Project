#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>
#include <linux/slab.h>

struct memory_chain ** memory_chains;
int nextPos = -1;

SYSCALL_DEFINE0(new_mem_chain) {
	struct memory_chain * mchain;

	if(nextPos > 0 && nextPos < 5){
		printk(KERN_EMERG "Inserting new chain at %d\n", nextPos);
		mchain = kmalloc(sizeof(mchain), GFP_KERNEL); //And maybe GPR_ATOMIC?
		memory_chains[nextPos] = mchain;
		nextPos++;
		return nextPos -1;
	}
	else if(nextPos == -1){
		printk(KERN_EMERG "Creating new chain");
		mchain = kmalloc(sizeof(mchain), GFP_KERNEL); //And maybe GPR_ATOMIC?
		memory_chains = kmalloc(sizeof(*mchain)*5, GFP_KERNEL);
		memory_chains[0] = mchain;
		nextPos = 1;
		return 0;
	}
	else{
		printk(KERN_EMERG "Success");
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

#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/mm_types.h>

SYSCALL_DEFINE0(new_mem_chain) {
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

	printk(KERN_EMERG "page: %ln\n", &page);

	if(PageActive(page)){
		printk(KERN_EMERG "active\n");
	}
	if(PageReferenced(page)){
		printk(KERN_EMERG "referenced\n");
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

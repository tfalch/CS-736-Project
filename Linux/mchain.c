#include <linux/mm.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/linkage.h>

SYSCALL_DEFINE0(new_mem_chain) {
    return 0;
}

asmlinkage long sys_set_mem_chain_attr(unsigned int c,
				       const memory_chain_attr_t * attr) {
    return 0;
}

SYSCALL_DEFINE3(link_addr_rng, unsigned int, c, unsigned long, start,
		size_t, length) {
	printk(KERN_EMERG "start: %d\n", start);
	
	vm_area_struct * vma;
	vma = find_vma(current->mm, start);

	printk(KERN_EMERG "vma.start: %d\n", vma->vm_start);

	struct page * page;
	page = follow_page(vma, start);

	if(PageActive(page)){
		printk(KERN_EMERG "active\n");
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

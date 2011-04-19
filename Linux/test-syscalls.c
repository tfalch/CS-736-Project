#include <stdio.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define NR_sys_new_mem_chain      341
#define NR_sys_set_mem_chain_attr 342
#define NR_sys_link_addr_rng      343
#define NR_sys_anchor             344
#define NR_sys_unlink_addr_rng    345
#define NR_sys_brk_mem_chain      346
#define NR_sys_rls_mem_chain      347

int main() {
/*
    int i = 0;

    int passed = 0;
    int nr_passed = 0;

    
    const char * sys_calls[] = {
        "new_mem_chain(void)",
	"set_mem_chain_attr(unsigned int, const memory_chain_attr *)",
	"link_addr_rng(unsigned int, void *, size_t)",
	"anchor(unsigned int, void *)",
	"unlink_addr_rng(unsigned int, void *, size_t)",
	"brk_mem_chain(unsigned int)",
	"rls_mem_chain(unsigned int)"
    };

    int n = sizeof(sys_calls) / sizeof(sys_calls[0]);
    int total = n;

    fprintf(stdout, "testing system calls...\n");
    for (i = 0; i < n; ) {

        /* syscall does not perform any parameter checking/validation. 
        passed = syscall(341+i) == 0;
	 
        fprintf(stdout, "test %d: %s...", ++i, sys_calls[i]);  
	fprintf(stdout, "%s\n", strerror(errno));
	nr_passed += passed;
    }      
    fprintf(stdout, "test complete. passed %d of %d\n",
	    nr_passed, total);

*/
	syscall(341);
	syscall(341);
	syscall(341);
	syscall(341);
	syscall(341);
	syscall(341);
	syscall(341);

    int* lol = malloc(sizeof(int)*2000);
    int* lol2 = malloc(sizeof(int)*2000);

    syscall(343, 0, lol, 4);
    syscall(343, 0, lol2, 4);
    return 0;
}

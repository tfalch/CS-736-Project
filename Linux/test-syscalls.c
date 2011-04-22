#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "mchain.h"

int main() {

    size_t len = sizeof(int) * 12000;
    int * array = malloc(len);
    
    if (array != NULL) {
      
        int chain_id = mchain();
	set_mchain_attr(0, 0);  
	
	if (chain_id >= 0) {
	    mlink(chain_id, array, len);
	    munlink(array, len);
	    rls_mchain(chain_id);
	} else {
	  fprintf(stdout, "chain allocation failed: (%d)", chain_id);
	}
    } else {
        fprintf(stdout, "unable to allocate array of size %d\n", len);
    }
    
    return 0;
}


int test_sys_calls() {

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

        /* syscall does not perform any parameter checking/validation. */
        passed = syscall(341+i) == 0;
	 
        fprintf(stdout, "test %d: %s...", ++i, sys_calls[i]);  
	fprintf(stdout, "%s\n", strerror(errno));
	nr_passed += passed;
    }      
    fprintf(stdout, "test complete. passed %d of %d\n",
	    nr_passed, total);


}

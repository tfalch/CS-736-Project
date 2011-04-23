#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "mchain.h"

#define TEST_RESULT(r, e) r == e ? "passed" : mchain_strerror(r)

#define NPAGES 2
#define LENGTH sizeof(int) * 1024 * NPAGES

void exec_test_suite();
void test_create_rls_mchain();
void test_link_dynamic();
void test_link_static();

int main() {
  exec_test_suite();
  return 0;
}

void exec_test_suite() {

  void (*test_suite[])(void) = {
    &test_create_rls_mchain,
    &test_link_dynamic,
    &test_link_static,
    NULL
  };
  
  int f = 0;
  for ( ; test_suite[f]; f++) {
    (*test_suite[f])();
    fprintf(stdout, "\n");
  }
}

void test_link_static() {

  int test_nr = 1;
  int chain = mchain();
  int array[1024];
  int r = 0;

  fprintf(stdout, 
	  "Test: Link Statically Allocated Memory\n"	\
	  "======================================\n");

  r = mlink(chain, array, sizeof(array));
  fprintf(stdout, "Test %d: Link Pages On Stack. Result=%s\n", test_nr++,
	  TEST_RESULT(r, 0));

  r = rls_mchain(chain);
  fprintf(stdout, "Test %d: Release Memory Chain. Result=%s\n", test_nr++,
	  TEST_RESULT(r, 0));

  fprintf(stdout, "Test Complete\n");
}

void test_link_dynamic() {

  int test_nr = 1;
  int chain = mchain();
  int len = sizeof(int) * 4000;
  int * array = malloc(len);
  int r = 0;

  fprintf(stdout, 
	  "Test: Link Dynamically Allocated Memory\n"	\
	  "=======================================\n");

  r = mlink(chain, array, len);
  fprintf(stdout, "Test %d: Link Pages On Heap. Result=%s\n", test_nr++,
	  TEST_RESULT(r, 0));      

  r = rls_mchain(chain);
  fprintf(stdout, "Test %d: Release Memory Chain. Result=%s\n", test_nr++,
	  TEST_RESULT(r, 0));

  fprintf(stdout, "Test Complete\n");

}

void test_create_rls_mchain() {
  
  int i = 0;
  int chains[5];
  int max_chains = 5;
  int test_nr = 1;

  fprintf(stdout, 
	  "Test: Create & Release Multiple Memory Chains\n" \
	  "=============================================\n");

  for (i = 0; i < max_chains; i++) {  
    chains[i] = mchain();
    fprintf(stdout, "Test %d: Create Memory Chain. Result=%s\n", test_nr++,
	    TEST_RESULT(chains[i] >= 0, 1));      
  }

  i = mchain();
  fprintf(stdout, "Test %d: Create Memory Chain. Result=%s\n", test_nr++,
	  TEST_RESULT(i, -1));      

  for (i = 0; i < max_chains; i++) {
    int r = rls_mchain(chains[i]);
    fprintf(stdout, "Test %d: Release Memory Chain. Result=%s\n", test_nr++,
	    TEST_RESULT(r, 0));      
  }

  i = mchain();
  fprintf(stdout, "Test %d: Create Memory Chain. Result=%s\n", test_nr++,
	  TEST_RESULT(i, 0));

  rls_mchain(i);
	    
  fprintf(stdout, "Test Complete\n");
}

/*
main()
 
  int i = 0;
  int * array = malloc(LENGTH);

  if (array != NULL) {
    
    int chains[] = { mchain(), mchain() };
    
    set_mchain_attr(0, 0);  
    
    if (chains[0] >= 0) {
      mlink(chains[0], array, LENGTH);
      //mlink(chains[0], array, LENGTH);

      if (chains[1] >= 0) {
	mlink(chains[1], array, LENGTH / 2);
	mlink(chains[0], array, LENGTH / 2);
	rls_mchain(chains[1]);
      }
      rls_mchain(chains[0]);
  
    } else {
      fprintf(stdout, "chain allocation failed");
    }
  }else {
    fprintf(stdout, "unable to allocate array of size %d\n", LENGTH);
  }
 
  for (i = 0; i < 5; i++) {
    int id = mchain();
    if (id < 0)
      fprintf(stderr, "%d: %s\n", id, mchain_strerror(id));
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

        /* syscall does not perform any parameter checking/validation. * /
        passed = syscall(341+i) == 0;
	 
        fprintf(stdout, "test %d: %s...", ++i, sys_calls[i]);  
	fprintf(stdout, "%s\n", strerror(errno));
	nr_passed += passed;
    }      
    fprintf(stdout, "test complete. passed %d of %d\n",
	    nr_passed, total);


}
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "mchain.h"

#define TEST_RESULT(r, e) r == e ? "passed" : mchain_strerror(errno)

#define NPAGES 5
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
  int array[LENGTH];
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
  int chain = mchain();;
  int * array = malloc(LENGTH);
  int r = 0;

  fprintf(stdout, 
	  "Test: Link Dynamically Allocated Memory\n"	\
	  "=======================================\n");

  r = mlink(chain, array, LENGTH);
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

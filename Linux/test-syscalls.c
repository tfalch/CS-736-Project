#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "mchain.h"

#define TEST_RESULT(r, e) r == e ? "passed" : mchain_strerror(errno)

#define INTS_PER_PAGE 1024
#define NPAGES 50000
#define LENGTH sizeof(int) * INTS_PER_PAGE * NPAGES

void exec_test_suite();

void test_create_rls_mchain();
void test_link_dynamic();
void test_link_static();

double simple_speed_test(int flag, int count);

int main(int argc, char ** argv) {

  int c = argc > 1 ? atoi(argv[1]) : 100000;
  int l = argc > 2 ? atoi(argv[2]) : 1;
  int n = argc > 3 ? atoi(argv[3]) : 10;
  int i = 0;
  int total = 0;

  for (i = 0; i < n; i++) {
    total += simple_speed_test(l, c);
  }

  total /= n;
  fprintf(stdout, "test=%d, # runs=%d, avg-time=%ds %dus\n",
	  l, n, (total / 1000000), (total % 1000000));

  //exec_test_suite();

  return 0;
}

void exec_test_suite() {

  void (*test_suite[])(void) = {
    &test_create_rls_mchain,
    &test_link_dynamic,
    &test_link_static,
    NULL
  };
  fprintf(stderr, "running");
  int f = 0;
  for ( ; test_suite[f]; f++) {
    (*test_suite[f])();
    fprintf(stdout, "\n");
  }
}

void test_link_static() {

  int test_nr = 1;
  int chain = mchain();
  int array[5000];
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

  int length = LENGTH;;
  int test_nr = 1;
  int chain = mchain();;
  int * array = malloc(length);

  fprintf(stdout, 
	  "Test: Link Dynamically Allocated Memory\n"	\
	  "=======================================\n");

  if (array) {
    int r = 0;
    int i = 0;
    int n = length / sizeof(int);
    int counts[100];
    int unique = sizeof(counts) / sizeof(counts[0]);

    memset(counts, 0, sizeof(counts));

    r = mlink(chain, array, length);
    fprintf(stdout, "Test %d: Link Pages On Heap. Result=%s\n", test_nr++,
	    TEST_RESULT(r, 0));

    /* initialize array. */
    for (i = 0; i < n; i++) {
      array[i] = 0;
    }

    /* set array to random value. */
    for (i = 0; i < n; i++) {
      array[i] = abs(rand()) % unique;
      assert(array[i] >= 0 && array[i] < unique);
    }
    
    /* count each unique value. */
    for (i = 0; i < n; i++) {
        counts[array[i]]++;
    }

    r = rls_mchain(chain);
    fprintf(stdout, "Test %d: Release Memory Chain. Result=%s\n", test_nr++,
	    TEST_RESULT(r, 0));
    
    free(array);
  }
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

#define USEC_PER_SEC 1000000

int time_difference(struct timeval * result, 
		    struct timeval * x, struct timeval * y) {

  int sec = y->tv_sec;
  int usec = y->tv_usec;

  if (x->tv_usec < usec) {
    int nsec = (usec - x->tv_usec) / USEC_PER_SEC + 1;
    usec -= USEC_PER_SEC * nsec;
    sec += nsec;
  }

  if (x->tv_usec - usec > USEC_PER_SEC) {
    int nsec = (x->tv_usec - usec) / USEC_PER_SEC;
    usec += USEC_PER_SEC * nsec;
    sec -= nsec;
  }

  result->tv_sec = x->tv_sec - sec;
  result->tv_usec = x->tv_usec - usec;

  return 0;
}

void compare(int * result, int * R, int size_r, int * S, int size_s) {

  int i = 0;
  for (i = 0; i < size_r; i++) {
    int j = 0;
    for (j = 0; j < size_s; j++) {
      if (R[i] == S[j]) {
	result[R[i]]++;
      }
    }
  }
}

double simple_speed_test(int link, int c) {

  int length = sizeof(int) * INTS_PER_PAGE * c;
  int chain = mchain();;

  int * R = malloc(length);
  int * S[] = { 
    malloc(length),
    malloc(length),
    malloc(length),
    malloc(length),
    malloc(length),
    NULL
  };

  fprintf(stdout, 
	  "Simple Speed Test\n"	\
	  "==================\n");

  if (R) {
    int r = 0;
    int i = 0;
    int n = length / sizeof(int);
    int counts[5000];
    int unique = sizeof(counts) / sizeof(counts[0]);

    struct timeval start;
    struct timeval stop;
    struct timeval difference;

    memset(counts, 0, sizeof(counts));

    /* set array to random value. */
    for (i = 0; i < n; i++) {
      R[i] = abs(rand()) % unique;
    }

    for (i = 0; S[i]; i++) {
      int j = 0;
      for (j = 0; j < n; j++) {
	S[i][j] = 0; //abs(rand()) % unique;
      }
    }

    fprintf(stdout, "Test Started..."); fflush(stdout);
    gettimeofday(&start, NULL);
    
    if (link) {
      r = mlink(chain, R, length);
    }
    
    /* count # times each value appears in all arrays. */
    for (i = 0; S[i]; i++) {
      int j = 0;
      for (j = 0; j < c; j++) {
	int k = 0;
	for (k = 0; k < c; k++) {
	  compare(counts, R + (k * INTS_PER_PAGE), INTS_PER_PAGE,
		  S[i] + (j * INTS_PER_PAGE), INTS_PER_PAGE);
	}
      }
    }

    if (link)
      r = rls_mchain(chain);

    gettimeofday(&stop, NULL);
    
    time_difference(&difference, &stop, &start);
    fprintf(stdout, "Test Complete\n");
    fprintf(stdout, 
	    "Started At %s" \
	    "Ended At: %s" \
	    "Elapsed: %ds %dus\n",
	    "", "", difference.tv_sec, difference.tv_usec);
	    
    free(R);
    for (i = 0; S[i]; i++)
      free(S[i]);

    return difference.tv_sec * USEC_PER_SEC + difference.tv_usec;
  } else {
    fprintf(stderr, "************ARRAY TOO LARGE*****************\n");
  }

  return 0;
}

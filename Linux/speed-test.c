#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "mchain.h"

#define TEST_RESULT(r, e) r == e ? "passed" : mchain_strerror(errno)

#define INTS_PER_PAGE 1024
#define NPAGES 50000
#define LENGTH sizeof(int) * INTS_PER_PAGE * NPAGES

void exec_test_suite();

int simple_speed_test(int, int);
int looping_speed_test(int, int);

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
  fprintf(stderr, "test=%d, # runs=%d, avg-time=%ds %dus\n",
	  l, n, (total / 1000000), (total % 1000000));

  fprintf(stderr, "press enter to exit...");
  //exec_test_suite();
  //  getchar();

  return 0;
}

void exec_test_suite() {

  /*
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
  */
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

int simple_speed_test(int link, int nr_pages) {

  int c = 0;
  int i = 0;
  int length = sizeof(int) * INTS_PER_PAGE * nr_pages;
  int n = length / sizeof(int);
  int * S[] = {malloc(length), malloc(length), malloc(length), 
	       malloc(length), malloc(length), malloc(length), 
	       NULL};
  int chain = mchain();
  int * R = malloc(length);
  int * totals = malloc(length);  

  fprintf(stderr, "initializing S arrays...");
  for (i = 0; S[i]; i++) {
    int j = 0;
    fprintf(stderr, "S[%d]...", i);
    for (j = 0; j < n; j++) {
      S[i][j] = abs(rand()) % 10;
    }
  }
  fprintf(stderr, "done\n");

  fprintf(stderr, "initializing total array...");
  memset(totals, 0, length);
  fprintf(stderr, "done\n");

  fprintf(stderr, "initializing R array...");
  for (i = 0; i < n; i++) {
    R[i] = abs(rand()) % 10;
  }
  fprintf(stderr, "done\n");


  if (link) {
    fprintf(stderr, "linking R array...");
    mlink(chain, R, length);
    fprintf(stderr, "done\n");
  }

  fprintf(stderr, "carrying out computations...");
  for (i = 0; S[i]; i++) {
    int j = 0;
    fprintf(stderr, "S[%d]...", i);
    for (j = 0; j < n; j++) {
      totals[j] = R[j] * S[i][j];
    }
    fprintf(stderr, "done...");
  }
  fprintf(stderr, "done\n");

  if (link) {
    fprintf(stderr, "releasing chain...");
    rls_mchain(chain);
    fprintf(stderr, "...done\n");
  }

  for (i = 0; S[i]; i++) 
    free(S[i]);

  free(R);
}
				  

  /*

  for (i = 0; array[i]; i++) {
    int j = 0;
    /* initialize array * /
    for (j = 0; j < n; j++)
      array[i][j] = 0;
    mlink(chain, array[i], length);
    fprintf(stderr, "linked addr-rng[%p,%p] to chain %d\n", 
	    array[i], array[i] + length, chain);
  }


  for (i = 0; array[i]; i++) {
    //free(array[i++]);
    if (!array[i])
      break;
  }
* /
  rls_mchain(chain);

  /*
  //

  for (i = 0; array[i]; i++) {
    free(array[i]);
  }
  */
    

  /*
  int * array = malloc(length);
  
  if (array) {
    int i = 0;
    int chain = -1;
    int n = length / sizeof(int);
    int counts[5000];
    int unique = sizeof(counts) / sizeof(counts[0]);

    /* initialize array * /
    for (i = 0; i < n; i++)
      array[i] = 0;
    
    if (link) {
      fprintf(stderr, "linking....");
      chain = mchain(); 
      mlink(chain, array, length);

      /*
      for (i = 0; i < nr_pages; i++) {
	array[i * INTS_PER_PAGE] = 0;
	mlink(chain, array + i * INTS_PER_PAGE, 1024);
      }
      * /
      fprintf(stderr, "done\n");
    }

    
    fprintf(stderr, "assigning rand values....");
    for (i = 0; i < n; i++) {
      //      array[i] = abs(rand()) % unique;
    }
    fprintf(stderr, "done\n");

    fprintf(stderr, "gathering statistics....");
    for (i = 0; i < n; i++)  {
      //      counts[array[i]]++;
    }
    fprintf(stderr, "done\n");

    fprintf(stderr, "releasing linked pages....");
    if (link) {
      //      rls_mchain(chain);
    }
    fprintf(stderr, "done\n");
      
    free(array);
  }
  fprintf(stderr, "test complete\n");
}
  */

int looping_speed_test(int link, int c) {

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

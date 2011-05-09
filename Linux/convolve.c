#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "mchain.h"

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

double getRandomNumber(){
	return rand();
}

int getNoOfColsForMB(int mb){
	return ((double)mb)*1e6/((double)sizeof(double));
}

int main(int argc, char** argv){
	int noOfRows = 5;
	int noOfCols = 1000;
	int love = 0;

	if(argc > 1){
		noOfCols = getNoOfColsForMB(atoi(argv[1]));
	}
	if(argc > 2){
		love = atoi(argv[2]);
		
	}
	if(argc > 3){
		noOfRows = atoi(argv[3]);
	}


	double **rows = malloc(sizeof(double*) * noOfRows);
	for(int c = 0; c < noOfRows; c++){
		rows[c] = malloc(sizeof(double) * noOfCols);
		
		for(int d = 0; d < noOfCols; d++){
			rows[c][d] = getRandomNumber();
		}
	}

	struct timeval start;
	struct timeval stop;
	struct timeval difference;

	gettimeofday(&start, NULL);


	double totalSum = 0.0;
	int cids[2];
	int ptr = 0;

	if(love){
		cids[0] = mchain();
		cids[1] = mchain();
	}

	for(int row = 1; row < noOfRows -1; row++){
		
		if(love){
			brk_mchain(cids[ptr]);

			if(row > 1){
				hate(rows[row - 2], noOfCols*sizeof(double));
			}

			mlink(cids[ptr], rows[row], noOfCols*sizeof(double));

			ptr += 1;
			ptr = ptr%2;
		}


		for(int col = 1; col < noOfCols -1; col++){

			double sum = 0.0;
			for(int i = -1; i < 2; i++){
				for(int j = -1; j < 2; j++){
					sum += rows[row + i][col + j] * (1.0/9.0);
				}
			}
			rows[row][col] = sum;
			totalSum += sum;
		}
	}

	gettimeofday(&stop, NULL);

	time_difference(&difference, &stop, &start);

    printf("Done.\n");
    printf("Time: %ds %dus\n", difference.tv_sec, difference.tv_usec);

	return 0;

}

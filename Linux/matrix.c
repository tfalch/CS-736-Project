//Compile with : gcc -std=c99 -lm matrix.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
	return 4.0;
}

int findMatrixSize(int megabytes){
	double megabytesPrMatrix = ((double)megabytes)/3.0;
	double bytesPrMatrix = megabytesPrMatrix * 1e6;
	double matrixSize = bytesPrMatrix/((double)sizeof(double));
	matrixSize = sqrt(matrixSize);
	return (int)matrixSize;
}


int main(int argc, char** argv){
	int matrixSize = 3;
	int output = 0;
	int love = 0;
	if(argc > 1){
		matrixSize = findMatrixSize(atoi(argv[1]));
	}
	if(argc > 2){
		love = atoi(argv[2]);
	}

	if(argc > 3){
		output = atoi(argv[3]);
	}


	double** firstMatrix = malloc(sizeof(double*)*matrixSize);
	for(int i = 0; i < matrixSize; i++){
		firstMatrix[i] = malloc(sizeof(double)*matrixSize);

		for(int j = 0; j < matrixSize; j++){
			firstMatrix[i][j] = getRandomNumber();
		}
	}

	double** secondMatrix = malloc(sizeof(double*)*matrixSize);
	for(int i = 0; i < matrixSize; i++){
		secondMatrix[i] = malloc(sizeof(double)*matrixSize);

		for(int j = 0; j < matrixSize; j++){
			secondMatrix[i][j] = getRandomNumber();
		}
	}

	double** resultMatrix = malloc(sizeof(double*)*matrixSize);
	for(int i = 0; i < matrixSize; i++){
		resultMatrix[i] = malloc(sizeof(double)*matrixSize);
	}

	struct timeval start;
	struct timeval stop;
	struct timeval difference;

	gettimeofday(&start, NULL);
	
	int cid;

	if(love){
		cid = mchain();
	}


	for(int i = 0; i < matrixSize; i++){

		double* row = firstMatrix[i];
		
		if(love){
			mlink(0, row, sizeof(row));
		}

		for(int j = 0; j < matrixSize; j++){

			double* col = secondMatrix[j];

			double sum = 0.0;
			for(int k = 0; k < matrixSize; k++){
				sum += row[k]*col[k];
			}

			resultMatrix[i][j] = sum;
			if(output){
				printf("%d %d %f\n", i, j, sum);
			}
		}
		if(love){
			brk_mchain(cid);
			//hate col
			//hate matrix[i][j]
		}
	}
	
	rls_mchain(cid);

	gettimeofday(&stop, NULL);
	
	time_difference(&difference, &stop, &start);
    	fprintf(stdout, "Done.\n");
    	fprintf(stdout, 
	    "Time: %ds %dus\n",
	    difference.tv_sec, difference.tv_usec);
	

	return 0;
}

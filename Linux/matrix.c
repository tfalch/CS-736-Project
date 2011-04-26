#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "mchain.h"

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

	gettimeofday(&start, NULL);

	if(love){
		mchain();
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
			munlink(row, sizeof(row));
			//hate col
			//hate matrix[i][j]
		}
	}

	gettimeofday(&stop, NULL);
	

	return 0;
}

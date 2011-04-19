#include <stdio.h>
#include <stdlib.h>

double getRandomNumber(){
	return 4.0;
}

int main(int argc, char** argv){
	int matrixSize = 3;
	int output = 0;
	if(argc > 1){
		matrixSize = atoi(argv[1]);
	}
	if(argc > 2){
		output = 1;
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


	for(int i = 0; i < matrixSize; i++){
		for(int j = 0; j < matrixSize; j++){

			double* row = firstMatrix[i];
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
	}
	return 0;
}

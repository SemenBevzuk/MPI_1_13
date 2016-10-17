#include "stdio.h"
#include "stdlib.h"
#include <time.h>
#include <random>
#include <thread>

#define EPS 0.0001

int* CreateAndFillMatrixVector(int n, int m) {
	int i;
	int* matrix = (int*)malloc(n * m * sizeof(int));
	if (matrix == NULL)
	{
		printf_s("No memory!\n");
		exit(0);
	}
	else {
		for (i = 0; i < n*m; i++) {
			matrix[i] = 0 + rand() % 100;
		}
	}
	return matrix;
}

void PrintMatrixVector(int* matrix, int n, int m) {
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			printf_s("%5.1d ", matrix[i*m + j]);
		}
		printf_s("\n");
	}
}

void DeleteMatrix(int* matrix) {
	free(matrix);
}

int FindMaxInMatrix(int* matrix, int n, int m) {
	int i, j;
	int max = matrix[0];
	int *temp = matrix;
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			if (*temp > max) {
				max = matrix[i*n+j];
			}
			temp++;
		}
	}
	return max;
}

void PrintVector(int *data, int dataSize) {
	int i;
	for (i = 0; i < dataSize; i++) {
		printf_s("%5d ", data[i]);
		if (i % 10 == 9) {
			printf_s("\n");
		}
	}
	printf_s("\n");
}

using namespace std;
int main() { //int main(int argc, char* argv[]) {
	clock_t time;
	double delta_time;
	FILE *f;
	int n, m;
	int *matrix = NULL;
	int max;
	printf_s("Enter n = ");
	scanf_s("%d", &n);
	printf_s("Enter m = ");
	scanf_s("%d", &m);
	matrix = CreateAndFillMatrixVector(n,m);

	time = clock();
	max = FindMaxInMatrix(matrix, n, m);
	time = clock() - time;
	delta_time = (double)time / CLOCKS_PER_SEC;
	if (n*m<=100)
	{
		PrintMatrixVector(matrix, n, m);
	}
	printf("Time = %f\n", delta_time);
	printf_s("Max = %d\n", max);
	if (delta_time>EPS)
	{
		fopen_s(&f, "../log/consistent.txt", "a");
		fprintf(f, "%f %d %d\n", delta_time, n, m);
		fflush(f);
		fclose(f);
	}
	return 0;
}
// ParallelPI.cpp : Defines the entry point for the MPI application.
#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include <random>

#define EPS 0.0001

int* CreateAndFillMatrixVector(int n, int m) {
	int i;
	int* matrix = (int*)malloc(n * m * sizeof(int));
	for (i = 0; i < n*m; i++) {
		matrix[i] = 0 + rand() % 100;
	}
	return matrix;
}

int FindMaxInMatrix(int* matrix, int n, int m) {
	int i, j;
	int max = matrix[0];
	int *temp = matrix;
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			if (*temp > max) {
				max = matrix[i*n + j];
			}
			temp++;
		}
	}
	return max;
}

void DeleteMatrix(int* matrix) {
	free(matrix);
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
int main(int argc, char* argv[]) {
	double time1, time2, delta_time_parallel, delta_time_consistent;
	MPI_Status status;
	FILE *f=NULL;
	int i;
	int n, m;
	int *vector;
	int *matrix = NULL;
	int thread_count, rank;
	int dataSize, bufferSize;
	int deltaSize;
	int LocalMax;
	int remainingData;
	int GlobalMax;
	int res;

	if (argc >= 3) {
		n = atoi(argv[1]);
		m = atoi(argv[2]);
	}
	else {
		printf_s("Error with argv: argc!=3\n");
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &thread_count);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	dataSize = (n*m) / (thread_count - 1); //элементов на поток
	remainingData = n*m - dataSize*(thread_count-1); //добавка к стандартному размеру полоски
	if (rank==0){
		bufferSize = remainingData;
	}
	else{
		bufferSize = dataSize;
	}
	vector = new int[bufferSize];

	if (rank == 0){
		matrix = CreateAndFillMatrixVector(n, m);
		//printf_s("n = %d, m = %d\n", n, m);
		//printf_s("bufferSize = %d, remainingData = %d\n", dataSize, remainingData);
		if (n*m <= 100) {
			PrintMatrixVector(matrix, n, m);
		}
		time1 = MPI_Wtime();

		int *temp_start_matrix = matrix;

		for (i = 1; i < thread_count; i++) {
			MPI_Send(temp_start_matrix, dataSize, MPI_INT, i, 0, MPI_COMM_WORLD);
			temp_start_matrix = temp_start_matrix + dataSize;
		}
		if (remainingData!=0) {
			LocalMax = matrix[dataSize*(thread_count - 1)];
			for (i = dataSize*(thread_count - 1)+1; i < n*m; i++) {
				if (matrix[i]>LocalMax)
				{
					LocalMax = matrix[i];
				}
			}
		}
	}

	if (rank!=0) {
		MPI_Recv(vector, bufferSize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		LocalMax = vector[0];
		for (i = 0; i < bufferSize; i++) {
			if (vector[i]>LocalMax) {
				LocalMax = vector[i];
			}
		}
	}

	MPI_Reduce(&LocalMax, &GlobalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		time2 = MPI_Wtime();
		delta_time_parallel = time2 - time1;
		printf_s("MPI:\n");
		printf_s("	Time = %f\n", delta_time_parallel);
		printf_s("	Threads = %d\n", thread_count);
		printf_s("	GlobalMax = %d\n\n", GlobalMax);
		if (delta_time_parallel > EPS) {
			fopen_s(&f, "../../log/parallel.txt", "a");
			fprintf_s(f, "%f %d %d %d\n", delta_time_parallel, n, m, thread_count);
			fflush(f);
			fclose(f);
		}
	}

	if (rank == 0) {
		time1 = MPI_Wtime();
		GlobalMax = FindMaxInMatrix(matrix, n, m);
		time2 = MPI_Wtime();

		delta_time_consistent = time2 - time1;
		printf_s("Сonsistent implementation:\n");
		printf_s("	Time = %f\n", delta_time_consistent);
		printf_s("	GlobalMax = %d\n", GlobalMax);
		if (delta_time_consistent>EPS) {
			fopen_s(&f, "../../log/consistent.txt", "a");
			fprintf(f, "%f %d %d\n", delta_time_consistent, n, m);
			fflush(f);
			fclose(f);
		}

		fopen_s(&f, "../../log/result.txt", "a");
		printf_s("\nOptimal choice: ");
		if (delta_time_parallel<delta_time_consistent)
		{
			printf_s("parallel implementation. Delta_time = %f;\n", delta_time_consistent - delta_time_parallel);
			fprintf(f, "Parallel implementation;  DeltaTime = %f; Threads = %d; n = %d; m = %d;\n", delta_time_consistent - delta_time_parallel, thread_count, n, m);
		}
		else
		{
			printf_s("consistent implementation. Delta_time = %f;\n", delta_time_parallel - delta_time_consistent);
			fprintf(f, "Consistent implementation; DeltaTime = %f; Threads = %d; n = %d; m = %d;\n", delta_time_parallel - delta_time_consistent, thread_count, n, m);
		}
		fflush(f);
		fclose(f);
		DeleteMatrix(matrix);
	}
	free(vector);
	MPI_Finalize();
	return 0;
}
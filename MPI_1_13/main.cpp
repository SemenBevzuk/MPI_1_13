// ParallelPI.cpp : Defines the entry point for the MPI application.

//матрицы в любой реализации хранятся линейно! Переделать послед.

#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include <random>

int* CreateAndFillMatrixVector(int n, int m) {
	int i;
	int* matrix = (int*)malloc(n * m * sizeof(int));
	for (i = 0; i < n*m; i++) {
		matrix[i] = 0 + rand() % 100;
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

void DeleteMatrix(int** matrix, int n, int m) {
	int i;
	for (i = 0; i < n; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

int FindMaxInMatrix(int** matrix, int n, int m) {
	int i, j;
	int max = matrix[0][0];
	for (i = 0; i < n; i++) {
		for (j = 0; j < m; j++) {
			if (matrix[i][j]>max) {
				max = matrix[i][j];
			}
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
int main(int argc, char* argv[]) {
	double time1, time2, delta_time;
	MPI_Status status;
	int i;
	int n, m;
	int *vector;
	int *matrix = NULL;

	int thread_count, rank;

	int dataSize, bufferSize;
	int deltaSize;

	if (argc >= 3) {
		n = atoi(argv[1]);
		m = atoi(argv[2]);
	}
	else {
		printf_s("Error with argv: argc!=3\n");
	}

	MPI_Init(&argc, &argv);
	time1 = MPI_Wtime();
	MPI_Comm_size(MPI_COMM_WORLD, &thread_count);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	dataSize = (n*m) / (thread_count - 1); //элементов на поток
	deltaSize = n*m - dataSize*(thread_count - 1); //добавка к стандартному размеру полоски

	bufferSize = dataSize;
	if (rank < (deltaSize + 1) && rank != 0) {
		bufferSize = dataSize + 1;
	}

	if (rank == 0) {
		matrix = CreateAndFillMatrixVector(n, m);
		vector = new int[thread_count];
		printf_s("n = %d, m = %d, thread_count = %d\n", n, m, thread_count);
		PrintMatrixVector(matrix, n, m);
	}
	else {
		vector = new int[bufferSize];
	}

	if (rank == 0) {
		int *temp_start_matrix = matrix;

		for (i = 1; i < thread_count; i++) {
			if (i<deltaSize + 1) {
				MPI_Send(temp_start_matrix, bufferSize + 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				temp_start_matrix = temp_start_matrix + bufferSize + 1;
			}
			else {
				MPI_Send(temp_start_matrix, bufferSize, MPI_INT, i, 0, MPI_COMM_WORLD);
				temp_start_matrix = temp_start_matrix + bufferSize;
			}
		}
	}
	else {
		MPI_Recv(vector, bufferSize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	}

	if (rank != 0) {
		int MaxNumber = vector[0];
		for (i = 0; i < bufferSize; i++) {
			if (vector[i]>MaxNumber) {
				MaxNumber = vector[i];
			}
		}
		//printf_s("rank = %d; buffer = %d; Max Number = %d; \n", rank, bufferSize, MaxNumber);
		//PrintVector(vector, bufferSize);
		MPI_Send(&MaxNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		for (i = 1; i < thread_count; i++) {
			int number;
			MPI_Recv(&number, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			vector[status.MPI_SOURCE] = number;
		}
	}

	int MaxNumber;
	if (rank == 0) {
		MaxNumber = vector[1];
		for (i = 1; i < thread_count; i++) {
			if (vector[i]>MaxNumber) {
				MaxNumber = vector[i];
			}
		}
		printf_s("Max = %d\n", MaxNumber);
	}
	free(vector);
	time2 = MPI_Wtime();
	delta_time = time2 - time1;
	printf_s("Time = %f; Time1 = %f; Time 2 = %f\n", delta_time, time1, time2);
	MPI_Finalize();

	return 0;
}
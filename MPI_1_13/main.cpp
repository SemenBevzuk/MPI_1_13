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
		matrix[i] = 0 + rand() % 1000;
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
	double time1, time2, delta_time_1, delta_time_2;
	MPI_Status status;
	FILE *f=NULL;
	int i;
	int n, m;
	int *vector;
	int *matrix = NULL;
	int thread_count, rank;
	int dataSize, bufferSize;
	int deltaSize;
	int MaxNumber;

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
	deltaSize = n*m - dataSize*(thread_count - 1); //добавка к стандартному размеру полоски

	bufferSize = dataSize;
	if (rank < (deltaSize + 1) && rank != 0) {
		bufferSize = dataSize + 1;
	}

	if (rank == 0) {
		matrix = CreateAndFillMatrixVector(n, m);
		printf_s("n = %d, m = %d\n", n, m);

		if (n*m<=100)
		{
			PrintMatrixVector(matrix, n, m);
		}

		time1 = MPI_Wtime();
		vector = new int[thread_count];

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
		for (i = 1; i < thread_count; i++) {
			int number;
			MPI_Recv(&number, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
			vector[status.MPI_SOURCE] = number;
		}
		MaxNumber = vector[1];
		for (i = 1; i < thread_count; i++) {
			if (vector[i]>MaxNumber) {
				MaxNumber = vector[i];
			}
		}
		time2 = MPI_Wtime();

		delta_time_1 = time2 - time1;
		printf_s("MPI:\n");
		printf_s("	Time = %f\n", delta_time_1);
		printf_s("	Threads = %d\n", thread_count);
		printf_s("	Max = %d\n\n", MaxNumber);
		if (delta_time_1 > EPS) {
			fopen_s(&f, "../../log/parallel.txt", "a");
			fprintf_s(f, "%f %d %d %d\n", delta_time_1, n, m, thread_count);
			fflush(f);
			fclose(f);
		}
	}
	else {
		vector = new int[bufferSize];
		MPI_Recv(vector, bufferSize, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
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
		time1 = MPI_Wtime();
		MaxNumber = FindMaxInMatrix(matrix, n, m);
		time2 = MPI_Wtime();

		delta_time_2 = time2 - time1;
		printf_s("Сonsistent implementation:\n");
		printf_s("	Time = %f\n", delta_time_2);
		printf_s("	Max = %d\n", MaxNumber);
		if (delta_time_2>EPS) {
			fopen_s(&f, "../../log/consistent.txt", "a");
			fprintf(f, "%f %d %d\n", delta_time_2, n, m);
			fflush(f);
			fclose(f);
		}

		printf_s("\nOptimal choice: ", MaxNumber);
		if (delta_time_1<delta_time_2)
		{
			printf_s("parallel implementation.\n");
		}
		else
		{
			printf_s("consistent implementation.\n");
		}
		DeleteMatrix(matrix);
	}
	free(vector);
	MPI_Finalize();
	return 0;
}
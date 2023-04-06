#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define N 500
#define IDX(i, j) (((i)*N) + (j))
void compute(double *A, double *B, int num)
{
    for (int i = 1; i < N - 1; i++)
    {
        for (int j = 1; j < N - 1; j++)
        {
            B[IDX(i, j)] = (A[IDX(i - 1, j)] + A[IDX(i, j + 1)] + A[IDX(i + 1, j)] + A[IDX(i, j - 1)]) / 4.0;
        }
    }
}
void gen_rand_array(double *a, int num)
{
    for (int i = 0; i < num; i++)
    {
        srand(time(NULL));
        a[i] = rand() % 100;
    }
}
int check_ans(double *B, double *C)
{
    for (int i = 1; i < N - 1; i++)
    {
        for (int j = 1; j < N - 1; j++)
        {
            if (fabs(B[IDX(i, j)] - C[IDX(i, j)]) >= 1e-4)
            {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int id_procs, num_procs, num_1;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);
    double *A, *B, *B2;
    A = (double *)malloc(N * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));
    B2 = (double *)malloc(N * N * sizeof(double));
    num_1 = num_procs - 1;
    // Proc#N-1 randomize the data
    if (id_procs == num_1)
    {
        gen_rand_array(A, N * N);
        compute(A, B2, N * N);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    int ctn = 0;
    for (int i = 0; i < N - 2; i++)
    {
        if (id_procs == num_1)
        {
            int dest = i % num_1;
            int tag = i / num_1;
            MPI_Send(&A[IDX(i, 0)], N * 3, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD);
        }
    }

    for (int i = 0; i < (N - 2) / num_1; i++)
    {
        if (id_procs != num_1)
        {
            MPI_Recv(&A[IDX(3 * ctn, 0)], 3 * N, MPI_DOUBLE, num_1, ctn, MPI_COMM_WORLD, &status);
            ctn++;
        }
    }
    if (id_procs < (N - 2) % num_1)
    {
        MPI_Recv(&A[IDX(ctn * 3, 0)], 3 * N, MPI_DOUBLE, num_1, ctn, MPI_COMM_WORLD, &status);
        ctn++;
    }

    // compute
    if (id_procs != num_1)
    {
        for (int i = 1; i <= 3 * ctn - 2; i += 3) //!  每三行计算一次
        {
            for (int j = 1; j < N - 1; j++)
            {
                B[IDX((i + 2) / 3, j)] = (A[IDX(i - 1, j)] + A[IDX(i, j + 1)] + A[IDX(i + 1, j)] + A[IDX(i, j - 1)]) / 4.0;
            }
        }
    }

    // Gather
    for (int i = 0; i < N - 2; i++)
    {
        if (id_procs == num_1)
        {
            int src = i % num_1;
            MPI_Recv(&B[IDX(i + 1, 1)], N - 2, MPI_DOUBLE, src, i / num_1 + N, MPI_COMM_WORLD, &status);
        }
        else
        {
            for (int j = 0; j < ctn; j++)
                MPI_Send(&B[IDX(j + 1, 1)], N - 2, MPI_DOUBLE, num_1, j + N, MPI_COMM_WORLD);
        }
    }

    if (id_procs == num_1)
    {
        if (check_ans(B, B2))
        {
            printf("Done.No Error\n");
        }
        else
        {
            printf("Error Occured!\n");
        }
    }
    free(A);
    free(B);
    free(B2);
    MPI_Finalize();
    return 0;
}

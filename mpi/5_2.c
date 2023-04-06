#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define N 500
#define IDX(i, j) (((i)*N) + (j))

void gen_rand_array(double *a, int num)
{
    for (int i = 0; i < num; i++)
    {
        srand(time(NULL));
        a[i] = rand() % 100;
    }
}

void compute(double *A, double *B, int a, int b)
{
    for (int i = 1; i <= a; i++)
    {
        for (int j = 1; j <= b; j++)
        {
            B[IDX(i, j)] = (A[IDX(i - 1, j)] + A[IDX(i, j + 1)] + A[IDX(i + 1, j)] + A[IDX(i, j - 1)]) / 4.0;
        }
    }
}

int check_ans(double *B, double *A)
{
    for (int i = 1; i < N - 1; i++)
    {
        for (int j = 1; j < N - 1; j++)
        {
            if (fabs(B[IDX(i, j)] - A[IDX(i, j)]) >= 1e-2)
            {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int id_procs, num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);
    MPI_Status status;
    MPI_Datatype SubMat;
    int rows = sqrt(num_procs);
    int cols = num_procs / rows;
    int a = (N - 2 + rows - 1) / rows;
    int b = (N - 2 + cols - 1) / cols;
    int alloc_num = (a + 1) * (b + 1) * num_procs;
    double A[alloc_num];
    double B[alloc_num];
    double B2[alloc_num];

    // Proc#0 randomize the data
    if (id_procs == 0)
    {
        gen_rand_array(A, N * N);
        compute(A, B2, N - 2, N - 2);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Proc#0 broadcast (a+2)x(b+2) mat
    MPI_Type_vector(a + 2, b + 2, N, MPI_DOUBLE, &SubMat);
    MPI_Type_commit(&SubMat);

    if (id_procs == 0)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (i == 0 && j == 0)
                    continue;
                MPI_Send(A + i * a * N + b * j, 1, SubMat, j + cols * i, 0, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        MPI_Recv(A, 1, SubMat, 0, 0, MPI_COMM_WORLD, &status);
    }

    // computeute
    compute(A, B, a, b);

    // Gather result
    MPI_Datatype SubMat_B;
    MPI_Type_vector(a, b, N, MPI_DOUBLE, &SubMat_B);
    MPI_Type_commit(&SubMat_B);
    if (id_procs == 0)
    {
        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                if (i == 0 && j == 0)
                    continue;
                MPI_Recv(&B[IDX(a * i + 1, b * j + 1)], 1, SubMat_B, i * cols + j, 1, MPI_COMM_WORLD, &status);
            }
        }
    }
    else
    {
        int x = id_procs / cols;
        int y = id_procs % cols;
        MPI_Send(&B[IDX(1, 1)], 1, SubMat_B, 0, 1, MPI_COMM_WORLD);
    }

    if (id_procs == 0)
        if (check_ans(B, B2))
        {
            printf("Done.No Error\n");
        }
        else
        {
            printf("Error!\n");
        }
    MPI_Finalize();
    return 0;
    return 0;
}

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define IDX(i, j, N) (((i) * (N)) + (j))
void gen_rand_mat(int *a, int num)
{
    for (int i = 0; i < num; i++)
    {
        srand(clock());
        for (int j = 0; j < num; j++)
        {
            a[IDX(i, j, num)] = rand() % 100;
        }
    }
}

void print_mat(int *a, int num, int id)
{
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num; j++)
        {
            printf("|%d :  %d ", id, a[IDX(i, j, num)]);
        }
        printf("\n");
    }
}

void compute(int *A, int *B, int *C, int num)
{
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num; j++)
        {
            for (int k = 0; k < num; k++)
                C[IDX(i, j, num)] += A[IDX(i, k, num)] * B[IDX(k, j, num)];
        }
    }
}

int check(int *C, int *nC, int num)
{
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num; j++)
        {
            if (C[IDX(i, j, num)] != nC[IDX(i, j, num)])
            {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int id_procs, num_procs;
    int block_size, sqrt_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);

    sqrt_procs = sqrt(num_procs);
    if (sqrt_procs * sqrt_procs != num_procs)
    {
        fprintf(stderr, "num of procs must be square\n");
        return 1;
    }
    if (argc != 2)
    {
        fprintf(stderr, "you need to provide block size\n");
        return 1;
    }
    block_size = atoi(argv[1]);
    int *sA, *sB, *sC;
    int N = block_size * sqrt_procs;
    if (id_procs == 0)
    {
        sA = (int *)malloc(N * N * sizeof(int));
        sB = (int *)malloc(N * N * sizeof(int));
        sC = (int *)malloc(N * N * sizeof(int));

        memset(sC, 0, N * N * sizeof(int));
        gen_rand_mat(sA, N);
        gen_rand_mat(sB, N);
        compute(sA, sB, sC, N);
    }
    int A[block_size * block_size];
    int B[block_size * block_size];
    int C[block_size * block_size];
    int ans[block_size * block_size];
    int A_in[block_size * block_size];
    int B_in[block_size * block_size];
    memset(C, 0, block_size * block_size * sizeof(int));
    MPI_Datatype SubMat, Mat;
    MPI_Status status;
    MPI_Request request;
    MPI_Type_vector(block_size, block_size, N, MPI_INT, &SubMat);
    MPI_Type_commit(&SubMat);
    MPI_Type_vector(block_size, block_size, block_size, MPI_INT, &Mat);
    MPI_Type_commit(&Mat);
    if (id_procs == 0)
    {
        for (int i = 0; i < sqrt_procs; i++)
        {
            int lineoff = block_size * N * i;
            for (int j = 0; j < sqrt_procs; j++)
            {
                if (i == 0 && j == 0)
                {
                    MPI_Isend(sA, 1, SubMat, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Irecv(A, 1, Mat, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    MPI_Isend(sB, 1, SubMat, 0, 1, MPI_COMM_WORLD, &request);
                    MPI_Irecv(B, 1, Mat, 0, 1, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    continue;
                }
                int offset = j * block_size + lineoff;
                MPI_Send(sA + offset, 1, SubMat, i * sqrt_procs + j, 0, MPI_COMM_WORLD);
                MPI_Send(sB + offset, 1, SubMat, i * sqrt_procs + j, 1, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        MPI_Recv(A, 1, Mat, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(B, 1, Mat, 0, 1, MPI_COMM_WORLD, &status);
    }

    MPI_Comm row_comm, col_comm;
    int rank_A, size_A;
    int color_A;
    int key_A;

    int rank_B, size_B;
    int color_B;
    int key_B;

    key_A = id_procs % sqrt_procs;
    color_A = id_procs / sqrt_procs;
    MPI_Comm_split(MPI_COMM_WORLD, color_A, key_A, &row_comm);
    MPI_Comm_rank(row_comm, &rank_A);
    MPI_Comm_size(row_comm, &size_A);

    key_B = id_procs / sqrt_procs;
    color_B = id_procs % sqrt_procs;
    MPI_Comm_split(MPI_COMM_WORLD, color_B, key_B, &col_comm);
    MPI_Comm_rank(col_comm, &rank_B);
    MPI_Comm_size(col_comm, &size_B);

    for (int k = 0; k < sqrt_procs; k++)
    {
        if (rank_A == (color_A + k) % size_A)
        {
            memcpy(A_in, A, block_size * block_size * sizeof(int));
        }
        MPI_Bcast(A_in, 1, Mat, (color_A + k) % size_A, row_comm);
        compute(A_in, B, C, block_size);
        int dest = (rank_B - 1 + size_B) % size_B;
        MPI_Send(B, 1, Mat, dest, 0, col_comm);
        MPI_Recv(B_in, 1, Mat, (rank_B + 1) % size_B, 0, col_comm, &status);
        memcpy(B, B_in, block_size * block_size * sizeof(int));
    }

    if (id_procs == 0)
    {
        for (int i = 0; i < sqrt_procs; i++)
        {
            for (int j = 0; j < sqrt_procs; j++)
            {
                if (i == 0 && j == 0)
                {
                    MPI_Isend(sC, 1, SubMat, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Irecv(ans, 1, Mat, 0, 0, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    continue;
                }
                int offset = j * block_size + block_size * N * i;
                MPI_Send(sC + offset, 1, SubMat, i * sqrt_procs + j, 100, MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        MPI_Recv(ans, 1, Mat, 0, 100, MPI_COMM_WORLD, &status);
    }

    print_mat(ans, block_size, id_procs);
    print_mat(C, block_size, id_procs);
    if (check(C, ans, block_size))
    {
        printf("Proc#%d Done.\n", id_procs);
    }

    if (id_procs == 0)
    {
        free(sA);
        free(sB);
        free(sC);
    }
    MPI_Finalize();
    return 0;
}

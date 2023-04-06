#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#define IN(i, j, line) ((i)*line + j)
#define M 20
#define N 20
void random_init(int *a, int num)
{
    srand(time(NULL));
    for (int i = 0; i < num; i++)
    {
        a[i] = rand() % 1000 - 500;
    }
}

int check_ans(int *a, int *b, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (a[i] != b[i])
        {
            printf("%d\n", i);
            return 0;
        }
    }
    return 1;
}

void copy_array(int *dst, int *src, int num)
{
    for (int i = 0; i < num; i++)
    {
        dst[i] = src[i];
    }
}

int loop1()
{
    int n = (M + 2) * N;

    int A[n];
    int B[n];
    int C = 41734;

    omp_set_num_threads(4);

    random_init(A, n);
    copy_array(B, A, n);

    clock_t start = clock();
    for (int i = 1; i <= M; i++)
    {
        for (int j = 1; j < N; j++)
        {
            A[IN(i + 1, j + 1, N)] = A[IN(i, j, N)] + C;
        }
    }
    clock_t end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 1; i <= M; i++)
    {
#pragma omp parallel for
        for (int j = 1; j < N; j++)
        {
            B[IN(i + 1, j + 1, N)] = B[IN(i, j, N)] + C;
        }
    }
    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, B, n);
}

int loop2()
{
    int X[101];
    int X2[101];
    int Y[201];
    int Y2[201];
    int B[101];
    int B2[101];
    int n = 110 * 110;
    int A[n], C[n], A2[n];
    random_init(A, n);
    random_init(C, n);
    random_init(Y, 201);
    // random_init(B, 101);
    copy_array(B2, B, 101);
    copy_array(X2, X, 101);
    copy_array(A2, A, n);
    copy_array(Y2, Y, 201);

    omp_set_num_threads(4);
    clock_t start = clock();

    for (int i = 1; i <= 100; i++)
    {
        X2[i] = Y2[i] + 10;
        for (int j = 1; j <= 100; j++)
        {
            B2[j] = A2[IN(j, N, 110)];
            for (int k = 1; k <= 100; k++)
            {
                A2[IN(j + 1, k, 110)] = B2[j] + C[IN(j, k, 110)];
            }
            Y2[i + j] = A2[IN(j + 1, N, 110)];
        }
    }
    clock_t end = clock();
    printf("normal loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    // #pragma omp parallel for
    for (int i = 1; i <= 100; i++)
    {
        // #pragma omp parallel for
        for (int j = 1; j <= 100; j++)
        {
            B[j] = A[IN(j, N, 110)];
#pragma omp parallel for
            for (int k = 1; k <= 100; k++)
            {
                A[IN(j + 1, k, 110)] = B[j] + C[IN(j, k, 110)];
            }
        }
    }

    for (int i = 1; i <= 100; i++)
    {
#pragma omp parallel for
        for (int j = 1; j <= 100; j++)
        {
            Y[i + j] = A[IN(j + 1, N, 110)];
        }
    }

#pragma omp parallel for
    for (int i = 1; i <= 100; i++)
    {
        X[i] = Y[i] + 10;
    }

    end = clock();
    printf("openmp loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, A2, n) && check_ans(B, B2, 100) && check_ans(X, X2, 100);
}

int main()
{
    if (loop1())
        printf("loop1 done!\n");
    else
        printf("loop1 error!\n");
    if (loop2())
        printf("loop2 done!\n");
    else
        printf("loop2 error!\n");
}
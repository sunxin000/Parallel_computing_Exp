#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#define IN(i, j, line) ((i)*line + j)
#define min(i, j) (((i) < (j)) ? (i) : (j))
#define max(i, j) (((i) > (j)) ? (i) : (j))
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
    int i, j, k;
    int n = 20;
    int B[n * n];
    int B2[n * n];
    int B3[n * n];

    omp_set_num_threads(4);
    random_init(B2, n * n);
    copy_array(B, B2, n * n);
    copy_array(B3, B2, n * n);
    clock_t start, end;
    start = clock();
    for (i = 2; i <= 10; i++)
    {
        for (j = i; j <= 10; j++)
        {
            B[IN(i, j, 20)] = (B[IN(i, j - 1, 20)] + B[IN(i - 1, j, 20)]) * 0.5;
        }
    }
    end = clock();

    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (i = 4; i <= 20; i++)
    {
#pragma omp parallel for
        for (j = max(2, i - 10); j <= min(i / 2, 10); j++)
        {
            B3[IN(j, i - j, 20)] = (B3[IN(j, i - j - 1, 20)] + B3[IN(j - 1, i - j, 20)]) * 0.5;
        }
    }
    end = clock();
    printf("openmp diagonal parallel loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(B, B3, n * n);
}

int loop2()
{
    int i;
    int A[20];
    int B[20];
    int A2[20];
    random_init(A, 20);
    copy_array(A2, A, 20);

    omp_set_num_threads(4);

    clock_t start, end;

    start = clock();

    for (int i = 1; i <= 16; i++)
    {
        A[i + 3] = A[i] + B[i];
    }
    end = clock();

    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();

    for (int k = 1; k <= 16; k += 3)
    {
#pragma omp parallel for
        for (int i = k; i <= min(16, k + 2); i++)
        {
            A2[i + 3] = A2[i] + B[i];
        }
    }
    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A2, A, 20);
}

int loop3()
{
    int i, j, k;
    int A[20];
    int B[20];
    int A2[20];
    random_init(A, 20);
    copy_array(A2, A, 20);

    omp_set_num_threads(4);

    clock_t start, end;

    start = clock();
    for (k = 1; k <= 16; k += 5)
    {
        for (i = k; i <= min(16, k + 4); i++)
        {
            A[i + 3] = A[i] + B[i];
        }
    }
    end = clock();

    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();

    for (int k = 1; k <= 16; k += 3)
    {
#pragma omp parallel for
        for (int i = k; i <= min(16, k + 2); i++)
        {
            A2[i + 3] = A2[i] + B[i];
        }
    }
    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A2, A, 20);
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
    if (loop3())printf("loop3 done\n");
}

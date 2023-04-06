#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#define IN(i, j, line) ((i)*line + j)
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
        if (a[i] != b[i]){
	printf("%d\n",i);
            return 0;
	}}
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
    int A[256];
    int B[256]; // 可初始化相同的值然后检查两个数组是否相同。
    omp_set_num_threads(4);
    random_init(A, 256);
    copy_array(B, A, 256);
    clock_t start = clock();
    for (int i = 2; i <= 10; i++)
    {
        for (int j = 2; j <= 10; j++)
        {
            A[IN(i, j, 16)] = 0.5 * (A[IN(i - 1, j - 1, 16)] + A[IN(i + 1, j + 1, 16)]);
        }
    }
    clock_t end = clock();
    printf("normal loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 2; i <= 10; i++)
    {
#pragma omp parallel for
        for (int j = 2; j <= 10; j++)
        {
            B[IN(i, j, 16)] = 0.5 * (B[IN(i - 1, j - 1, 16)] + B[IN(i + 1, j + 1, 16)]);
        }
    }
    end = clock();
    printf("openmp loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, B, 256);
}

int loop2()
{

    int A[100];
    int B[100];
    int A2[100];
    int B2[100];
    random_init(A, 100);
    random_init(B, 100);
    copy_array(A2, A, 100);
    copy_array(B2, B, 100);

    int i;
    clock_t start = clock();
    for (i = 2; i <= 20; i++)
    {
        A[2 * i + 2] = A[2 * i - 2] + B[i];
    }
    clock_t end = clock();
    printf("normal loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();

#pragma omp parallel num_threads(2) private(i)
    {
        int tid = omp_get_thread_num();
        for (i = 2 + tid; i <= 20; i += 2)
        {
            A2[2 * i + 2] = A2[2 * i - 2] + B2[i];
        }
    }

    end = clock();
    printf("openmp loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, A2, 100);
}

int loop3()
{
    int A[100];
    int B[100];
    int C[100];
    int D[100];
    int B2[100];
    int C2[100];
    random_init(A, 100);
    random_init(B, 100);
    random_init(C, 100);
    copy_array(B2, B, 100);
    copy_array(C2, C, 100);
    int k;
    clock_t start = clock();
    for (int i = 2; i < 20; i++)
    {
        if (A[i] > 0)
            B[i] = C[i - 1] + 1;
        else
            C[i] = B[i] - 1;
    }
    clock_t end = clock();
    printf("normal loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    D[0]=2;
    int m = 1;
    for (int i = 2; i < 20; i++)
    {
        if ((A[i - 1] < 0) && (A[i] > 0))
        {
            D[m] = i;
            m++;
        }
    }
    D[m] = 20;
    for (int i = 0; i < m; i++)
    {
#pragma omp parallel for
        for (k = D[i]; k < D[i + 1]; k++)
        {
            if (A[k] > 0)
            {
                B2[k] = C2[k - 1] + 1;
            }
            else
            {
                C2[k] = B2[k] - 1;
            }
        }
    }
    end = clock();
    printf("openmp loop costs: %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(C, C2, 100) && check_ans(B, B2, 100);
}

int main()
{
    omp_set_num_threads(4);
    if (loop1()) printf("loop1 done!\n");
    if (loop2()) printf("loop2 done!\n");
    if (loop3()) printf("loop3 done!\n");
}

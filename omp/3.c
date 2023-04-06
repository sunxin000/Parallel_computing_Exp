#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>

#define IN(i, j, line) ((i)*line + j)
void random_init(int *a, int num)
{
    srand(time(NULL));
    for (int i = 0; i < num; i++)
    {
        a[i] = rand() % 1000 - 500;
    }
}

void random_init_f(float *a, int num)
{
    srand(time(NULL));
    for (int i = 0; i < num; i++)
    {
        a[i] = (float)(rand() % 1000 - 500);
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

int check_ans_f(float *a, float *b, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (fabs(a[i] - b[i]) > 0.000001)
        {
            printf("%d\n", i);
           printf("%f %f \n", a[i], b[i]);
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

void copy_array_f(float *dst, float *src, int num)
{
    for (int i = 0; i < num; i++)
    {
        dst[i] = src[i];
    }
}

int loop1()//此循环不能并行化，见第一次作业，会输出error
{
    int i, j, k;
    int n = 500;
    int A[n * n];
    int B[n * n];
    int A2[n * n];
    int B2[n * n];
    clock_t start, end;
    random_init(A, n * n);
    random_init(B, n * n);
    copy_array(A2, A, n * n);
    copy_array(B2, B, n * n);

    omp_set_num_threads(4);

    start = clock();
    for (i = 1; i <= 100; i++)
    {
        for (j = 1; j <= 50; j++)
        {
            A[IN(3 * i + 2, 2 * j - 1, 500)] = A[IN(5 * j, i + 3, 500)] + 2;
        }
    }
    end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (i = 1; i <= 100; i++)
    {
#pragma omp parallel for
        for (j = 1; j <= 50; j++)
        {
           A2[IN(3 * i + 2, 2 * j - 1, 500)] = A2[IN(5 * j, i + 3, 500)] + 2;
        }
    }

    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    return check_ans(A2, A, n * n);
}

int loop2()
{
    int i, j, k;
    float x, y, z;
    x =5;
    y = 10;
    z = 20;
    float z2 = z;
    clock_t start, end;

    float A[200], B[200], C[200], D[10000];
    random_init_f(A, 200);
    random_init_f(B, 200);
    random_init_f(C, 200);
    random_init_f(D, 10000);

    float A2[200], B2[200], C2[200], D2[10000];
    copy_array_f(A2, A, 200);
    copy_array_f(B2, B, 200);
    copy_array_f(C2, C, 200);
    copy_array_f(D2, D, 10000);

    // copy_array(W, A, 100);
    omp_set_num_threads(4);
    start = clock();
    x = y * 2;
    for (i = 1; i <= 100; i++)
    {
        C[i] = B[i] + x;
        A[i] = C[i - 1] + z;
        C[i + 1] = A[i] * B[i];
        for (j = 1; j <= 50; j++)
        {
            D[IN(i, j, 60)] = D[IN(i, j - 1, 60)] + x;
        }
    }
    z = y + 4;
    end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    x = y * 2;
#pragma omp parallel for
    for (i = 1; i <= 100; i++)
    {
        C2[i] = B2[i] + x;
    }
#pragma omp parallel for private(i, j)
    for (i = 1; i <= 100; i++)
    {
        A2[i] = C2[i - 1] + z2;
        for (j = 1; j <= 50; j++)
        {
            D2[IN(i, j, 60)] = D2[IN(i, j - 1, 60)] + x;
        }
    }
    C2[101] = A2[100] *  B2[100];
    z2 = y + 4;
    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans_f(D2, D, 200)&&check_ans_f(C2, C, 200);
}

int main()
{
    if (loop1())
        printf("loop1 done\n");
    else
        printf("loop1 error\n");

    if(loop2()) printf("loop2 done\n");
}

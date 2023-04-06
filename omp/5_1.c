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
    int A[200], B[200], C[200], D[200];
    int A2[200], B2[200], C2[200], D2[200];
    random_init(A, 200);
    random_init(B, 200);
    random_init(C, 200);
    random_init(D, 200);
    copy_array(A2, A, 200);
    copy_array(B2, B, 200);
    copy_array(C2, C, 200);
    copy_array(D2, D, 200);

    clock_t start, end;

    omp_set_num_threads(4);

    start = clock();

    for (i = 1; i <= 100; i++)
    {
        A[i] = A[i] + B[i - 1];
        B[i] = C[i - 1] * 2;
        C[i] = 1 + B[i]; // 除法边加法避免浮点数
        D[i] = C[i] * C[i];
    }

    end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
    for (i = 1; i <= 100; i++)
    {
        B2[i] = C2[i - 1] * 2;
        C2[i] = 1 + B2[i];
    }

#pragma omp parallel for
    for (i = 1; i <= 100; i++)
    {
        A2[i] = A2[i] + B2[i - 1];
        D2[i] = C2[i] * C2[i];
    }

    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, A2, 200);
}

int loop2()
{
    int i, j, k;
    int A[1001], B[1001], C[1001], D[1001];
    int A2[1001], B2[1001], C2[1001], D2[1001];
    random_init(A, 1001);
    random_init(B, 1001);
    random_init(C, 1001);
    random_init(D, 1001);
    copy_array(A2, A, 1001);
    copy_array(B2, B, 1001);
    copy_array(C2, C, 1001);
    copy_array(D2, D, 1001);

    clock_t start, end;


    start = clock();

    for (i = 1; i <= 1000; i++)
    {
        A[i] = B[i] + C[i];
        D[i] = (A[i] + A[999 - i + 1]) / 2;
    }

    end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();
#pragma omp parallel for
    for (i = 1; i <= 500; i++)
    {
        A2[i] = B2[i] + C2[i];
        D2[i] = (A2[i] + A2[1000 - i]) / 2;
    }

#pragma omp parallel for
    for (i = 501; i <= 999; i++)
    {
        A2[i] = B2[i] + C2[i];
        D2[i] = (A2[i] + A2[1000 - i]) / 2;
    }

    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);
    return check_ans(A, A2, 1000);
}

int loop3()
{
    int i, j, k;
    int n = 510 * 510;
    int B[n], D[n];
    int B2[n], D2[n];
    int A[510][510], A2[510][510], C[510][510], C2[510][510];

    omp_set_num_threads(4);
    for (i = 0; i < 510; i++)
        for (j = 0; j < 510; j++)
        {
            A[i][j] = rand() % 1000 - 500;
            A2[i][j] = A[i][j];
            C[i][j] = rand() % 1000 - 500;
            C2[i][j] = C[i][j];
        }

    // random_init(A, n);
    random_init(B, n);
    // random_init(C, n);
    random_init(D, n);
    // copy_array(A2, A, n);
    copy_array(B2, B, n);
    // copy_array(C2, C, n);
    copy_array(D2, D, n);

    clock_t start, end;

    omp_set_num_threads(4);

    start = clock();

    for (i = 1; i <= 100; i++)
    {
        for (j = 1; j <= 100; j++)
        {
            A[3 * i + 2 * j][2 * j] = C[i][j] * 2;
            D[IN(i, j, 510)] = A[i - j + 6][i + j];
        }
    }

    end = clock();
    printf("normal loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    start = clock();

#pragma omp parallel for collapse(2)
    for (i = 1; i <= 100; i++)
    {
        for (j = 1; j <= 100; j++)
        {
            A2[3 * i + 2 * j][2 * j] = C2[i][j] * 2;
            D2[IN(i, j, 510)] = A2[i - j + 6][i + j];
        }
    }

    end = clock();
    printf("openmp loop costs : %Lf\n", (long double)(end - start) / CLOCKS_PER_SEC);

    for (i = 0; i < 510; i++)
        for (j = 0; j < 510; j++)
        {
            if (A[i][j] != A2[i][j])
            {
                printf("%d  %d\n", i, j);
                printf("%d  %d\n", A[i][j], A2[i][j]);
                return 0;
            }
        }
    return 1;
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

    if (loop3())
        printf("loop3 done!\n");
    else
        printf("loop3 error!\n");
}

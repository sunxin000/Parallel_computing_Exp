#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#define SERVER_NUM 2

void serve(MPI_Comm server_comm, int id, int num)
{
    int num_workers = num - SERVER_NUM;
    int recv_size = num_workers / SERVER_NUM + 1;
    int recv_data[recv_size];
    int gather_buff[recv_size * SERVER_NUM];
    int average;
    int sum, ctn;
    MPI_Status status;
    while (1)
    {
        memset(recv_data, 0, recv_size * sizeof(int));
        memset(gather_buff, 0, recv_size * SERVER_NUM * sizeof(int));
        sum = 0;
        ctn = 0;
        for (int i = 1; i * SERVER_NUM + id < num; i++)
        {
            MPI_Recv(recv_data + i - 1, 1, MPI_INT, i * SERVER_NUM + id, 0, MPI_COMM_WORLD, &status);
            ctn++;
        }

        MPI_Allgather(recv_data, recv_size, MPI_INT, gather_buff, recv_size, MPI_INT, server_comm);

        for (int i = 0; i < recv_size * SERVER_NUM; i++)
        {
            sum += gather_buff[i];
        }
        average = sum / num_workers;
        printf("Proc#%d send average data = %d\n", id, average);
        MPI_Barrier(server_comm);
        for (int i = 1; i <= ctn; i++)
        {
            MPI_Send(&average, 1, MPI_INT, i * SERVER_NUM + id, 1, MPI_COMM_WORLD);
        }
    }
}

void work(int id)
{
    int randata;
    int recvdata;
    MPI_Status status;

    while (1)
    {
        srand(time(NULL) + id);
        randata = rand() % 100;
        printf("proc#%d send data = %d\n", id, randata);
        MPI_Send(&randata, 1, MPI_INT, id % SERVER_NUM, 0, MPI_COMM_WORLD);
        MPI_Recv(&recvdata, 1, MPI_INT, id % SERVER_NUM, 1, MPI_COMM_WORLD, &status);
        printf("Proc#%d receive average data = %d\n", id, recvdata);
        sleep(5);
    }
}

int main(int argc, char *argv[])
{
    int id_procs, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);

    int P, Q;
    P = SERVER_NUM;
    Q = num_procs - P;

    MPI_Comm server_comm;
    MPI_Comm_split(MPI_COMM_WORLD, id_procs / P, id_procs, &server_comm);

    if (id_procs > P - 1)
    {
        work(id_procs);
    }
    else
    {
        serve(server_comm, id_procs, num_procs);
    }

    MPI_Finalize();
    return 0;
}

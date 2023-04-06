#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int id_procs,num_procs;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);

	int data = id_procs;
	int recvdata;
	MPI_Status status;

	for (int i =2; i<= num_procs; i<<=1)
	{
		int tag = i>>1;
		int diff = id_procs & tag;
		if (diff){
			MPI_Send(&data, 1, MPI_INT, id_procs-tag, tag, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Recv(&recvdata, 1, MPI_INT, id_procs+tag, tag, MPI_COMM_WORLD, &status);
		}

		data += recvdata;
	}

	for (int i = num_procs; i>=2; i>>=1){
		int tag =i;
		if(id_procs % i == 0)
		{
			MPI_Send(&data, 1, MPI_INT, id_procs+(i>>1), tag, MPI_COMM_WORLD);
		}
		else if (id_procs % (i>>1) ==0)
		{
			MPI_Recv(&data, 1, MPI_INT,id_procs-(i>>1), tag, MPI_COMM_WORLD, &status);
		}
	}

	printf("%d Sum is = %d\n", id_procs, data);

	MPI_Finalize();
	return 0;
}

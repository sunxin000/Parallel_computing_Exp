#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int id_procs, num_procs;
	int msg = 10;
	int tag = 5;
	char seq[16] = "hello mpi!";
	char seqin[16];
	char hostname[100];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);

	int color = id_procs / 4;
	int key = id_procs % 4;
	gethostname(hostname, sizeof(hostname));
	MPI_Comm split_comm_world;
	MPI_Status status;
	int rank, size, msgin;

	MPI_Comm_split(MPI_COMM_WORLD, color, key, &split_comm_world);
	MPI_Comm_rank(split_comm_world, &rank);
	MPI_Comm_size(split_comm_world, &size);
	printf("id_procs: %d. process %d of %d. comm: %d. host: %s\n", id_procs, rank, size, color, hostname);
	MPI_Barrier(MPI_COMM_WORLD);	
	if(id_procs == 0){
		strcpy(seqin, seq);
		MPI_Send(&seq, 16, MPI_CHAR, 4, tag, MPI_COMM_WORLD);
	}

	else if (id_procs==4){
		MPI_Recv(&seqin, 16, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);
	}

	MPI_Bcast(&seqin, 16, MPI_CHAR, 0, split_comm_world);
	printf("MPI Commm rand %d, original id %d, size %d. The new msg is %s\n", rank, id_procs, size, seqin);
	MPI_Finalize();
	return 0;

}

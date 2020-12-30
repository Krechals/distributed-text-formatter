#include "mpi.h"
#include <bits/stdc++.h>

#define ROOT 0

using namespace std;

int main(int argc, char *argv[]) {
	int numtasks, rank, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);


	MPI_Finalize();
	return 0;
}

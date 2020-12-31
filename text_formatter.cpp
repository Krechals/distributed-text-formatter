#include "mpi.h"
#include <bits/stdc++.h>

#define ROOT 0
#define MASTER_THREADS 4
#define HORROR_ID 0
#define COMEDY_ID 1
#define FANTASY_ID 2
#define SCIFI_ID 3

using namespace std;

void parse_paragraph(ifstream &fin) {
	std::string line;

	do {
		getline(fin, line);
	} while (line != "");
}

void parse(int id) {
	ifstream fin("input.txt");
	std::string line;

	while (getline(fin, line)) {
		if (line == "horror" && id == HORROR_ID) {
			parse_paragraph(fin);
		} else if (line == "comedy" && id == COMEDY_ID) {
			parse_paragraph(fin);
		} else if (line == "fantasy" && id == FANTASY_ID) {
			parse_paragraph(fin);
		} else if (line == "science-fiction" && id == SCIFI_ID) {
			parse_paragraph(fin);
		} 
 	}
}

int main(int argc, char *argv[]) {
	int numtasks, rank, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == ROOT) {
    	std::thread parsers[MASTER_THREADS];

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i] = std::thread(parse, i);
    	}

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i].join();
    	}
    }

	MPI_Finalize();
	return 0;
}

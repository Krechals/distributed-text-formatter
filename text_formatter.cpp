#include "mpi.h"
#include <bits/stdc++.h>
#include <condition_variable>
#include "producers_consumers.h"

#define ROOT 0
#define MASTER_THREADS 4
#define HORROR_ID 1
#define COMEDY_ID 2
#define FANTASY_ID 3
#define SCIFI_ID 4
#define MAX_PARAGRAPH 10000
#define END_OF_FILE "==EOF=="

using namespace std;

string parse_paragraph(ifstream &fin) {
	std::string ans = "";
	std::string line;

	do {
		ans += '\n';
		getline(fin, line);
		ans += line;
	} while (line != "");

	return ans;
}

void parse(int id) {
	ifstream fin("input.txt");
	std::string line;
	std::string paragraph;
 	char buff[MAX_PARAGRAPH] = {0};

	while (getline(fin, line)) {
		if (line == "horror" && id == HORROR_ID) {
			paragraph = "horror" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(HORROR_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, HORROR_ID, 0, MPI_COMM_WORLD, &status);
			cout << buff << endl;
		} else if (line == "comedy" && id == COMEDY_ID) {
			paragraph = "comedy" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			cout << buff << endl;
		} else if (line == "fantasy" && id == FANTASY_ID) {
			paragraph = "fantasy" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD); 
		} else if (line == "science-fiction" && id == SCIFI_ID) {
			paragraph = "science-fiction" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD); 
		} 
 	}

 	paragraph = END_OF_FILE;
 	MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD); 
}

int main(int argc, char *argv[]) {
	int numtasks, rank, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    if (rank == ROOT) {
    	std::thread parsers[MASTER_THREADS];

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i] = std::thread(parse, i + 1);
    	}

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i].join();
    	}
    } else if (rank == HORROR_ID) {
    	Producer_Consumer horror;
    	horror.parallelise(HORROR_ID);
    	
    } else if (rank == COMEDY_ID) {
    	Producer_Consumer comedy;
    	comedy.parallelise(COMEDY_ID);
    } else if (rank == FANTASY_ID) {
    	// TODO
    } else if (rank == SCIFI_ID) {
    	// TODO
    } 

	MPI_Finalize();
	return 0;
}

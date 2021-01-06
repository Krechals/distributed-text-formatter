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

std::atomic<unsigned int> paragraph_id(1);
std::mutex mtx;
std::condition_variable order;
std::string output_text;

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
	std::unique_lock<std::mutex> lck(mtx);

	int current_paragraph = 0;
	unordered_map<int, string> paragraph_order;
	ifstream fin("input.in");
	
	std::string line;
	std::string paragraph;
 	char buff[MAX_PARAGRAPH] = {0};

	while (getline(fin, line)) {
		if (line == "horror") {
			++current_paragraph;
			if (id != HORROR_ID) {
				continue;
			}

			paragraph = "horror" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(HORROR_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, HORROR_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;

			paragraph_order[current_paragraph] = buffer;
			// cout << buff << endl;
		} else if (line == "comedy") {
			++current_paragraph;
			if (id != COMEDY_ID) {
				continue;
			}

			paragraph = "comedy" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;

			paragraph_order[current_paragraph] = buffer;
			// cout << buff << endl;
		} else if (line == "fantasy") {
			++current_paragraph;
			if (id != FANTASY_ID) {
				continue;
			}

			paragraph = "fantasy" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(FANTASY_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, FANTASY_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;

			paragraph_order[current_paragraph] = buffer;
			// cout << buff << endl;
		} else if (line == "science-fiction") {
			++current_paragraph;
			if (id != SCIFI_ID) {
				continue;
			}

			paragraph = "science-fiction" + parse_paragraph(fin);
			MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);

			MPI_Status status;
			int length = 0;

			MPI_Probe(SCIFI_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, SCIFI_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;

			paragraph_order[current_paragraph] = buffer;
			// cout << buff << endl;
		} 
 	}

 	paragraph = END_OF_FILE;
 	MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);


 	for (unsigned int i = 1; i <= current_paragraph; ++i) {
 		if (paragraph_order.find(i) != paragraph_order.end()) {
 			while (i != paragraph_id) {
 				order.wait(lck);
 			}

 			if (i < current_paragraph) {
 				output_text += paragraph_order[i] + '\n';
 			} else {
 				output_text += paragraph_order[i];
 			}

 			++paragraph_id;
 			order.notify_all();
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
    		parsers[i] = std::thread(parse, i + 1);
    	}

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i].join();
    	}
    	ofstream fout("input.out");
    	fout << output_text;
    } else if (rank == HORROR_ID) {
    	Producer_Consumer horror;
    	horror.parallelise(HORROR_ID);
    	
    } else if (rank == COMEDY_ID) {
    	Producer_Consumer comedy;
    	comedy.parallelise(COMEDY_ID);
    } else if (rank == FANTASY_ID) {
    	Producer_Consumer fantasy;
    	fantasy.parallelise(FANTASY_ID);
    } else if (rank == SCIFI_ID) {
    	Producer_Consumer scifi;
    	scifi.parallelise(SCIFI_ID);
    } 

	MPI_Finalize();
	return 0;
}

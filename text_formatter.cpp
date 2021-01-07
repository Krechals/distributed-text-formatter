#include "producers_consumers.h"
#include "mpi.h"
#include <algorithm>
#include <fstream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <unordered_map>
#include <atomic>

#define ROOT 0
#define MASTER_THREADS 4
#define HORROR_ID 1
#define COMEDY_ID 2
#define FANTASY_ID 3
#define SCIFI_ID 4
#define MAX_PARAGRAPH 2000000
#define END_OF_FILE "==EOF=="
#define END_OF_PARA "==EOP=="

using namespace std;

std::atomic<unsigned int> paragraph_id{1};
unordered_map<int, string> paragraph_order;
std::mutex mtx;
std::string output_text;
int max_paragraph = 0;

void send_paragraph(ifstream &fin, int worker_id) {
	std::string ans = "";
	std::string line;

	do {
		int line_nr = 0;
		ans = "";
		do {
			getline(fin, line);
			ans += line + '\n';
			++line_nr;
		} while (line != "" && !fin.eof() && line_nr < 20);
		ans.pop_back();
		MPI_Send(&ans[0], ans.size() + 1, MPI_CHAR, worker_id, 0, MPI_COMM_WORLD);

	} while (line != "" && !fin.eof());

	ans = END_OF_PARA;
	MPI_Send(&ans[0], ans.size() + 1, MPI_CHAR, worker_id, 0, MPI_COMM_WORLD);
}

void add_paragraph(int index, const string& buffer) {
	mtx.lock();
	paragraph_order[index] = buffer;
	mtx.unlock();
}

void parse(int id, char* input_file) {
	int current_paragraph = 0;
	ifstream fin(input_file);

	std::string line;
	std::string paragraph;
 	char buff[MAX_PARAGRAPH] = {0};
	while (getline(fin, line)) {
		if (line == "horror") {
			++current_paragraph;
			if (id != HORROR_ID) {
				continue;
			}
			send_paragraph(fin, HORROR_ID);
			MPI_Status status;
			int length = 0;

			MPI_Probe(HORROR_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, HORROR_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;
			buffer = "horror\n" + buffer;

			add_paragraph(current_paragraph, buffer);
			// cout << buff << endl;
		} else if (line == "comedy") {
			++current_paragraph;
			if (id != COMEDY_ID) {
				continue;
			}
			send_paragraph(fin, COMEDY_ID);
			MPI_Status status;
			int length = 0;

			MPI_Probe(COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, COMEDY_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;
			buffer = "comedy\n" + buffer;

			add_paragraph(current_paragraph, buffer);
		} else if (line == "fantasy") {
			++current_paragraph;
			if (id != FANTASY_ID) {
				continue;
			}

			send_paragraph(fin, FANTASY_ID);
			MPI_Status status;
			int length = 0;

			MPI_Probe(FANTASY_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, FANTASY_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;
			buffer = "fantasy\n" + buffer;

			add_paragraph(current_paragraph, buffer);
		} else if (line == "science-fiction") {
			++current_paragraph;
			if (id != SCIFI_ID) {
				continue;
			}

			send_paragraph(fin, SCIFI_ID);
			MPI_Status status;
			int length = 0;

			MPI_Probe(SCIFI_ID, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, SCIFI_ID, 0, MPI_COMM_WORLD, &status);
			string buffer = buff;
			buffer = "science-fiction\n" + buffer;

			add_paragraph(current_paragraph, buffer);
		} 
 	}
 	paragraph = END_OF_FILE;
 	MPI_Send(&paragraph[0], paragraph.size() + 1, MPI_CHAR, id, 0, MPI_COMM_WORLD);
 	max_paragraph = current_paragraph;
}

int main(int argc, char *argv[]) {
	int numtasks, rank, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if (rank == ROOT) {
    	std::thread parsers[MASTER_THREADS];

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i] = std::thread(parse, i + 1, argv[1]);
    	}

    	for (int i = 0; i < MASTER_THREADS; ++i) {
    		parsers[i].join();
    	}
    	string output_file = argv[1];
    	output_file[output_file.size() - 3] = 'o';
    	output_file[output_file.size() - 2] = 'u';
    	output_file[output_file.size() - 1] = 't';
    	ofstream fout(output_file);
    	fout << output_text;
    	for (int i = 1; i <= max_paragraph; ++i) {
    		if (i < max_paragraph) {
    			fout << paragraph_order[i] << '\n';
    		} else {
    			fout << paragraph_order[i];
    		}
    	}
    } else if (rank == HORROR_ID) {
    	parallelise(HORROR_ID);
    	
    } else if (rank == COMEDY_ID) {
    	parallelise(COMEDY_ID);
    } else if (rank == FANTASY_ID) {
    	parallelise(FANTASY_ID);
    } else if (rank == SCIFI_ID) {
    	parallelise(SCIFI_ID);
    } 

	MPI_Finalize();
	return 0;
}
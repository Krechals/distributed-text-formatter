#pragma once

#include "mpi.h"
#include <bits/stdc++.h>
#include <atomic>

#define ROOT 0
#define HORROR_ID 1
#define COMEDY_ID 2
#define FANTASY_ID 3
#define SCIFI_ID 4
#define MAX_PARAGRAPH 10000
#define END_OF_FILE "==EOF=="

using namespace std;

class Producer_Consumer {
 private:
 	string buffer;
 	std::mutex mtx;
	std::condition_variable produce, consume;
	unsigned int line_nr;
	int consumers_nr;
	std::atomic<bool> end;
	std::atomic<unsigned int> current_ticket;
	pthread_barrier_t barrier;

	int count_lines(string buffer) {
		int ans = 0;

		for (unsigned int i = 0; i < buffer.size(); ++i) {
			if (buffer[i] == '\n') {
				++ans;
			}
		}
		return ans;
	}

	unsigned int get_ticket() {
		return current_ticket++;
	}

	bool consonant(char letter) {
		return letter != 'A' && letter != 'E' && letter != 'I' && letter != 'O' && letter != 'U' &&
			letter != 'a' && letter != 'e' && letter != 'i' && letter != 'o' && letter != 'u' && 
			letter != '\n' && letter != '.' && letter != ' ';
	}

	bool letter(char letter) {
		return (letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z');
	} 

 public:
 	Producer_Consumer() {
 		end = false;
 		current_ticket = 0;
 		pthread_barrier_init(&barrier, NULL, std::thread::hardware_concurrency());
 	}

 	void parallelise(int id) {
 		int num_threads = std::thread::hardware_concurrency() - 1;
    	std::thread consumers[num_threads];
    	for (int i = 0; i < num_threads; ++i) {
    		if (id == HORROR_ID) {
    			consumers[i] = std::thread(&Producer_Consumer::consumer_horror, this);
    		} else if (id == COMEDY_ID) {
    			consumers[i] = std::thread(&Producer_Consumer::consumer_comedy, this);
    		}
    	}
    	std::thread receiver = std::thread(&Producer_Consumer::receiver, this);

		for (int i = 0; i < num_threads; ++i) {
			consumers[i].join();
		}
		receiver.join();

 	}

 	void receiver() {
 		std::unique_lock<std::mutex> lck(mtx);
 		char buff[MAX_PARAGRAPH] = {0};
		do {
			MPI_Status status;
			int length = 0;

			MPI_Probe(ROOT, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			MPI_Recv(&buff, length, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD, &status);
			buffer = buff;
			line_nr = count_lines(buffer) - 1;
			consumers_nr = std::min(1 + line_nr / 20, std::thread::hardware_concurrency() - 1);

			if (buffer != END_OF_FILE) {
				for (int i = 0; i < consumers_nr; ++i) {
					consume.notify_one();
				}

				produce.wait(lck);
				MPI_Send(&buffer[0], buffer.size() + 1, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD);
			}
		} while(strcmp(buff, END_OF_FILE));
		end = true;
		consume.notify_all();
	}

	void consumer_comedy() {
		std::unique_lock<std::mutex> lck(mtx);
		while(!end.load()) {
			consume.wait(lck);
			if (end.load() == true) {
				break;
			}
		 	int ticket = get_ticket();
		 	int remaining_lines = 0;

		 	while(ticket <= line_nr / 20) {
		 		remaining_lines = 20 * ticket + 1;
		 		int i = 0;
		 		while (i < buffer.size() && remaining_lines > 0) {
		 			if (buffer[i] == '\n') {
		 				--remaining_lines;
		 			}
		 			++i;
		 		}

		 		remaining_lines = 20;
		 		int letter_index = 0;
		 		while (i < buffer.size() && remaining_lines > 0) {
		 			if (buffer[i] == '\n') {
		 				--remaining_lines;
		 			}

		 			if (buffer[i] == ' ') {
		 				letter_index = 0;
		 			} else if (letter(buffer[i])) {
		 				if (letter_index++ % 2) {
		 					buffer[i] = std::toupper(buffer[i]);
		 				}
		 			}
		 			++i;
		 		}

		 		ticket = get_ticket();
		 	}
		 	if (remaining_lines > 0) {
		 		produce.notify_one();
		 		current_ticket = 0;
		 	}
		}
	}

 	void consumer_horror() {
 		std::unique_lock<std::mutex> lck(mtx);
		while(!end.load()) {
			consume.wait(lck);
			if (end.load() == true) {
				break;
			}
		 	int ticket = get_ticket();
		 	int remaining_lines = 0;

		 	while(ticket <= line_nr / 20) {
		 		remaining_lines = 20 * ticket + 1;
		 		int i = 0;
		 		while (i < buffer.size() && remaining_lines > 0) {
		 			if (buffer[i] == '\n') {
		 				--remaining_lines;
		 			}
		 			++i;
		 		}

		 		remaining_lines = 20;
		 		while (i < buffer.size() && remaining_lines > 0) {
		 			if (buffer[i] == '\n') {
		 				--remaining_lines;
		 			}
		 			if (consonant(buffer[i])) {
		 				std::string lower (1, std::tolower(buffer[i]));
		 				buffer.insert(i + 1, lower);
		 				++i;
		 			}
		 			++i;
		 		}

		 		ticket = get_ticket();
		 	}
		 	if (remaining_lines > 0) {
		 		produce.notify_one();
		 		current_ticket = 0;
		 	}
		}
	}
};

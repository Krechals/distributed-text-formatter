#pragma once

#include "mpi.h"

#include <algorithm>
#include <fstream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <vector>
#include <atomic>

#define ROOT 0
#define HORROR_ID 1
#define COMEDY_ID 2
#define FANTASY_ID 3
#define SCIFI_ID 4
#define MAX_PARAGRAPH 2000000
#define END_OF_PARA "==EOP=="
#define END_OF_FILE "==EOF=="

using namespace std;

string buffer;
std::mutex mtx_worker;
std::condition_variable produce, consume;
unsigned int line_nr = 0;
int consumers_nr = 0;;
std::atomic<bool> end{false};
std::atomic<int> current_ticket{0};
std::atomic<bool> end_turn{true};
int num_threads = std::thread::hardware_concurrency() - 1;
std::vector<std::string> small_paragraphs;  

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

int count_letters(int start, int ticket) {
	int ans = 0;

	for (int letter_index = start; small_paragraphs[ticket][letter_index] != ' ' && 
								   small_paragraphs[ticket][letter_index] != '\n' && 
								   small_paragraphs[ticket][letter_index] != '\0'; ++letter_index) {
		++ans;
	}
	return ans;
}

void consumer_scifi() {
	int ticket = 0;
	while(ticket <= small_paragraphs.size()) {
	 	ticket = get_ticket();
	 	int i = 0;
	 	if (ticket < small_paragraphs.size()) {
	 		int word_index = 0;
	 		while (i < small_paragraphs[ticket].size()) {
	 			if (small_paragraphs[ticket][i] == '\n') {
	 				word_index = 0;
	 			}

	 			if (i == 0 || small_paragraphs[ticket][i - 1] == ' ' || small_paragraphs[ticket][i - 1] == '\n') {
	 				++word_index;
	 				if (word_index == 7) {
	 					int left = i, right = i + count_letters(i, ticket) - 1;

	 					while (left < right) {
	 						swap(small_paragraphs[ticket][left], small_paragraphs[ticket][right]);
	 						++left;
	 						--right;
	 					}
	 					word_index = 0;
	 				}
	 			}
	 			++i;
	 		}
	 	}
	}
}

void consumer_fantasy() {
	int ticket = 0;
	while(ticket <= small_paragraphs.size()) {
	 	ticket = get_ticket();
	 	int i = 0;
	 	if (ticket < small_paragraphs.size()) {
	 		while (i < small_paragraphs[ticket].size()) {
	 			if ((i == 0 || small_paragraphs[ticket][i - 1] == ' ' || small_paragraphs[ticket][i - 1] == '\n') && letter(small_paragraphs[ticket][i])) {
	 				small_paragraphs[ticket][i] = std::toupper(small_paragraphs[ticket][i]);
	 			}
	 			++i;
	 		}
	 	}
	}
}

void consumer_comedy() {
	int ticket = 0;
	while(ticket <= small_paragraphs.size()) {
	 	ticket = get_ticket();
	 	int i = 0;
	 	if (ticket < small_paragraphs.size()) {
	 		int letter_index = 0;
	 		while (i < small_paragraphs[ticket].size()) {
	 			if (small_paragraphs[ticket][i] == '\n') {
	 				letter_index = 0;
	 			}

	 			if (small_paragraphs[ticket][i] == '.') {
	 				letter_index++;
	 			}
	 			if (small_paragraphs[ticket][i] == ' ') {
	 				letter_index = 0;
	 			} else if (letter(small_paragraphs[ticket][i])) {
	 				if (letter_index++ % 2) {
	 					small_paragraphs[ticket][i] = std::toupper(small_paragraphs[ticket][i]);
	 				}
	 			}
	 			++i;
	 		}
	 	}
	}
}

void consumer_horror() {
	int ticket = 0;
	while(ticket <= small_paragraphs.size()) {
	 	ticket = get_ticket();
	 	int i = 0;
	 	if (ticket < small_paragraphs.size()) {

	 		while (i < small_paragraphs[ticket].size()) {
	 			if (consonant(small_paragraphs[ticket][i])) {
	 				std::string lower (1, std::tolower(small_paragraphs[ticket][i]));
	 				small_paragraphs[ticket].insert(i + 1, lower);
	 				++i;
	 			}
	 			++i;
	 		}
	 	}
	}
}

void recv(int id) {
	char buff[MAX_PARAGRAPH] = {0};
	std::thread consumers[num_threads];
	while(1) {
		small_paragraphs.clear();  
		while(1) {
			MPI_Status status;
			int length = 0;

			MPI_Probe(ROOT, 0, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_CHAR, &length);

			memset(buff, 0, MAX_PARAGRAPH);
			MPI_Recv(&buff, length, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD, &status);
			buffer = "";
			buffer = buff;
			if (buffer == END_OF_FILE) {
				return;
			}
			if (buffer == END_OF_PARA) {
				break;
			}
			small_paragraphs.push_back(buffer);
		}

		unsigned int potential_threads = small_paragraphs.size();
		consumers_nr = std::min(potential_threads, std::thread::hardware_concurrency() - 1);

    	for (int i = 0; i < consumers_nr; ++i) {
    		if (id == HORROR_ID) {
    			consumers[i] = std::thread(consumer_horror);
    		} else if (id == COMEDY_ID) {
    			consumers[i] = std::thread(consumer_comedy);
    		} else if (id == FANTASY_ID) {
    			consumers[i] = std::thread(consumer_fantasy);
    		} else if (id == SCIFI_ID) {
    			consumers[i] = std::thread(consumer_scifi);
    		}
    	}
    	for (int i = 0; i < consumers_nr; ++i) {
			consumers[i].join();
		}
		current_ticket = 0;

		buffer = "";
		for (int i = 0; i < small_paragraphs.size(); ++i) {
			if (i != small_paragraphs.size() - 1) {
				buffer += small_paragraphs[i] + '\n';
			} else {
				buffer += small_paragraphs[i];
			}
		}
		MPI_Send(&buffer[0], buffer.size() + 1, MPI_CHAR, ROOT, 0, MPI_COMM_WORLD);
	}
}

void parallelise(int id) {
	std::thread receiver = std::thread(recv, id);
	receiver.join();
}

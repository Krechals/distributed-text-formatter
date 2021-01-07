CC = mpic++
CFLAGS = -Wall -Wextra 

build:
	$(CC) $(CFLAGS) text_formatter.cpp -o main -lpthread
run:
	mpirun -np 5 main
clean:
	rm main
	

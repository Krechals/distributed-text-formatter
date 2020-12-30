CC = mpic++
CFLAGS = -Wall -Wextra

build:
	$(CC) $(CFLAGS) text_formatter.cpp -o text_formatter
run:
	mpirun -np 5 text_formatter
clean:
	rm text_formatter
	

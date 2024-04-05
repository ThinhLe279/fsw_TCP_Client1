CC = gcc
CFLAGS = -Wall -Wextra -std=c11

.PHONY: all clean

all: client1

client1: client1.c
	$(CC) $(CFLAGS) -o client1 client1.c

clean:
	rm -f client1 clien1.o

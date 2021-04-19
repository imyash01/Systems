CC = gcc
CFLAGS=-fsanitize=address

all:
	$(CC) $(CFLAGS) compare.c bst.c stringbuffer.c queue.c thread.c -o compare -lm -pthread

clean:
	rm -rf compare
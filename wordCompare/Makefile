CC = gcc
CFLAGS=-Wall -Werror -fsanitize=address

all:
	$(CC) $(CFLAGS) compare.c stringbuffer.c -o compare -pthread

clean:
	rm -rf compare
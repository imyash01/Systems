CC = gcc
CFLAGS=-Wall -Werror -fsanitize=address

all:
	$(CC) $(CFLAGS) compare.c -o compare -pthread

clean:
	rm -rf compare
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#define SIZE 128

typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

int sb_init(strbuf_t *L, size_t length)
{
    L->data = malloc(sizeof(char) * length);
    if (!L->data) return 1;

    L->length = length;
    L->used   = 1;
    L->data[0] = '\0';

    return 0;
}

void sb_destroy(strbuf_t *L)
{
    free(L->data);
}


int sb_append(strbuf_t *L, char letter)
{
    if (L->used == L->length) {
	size_t size = L->length * 2;
	char *p = realloc(L->data, sizeof(char) * size);
	if (!p) return 1;

	L->data = p;
	L->length = size;

	if (DEBUG) printf("Increased size to %lu\n", size);
    }

    L->data[L->used-1] = letter;
    L->data[L->used] = '\0';
    ++L->used;

    return 0;
}

int wrap(int width, int input_fd, int output_fd){
    //wrap does the reading from a file or stdin and the writing to either a file or stdout
    char *read_buff = (char*) calloc(SIZE, sizeof(char)); //don't attempt to read entire file in read

    strbuf_t word;

    int currLength = 0;
    int newWord = 1;
    //read until there is nothing to read
    while(read(input_fd,read_buff,SIZE) != 0){
        for(int i = 0; i < SIZE; i++){
            char curr = read_buff[i];

            //checks if curr is a space if is it it gets written and destroyed otherwise appended to the word.
            if(!isspace(curr)){
                if(newWord) sb_init(&word,32);
                sb_append(&word, curr);
                newWord = 0;
            }
            else{
                if(word.used - 1 > width){
                    write(output_fd,word.data,word.used);
                    sb_destroy(&word);
                    return EXIT_FAILURE;
                }
                currLength += word.used-1;
                if(currLength > width || (curr == '\n' && currLength == 0)){ //check this for blank lines it adds a \n after a line
                    write(output_fd,'\n',1);
                    currLength = 0;
                }
                else{
                    write(output_fd,word.data,word.used);
                    sb_destroy(&word);
                    newWord = 1;
                }
            }
        }
    }
    return 0;
}

int main (int argc, char* argv[] ) {

    if(argv[1] < 1) return EXIT_FAILURE;

    int fd = open(argv[2], O_RDONLY);

    if(fd == -1) {
        return EXIT_FAILURE;
    }

    
    
    //need to check if the argument given is a file or a directory

    // 3 scenarios
    if(fd == 0) { // input is from std input

    } else if (fd == 1) { //input is from file
        
    } else if (fd == 2) { //file name is a directory

    } else {
        return EXIT_FAILURE;
    }

    return 0;
}
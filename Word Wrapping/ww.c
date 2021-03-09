#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

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
    char *read_buff;
    
    read_buff = (char*) calloc(SIZE, sizeof(char)); //don't attempt to read entire file in read

    strbuf_t word;

    char prev[2] = " ";

    int currLength = 0;
    int newWord = 1; //bool to check if its a new word
    int firstWord = 1;// bool to check if its the first word
    int nRead = 0;
    //read until there is nothing to read
    while(read(input_fd,read_buff,SIZE) != 0){
        for(int i = 0; i < SIZE; i++){ //make a while loop until '\n'
            char curr = read_buff[i];

            if(read_buff[i-1] == 0 && read_buff[i] == 0)
                break; 

            //checks if curr is a space if is it it gets written and destroyed otherwise appended to the word.
            if(!isspace(curr) && curr != '\0'){
                if(newWord) 
                    sb_init(&word,32);

                sb_append(&word, curr);
                newWord = 0;
            }
            else{
                if(word.used - 1 > width){
                    write(output_fd,word.data,word.used);
                    sb_destroy(&word); // reset the values
                    return EXIT_FAILURE;
                }
                currLength += word.used-1;
                
                if(currLength > width){ //check this for blank lines, paragraph or it goes over width it adds a \n after a line
                        
                        write(output_fd,"\n",1); //append new line and destroy
                    
                        write(output_fd,word.data,word.used-1);
                        sb_destroy(&word); // reset the values just to be safe
                        newWord = 1;
                        currLength = word.used-1;
                    
                }
                else if(prev[0] == '\n' && curr == '\n'){
                    write(output_fd,"\n",1); //append new line
                    write(output_fd,"\n",1); //append new line
                    firstWord = 1;
                }
                else{
                    if(!firstWord){
                        write(output_fd," ",1);
                        currLength++;
                    }

                    write(output_fd,word.data,word.used-1);
                    sb_destroy(&word); // reset the values just to be safe
                    newWord = 1;
                    firstWord = 0;
                }
            }
            prev[0] = curr;
        }
    }
    return 0;
}

int isdir(char *name) {
	struct stat data;
	
	int err = stat(name, &data);
	
	// should confirm err == 0
	if (err) {
		perror(name);  // print error message
		return 0;
	}
	
	if (S_ISDIR(data.st_mode)) {
		// S_ISDIR macro is true if the st_mode says the file is a directory
		// S_ISREG macro is true if the st_mode says the file is a regular file

		return 1;
	} 
	
	return 0;
}

int main (int argc, char* argv[] ) {

    if(argv[1] < 1) return EXIT_FAILURE;

    int width = atoi(argv[1]);

    if(argc == 2) {
         wrap(0, width, 1);  // scneario 1: read from STDIN and print in STDOUT
    }

    
    int fd = open(argv[2], O_RDONLY);
    
    if(fd == -1) {
        return EXIT_FAILURE;
    }
    
    
    //need to check if the argument given is a file or a directory
    // 2 scenarios
    
     
    //check to see if second argument is directory or file
    int check = isdir(argv[2]);
    if(check == 1) {
        wrap(fd, width, 1); //the second argument is a file, read from file and then display in std output : scenario 2
    } 
    
    DIR *dirp = opendir(argv[2]);  // open the current directory
    struct dirent *de;

    while ((de = readdir(dirp))) {
        //puts(de->d_name);
        printf("%lu %d %s\n",
            de->d_ino,
            de->d_type,
            de->d_name);
    }

    closedir(dirp); // should check for failure
    return EXIT_SUCCESS;

    //now need to manipulate files in directory 
    
    
    return 0;
}
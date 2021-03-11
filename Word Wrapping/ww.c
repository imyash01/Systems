#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>


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

	//if (deBUG) printf("Increased size to %lu\n", size);
    }

    L->data[L->used-1] = letter;
    L->data[L->used] = '\0';
    ++L->used;

    return 0;
}

int sb_concat(strbuf_t* list, char* str){
    int i = 0;
    while(str[i] != '\0'){
        if(sb_append(list, str[i])){
            return 1;
        }
        i++;
    }
    return 0;
}

int wrap(int width, int input_fd, int output_fd){
    char *read_buff;
    
    read_buff = (char*) calloc(SIZE, sizeof(char)); //don't attempt to read entire file in read

    strbuf_t word;

    char prev[2] = " ";

    int currLength = -1; //to remove the space for firstword in file
    int newWord = 1; //bool to check if its a new word
    int firstWord = 1;// bool to check if its the first word
    int error = 0;

    //read until there is nothing to read
    while(read(input_fd,read_buff,SIZE) != 0){
        for(int i = 0; i < SIZE; i++){ //make a while loop until '\n'
            char curr = read_buff[i];

            if(i > 0 && (read_buff[i-1] == 0 && read_buff[i] == 0)){
                if(currLength > 0)
                    write(output_fd,"\n",1);
                break;
            } 

            //checks if curr is a space if is it it gets written and destroyed otherwise appended to the word.
            if(!isspace(curr) && curr != '\0'){
                if(newWord) 
                    sb_init(&word,32);

                sb_append(&word, curr);
                newWord = 0;
            }
            else{

                    if(word.used - 1 > width){

                        if(currLength >0)
                            write(output_fd,"\n",1);

                        write(output_fd,word.data,word.used-1);
                        write(output_fd,"\n",1);

                        sb_destroy(&word); // reset the values
                        word.used = 1;
                        newWord = 1;
                        firstWord = 1;
                        currLength = -1; //removes space in first word of a paragraph
                        error = 1;
                    }
                    else{
                        currLength += word.used; //adds the 1 for space
                    
                        if(currLength > width && word.used > 1){ //check this for blank lines, paragraph or it goes over width it adds a \n after a line
                                write(output_fd,"\n",1); //append new line and destroy
                            
                                write(output_fd,word.data,word.used-1);
                                sb_destroy(&word);
                                newWord = 1;
                                currLength = word.used-1;
                                word.used = 1;
                            
                        }
                        else if(prev[0] == '\n' && curr == '\n'){
                            write(output_fd,"\n",1); //append new line
                            write(output_fd,"\n",1); //append new line
                            firstWord = 1;
                            currLength = -1; //removes space in first word of a paragraph
                        }
                        else if(word.used > 1){
                            if(!firstWord){
                                write(output_fd," ",1);
                            }

                            write(output_fd,word.data,word.used-1);
                            sb_destroy(&word);
                            word.used = 1;
                            newWord = 1;
                            firstWord = 0;
                        }
                        else{
                            currLength--;
                        }
                    }

            }
            prev[0] = curr;
        }
        free(read_buff);
        read_buff = (char*) calloc(SIZE, sizeof(char));
    }
    free(read_buff);
    
    if(error)
        return 1;
    else
        return 0;
}

int isdir(char *name) {
	struct stat data;
	
	int err = stat(name, &data);
	
	if (err == -1) {
		perror(name);  // print error message
		return -1;
	}
    // S_ISDIR macro is true if the st_mode says the file is a directory
	// S_ISREG macro is true if the st_mode says the file is a regular file
	if (S_ISDIR(data.st_mode)) {
		return 1;
	} 
    else if (S_ISREG(data.st_mode)) {
        return 0;
    } 
	
	return -1;
}

int main (int argc, char* argv[] ) {

   if(argc < 2) return EXIT_FAILURE;

    int width = atoi(argv[1]);

    if(width < 1) return EXIT_FAILURE;

    int error = 0;

    if(argc == 2) {
         if(wrap(width, 0, 1))  // scneario 1: read from STDIN and print in STDOUT
            return EXIT_FAILURE;
         return EXIT_SUCCESS;
    }    
     
    //check to see if second argument is directory or file
    int check = isdir(argv[2]);
    if(check == 0) {
        int fd = open(argv[2], O_RDONLY);
        if(wrap(width, fd, 1)){ //the second argument is a file, read from file and then display in std output : scenario 2
            close(fd);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
        close(fd);
    } 
    else if(check == 1) { 

        DIR *dirp = opendir(argv[2]);  // open the current directory
        struct dirent *folder;

        while ((folder = readdir(dirp))) {

            strbuf_t path;
            sb_init(&path, 32);
            sb_concat(&path,argv[2]);
            sb_append(&path,'/');
            sb_concat(&path,folder->d_name);
            
            int check2 = isdir(path.data);

            if(!check2){

                if(folder->d_name[0] == 'w' && folder->d_name[1] == 'r' && folder->d_name[2] == 'a' && folder->d_name[3] == 'p' && folder->d_name[4] == '.') {
                    sb_destroy(&path);
                    continue;
                }
                //if file starts with .
                if(folder->d_name[0] == '.') {
                    sb_destroy(&path);
                    continue;
                }

                int fd2 = open(path.data, O_RDONLY);

                //create a new file
                strbuf_t out_path;
                sb_init(&out_path, 32);
                sb_concat(&out_path,argv[2]);
                sb_append(&out_path,'/');
                sb_concat(&out_path,"wrap.");
                sb_concat(&out_path, folder->d_name);

                //now write file, create the file
                int fd3 = open(out_path.data, O_WRONLY|O_CREAT|O_TRUNC, 0600);
                if(wrap(width, fd2, fd3))
                    error = 1;

                close(fd3);
                close(fd2);
                sb_destroy(&out_path);
                sb_destroy(&path);
            }
            else if(check2 == 1){ //this means we have a directory and we just skip
                sb_destroy(&path);
                continue;
            }
            else{
                error = 1;
                sb_destroy(&path);
                continue;
            }
            
        }
        closedir(dirp); // should check for failure
    }
    else{
        return EXIT_FAILURE;
    }
    if(error)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int wrap(int width, int input_fd, int output_fd){
    //wrap does the reading from a file or stdin and the writing to either a file or stdout
    char *readData = (char*) calloc(100, sizeof(char)); //don't attempt to read entire file in read
    int currentLength = 0;
    int tempLength = 0;
    while(read(input_fd,readData,100) != 0){
        
    }
}

int main (int argc, char* argv[] ) {

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
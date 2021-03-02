#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[] ) {

    int fd = open(argv[2], O_RDONLY);

    if(fd == -1) {
        return EXIT_FAILURE;
    }



    return 0;
}
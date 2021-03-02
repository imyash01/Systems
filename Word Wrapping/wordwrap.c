#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char* argv[] ) {

    int fileName = open(argv[2], O_RDONLY);

    if(fileName == -1) {
        return EXIT_FAILURE;
    }

    
}
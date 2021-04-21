Project 2 - CS214

Authors: Yash Patel(yp315),Neil Patel(ndp91)

To test our code we worked with multiple set of files (small and large) while also
testing the files with single/multiple threads to read directory entries. 

To test our program, we worked with multiple directories with multiple text files with 
different characters including letters, numbers, dashes, apostrophe, etc. We checked to 
see if our WFD and JSD calculations were correct as well by following the examples that
were mentioned in the write-up. 

We also tested some edge cases which include: 
    - empty files/directories
    - failure to create threads
    - fewer than 2 files for analysis phase
    - used more threads than files given
    - any word frequencies that are outside the range [0,1]

To determine whether out program worked correctly we tested all these scnearios and
we also used valgrind,UBSan,AddressSanitizer to check for any memory leaks and errors (e.g. any infinite
loops, segmentation faults, undefined behaviour etc).
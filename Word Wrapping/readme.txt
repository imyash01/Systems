Project 1 - CS214

Authors: Yash Patel(yp315),Neil Patel(ndp91)

To test our code we made multiple large and small textfiles and we also checked 
for some special scnearios which include but not limited to: 
    - equal amount of characters to the buffer size
    - various width sizes
    - having multiple special charactera (e.g. spaces and tabs)

To test if the text was from stdin, we wrote into stdin with various lengths of text
to see if the text would wordwrap

To test if our program could handle directories and create textfiles that had 
word-wrapped text in it we mainly created directories with multiple big/small files
We also tested for: 
    - absolute file paths
    - relative file paths
    - directories within directories 

To determine whether out program worked correctly we tested all these scnearios and
we also used valgrind to check for any memory leaks and errors (e.g. any infinite
loops, segmentation faults, etc). 
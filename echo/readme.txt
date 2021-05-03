Project 3 - CS214

Authors: Yash Patel(yp315),Neil Patel(ndp91)

To determine to test if our program is correct, we tested with different port numbers and tested to see if our server program 
opens up correctly with the connection. 

We tested if our key-value data structure was working properly by adding in different keys with set values and then testing with the 
GET function to receive those values from the keys and testing our DEL function to delete the key from the storage. 

The data structure we used to store all the values was a BST (it was easier to store in alphabetical order). 

Some of the edge cases we tested: 
    - If user tries to GET an unknwon key, we return an key-not-found error
    - If user tries to DEL an unknown key, we also return a key-not-found error
    - Error messages (e.x BAD, LEN), server closes when any errors are reached in error cases
    - If first byte in a message is X, report error immediately
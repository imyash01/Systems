#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>

typedef struct node_bst
{
    char* value;
    char* key;
    struct node_bst *left;
    struct node_bst *right;  
} node_bst;

typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

#define BACKLOG 5

int running = 1;

// the argument we will pass to the connection-handler threads
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
    node_bst* root;
};

#ifndef DEBUG
#define DEBUG 0
#endif

int server(char *port);
void *echo(void *arg);
node_bst* toAdd(char* value, char* key, node_bst* root);
node_bst * makeNode(char* value, char* key);
node_bst* findWord(node_bst * root, char* key);
int sb_init(strbuf_t *L, size_t length);
void sb_destroy(strbuf_t *L);
int sb_append(strbuf_t *L, char letter);
int sb_concat(strbuf_t* list, char* str);
node_bst* minValueNode(node_bst* node);
node_bst* deleteNode(node_bst* root, char* key);

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    (void) server(argv[1]);
    return EXIT_SUCCESS;
}

void handler(int signal)
{
	running = 0;
}


int server(char *port)
{
    struct addrinfo hint, *info_list, *info;
    struct connection *con;
    int error, sfd;
    pthread_t tid;
    node_bst* root = NULL;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    	// setting AI_PASSIVE means that we want to create a listening socket

    // get socket and address info for listening port
    // - for a listening socket, give NULL as the host name (because the socket is on
    //   the local host)
    error = getaddrinfo(NULL, port, &hint, &info_list);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        // if we couldn't create the socket, try the next method
        if (sfd == -1) {
            continue;
        }

        // if we were able to create the socket, try to set it up for
        // incoming connections;
        // 
        // note that this requires two steps:
        // - bind associates the socket with the specified port on the local host
        // - listen sets up a queue for incoming connections and allows us to use accept
        if ((bind(sfd, info->ai_addr, info->ai_addrlen) == 0) &&
            (listen(sfd, BACKLOG) == 0)) {
            break;
        }

        // unable to set it up, so try the next method
        close(sfd);
    }

    if (info == NULL) {
        // we reached the end of result without successfuly binding a socket
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(info_list);

	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	
	sigset_t mask;
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);


    // at this point sfd is bound and listening
    printf("Waiting for connection\n");
	while (running) {
    	// create argument struct for child thread
		con = malloc(sizeof(struct connection));
        con->addr_len = sizeof(struct sockaddr_storage);
        	// addr_len is a read/write parameter to accept
        	// we set the initial value, saying how much space is available
        	// after the call to accept, this field will contain the actual address length
        
        // wait for an incoming connection
        con->fd = accept(sfd, (struct sockaddr *) &con->addr, &con->addr_len);
        	// we provide
        	// sfd - the listening socket
        	// &con->addr - a location to write the address of the remote host
        	// &con->addr_len - a location to write the length of the address
        	//
        	// accept will block until a remote host tries to connect
        	// it returns a new socket that can be used to communicate with the remote
        	// host, and writes the address of the remote hist into the provided location
        
        // if we got back -1, it means something went wrong
        if (con->fd == -1) {
            perror("accept");
            continue;
        }
        
        con->root = root;
        // temporarily block SIGINT (child will inherit mask)
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	abort();
        }

		// spin off a worker thread to handle the remote connection
        error = pthread_create(&tid, NULL, echo, con);

		// if we couldn't spin off the thread, clean up and wait for another connection
        if (error != 0) {
            fprintf(stderr, "Unable to create thread: %d\n", error);
            close(con->fd);
            free(con);
            continue;
        }

		// otherwise, detach the thread and wait for the next connection request
        pthread_detach(tid);

        // unblock SIGINT
        error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	abort();
        }

    }

	puts("No longer listening.");
	pthread_detach(pthread_self());
	pthread_exit(NULL);

    // never reach here
    return 0;
}

#define BUFSIZE 8

void *echo(void *arg)
{
    char host[100], port[10], buf[BUFSIZE + 1];
    struct connection *c = (struct connection *) arg;
    int error, nread;

	// find out the name and port of the remote host
    error = getnameinfo((struct sockaddr *) &c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);
    	// we provide:
    	// the address and its length
    	// a buffer to write the host name, and its length
    	// a buffer to write the port (as a string), and its length
    	// flags, in this case saying that we want the port as a number, not a service name
    if (error != 0) {
        fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        close(c->fd);
        return NULL;
    }

    strbuf_t code;
    sb_init(&code,16);
    strbuf_t sKey;
    sb_init(&sKey,16);
    int part = 0;
    int bytes = 0;
    char command = '\0';
    //printf("[%s:%s] connection\n", host, port);

    while ((nread = read(c->fd, buf, BUFSIZE)) > 0) {
        for(int i = 0; i < nread; i++){
            if(buf[i] != '\n'){
                sb_append(&code,buf[i]);
            }
            else{
                part++;
                if(part == 1){
                    if(code.data[0] == 'S' && code.data[1] == 'E' && code.data[2] == 'T'){
                        command = 's';
                    }
                    else if(code.data[0] == 'G' && code.data[1] == 'E' && code.data[2] == 'T'){
                        command = 'g';
                    }
                    else if(code.data[0] == 'D' && code.data[1] == 'E' && code.data[2] == 'L'){
                        command = 'd';
                    }
                    else{
                        //error
                    }
                    sb_destroy(&code);
                    sb_init(&code,16);
                }
                else if(part == 2){
                    bytes = atoi(code.data);
                    if(bytes == 0){
                        //error
                    }
                    sb_destroy(&code);
                    sb_init(&code,16);
                }
                else if(part == 3){
                    if(command == 'g'){
                        node_bst* temp;
                        temp = findWord(c->root,code.data);
                        printf("%s\n", temp->value);
                    }
                    else if(command == 'd'){
                        node_bst* temp;
                        temp = findWord(c->root,code.data);
                        printf("%s\n", temp->value);
                        deleteNode(c->root,code.data);
                    }
                    else if(command == 's'){
                        sb_concat(&sKey,code.data);
                    }
                    else{
                        //error
                    }
                    sb_destroy(&code);
                    sb_init(&code,16);
                }
                else if(part == 4){
                    if(command == 's'){
                        c->root = toAdd(code.data,sKey.data,c->root);
                        printf("OKS\n");
                    }
                    else{
                        //error
                    }
                    sb_destroy(&code);
                    sb_init(&code,16);
                    sb_destroy(&sKey);
                    sb_init(&sKey,16);
                    part = 0;
                }
            }
        }
        
        
    }
    

    close(c->fd);
    free(c);
    return NULL;
}

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

node_bst* toAdd(char* value, char* key, node_bst* root){
    //mutex
    if(root ==  NULL){
        return makeNode(value, key);
    }
    int comp = strcmp(key,root->key);
    if(comp == 0){
        // already set, check for errors, user is trying to set key again
    }
    else if(comp < 0){
        root->left = toAdd(value, key, root->left);
    }
    else{
        root->right = toAdd(value, key, root->right);
    }
    return root;
    //mutex
}

node_bst * makeNode(char* value, char* key){
    node_bst *newNode = malloc(sizeof(node_bst));
    char * temp = malloc(strlen(value) + 1);
    char* temp1 = malloc(strlen(key) + 1);
    if(newNode == NULL || temp == NULL || temp1 == NULL){
        write(2, "malloc failed\n",15);
        exit(1);
    }
    strcpy(temp, value);
    strcpy(temp1, key);
    newNode->value = temp;
    newNode->key = temp1;
    newNode->left = NULL;
    newNode->right = NULL;


    return newNode;
}

node_bst* findWord(node_bst* root, char* key) {
    //mutex
    if(root == NULL || strcmp(key, root->key) == 0) { //return NULL
        return root;
    }
    else if(strcmp(key, root->key) < 0) { //if word we are searching for is less than current, search left
        findWord(root->left, key);
    }
    else {
        findWord(root->right, key); //if word we are searching is greater, search right
    }
    //mutex
}

node_bst* minValueNode(node_bst* node)
{
    node_bst* current = node;
 
    /* loop down to find the leftmost leaf */
    while (current && current->left != NULL)
        current = current->left;
 
    return current;
}

node_bst* deleteNode(node_bst* root, char* key)
{
    // base case
    if (root == NULL) // return error code???
        return root;
 
    int comp = strcmp(key,root->key);

    if (comp < 0)
        root->left = deleteNode(root->left, key);
 
    else if (comp > 0)
        root->right = deleteNode(root->right, key);
 
    else {
        // node with only one child or no child
        if (root->left == NULL) {
            node_bst* temp = root->right;
            free(root);
            return temp;
        }
        else if (root->right == NULL) {
            node_bst* temp = root->left;
            free(root);
            return temp;
        }
 
        node_bst* temp = minValueNode(root->right);
        root->key = temp->key;
 
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}
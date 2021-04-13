#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define SIZE 256

typedef struct node
{
    int occurences;
    int frequency;
    char* word;
    struct node *left;
    struct node *right;  
} node;
typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

node * makeNode(char* word){
    node *newNode = malloc(sizeof(node));
    newNode->word = word;
    newNode->left = NULL;
    newNode->right = NULL;

    return newNode;
}

node* toAdd(char* word, node* root){
    if(root ==  NULL){
        return makeNode(word);
    }
    int comp = strcmp(word,root->word);
    if(comp == 0){
        root->occurences ++;
    }
    if(comp < 0){
        root->left = toAdd(word, root->left);
    }
    else{
        root->right = toAdd(word, root->right);
    }
    return root;
}

void toPrint(node * root){
    if(root == NULL){
        return;
    }
    toPrint(root->left);
    printf("%s\t", root->word);
    toPrint(root->right);
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

int tokenize(char* filePath) {
   
    FILE *fp = fopen(filePath, "r");

    char temp;
    node *root = NULL;
    strbuf_t word;
    sb_init(&word, 32);
    while((temp = fgetc(fp)) != EOF ) {
        //strbuf_t word;
    // sb_init(&word, 32);
        //node *root = NULL;
        if(isalpha(temp) == 0) {
            continue;
        }
        if(!isspace(temp) && word.used != 1) {
            sb_append(&word, temp);

        } 
        else {
            char *temp2 = malloc(sizeof(char) * word.used);
            strcpy(temp2, word.data);
            sb_destroy(&word);
            sb_init(&word, 32);
            root = toAdd(temp2, root);
        }
        
    }
    toPrint(root);
    return 0;
}

int main(int argc, char* argv[]){
    tokenize("file1.txt");

}
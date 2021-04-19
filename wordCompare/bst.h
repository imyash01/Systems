#ifndef BST
#define BST

typedef struct node_bst
{
    int occurences;
    double frequency;
    char* word;
    struct node_bst *left;
    struct node_bst *right;  
} node_bst;

typedef struct parent_node
{
    int totalWords;
    node_bst* child_root;
}parent_node;

node_bst * makeNode(char*);
parent_node* makeParent();
node_bst* toAdd(char*, node_bst*);
void toFreq(node_bst *, int);
void toPrint(node_bst*);
void toFree(node_bst *);
parent_node* tokenize(char*);
node_bst* findWord(node_bst *, char *);
void getMeanFreqHelper(node_bst*, node_bst*, parent_node*);
parent_node* getMeanFreq(parent_node*, parent_node*);
double getKLD(node_bst*, node_bst*);
double getJSD(node_bst*, node_bst*, node_bst*);

#endif
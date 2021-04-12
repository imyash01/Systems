#include <stdlib.h>
#include <stdio.h>
#include "stringbuffer.h"

#ifndef DEBUG
#define DEBUG 0
#endif

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


int sb_remove(strbuf_t *L, char *letter)
{
    if (L->used <= 1) return 1;

    --L->used;

    if (letter) *letter = L->data[L->used-1];

    L->data[L->used-1] = '\0';

    return 1;
}

int sb_insert(strbuf_t *list, int index, char letter){
    if(index >= list-> length - 1 || list -> used == list ->length){
        size_t newLength = 2 * list-> length;
        if(newLength <= index){
            newLength = index + 2;
        }
        char *p = realloc(list->data, sizeof(char) * newLength);
        if(!p) return 1;

        list ->data = p;
        list ->length = newLength;
    }

    char prev = letter;
    char current = list->data[index];
    for(int i = index; i < list ->length; i++){
        list->data[i] = prev;
        prev = current;
        current = list->data[i+1]; //dont need to worry about array outofbounds because the if before will increase the size if index <= length
    }
    if(list->used <= index){
        list->used = index + 2;
    }
    else{
        list->used = list -> used + 1;
    }
    list->data[list->used] = '\0';
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
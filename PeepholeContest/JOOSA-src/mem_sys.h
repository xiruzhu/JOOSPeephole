#ifndef __GoLiteCompiler__MEMSYS__H__
#define __GoLiteCompiler__MEMSYS__H__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAX_SIZE 32767

struct memory_node ** table;

typedef struct memory_node{
    void * ptr;
    struct memory_node * next;
}mem_node;


size_t simple_hash(void * ptr){
        size_t shift = (size_t)log2(1 + sizeof(ptr));
        return ((size_t)(ptr) >> shift)%MAX_SIZE;
}

void init_table(){
    table = malloc(MAX_SIZE * sizeof(mem_node));
    if(!table){
        printf("Could not allocate memory!");
        exit(-1);
    }
    for(int i = 0; i < MAX_SIZE; i++){
        table[i] = NULL;
    }
}

void free_table(){
    if(table != NULL){
    for(int i = 0; i < MAX_SIZE; i++){
        for(mem_node * j = table[i]; j != NULL;){
            if(j != NULL){
            void * next = j->next;
            free(j->ptr);
            free(j);
            j = next;
            }
        }
    }
    }
    free(table);
}

/*
* -----------------------------------------------------------------------------------------------------------
* Malloc Function
* -----------------------------------------------------------------------------------------------------------
*/

static inline void *alloc(int number, int size)
{
    void *AllocMem = calloc(number, size);
    /* Some implementations return null on a 0 length alloc,
     * we may as well allow this as it increases compatibility
     * with very few side effects */
    if(!AllocMem && size * number)
    {
        printf("Could not allocate memory!");
        exit(-1);
    }

    for(mem_node * i = table[simple_hash(AllocMem)];; i = i->next){
        if(i == NULL){
           mem_node * node = malloc(sizeof(mem_node));
           node->ptr = AllocMem;
           node->next = NULL;
           break;
        }
    }

    return AllocMem;
}

static inline void *ralloc(void * pointer, int size)
{
    void *AllocMem = realloc(pointer, size);
    /* Some implementations return null on a 0 length alloc,
     * we may as well allow this as it increases compatibility
     * with very few side effects */
    if(!AllocMem && size)
    {
        printf("Could not allocate memory!");
        exit(-1);
    }
    for(mem_node * i = table[simple_hash(AllocMem)]; i != NULL; i = i->next){
        if(i != NULL){
        if(i == pointer){
            pointer = AllocMem;
        }
        }
    }

    return AllocMem;
}


#endif
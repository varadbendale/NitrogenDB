#define no_of_data 340 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


typedef struct leaf_node_data{
    int page_id ; 
    int file_offset ; 
    int slot_num  ; 
}leaf_node_data ; 

typedef struct internal_node{
    leaf_node_data data[no_of_data] ; 
    int num_of_data_pushed ; 
    int num_of_leaf ; 
    int primary_key[no_of_data] ; 
    int root_compare ; 
    struct internal_node *children[no_of_data + 1 ] ; 
    int num_of_children ; 
    struct internal_node *next ; 
    bool leaf ; 
}internal_node ; 


void tree_func_for_insert(internal_node * node, int pk, char * data, int page_id, int offset, int slot_num);





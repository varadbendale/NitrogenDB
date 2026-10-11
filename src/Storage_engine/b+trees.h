#define no_of_data 340 

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






#include<strings.h>
#define PAGE_SIZE 4096
enum { 
    int_num = 1 , 
    blob_num , 
    string_num , 
    null , 
    integer_op   , 
    begin_op , 
    next_op , 
    print_op , 
    halt  , 
    blob_op , 
    string_op , 
    null_op , 
    real_op ,
    real_num , 
    add_op , 
    subs_op , 
    mul_op , 
    divide_op , 
    eq_op , 
    ne_op , 
    gt_op
    lt_op , 
    le_op , 
    ge_op , 
    end_num , 
    cursor_read , 
    boolean_op , 
    cursor_write  , 
    open_write_op , 
    open_read_op
}

typedef struct data_bin{
    union{
        long i ; 
        char *s ; 
        float r ; 
    }compare ; 
    union{
        long i ; 
        char *s ; 
        float r ; 
    }primary_key_first ; 
    union{
        long i ; 
        char *s ; 
        float r ; 
    }primary_key_second ; 
}

typedef struct bins{
    data_bin data[300] ; 
    int row_number ; 
    char * tbl ; 
    int num_of_entires ; 
}

typedef struct hash_bins_str{
    bins bn[300] ;
    int num_of_bins ; 
}

typedef enum collation { 
    BINARY = 1 ,
    NOCASE,
    RTRIM 
} ;

typedef enum direction { 
    ASC = 1 ,
    DESC 
} ;

typedef struct Pager{
    FILE *file ; 
}

typedef struct type{
    int operand ; 
    int p1 ; 
    int p2 ; 
    int p3 
    void * p4 ; 
    int p4_type ; 
} ; 


typedef struct reg{
    int type ; 
    int lenght ; 
    union{
        long i ; 
        char *s ; 
        float r ; 
    }val ; 
}

typedef struct path_entry{
    uint32_t page_num     ; 
    uint16_t  cell_index     ; 
}

typedef struct leaf_node_data{
    int page_id ; 
    int file_offset ; 
    int slot_num  ; 
}leaf_node_data ; 

typedef struct bptree{
    table * Table ; 
    int  start_root_num ; 
    leaf_node_data data ; 
    internal_node tree ; 
    internal_node current_node ; 
    int current_number ; 
    bool start ; 
    int data_type ; 
    bool end ; 
    int total ; 
    int mode ; 
    bool null ; 
}

typedef struct aggregate{
    reg min ; 
    reg max ; 
    double total ; 
    int type ; 
    char *concat_string ; 
    int * string_len ; 
    bool have_value ; 
    int count ; 
}

typedef struct sort_arr{
    unsigned char * key ; 
    int key_type ; 
    unsigned char * array ; 
    int len ;  
}

typedef struct Key_info{
    int feild ; 
    collation * coll  ; 
    direction * dir  ; 
}

typedef struct sorter{
    int row_count ; 
    sort_arr * array ; 
    int capacity ;
    int keycols;  
    int cursor ; 
    int  cols_to_look ;  
    Key_info * keyinfo ; 
}


typedef struct byte{
    type prog[300] ; 
    int prog_count ; 
    int ins_count ; 
    reg regis[300] ; 
    int reg_start ; 
    int reg_end ; 
    int reg_mode ; 
    hash_bins_str * hash_bins ; 
    aggregate agg[300] ; 
    bptree btr[300] ; 
    table *db ; 
    Pager *pager ; 
    sorter * sort ; 
}

typedef struct page_header{
    uint8_t page_type ; 
    uint16_t free_space ; 
    uint16_t num_cells  ; 
}__attribute__((packed)) 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
struct Tree_def ;   
typedef struct Create_tree_def {
    char * comp ; 
    int type ; 
    int type_bits_count ; 
    int first_decimal ; 
    int second_decimal ; 
    int  primary_key_counter  ; 
    char ** primary_key ; 
    bool not_null ; 
    char *  default_val ; 
    bool unique ; 
    int  foreign_key_counter ; 
    char ** foreign_key ; 
    int references ; 
    int line;                 
    int col;
    struct Create_tree_def** children;   
    int num ; 
    tree * as ; 
}ctree



typedef struct  insert_parser_tree{
    char *comp;
    char *table;
    char **table_cols;
    char **insert_values;
    struct insert_parser_tree **children;   
    int num;
    int table_col_num;
    int insert_val_num;
    int ignore;
    int replace  ;
} itree 



enum type_of_cols {
    INT  = 50 ,    
    BIGINT ,  
    SMALLINT,   
    TINYINT,     
    FLOAT,      
    DOUBLE ,     
    CHAR ,  
    VARCHAR, 
    BOOL ,
    DATE ,
    TIMESTAMP 
} ;

typedef struct Tree_def {
    char* comp; 
    struct Tree_def** children;   
    char* as  ; 
    char **direction  ; 
    int line;                 
    int col;
    int num  ; 
} Tree_def;

typedef Tree_def tree  ; 

typedef struct delete_parser_tree {
    char * comp ;
    char * as ;
    struct delete_parser_tree ** children ;
    tree * sub ;
    int num ;
    int line ;
    int col ;
} dtree ;

dtree* createNodeDelete( char* comp ) ;
dtree* delete_parser() ;



Tree_def* make_leaf(char* value , int row , int col ) {
    Tree_def* n = malloc(sizeof(Tree_def));
    if (n == NULL) {
        return NULL;   
    }
    if ( value != NULL ){
      n->comp = strdup(value);
    }
    n->as = NULL;
    n->children[300] = NULL;
    n->direction = NULL ; 
    n->line = row ; 
    n->col = col;
    return n;
}

bool if_function( char* word ) ; 
bool if_sql_syntax( char* word ) ; 
int priority(char *string) ; 
tree* expression(char ***buf , tree * node , int check , int i , int j  , int end_row , int end_col  ) ; 
tree *function( char ***buf , tree * node ,int check  ,  int i , int j  , int end_row , int end_col ) ;
tree *case_expr(char ***buf , int check ,  int i , int j , int end_row , int end_col ) ; 
tree *comp_1(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col ) ; 
tree *comp_2(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col ) ; 
tree *comp_3(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col  , int pain  , int having ) ; 
tree * select_query( int row , int col , int check , int end_row , int end_col , int pain) ; 



ctree* createNode( char* comp)  ; 
bool validate_default(char *default_val, int type)  ; 
ctree* create_parser() ; 


insert_parser_tree *createNodeInsert(char *comp)  ; 
insert_parser_tree* expression_insert(char ***buf, insert_parser_tree *node,  int i, int j, int end_row, int end_col) ; 
itree * insert_parser() ; 


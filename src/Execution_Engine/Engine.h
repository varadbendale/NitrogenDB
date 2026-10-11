#ifndef Engine_h
#define Engine_h

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX 300

typedef struct compiler compiler;
typedef struct plan plan;
typedef struct comp comp;
typedef struct select_select_info select_select_info;
typedef struct select_join_info select_join_info;
typedef struct select_from_info select_from_info;
typedef struct extra_info_ob extra_info_ob;
typedef struct select_ob_info select_ob_info;
typedef struct table_range table_range;
typedef struct table_and_col_hash_info table_and_col_hash_info;
typedef struct join_extra_info join_extra_info;
typedef struct select_info select_info;
typedef struct sql_master sql_master;
typedef struct col_det col_det;
typedef struct table table;
typedef struct tables_list tables_list;
typedef struct orderby_key_cols orderby_key_cols;
typedef struct temp_info_for_path  temp_info_for_path;
typedef struct final_finished_equation final_finished_equation;
typedef struct type type;

typedef enum column_types {
    expression = 1,
    column_name,
    aggregate_func,
    normal
} column_types;

typedef struct type {
    int operand;
    int p1;
    int p2;
    int p3;
    void *p4;
} type;

typedef struct instruction {
    compiler *c;
    int op_code;
    int p1;
    int p2;
    int p3;
    void *p4;
} instruction;

typedef struct plan {
    bool where;
    bool orderby;
    bool groupby;
    bool join;
    bool from;
    bool select;
    bool having;
} plan;

typedef struct comp {
    plan pl;
    int num_of_stuff;
    int current_num;
} comp;

typedef struct select_select_info {
    column_types col_tp;
    char *col_name;
    char *extra_col;
    char *operator;
    select_select_info *left;
    select_select_info *right;
    char *as;
    float float_val;
    unsigned char *blob;
    int num_value;
    char *char_value;
    int acc_reg;
} select_select_info;

typedef struct select_join_info {
    char *table_or_col_name;
    float *float_val;
    unsigned char *blob;
    int *num_value;
    char *char_value;
    char *operator;
    char *as;
    char *extra_table_or_col_name;
    select_join_info *left;
    select_join_info *right;
    select_select_info *on;
} select_join_info;

typedef struct select_from_info {
    char *table_name;
    select_join_info *join;
    char *as;
} select_from_info;

typedef struct extra_info_ob {
    char *nulls;
    char *order;
} extra_info_ob;

typedef struct select_ob_info {
    char *ob_name;
    char *extra_ob_name;
    char *dir;
    extra_info_ob *ex;
    char *as;
    char *operator;
    select_ob_info *left;
    select_ob_info *right;
    float *float_val;
    unsigned char *blob;
    int *num_value;
    char *char_value;
    int acc_reg;
} select_ob_info;

typedef struct table_range {
    int *range;
    int table_name;
    int col_name;
} table_range;

typedef struct table_and_col_hash_info {
    int *table_num;
    int *column_used[300];
    int col_conter;
    table_range *range[300];
    int range_counter;
} table_and_col_hash_info;

typedef struct join_extra_info {
    table_and_col_hash_info *join_select_unique_table[300];
    table_and_col_hash_info *join_select_hash_unique_table[300];
    int *tables_occuring_number_of_times[300];
    int join_table_counter;
    int join_hash_counter;
} join_extra_info;

typedef struct select_info {
    select_select_info *sel[300];
    int col_counter;
    bool select_agg;
    select_from_info *from[300];
    int tables_counter;
    tables_list *tab;
    select_from_info *where;
    select_select_info *groupby[300];
    int groupby_counter;
    int *gb_select_unique[300];
    int *hash[300];
    int sel_uni_counter;
    select_from_info *having;
    select_ob_info *orderby[300];
    int orderby_counter;
    join_extra_info *join[300];
    int join_counter;
} select_info;

typedef struct sql_master {
    uint32_t root_page_num;
} sql_master;

typedef struct col_det {
    char *name;
    int data_type;
} col_det;

typedef struct table {
    char *name;
    int num_of_columns;
    col_det * col[300];
    int root_page_num;
} table;

typedef struct tables_list {
    table *tables[300];
    int num_of_tables;
} tables_list;

typedef struct compiler {
    type *typ;
    int count;
    int capacity;
    int register_counter;
    int sorter_cursor;
    Tree_def * tree ; 
    int register_start;
    tables_list *tl;
    int cursor_num;
    select_info *select;
} compiler;



typedef struct orderby_key_cols {
    int reg_num;
    int direction;
    int nullsfirst;
    int nullslast;
} orderby_key_cols;


typedef struct temp_info_for_path {
    int path[300];
    int counter;
} temp_info_for_path;

typedef struct final_finished_equation {
    int *range;
    char *operator;
    int target_table;
    int target_column;
} final_finished_equation;


void  emit(compiler *c, int op_code, int p1, int p2, int p3, void *p4);
int is_blob_literal(char *temp);
int data_type_check(char *str);
int string_length(char *table_name);
char *operand_thing(char *name);
char *table_thing(char *name);

int col_name_to_int(char *column_name, table *t);
int table_num(tables_list *tab, char *table_name);
char  *table_name_from_num(tables_list *tab, int num);
table *lookup_table(tables_list *tab, char *table_name);
int primary_key_offset(dab *db, int table_num);


int col_name_to_int_main(char *column_name, select_info *sf);   

select_select_info *expre(select_select_info *ans, compiler *c, tree *temp);
select_join_info *join_expre(select_join_info *ans, compiler *c, tree *temp);
select_ob_info *expre_order_by(select_ob_info *ans, compiler *c, tree *temp);

int get_the_join_cursor(compiler *c, char *operator);
unsigned char *sorter_orderby_init(compiler *c, int groupby);
char *blob_of_data(compiler *c, int pk_1, int pk_2, int row_num, char *table_name);
void get_the_tree_hash(select_select_info *temp, select_info *sf);
void get_the_data_tree(select_select_info *temp, select_info *sf);
void get_all_hash_covered_gb(compiler *c);
void *just_for_extra_safety(compiler *c);
void from_parser_to_struct(compiler *c, tree *select);
void groupby_parser_to_struct(compiler *c , tree * select ) ; 
void where_parser_to_struct(compiler *c, tree *select);
void select_parser_to_struct(compiler *c, tree *select);
void *tables_and_their_hash_cursor_num(compiler *c, select_select_info *from);
void get_all_select_stuff(compiler *c);
int tables_and_thier_cursor_num(compiler *c, select_select_info *from);
void check_if_the_stuff_negation_or_not_without_solver(compiler *c, int *ans);
int  find_the_path_of_the_stuff_main(temp_info_for_path *tp, compiler *c, int target, select_select_info *tree);
void find_the_path_of_the_stuff(temp_info_for_path *tp, compiler *c, int target, select_select_info *tree);
int *take_care_of_expression(compiler *c, int target, select_select_info *from, int reg_1, int reg_2, int *done, int *counts);
void check_if_the_stuff_negation_or_not(compiler *c, int target, select_select_info *tree);
void *get_the_max_and_min(compiler *c);
final_finished_equation *found_the_target_to_be_deplished_solve_it( compiler *c, int target, int side, select_select_info *tree, int *left_ans);
void aggregate_select(compiler *c, select_select_info *node);
int func(compiler *c, select_select_info *node, bool final);
int group_by_func(compiler *c, select_select_info *node, bool final);
int orderby_func(compiler *c, select_ob_info *node, bool final);
int where_func(compiler *c, select_select_info *node, bool req);
int normal_func(compiler *c, select_select_info *node, bool req);
int orderby_func_main(compiler *c, select_ob_info *node);
int groupby_func(compiler *c, select_select_info *temp, bool final);
int join_func_main(compiler *c, select_select_info *node);
int join_func(compiler *c, select_select_info *node);
void sort_groupby(compiler *c);
int  check_the_stuff(compiler *c, select_from_info *from_org, char *table);
void join_equal_clause(compiler *c, select_select_info *from, select_from_info *from_org);
void *join_inequality_clause(compiler *c, select_select_info *from, select_from_info *from_org);
void compile_select(compiler *c);

#endif 
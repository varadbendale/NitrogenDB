#ifndef TYPES_H
#define TYPES_H
#define DB_FILE "nitrogen.db"

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <time.h>

#include "Parser.h"

#define integer_num "I"
#define real_num "R"
#define toast_num "T"
#define string_num "S"
#define date_num "D"
#define timestamp_num "M"

#define data_filename "normal_file"
#define toast_header_filename "toast_header"
#define toast_map_filename "toast_map"
#define toast_filename "toast_data"

extern int number_of_pages;
char *toast_tables_insert(char *data);

#define PAGE_SIZE 4096
#define INVALID_PAGE_ID 0xFFFFFFFFu

#define MAX_COLUMNS 16
#define MAX_TABLES 300

enum page_types {
    toast_page = 120,
    normal,
    hash
};

enum frame_request {
    frame_force = 1
};

#define use_page_size 2736
#define max_no_of_slots 100
#define data_size (PAGE_SIZE - sizeof(page_header_struct) - (sizeof(slots) * max_no_of_slots))

typedef struct {
    char col_name[64];
    uint8_t col_type;
    int32_t type_bits_count;
    int32_t first_decimal;
    int32_t second_decimal;
    bool primary_key;
    bool foreign_key;
    bool default_stuff;
    bool not_null;
    bool unique;
    char default_val[32];
    char foreign_key_table[64];
    char foreign_key_column[64];
} column_info;

typedef struct {
    char table_name[64];
    uint32_t num_columns;
    uint64_t root_page;
    column_info columns[MAX_COLUMNS];
} table_info;

typedef struct {
    uint32_t db_magic;
    uint16_t version;
    uint16_t page_size;
    uint32_t num_tables;
    table_info tables[MAX_TABLES];
} pagezero;

typedef struct {
    int hash[307];
    int num;
    int primary_key[307];
    int pri_num;
} info;

typedef struct acutal_values {
    int error_handling;
    char **info;
    itree *condition;
} acutal_values;

typedef struct info_detail {
    char *table;
    int row_num;
    acutal_values **info;
} info_detail;

typedef struct insert_page {
    int table_num;
    info_detail **info;
} insert_page;

typedef struct page_header_struct {
    uint32_t page_num;
    int page_type;
    int num;
    int toast_table_num;
    uint8_t dead_slots[128];
    int current_offset;
    int free_size;
    int normal_free_size;
    int buffer_space;
} page_header_struct;

typedef struct slots {
    int offset;
    int size;
    float pk;
} slots;

typedef struct page {
    page_header_struct header;
    slots slot[max_no_of_slots];
    char data[data_size];
} page;

_Static_assert(sizeof(page) == PAGE_SIZE, "page must be exactly PAGE_SIZE bytes");
_Static_assert(use_page_size <= data_size, "use_page_size larger than page.data");

typedef struct map_out_toast {
    int chunk_id;
    int *offset_of_the_same;
    int offset_number;
} map_out_toast;

typedef struct toast_table_file_header {
    int pages_used;
    int total_num_of_chunks;
    uint8_t dead_slots[128];
    uint8_t dead_space_available[128];
    int dead_space;
    map_out_toast *map_out_toast;
    int num_of_lines_in_map;
} toast_table_file_header;

typedef struct toast {
    int chunk_id;
    int page;
    int slot_number;
} toast;

typedef struct toast_pointer {
    int chunk_id;
    int size;
} toast_pointer;

#define HS_BUCKETS 1021

typedef struct hash_node {
    char *val;
    struct hash_node *next;
} hash_node;

typedef struct hash_bucket {
    char *table;
    int col;
    hash_node *bucket[HS_BUCKETS];
    struct hash_bucket *next;
} hash_bucket;

typedef struct hs_node {
    char *val;
    struct hs_node *next;
} hs_node;

typedef struct hs_col {
    char *table;
    int col;
    hs_node *bucket[HS_BUCKETS];
    struct hs_col *next;
} hs_col;

#define frames_in_buf 1024
#define max_pages 65536

#define no_of_data 340

typedef struct leaf_node_data {
    int page_id;
    int file_offset;
    int slot_num;
} leaf_node_data;

typedef struct internal_node {
    leaf_node_data data[no_of_data];
    int num_of_data_pushed;
    int num_of_leaf;
    int primary_key[no_of_data];
    int root_compare;
    struct internal_node *children[no_of_data + 1];
    int num_of_children;
    struct internal_node *next;
    struct internal_node *prev;
    bool leaf;
} internal_node;

void default_column_info(column_info *col);
void default_table_info(table_info *ti);
void default_pagezero(pagezero *pg);

int hash_num(char *str);

void prepared_the_intel_hash(info *intel);
int take_care_of_stuff(ctree *tree, info *intel);
void do_the_foreign_key(ctree *tree, table_info *ti);
void main_create_parser(pagezero *pg);

int db_open(const char *filename);
void db_write(int fd, int page_num, uint8_t *buf);
void db_read(int fd, int page_num, uint8_t *buf);
void write_pagezero(pagezero *pg, const char *filename);
void update_pagezero(pagezero *pg, const char *table) ; 
void read_pagezero(pagezero *pg, const char *table) ;
void db_close(int fd);
pagezero *get_pagezero(char *table) ; 
page *get_page_from_file(int offset);
page *get_page_from_named_file(const char *filename, long offset);
int FSM_insert(char *insert_data, int page_num, int offset, table_info *table);
char *truncate_left(char *str, size_t max_bytes);
char *to_decimal(const char *str, int total_digits, int decimal_digits);
uint8_t check_hash(const char *val, uint32_t col, const char *table);
void add_hash(const char *table, uint32_t col, const char *val);
void struct_for_insert(itree *tree);

extern hash_bucket *hash_starter;
extern hs_col *hs_head;

void start_the_wal(void);
int begin_transaction(void);
void end_transaction(int id);
void insert_wal(int transaction_id, int page_id, int offset, char *data);

void buffer_pool_init(void);
void buffer_pool_flush(void);
void init_page_counter(void);

internal_node *get_root(char *table);
void tree_func_for_insert(internal_node *node, int pk, char *data, int page_id, int offset, int slot_num);

#include "buffer.h"

#endif
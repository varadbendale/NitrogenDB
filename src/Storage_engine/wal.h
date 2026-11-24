#ifndef WAL_H
#include "storage_engine.h"
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define default_segment_size 16777216
#define max_size_for_data 4096

static uint32_t crc_table[256];

typedef struct {
    int page_id;
    int offset;
    int lenght;
    char inserted_data[max_size_for_data];
} insert_info;

typedef struct {
    int lsn;
    int lenght;
    int type;
    int prev;
    int transaction;
    int is_clr;
    long long prev_offset;
    int undo_next_lsn;        
    long long undo_next_offset; 
} wal_page_header;

typedef struct {
    int page_id;
    int offset;
    int lenght;
    char deleted_data[max_size_for_data];
} delete_info;

typedef struct {
    int page_id;
    int offset;
    int updated_lenght;
    int prev_lenght;
    char data[2 * max_size_for_data];
} update_info;

typedef struct {
    int transactions_number;
    int running_transaction[max_size_for_data];
    int dirty_page_number;
    int dirty_page_id[max_size_for_data];
    int dirty_page_slot[max_size_for_data];
} checkpoint_struct;

typedef struct {
    uint32_t crc;
    uint8_t type;
    wal_page_header header;
    char table_name[64];
    union {
        insert_info insert;
        update_info update;
        delete_info delete;
        checkpoint_struct checkpoint;
    } record_value;
} records_struct;

#define records_sturct_size offsetof(records_struct, record_value.insert.inserted_data)

typedef struct {
    int checkpoint_lsn;
    int checkpoint_segment;
    int checkpoint_offset;
    int redo_lsn;
    uint32_t crc;
} control_struct;

typedef struct {
    int segment_id;
    int status;
    int size;
    int records_offset;
    int first_lsn;
    int seq;
} segment_struct;

typedef struct {
    int segment_num;
    int transaction_num;
    int lsn_num;
    int num_of_run_transactions;
    int running_transaction[256];
    int num_of_offset;
    int offset[256];
    int current_segment;
    int redo_checkpoint;
    int seq;
    int checkpoint_running;
    int last_lsn_in_segment[256];
    int running_first_lsn[256];
    int running_last_lsn[256];
    int the_farthest_lsn_required;
    long long previous_offset[256];
    int dead_segments[max_size_for_data];
    frames *checkpoint_frames;
    int data_fd;
    int retrying ; 
} req_data;

extern req_data *dat; 
void checkpoint(void);
#endif
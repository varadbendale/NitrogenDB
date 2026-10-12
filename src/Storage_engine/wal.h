#include <sys/stat.h>
#include <errno.h>
#define default_segment_size 16777216
#define max_size_for_data 8192
#include <stdint.h>
#include <stddef.h>

static uint32_t crc_table[256];

typedef struct {
    int page_id ; 
    int offset ; 
    int lenght ;  
    char inserted_data[max_size_for_data] ; 
} insert_info ;

typedef struct {
    int lsn ; 
    int lenght ; 
    int type ; 
    int prev ; 
    int transaction ; 
} wal_page_header ; 

typedef struct {
    int page_id ; 
    int offset ; 
    int lenght ; 
    char deleted_data[max_size_for_data] ; 
} delete_info ; 

typedef struct {
    int page_id ; 
    int offset ; 
    int updated_lenght ; 
    char updated_data[max_size_for_data] ; 
    int prev_lenght ; 
    char prev_data[max_size_for_data] ; 
} update_info ; 

typedef struct {
    int transactions_number ; 
    int running_transaction[max_size_for_data] ; 
    int dirty_page_number ; 
    int dirty_page_id[max_size_for_data] ; 
} checkpoint_struct ; 


typedef struct {
    uint32_t crc;
    uint8_t type; // 1 for insert 2 for delete 3 for update 4 for abort 5 for commit 6 for checkpoint
    union {
        insert_info insert;
        update_info update;
        delete_info delete ;  
        checkpoint_struct checkpoint;
    } record_value ;
    wal_page_header header;  
} records_struct ;


typedef struct {
    int segment_id ; 
    int status ;  // 1 active 2 dead
    int size ; 
    int records_offset ; 
}segment_struct ; 


typedef struct {
    int segment_num ; 
    int transaction_num ; 
    int lsn_num ; 
    int num_of_run_transactions ; 
    int running_transaction[max_size_for_data] ; 
    int num_of_dead_segments ; 
    int dead_segments[max_size_for_data] ; 
}req_data ; 
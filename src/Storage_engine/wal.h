w#include <sys/stat.h>
#include <errno.h>
#define default_segment_size 16777216
#define max_size_for_data 4096
#define records_sturct_size 40 
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
    int prev_offset ; 
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
    int prev_lenght ; 
    char data[2 * max_size_for_data] ; 
} update_info ; 

typedef struct {
    int transactions_number ; 
    int running_transaction[max_size_for_data] ; 
    int dirty_page_number ; 
    int dirty_page_id[max_size_for_data] ; 
} checkpoint_struct ; 


typedef struct {
    uint32_t crc ;
    uint8_t type ; // 1 for insert 2 for delete 3 for update 4 for abort 5 for commit 6 for checkpoint
    union {
        insert_info insert ;
        update_info update ;
        delete_info delete ;  
        checkpoint_struct checkpoint ;
    } record_value ;
    wal_page_header header;  
} records_struct ;

typedef struct {
    int checkpoint_lsn ;  
    int checkpoint_segment ; 
    int checkpoint_offset ; 
    int redo_lsn ;      
    uint32_t crc ;            
} control_struct ;


typedef struct {
    int segment_id ; 
    int status ;  // 1 active 2 dead
    int size ; 
    int records_offset ; 
    int first_lsn ; 
    int seq ; 
}segment_struct ; 


typedef struct {
    int segment_num ; 
    int transaction_num ; 
    int lsn_num ; 
    int num_of_run_transactions ; 
    int running_transaction[256] ; 
    int num_of_offset ; 
    int offset[256] ; 
    int current_segment ;     
    int redo_checkpoint ; 
    int seq ; 
    int checkpoint_running = 0 ;             
    int last_lsn_in_segment[256] ;   
    int running_first_lsn[256] ; 
    int running_last_lsn[256] ;
    int the_farthest_lsn_required ; 
    int previous_offset[256] ; 
    frames * checkpoint_frames ; 
    int data_fd  ;  
}req_data ; 

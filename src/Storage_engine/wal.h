typedef struct {
    int page_id ; 
    int offset ; 
    int lenght ; 
    int slot_num ; 
    int pk ; 
    char inserted_data[] ; 
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
    int slot_num ; 
    int lenght ; 
    int pk ; 
    char deleted_data[] ; 
} delete_info ; 

typedef struct {
    int page_id ; 
    int offset ; 
    int slot_num ; 
    int updated_lenght ; 
    int updated_pk ; 
    char updated_data[] ; 
    int prev_lenght ; 
    int prev_pk ; 
    char prev_data[] ; 
} update_info ; 

typedef struct {
    int running_transaction[] ; 
    int dirty_page_id[] ; 
} checkpoint_struct ; 



typedef struct {
    uint32_t crc;
    uint8_t type;
    union {
        insert_info insert;
        update_info update;
        delete_info delete ;  
        checkpoint_struct checkpoint;
    } record_value ;
    wal_page_header header;  
} records ;
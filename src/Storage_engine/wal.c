int wal_folder(const char *path) {
    if (mkdir(path , 0755) == 0){
        return 0 ;
    } 
    if (errno == EEXIST) {
        return 0 ; 
    }
    return -1 ;
}

req_data * dat  ; 

void crc_init(void) {                    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++)
            c = (c >> 1) ^ (0x82F63B78 & -(c & 1));
        crc_table[i] = c;
    }
}


int write_record(int fd, off_t offset, records_struct * record){
    ssize_t n = pwrite(fd, &record , sizeof record , offset);
    if (n == (ssize_t)sizeof record ){
        return 0;
    }
    return -1;
}


int write_header(int fd , segment_struct header){
    ssize_t n = pwrite(fd, &header, sizeof header, 0);
    if (n == (ssize_t)sizeof header)
        return 0;
    return -1;
}

int create_segment_file(int seg) {
    char name[64];
    snprintf(name, sizeof(name), "wal/wal_%08llu", (unsigned long long)seg);
    int fd = open(name, O_CREAT | O_RDWR, 0644);
    return fd;               
}

uint32_t crc32c(const char *data, size_t offset, size_t size) {
    const uint8_t *p = (const uint8_t *)data + offset;
    uint32_t crc = 0xFFFFFFFF;
    while (size > 0 ){
        crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
        size-- ; 
    }
    return ~crc;
}

void start_the_wal(){
    dat = malloc(sizeof(req_data)) ;
    memset(dat , 0 , sizeof(req_data)) ;
    dat->num_of_run_transactions = 0 ;
    dat->lsn_num = 0 ;
    dat->segment_num = 0 ;
    dat->transaction_num = 0 ;
    for (int i = 0 ; i < max_size_for_data ; i++ ){  
        dat->dead_segments[i] = -1 ;
    }
    wal_folder("wal") ; 
    crc_init() ; 
}

void default_segment(segment_struct  * seg_str  , int seg_num ){
    seg_str->segment_id = seg_num ; 
    seg_str->status = 0 ; 
    seg_str->size = sizeof(segment_struct) ; 
    seg_str->records_offset = 0 ; 
}

void default_record( records_struct *record  , int type   ){
    memset(record , 0 , sizeof(records_struct)) ; 
    record.crc = 0 ; 
    record.type = type  ; 
    record.header.lsn  = dat->lsn_num++ ; 
    record.header.lenght  = 0   ; 
    record.header.type = type ; 
    record.header.prev = -1   ; 
    record.header.is_clr = 0  ; 
    record.header.transaction  = dat->transaction_num ; 
    if (type == 1 ){
        record.record_value.insert.page_id = -1 ; 
        record.record_value.insert.offset = 0 ; 
        record.record_value.insert.lenght = 0 ; 
    }
    else if (type == 3 ){
        record.record_value.update.page_id = -1 ; 
        record.record_value.update.offset = 0 ; 
        record.record_value.update.updated_lenght = 0 ; 
        record.record_value.update.prev_lenght = 0 ; 
    }
    else if (type == 2 ) { 
        record.record_value.delete.page_id = -1 ; 
        record.record_value.delete.offset = 0 ; 
        record.record_value.delete.lenght = 0 ; 
    }
    else if ( type == 6 ) { 
        record.record_value.checkpoint.transactions_number = 0 ; 
        record.record_value.checkpoint.dirty_page_number = 0 ; 
    }
    return record ; 
}

// void insert_wal(int transaction_id , int page_id , int offset , char *data ){
//     int len = strlen(data)  ;
//     int data_putted  = 0 ;  
//     records_struct record ; 
//     int declration = 2 ;  
//     int slot = -1 ;
//     for (int i = 0 ; i < 256 ; i++){
//         if (dat->running_transaction[i] == transaction_id){
//             slot = i ;
//             break ;
//         }
//     }
//     if (slot == -1){
//         return ;
//     }
//     int prev_lsn = dat->running_last_lsn[slot] ; 
//     int record_size = records_sturct_size + max_size_for_data ; 
//     while ( len > 0 ){
//         int seg_num = dat->current_segment ; 
//         int file_num ; 

//         file_num = create_segment_file(seg_num) ; 
//         if (file_num == -1  ){
//             return  ; 
//         }
//         segment_struct seg_str ;
//         int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
//         if (size_of_the_thing != sizeof(seg_str) ){
//             default_segment(&seg_str , seg_num ) ; 
//         }
        
//         if (default_segment_size - seg_str.size < record_size){
//             close(file_num) ;
//             int next_seg_num = (seg_num + 1) % 256 ;
//             if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
//                 checkpoint() ;
//                 return ;            
//             }
//             dat->current_segment = next_seg_num ; 
//             seg_num = next_seg_num  ;
//             file_num = create_segment_file(seg_num) ;
//             if (file_num == -1){
//                 return ;
//             }
//             default_segment(&seg_str , seg_num) ; 
//             dat->seq++ ; 
//             seg_str.seq = dat->seq ; 
//             seg_str.first_lsn = dat->lsn_num ; 
//             seg_str.records_offset = 0 ;
//             dat->offset[seg_num] = seg_str.size ;
//         }
//         seg_str.status = 1 ; 
//         int space = default_segment_size - seg_str.size ;
//         int num = (space / record_size) * max_size_for_data ; 
//         if (len >= num ){
//             len = len - num ; 
//             while ( num > 0 ){
//                 default_record( &record , 1  , dat ) ;
//                 record.header.transaction = transaction_id ;
//                 if (num >= max_size_for_data){
//                     int data_we_put = records_sturct_size + max_size_for_data ;
//                     memcpy(record.insert.inserted_data , data + data_putted , max_size_for_data );
//                     data_putted = data_putted + max_size_for_data ; 
//                     record.header.lenght = max_size_for_data ; 
//                     record.insert.offset = offset ; 
//                     record.insert.page_id = page_id ; 
//                     record.insert.lenght = max_size_for_data ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     prev_lsn = record.header.lsn ; 
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     num  = num - max_size_for_data ;
//                 }
//                 else { 
//                     int data_we_put = records_sturct_size + num ;
//                     memcpy(record.insert.inserted_data , data + data_putted , num );
//                     data_putted = data_putted + num ;  
//                     record.header.lenght = num ; 
//                     record.insert.offset = offset ; 
//                     record.insert.page_id = page_id ; 
//                     record.insert.lenght = num ;  
//                     record.header.prev = prev_lsn ;
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     prev_lsn = record.header.lsn ; 
//                     seg_str.size = seg_str.size + data_we_put  ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     num  = 0  ;
//                 }
//             }
//             seg_str.records_offset = seg_str.records_offset + seg_str.size ; 
//             declration = write_header(file_num , seg_str ) ; 
//             if ( declration == -1 ){
//                 close(file_num) ;   
//                 return  ; 
//             }
//         }
//         else {  
//             while ( len > 0 ){
//                 default_record( &record ,  1  , dat ) ; 
//                 record.header.transaction = transaction_id ;  
//                 if (len >= max_size_for_data){
//                     int data_we_put = records_sturct_size + max_size_for_data ;
//                     memcpy(record.insert.inserted_data , data + data_putted , max_size_for_data );
//                     data_putted = data_putted + max_size_for_data ; 
//                     record.header.lenght = max_size_for_data ; 
//                     record.insert.offset = offset ; 
//                     record.insert.page_id = page_id ; 
//                     record.insert.lenght = max_size_for_data ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     prev_lsn = record.header.lsn ; 
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     len  = len - max_size_for_data ;
//                 }
//                 else { 
//                     int data_we_put = records_sturct_size + len ; 
//                     memcpy(record.insert.inserted_data , data + data_putted , len );
//                     data_putted = data_putted + len ;  
//                     record.header.lenght = len ; 
//                     record.insert.offset = offset ; 
//                     record.insert.page_id = page_id ; 
//                     record.insert.lenght = len ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4) ;
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     prev_lsn = record.header.lsn ; 
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     len  = 0  ;
//                 }
//             }
//             data_putted = 0 ; 
//         }
//         fsync(file_num) ;
//         close(file_num) ;

//     }
//     return ; 
// }


// void delete_wal(int transaction_id , int page_id , int offset , char *data ){
//     int len = strlen(data)  ;
//     int data_putted  = 0 ;  
//     records_struct record ; 
//     int declration = 2 ;  
//     int slot = -1 ;
//     for (int i = 0 ; i < 256 ; i++){
//         if (dat->running_transaction[i] == transaction_id){
//             slot = i ;
//             break ;
//         }
//     }
//     if (slot == -1){
//         return ;
//     }
//     int prev_lsn = dat->running_last_lsn[slot] ; 
//     int record_size = records_sturct_size + max_size_for_data ; 
//     while ( len > 0 ){
//         int seg_num = dat->current_segment ; 
//         int file_num ; 

//         file_num = create_segment_file(seg_num) ; 
//         if (file_num == -1  ){
//             return  ; 
//         }
//         segment_struct seg_str ;
//         int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
//         if (size_of_the_thing != sizeof(seg_str) ){
//             default_segment(&seg_str , seg_num ) ; 
//         }

//         if (default_segment_size - seg_str.size < record_size){
//             close(file_num) ;
//             int next_seg_num = (seg_num + 1) % 256 ;
//             if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
//                 checkpoint() ;
//                 return ;            
//             }
//             dat->current_segment = next_seg_num ; 
//             seg_num = next_seg_num  ;
//             file_num = create_segment_file(seg_num) ;
//             if (file_num == -1){
//                 return ;
//             }
//             default_segment(&seg_str , seg_num) ; 
//             seg_str.seq = ++dat->seq ; 
//             seg_str.first_lsn = dat->lsn_num ; 
//             seg_str.records_offset = 0 ;
//             dat->offset[seg_num] = seg_str.size ;
//         }
//         seg_str.status = 1 ; 

//         int space = default_segment_size - seg_str.size ;
//         int num = (space / record_size) * max_size_for_data ; 
//         if (len >= num ){
//             len = len - num ; 
//             while ( num > 0 ){
//                 default_record( &record , 2  , dat ) ;
//                 record.header.transaction = transaction_id ;
//                 if (num >= max_size_for_data){
//                     int data_we_put = records_sturct_size + max_size_for_data ;
//                     memcpy(record.delete.deleted_data , data + data_putted , max_size_for_data );
//                     data_putted = data_putted + max_size_for_data ; 
//                     record.header.lenght = max_size_for_data ; 
//                     record.delete.offset = offset ; 
//                     record.delete.page_id = page_id ; 
//                     record.delete.lenght = max_size_for_data ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     prev_lsn = record.header.lsn ; 
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     num  = num - max_size_for_data ;
//                 }
//                 else { 
//                     int data_we_put = records_sturct_size + num ;
//                     memcpy(record.delete.deleted_data , data + data_putted , num );
//                     data_putted = data_putted + num ;  
//                     record.header.lenght = num ; 
//                     record.delete.offset = offset ; 
//                     record.delete.page_id = page_id ; 
//                     record.delete.lenght = num ;  
//                     record.header.prev = prev_lsn ;
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     prev_lsn = record.header.lsn ; 
//                     seg_str.size = seg_str.size + data_we_put  ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     num  = 0  ;
//                 }
//             }
//             seg_str.records_offset = seg_str.records_offset + seg_str.size ; 
//             declration = write_header(file_num , seg_str ) ; 
//             if ( declration == -1 ){
//                 close(file_num) ;
//                 return  ; 
//             }
//         }
//         else {  
//             while ( len > 0 ){
//                 default_record( &record , 2  , dat ) ; 
//                 record.header.transaction = transaction_id ;
//                 if (len >= max_size_for_data){
//                     int data_we_put = records_sturct_size + max_size_for_data ;
//                     memcpy(record.delete.deleted_data , data + data_putted , max_size_for_data );
//                     data_putted = data_putted + max_size_for_data ; 
//                     record.header.lenght = max_size_for_data ; 
//                     record.delete.offset = offset ; 
//                     record.delete.page_id = page_id ; 
//                     record.delete.lenght = max_size_for_data ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     prev_lsn = record.header.lsn ; 
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     len  = len - max_size_for_data ;
//                 }
//                 else { 
//                     int data_we_put = records_sturct_size + len ;
//                     memcpy(record.delete.deleted_data , data + data_putted , len );
//                     data_putted = data_putted + len ;  
//                     record.header.lenght = len ; 
//                     record.delete.offset = offset ; 
//                     record.delete.page_id = page_id ; 
//                     record.delete.lenght = len ;  
//                     record.header.prev = prev_lsn ; 
//                     record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                     declration = write_record(file_num , seg_str.size , record ) ; 
//                     if (declration == -1  ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     prev_lsn = record.header.lsn ; 
//                     seg_str.size = seg_str.size + data_we_put ;
//                     dat->offset[seg_num] = seg_str.size ;
//                     dat->last_lsn_in_segment[seg_num] = record.header.lsn ; 
//                     if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                     dat->running_last_lsn[slot] = record.header.lsn ;
//                     declration = write_header(file_num , seg_str ) ; 
//                     if ( declration == -1 ){
//                         close(file_num) ;
//                         return  ; 
//                     }
//                     len  = 0  ;
//                 }
//             }
//             data_putted = 0 ; 
//         }
//         fsync(file_num) ;
//         close(file_num) ;

//     }
//     return ; 
// }


// void update_wal(int transaction_id , int page_id , int offset , char *prev_data , int prev_len , char *updated_data , int updated_len ){
//     int len = prev_len ;
//     if (updated_len > prev_len ){
//         len = updated_len ;
//     }
//     int prev_putted = 0 ;
//     int updated_putted = 0 ;
//     records_struct record ; 
//     int declration = 2 ;  
//     int slot = -1 ;
//     for (int i = 0 ; i < 256 ; i++){
//         if (dat->running_transaction[i] == transaction_id){
//             slot = i ;
//             break ;
//         }
//     }
//     if (slot == -1){
//         return ;
//     }
//     int prev_lsn = dat->running_last_lsn[slot] ; 
//     int record_size = records_sturct_size + 2 * max_size_for_data ;
//     while ( len > 0 ){
//         int seg_num = dat->current_segment ; 
//         int file_num ; 

//         file_num = create_segment_file(seg_num) ; 
//         if (file_num == -1  ){
//             return  ; 
//         }
//         segment_struct seg_str ;
//         int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
//         if (size_of_the_thing != sizeof(seg_str) ){
//             default_segment(&seg_str , seg_num ) ; 
//         }

//         if (default_segment_size - seg_str.size < record_size){
//             close(file_num) ;
//             int next_seg_num = (seg_num + 1) % 256 ;
//             if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
//                 checkpoint() ;
//                 return ;            
//             }
//             dat->current_segment = next_seg_num ; 
//             seg_num = next_seg_num  ;
//             file_num = create_segment_file(seg_num) ;
//             if (file_num == -1){
//                 return ;
//             }
//             default_segment(&seg_str , seg_num) ; 
//             seg_str.seq = ++dat->seq ; 
//             seg_str.first_lsn = dat->lsn_num ; 
//             seg_str.records_offset = 0 ;
//             dat->offset[seg_num] = seg_str.size ;
//         }
//         seg_str.status = 1 ; 

//         int space = default_segment_size - seg_str.size ;
//         int num = (space / record_size) * max_size_for_data ; 
//         if (len >= num ){
//             len = len - num ; 
//             while ( num > 0 ){
//                 int step = max_size_for_data ;
//                 if (num < max_size_for_data){
//                     step = num ;
//                 }
//                 int prev_chunk = prev_len - prev_putted ;
//                 if (prev_chunk > step){
//                     prev_chunk = step ;
//                 }
//                 int updated_chunk = updated_len - updated_putted ;
//                 if (updated_chunk > step){
//                     updated_chunk = step ;
//                 }
//                 int data_we_put = records_sturct_size + prev_chunk + updated_chunk ;
//                 default_record( &record , 3  , dat ) ;
//                 record.header.transaction = transaction_id ;
//                 memcpy(record.update.data , prev_data + prev_putted , prev_chunk );
//                 memcpy(record.update.data + prev_chunk , updated_data + updated_putted , updated_chunk );
//                 prev_putted = prev_putted + prev_chunk ; 
//                 updated_putted = updated_putted + updated_chunk ; 
//                 record.header.lenght = prev_chunk + updated_chunk ; 
//                 record.update.offset = offset ; 
//                 record.update.page_id = page_id ; 
//                 record.update.prev_lenght = prev_chunk ;  
//                 record.update.updated_lenght = updated_chunk ;  
//                 record.header.prev = prev_lsn ; 
//                 record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                 prev_lsn = record.header.lsn ; 
//                 declration = write_record(file_num , seg_str.size , record ) ; 
//                 if (declration == -1  ){
//                     close(file_num) ;
//                     return  ; 
//                 }
//                 seg_str.size = seg_str.size + data_we_put ;
//                 dat->offset[seg_num] = seg_str.size ;
//                 dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
//                 if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                 dat->running_last_lsn[slot] = record.header.lsn ;
//                 declration = write_header(file_num , seg_str ) ; 
//                 if ( declration == -1 ){
//                     close(file_num) ;
//                     return  ; 
//                 }
//                 num  = num - step ;
//             }
//             seg_str.records_offset = seg_str.records_offset + seg_str.size ; 
//             declration = write_header(file_num , seg_str ) ; 
//             if ( declration == -1 ){
//                 close(file_num) ;
//                 return  ; 
//             }
//         }
//         else {  
//             while ( len > 0 ){
//                 int step = max_size_for_data ;
//                 if (len < max_size_for_data){
//                     step = len ;
//                 }
//                 int prev_chunk = prev_len - prev_putted ;
//                 if (prev_chunk > step){
//                     prev_chunk = step ;
//                 }
//                 int updated_chunk = updated_len - updated_putted ;
//                 if (updated_chunk > step){
//                     updated_chunk = step ;
//                 }
//                 int data_we_put = records_sturct_size + prev_chunk + updated_chunk ;
//                 default_record( &record , 3  , dat ) ;
//                 record.header.transaction = transaction_id ;
//                 memcpy(record.update.data , prev_data + prev_putted , prev_chunk );
//                 memcpy(record.update.data + prev_chunk , updated_data + updated_putted , updated_chunk );
//                 prev_putted = prev_putted + prev_chunk ; 
//                 updated_putted = updated_putted + updated_chunk ; 
//                 record.header.lenght = prev_chunk + updated_chunk ; 
//                 record.update.offset = offset ; 
//                 record.update.page_id = page_id ; 
//                 record.update.prev_lenght = prev_chunk ;  
//                 record.update.updated_lenght = updated_chunk ;  
//                 record.header.prev = prev_lsn ; 
//                 record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
//                 prev_lsn = record.header.lsn ; 
//                 declration = write_record(file_num , seg_str.size , record ) ; 
//                 if (declration == -1  ){
//                     close(file_num) ;
//                     return  ; 
//                 }
//                 seg_str.size = seg_str.size + data_we_put ;
//                 dat->offset[seg_num] = seg_str.size ;
//                 dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
//                 if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
//                 dat->running_last_lsn[slot] = record.header.lsn ;
//                 declration = write_header(file_num , seg_str ) ; 
//                 if ( declration == -1 ){
//                     close(file_num) ;
//                     return  ; 
//                 }
//                 len = len - step ;
//             }
//             prev_putted = 0 ; 
//             updated_putted = 0 ; 
//         }
//         fsync(file_num) ;
//         close(file_num) ;

//     }
//     return ; 
// }

void insert_wal(int transaction_id , int page_id , int offset , char *data ){
    int len = strlen(data)  ;
    int data_putted  = 0 ;
    records_struct record ;
    int declration = 2 ;
    int slot = -1 ;
    for (int i = 0 ; i < 256 ; i++){
        if (dat->running_transaction[i] == transaction_id){
            slot = i ;
            break ;
        }
    }
    if (slot == -1){
        return ;
    }
    int prev_file_offset = dat->previous_offset[slot] ; 
    int prev_lsn = dat->running_last_lsn[slot] ;
    int record_size = records_sturct_size + max_size_for_data ;
    while ( len > 0 ){
        int seg_num = dat->current_segment ;
        int file_num ;

        file_num = create_segment_file(seg_num) ;
        if (file_num == -1  ){
            return  ;
        }
        segment_struct seg_str ;
        int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
        if (size_of_the_thing != sizeof(seg_str) ){
            default_segment(&seg_str , seg_num ) ;
        }

        if (default_segment_size - seg_str.size < record_size){
            close(file_num) ;
            int next_seg_num = (seg_num + 1) % 256 ;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
                checkpoint() ;
                return ;
            }
            dat->current_segment = next_seg_num ;
            seg_num = next_seg_num  ;
            file_num = create_segment_file(seg_num) ;
            if (file_num == -1){
                return ;
            }
            default_segment(&seg_str , seg_num) ;
            seg_str.seq = ++dat->seq ;
            seg_str.first_lsn = dat->lsn_num ;
            seg_str.records_offset = 0 ;
            dat->offset[seg_num] = seg_str.size ;
        }
        seg_str.status = 1 ;

        int step = max_size_for_data ;
        if (len < max_size_for_data){
            step = len ;
        }
        int data_we_put = records_sturct_size + step ;
        default_record( &record , 1  , dat ) ;
        record.header.transaction = transaction_id ;
        memcpy(record.insert.inserted_data , data + data_putted , step );
        data_putted = data_putted + step ;
        record.header.lenght = step ;
        record.insert.offset = offset ;
        record.insert.page_id = page_id ;
        record.insert.lenght = step ;
        record.header.prev = prev_lsn ;
        record.header.prev_offset = prev_file_offset ;
        record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
        prev_lsn = record.header.lsn ;

        declration = write_record(file_num , seg_str.size , record ) ;
        if (declration == -1  ){
            close(file_num) ;
            return  ;
        }
        seg_str.size = seg_str.size + data_we_put ;
        dat->offset[seg_num] = seg_str.size ;
        dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
        dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
        if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
        dat->running_last_lsn[slot] = record.header.lsn ;
        declration = write_header(file_num , seg_str ) ;
        if ( declration == -1 ){
            close(file_num) ;
            return  ;
        }
        len = len - step ;
        fsync(file_num) ;
        close(file_num) ;
    }
    return ;
}


void delete_wal(int transaction_id , int page_id , int offset , char *data ){
    int len = strlen(data)  ;
    int data_putted  = 0 ;
    records_struct record ;
    int declration = 2 ;
    int slot = -1 ;
    for (int i = 0 ; i < 256 ; i++){
        if (dat->running_transaction[i] == transaction_id){
            slot = i ;
            break ;
        }
    }
    if (slot == -1){
        return ;
    }
    int prev_lsn = dat->running_last_lsn[slot] ;
    int prev_file_offset = dat->previous_offset[slot] ; 
    int record_size = records_sturct_size + max_size_for_data ;
    while ( len > 0 ){
        int seg_num = dat->current_segment ;
        int file_num ;

        file_num = create_segment_file(seg_num) ;
        if (file_num == -1  ){
            return  ;
        }
        segment_struct seg_str ;
        int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
        if (size_of_the_thing != sizeof(seg_str) ){
            default_segment(&seg_str , seg_num ) ;
        }

        if (default_segment_size - seg_str.size < record_size){
            close(file_num) ;
            int next_seg_num = (seg_num + 1) % 256 ;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
                checkpoint() ;
                return ;
            }
            dat->current_segment = next_seg_num ;
            seg_num = next_seg_num  ;
            file_num = create_segment_file(seg_num) ;
            if (file_num == -1){
                return ;
            }
            default_segment(&seg_str , seg_num) ;
            seg_str.seq = ++dat->seq ;
            seg_str.first_lsn = dat->lsn_num ;
            seg_str.records_offset = 0 ;
            dat->offset[seg_num] = seg_str.size ;
        }
        seg_str.status = 1 ;

        int step = max_size_for_data ;
        if (len < max_size_for_data){
            step = len ;
        }
        int data_we_put = records_sturct_size + step ;
        default_record( &record , 2  , dat ) ;
        record.header.transaction = transaction_id ;
        memcpy(record.delete.deleted_data , data + data_putted , step );
        data_putted = data_putted + step ;
        record.header.lenght = step ;
        record.delete.offset = offset ;
        record.delete.page_id = page_id ;
        record.delete.lenght = step ;
        record.header.prev = prev_lsn ;
        record.header.prev_offset = prev_file_offset ;
        record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
        prev_lsn = record.header.lsn ;
        declration = write_record(file_num , seg_str.size , record ) ;
        if (declration == -1  ){
            close(file_num) ;
            return  ;
        }
        seg_str.size = seg_str.size + data_we_put ;
        dat->offset[seg_num] = seg_str.size ;
        dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
        if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
        dat->running_last_lsn[slot] = record.header.lsn ;
        dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
        declration = write_header(file_num , seg_str ) ;
        if ( declration == -1 ){
            close(file_num) ;
            return  ;
        }
        len = len - step ;
        fsync(file_num) ;
        close(file_num) ;
    }
    return ;
}


void update_wal(int transaction_id , int page_id , int offset , char *prev_data , int prev_len , char *updated_data , int updated_len ){
    int len = prev_len ;
    if (updated_len > prev_len ){
        len = updated_len ;
    }
    int prev_putted = 0 ;
    int updated_putted = 0 ;
    records_struct record ;
    int declration = 2 ;
    int slot = -1 ;
    for (int i = 0 ; i < 256 ; i++){
        if (dat->running_transaction[i] == transaction_id){
            slot = i ;
            break ;
        }
    }
    if (slot == -1){
        return ;
    }
    int prev_lsn = dat->running_last_lsn[slot] ;
    int prev_file_offset = dat->previous_offset[slot] ; 
    int record_size = records_sturct_size + 2 * max_size_for_data ;
    while ( len > 0 ){
        int seg_num = dat->current_segment ;
        int file_num ;

        file_num = create_segment_file(seg_num) ;
        if (file_num == -1  ){
            return  ;
        }
        segment_struct seg_str ;
        int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
        if (size_of_the_thing != sizeof(seg_str) ){
            default_segment(&seg_str , seg_num ) ;
        }

        if (default_segment_size - seg_str.size < record_size){
            close(file_num) ;
            int next_seg_num = (seg_num + 1) % 256 ;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
                checkpoint() ;
                return ;
            }
            dat->current_segment = next_seg_num ;
            seg_num = next_seg_num  ;
            file_num = create_segment_file(seg_num) ;
            if (file_num == -1){
                return ;
            }
            default_segment(&seg_str , seg_num) ;
            seg_str.seq = ++dat->seq ;
            seg_str.first_lsn = dat->lsn_num ;
            seg_str.records_offset = 0 ;
            dat->offset[seg_num] = seg_str.size ;
        }
        seg_str.status = 1 ;

        int step = max_size_for_data ;
        if (len < max_size_for_data){
            step = len ;
        }
        int prev_chunk = prev_len - prev_putted ;
        if (prev_chunk > step){
            prev_chunk = step ;
        }
        int updated_chunk = updated_len - updated_putted ;
        if (updated_chunk > step){
            updated_chunk = step ;
        }
        int data_we_put = records_sturct_size + prev_chunk + updated_chunk ;
        default_record( &record , 3  , dat ) ;
        record.header.transaction = transaction_id ;
        memcpy(record.update.data , prev_data + prev_putted , prev_chunk );
        memcpy(record.update.data + prev_chunk , updated_data + updated_putted , updated_chunk );
        prev_putted = prev_putted + prev_chunk ;
        updated_putted = updated_putted + updated_chunk ;
        record.header.lenght = prev_chunk + updated_chunk ;
        record.update.offset = offset ;
        record.update.page_id = page_id ;
        record.update.prev_lenght = prev_chunk ;
        record.update.updated_lenght = updated_chunk ;
        record.header.prev = prev_lsn ;
        record.header.prev_offset = prev_file_offset ;
        record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
        prev_lsn = record.header.lsn ;
        declration = write_record(file_num , seg_str.size , record ) ;
        if (declration == -1  ){
            close(file_num) ;
            return  ;
        }
        seg_str.size = seg_str.size + data_we_put ;
        dat->offset[seg_num] = seg_str.size ;
        dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
        dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
        if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
        dat->running_last_lsn[slot] = record.header.lsn ;
        declration = write_header(file_num , seg_str ) ;
        if ( declration == -1 ){
            close(file_num) ;
            return  ;
        }
        len = len - step ;
        fsync(file_num) ;
        close(file_num) ;
    }
    return ;
}

void commit( int commit_the_transaction_id ){
    int which_one = -1 ; 
    for ( int i = 0 ; i < 256 ; i++ ){
        if (dat->running_transaction[i] == commit_the_transaction_id ){
            which_one = i ; 
            break ; 
        }
    }
    if (which_one == -1){
        return ;
    }
    int prev_lsn = dat->running_last_lsn[which_one] ; 
    int declration = 2 ;
    int record_size = records_sturct_size ;
    records_struct  record  ; 
    default_record( &record , 4 , dat ) ; 
    record.header.transaction = commit_the_transaction_id ;
    record.header.lenght = 0 ;
    record.header.prev = prev_lsn ;
    record.crc = crc32c((char *)&record , 4 , records_sturct_size - 4) ; 

    int seg_num = dat->current_segment ; 
    int file_num ; 

    file_num = create_segment_file(seg_num) ; 
    if (file_num == -1  ){
        return  ; 
    }
    segment_struct seg_str ;
    int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
    if (size_of_the_thing != sizeof(seg_str) ){
        default_segment(&seg_str , seg_num ) ; 
    }
    if (default_segment_size - seg_str.size < record_size){
        close(file_num) ;
        int next_seg_num = (seg_num + 1) % 256 ;
        if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
            checkpoint() ;
            return ;            
        }
        dat->current_segment = next_seg_num ; 
        seg_num = next_seg_num  ;
        file_num = create_segment_file(seg_num) ;
        if (file_num == -1){
            return ;
        }
        default_segment(&seg_str , seg_num) ; 
        dat->seq++ ; 
        seg_str.seq = dat->seq ; 
        seg_str.first_lsn = dat->lsn_num ; 
        seg_str.records_offset = 0 ;
        dat->offset[seg_num] = seg_str.size ;
    }
    seg_str.status = 1 ; 

    declration = write_record(file_num , seg_str.size , record ) ;
    if (declration == -1){
        close(file_num) ;
        return ;
    }
    seg_str.size = seg_str.size + records_sturct_size ;
    dat->offset[seg_num] = seg_str.size ;
    dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
    declration = write_header(file_num , seg_str ) ;
    if (declration == -1){
        close(file_num) ;
        return ;
    }
    fsync(file_num) ;
    close(file_num) ;

    dat->running_transaction[which_one] = -1 ;
    dat->running_first_lsn[which_one] = -1 ;
    dat->running_last_lsn[which_one] = -1 ;
    dat->num_of_run_transactions-- ;
    update_farthest_lsn() ;
}




void abort( int transaction_id ){
    records_struct put_struct ;
    int declration ;
    int slot = -1 ;
    int i = 0 ; 
    int lsn = -1 ; 
    while (i < dat->num_of_run_transactions){
        if (dat->running_transaction[i] == transaction_id ){
            lsn = dat->running_last_lsn[i] ; 
            slot = i ;
            break ; 
        }
        i++ ; 
    }
    if (lsn == -1  ){
        return ;
    }
    records_struct record ; 
    default_record(&record , 4) ; 
    int the_lsn_file = -1 ; 
    for ( int i = 0 ; i < 256 ; i++ ){
        if ( dat->running_first_lsn[i] <= lsn  && dat->running_last_lsn[i] >= lsn ){
            int file_num = create_segment_file(i) ;
            if (file_num == -1){
                break ;
            }
            int size = sizeof(segment_struct) ;
            while ( size < 4096){
                pread(file_num , &record , records_sturct_size , size) ;       
                if (record.header.lsn == lsn){
                    pread(file_num , &record , records_sturct_size + record.header.lenght , size) ;   
                    break ;
                }
                size = size + records_sturct_size + record.header.lenght ;     
            }
            close(file_num) ;
            break ;
        }
    }
    int first = 0 ; 
    int commited = 0 ; 
    int offset_of_the_prev = -1 ; 
    while ( lsn != -1 ){
        int seg_num = dat->current_segment ; 
        int file_num ; 
        file_num = create_segment_file(seg_num) ; 
        if (file_num == -1  ){
            return  ; 
        }

        segment_struct seg_str ;
        int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
        if (size_of_the_thing != sizeof(seg_str) ){
            default_segment(&seg_str , seg_num ) ; 
        }
        if (default_segment_size - seg_str.size < record_size){
            close(file_num) ;
            int next_seg_num = (seg_num + 1) % 256 ;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
                checkpoint() ;
                return ;            
            }
            dat->current_segment = next_seg_num ; 
            seg_num = next_seg_num  ;
            file_num = create_segment_file(seg_num) ;
            if (file_num == -1){
                return ;
            }
            default_segment(&seg_str , seg_num) ; 
            dat->seq++ ; 
            seg_str.seq = dat->seq ; 
            seg_str.first_lsn = dat->lsn_num ; 
            seg_str.records_offset = 0 ;
            dat->offset[seg_num] = seg_str.size ;
        }
        seg_str.status = 1 ; 

        int file_seg_num ; 
        int offset_of_the_file ;

        if (first == 1 ){
            file_seg_num = offset_of_the_prev / 4096  ; 
            offset_of_the_file = offset_of_the_prev % 4096  ; 
            default_record( &record , 4) ; 
            int temp_file_num = create_segment_file(file_seg_num) ;
            if (temp_file_num == -1){
                record.header.lsn = -1 ;
                break ; 
            }
            int numes = pread(temp_file_num , &record , records_sturct_size , offset_of_the_file) ;
            if (numes != records_sturct_size){
                record.header.lsn = -1 ;
                close(temp_file_num) ;
                break ; 
            }
            pread(temp_file_num , &record , records_sturct_size + record.header.lenght , offset_of_the_file) ;
            close(temp_file_num) ;
        }

        if (record.header.is_clr == 1 ){
            close(file_num) ;
            first = 1 ; 
            lsn = record.header.undo_next_lsn ; 
            offset_of_the_prev = record.header.undo_next_offset ; 
            continue ;
        }

        if (record.header.type == 1 ){
            int step = record.insert.lenght ;
            put_struct.header.type = 8 ; 
            put_struct.type = 8 ; 
            put_struct.header.lenght = step ;
            put_struct.delete.page_id = record.insert.page_id ;
            put_struct.delete.offset = record.insert.offset ;
            put_struct.delete.lenght = step ;
            memcpy(put_struct.delete.deleted_data , record.insert.inserted_data , step );
            int data_we_put = records_sturct_size + put_struct.header.lenght ;
            put_struct.header.lsn = dat->lsn_num++ ;
            put_struct.header.is_clr = 1 ;
            put_struct.header.undo_next_lsn = record.header.prev_lsn ; 
            put_struct.header.undo_next_offset = record.header.prev_offset ;
            put_struct.header.prev_lsn = dat->running_last_lsn[slot] ; 
            put_struct.header.prev_offset = dat->previous_offset[slot] ;
            put_struct.crc = crc32c((char *)&put_struct , 4 , data_we_put - 4);
            declration = write_record(file_num , seg_str.size , put_struct ) ;
            if (declration == -1  ){
                close(file_num) ;
                return  ;
            }
            dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
            seg_str.size = seg_str.size + data_we_put ;
            dat->offset[seg_num] = seg_str.size ;
            dat->last_lsn_in_segment[seg_num] = put_struct.header.lsn ;
            if (dat->running_first_lsn[slot] == -1){
                dat->running_first_lsn[slot] = put_struct.header.lsn ;
            }
            dat->running_last_lsn[slot] = put_struct.header.lsn ;
            declration = write_header(file_num , seg_str ) ;
            if ( declration == -1 ){
                close(file_num) ;
                return  ;
            }
            fsync(file_num) ;
        }

        else if ( record.header.type == 2 ){
            FSM_insert( record.delete.deleted_data , record.delete.page_id , record.delete.offset  ) ; 
            int step = record.delete.lenght ;
            put_struct.header.type = 7 ; 
            put_struct.type = 7 ; 
            put_struct.header.lenght = step ;
            put_struct.insert.page_id = record.delete.page_id ;
            put_struct.insert.offset = record.delete.offset ;
            put_struct.insert.lenght = step ;
            memcpy(put_struct.insert.inserted_data , record.delete.deleted_data , step );
            int data_we_put = records_sturct_size + put_struct.header.lenght ;
            put_struct.header.lsn = dat->lsn_num++ ;
            put_struct.header.is_clr = 1 ;
            put_struct.header.undo_next_lsn = record.header.prev_lsn ; 
            put_struct.header.undo_next_offset = record.header.prev_offset ;
            put_struct.header.prev_lsn = dat->running_last_lsn[slot] ; 
            put_struct.header.prev_offset = dat->previous_offset[slot] ;
            put_struct.crc = crc32c((char *)&put_struct , 4 , data_we_put - 4);
            declration = write_record(file_num , seg_str.size , put_struct ) ;
            if (declration == -1  ){
                close(file_num) ;
                return  ;
            }
            dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
            seg_str.size = seg_str.size + data_we_put ;
            dat->offset[seg_num] = seg_str.size ;
            dat->last_lsn_in_segment[seg_num] = put_struct.header.lsn ;
            if (dat->running_first_lsn[slot] == -1){
                dat->running_first_lsn[slot] = put_struct.header.lsn ;
            }
            dat->running_last_lsn[slot] = put_struct.header.lsn ;
            declration = write_header(file_num , seg_str ) ;
            if ( declration == -1 ){
                close(file_num) ;
                return  ;
            }
            fsync(file_num) ;
        }

        else if ( record.header.type == 3 ){
            int pre_len = record.update.prev_lenght ;
            int cur_len = record.update.updated_lenght ;
            put_struct.header.type = 9 ; 
            put_struct.type = 9 ; 
            put_struct.header.lenght = pre_len + cur_len ;
            put_struct.update.page_id = record.update.page_id ;
            put_struct.update.offset = record.update.offset ;
            put_struct.update.prev_lenght = cur_len ;
            put_struct.update.updated_lenght = pre_len ;
            memcpy(put_struct.update.data , record.update.data + pre_len , cur_len );
            memcpy(put_struct.update.data + cur_len , record.update.data , pre_len );
            int data_we_put = records_sturct_size + put_struct.header.lenght ;
            put_struct.header.lsn = dat->lsn_num++ ;
            put_struct.header.is_clr = 1 ;
            put_struct.header.undo_next_lsn = record.header.prev_lsn ; 
            put_struct.header.undo_next_offset = record.header.prev_offset ;
            put_struct.header.prev_lsn = dat->running_last_lsn[slot] ; 
            put_struct.header.prev_offset = dat->previous_offset[slot] ;
            put_struct.crc = crc32c((char *)&put_struct , 4 , data_we_put - 4);
            declration = write_record(file_num , seg_str.size , put_struct ) ;
            if (declration == -1  ){
                close(file_num) ;
                return  ;
            }
            dat->previous_offset[slot] = seg_str.size + seg_num * max_size_for_data ;
            seg_str.size = seg_str.size + data_we_put ;
            dat->offset[seg_num] = seg_str.size ;
            dat->last_lsn_in_segment[seg_num] = put_struct.header.lsn ;
            if (dat->running_first_lsn[slot] == -1){
                dat->running_first_lsn[slot] = put_struct.header.lsn ;
            }
            dat->running_last_lsn[slot] = put_struct.header.lsn ;
            declration = write_header(file_num , seg_str ) ;
            if ( declration == -1 ){
                close(file_num) ;
                return  ;
            }
            fsync(file_num) ;
        }
        else if (  record.header.type == 5  ){
            commited = 1; 
            close(file_num) ;
            return ;
        }
        close(file_num) ;
        first = 1 ; 
        lsn = record.header.prev_lsn  ; 
        offset_of_the_prev = record.header.prev_offset ; 
    }

    int j = slot ;
    while ( j < dat->num_of_run_transactions - 1 ){
        dat->running_transaction[j] = dat->running_transaction[j + 1] ;
        dat->running_first_lsn[j] = dat->running_first_lsn[j + 1] ;
        dat->running_last_lsn[j] = dat->running_last_lsn[j + 1] ;
        dat->previous_offset[j] = dat->previous_offset[j + 1] ;
        j++ ;
    }
    dat->num_of_run_transactions-- ;
}

void checkpoint(){
    
}
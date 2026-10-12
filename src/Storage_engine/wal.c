int wal_folder(const char *path) {
    if (mkdir(path , 0755) == 0){
        return 0 ;
    } 
    if (errno == EEXIST) {
        return 0 ; 
    }
    return -1 ;
}

req_data * dat  = malloc(sizeof(req_data)) ; 

void crc_init(void) {                    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++)
            c = (c >> 1) ^ (0x82F63B78 & -(c & 1));
        crc_table[i] = c;
    }
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
    dat->num_of_run_transactions = 0 ;
    dat->lsn_num = 0 ;
    dat->segment_num = 0 ;
    dat->transaction_num = 0 ;
    crc_init() ; 
}

void default_segment(segment_struct  * seg_str  , int seg_num ){
    seg_str->segment_id = seg_num ; 
    seg_str->status = 0 ; 
    seg_str->size = 0 ; 
    seg_str->records_offset = 0 ; 
}

void default_record(records_struct * record , int type  , req_data dat ){
    record->crc = 0 ; 
    record->type = type  ; 
    record->header.lsn  = req_data->lsn_num++ ; 
    record->header.lenght  = 0   ; 
    record->header.type = type ; 
    record->header.prev = -1   ; 
    record->header.transaction  = dat->transaction_num ; 
    if (type == 1 ){
        record->record_value.insert.page_id = -1 ; 
        record->record_value.insert.offset = 0 ; 
        record->record_value.insert.lenght = 0 ; 
    }
    else if (type == 3 ){
        record->record_value.update.page_id = -1 ; 
        record->record_value.update.offset = 0 ; 
        record->record_value.update.updated_lenght = 0 ; 
        record->record_value.update.prev_lenght = 0 ; 
    }
    else if (type == 2 ) { 
        record->record_value.delete.page_id = -1 ; 
        record->record_value.delete.offset = 0 ; 
        record->record_value.delete.lenght = 0 ; 
    }
    else if ( type == 6 ) { 
        record->record_value.checkpoint.transactions_number = 0 ; 
        record->record_value.checkpoint.dirty_page_number = 0 ; 
    }
}

void  insert(int page_id , int offset , char *data ){
    int len = strlen(data)  ;
    int data_putted  = 0 ;  
    records_struct record ; 
    int prev_lsn = -1 ; 
    int offset = 0 ; 
    int transaction_id = dat->transaction_num++ ; 
    while ( len > 0 ){
        int seg_num = 0 ; 
        if (dat->num_of_dead_segments > 0 ){
            int i = 0 ; 
            while ( dat->dead_segments[i] != -1 ){
                i++ ; 
            }
            seg_num = dat->dead_segments[i] ; 
        }
        else if (dat->segment_num > 0 ){
            seg_num = dat->segment_num  ;
            dat->segment_num++  ;
        }
        segment_struct  * seg_str  = fread()  ; // the file name and it is the meta data 
        if (seg_str == NULL ){
            default_segment(seg_str , seg_num ) ; 
        }
        seg_str->status = 1 ; 
        for ( int i  = 0 ; i < max_size_for_data ; i++ ){
            if ( dat->dead_segments[i] == seg_num ){
                dat->dead_segments[i] = -1 ; 
                dat->num_of_dead_segments-- ; 
                break ; 
            }
        }
        if (len >= default_segment_size - (sizeof(segment_struct) + seg_str->size)){
            int num = default_segment_size - (sizeof(segment_struct) + seg_str->size) ; 
            seg_str->size = seg_str->size + num ; 
            len = len - num ; 
            while ( num > 0 ){
                default_record(&record , 1  , dat ) ; 
                if (num >= max_size_for_data){
                    memcpy(record.insert.inserted_data , data + data_putted , max_size_for_data );
                    data_putted = data_putted + max_size_for_data ; 
                    record.header.lenght = max_size_for_data ; 
                    record.insert.offset = offset ; 
                    record.insert.page_id = page_id ; 
                    record.insert.lenght = max_size_for_data ;  
                    record.header.prev = prev_lsn ; 
                    record.crc = crc32c((char *)&record , 4 , rec.header.length - 4);
                    prev_lsn = record.header.lsn ; 
                    // write the thing in the file 
                    seg_str->size = seg_str->size + max_size_for_data ;
                    num  = num - max_size_for_data ;
                }
                else { 
                    memcpy(record.insert.inserted_data , data + data_putted , num );
                    record.crc = crc32c((char *)&record , 4 , rec.header.length - 4);
                    data_putted = data_putted + num ;  
                    record.header.lenght = num ; 
                    record.insert.offset = offset ; 
                    record.insert.page_id = page_id ; 
                    record.insert.lenght = num ;  
                    record.header.prev = prev_lsn ; 
                    // write the thing in the file 
                    prev_lsn = record.header.lsn ; 
                    seg_str->size = seg_str->size + num ;
                    num  = 0  ;
                }
            }
            seg_str->records_offset = seg_str->records_offset + seg_str->size ; 
        }
        else {  
            while ( len > 0 ){
                default_record(&record , 1  , dat ) ; 
                if (len >= max_size_for_data){
                    memcpy(record.insert.inserted_data , data + data_putted , max_size_for_data );
                    data_putted = data_putted + max_size_for_data ; 
                    record.header.lenght = max_size_for_data ; 
                    record.insert.offset = offset ; 
                    record.insert.page_id = page_id ; 
                    record.insert.lenght = max_size_for_data ;  
                    record.header.prev = prev_lsn ; 
                    record.crc = crc32c((char *)&record , 4 , rec.header.length - 4);
                    prev_lsn = record.header.lsn ; 
                    // write the thing in the file 
                    seg_str->size = seg_str->size + max_size_for_data ;
                    len  = len - max_size_for_data ;
                }
                else { 
                    memcpy(record.insert.inserted_data , data + data_putted , len );
                    record.crc = crc32c((char *)&record , 4 , rec.header.length - 4);
                    data_putted = data_putted + len ;  
                    record.header.lenght = len ; 
                    record.insert.offset = offset ; 
                    record.insert.page_id = page_id ; 
                    record.insert.lenght = len ;  
                    record.header.prev = prev_lsn ; 
                    // write the thing in the file 
                    prev_lsn = record.header.lsn ; 
                    seg_str->size = seg_str->size + len ;
                    len  = 0  ;
                }
            }
            prev_lsn = -1 ; 
            data_putted = 0 ; 
            dat->transaction_num++ ; 
        }

    }
    return ; 
}


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

void start_the_wal(){
    dat->num_of_run_transactions = 0 ;
    dat->lsn_num = 0 ;
    dat->segment_num = 0 ;
    dat->transaction_num = 0 ;
}

void default_segment(segment_struct  * seg_str  , int seg_num ){
    seg_str->segment_id = seg_num ; 
    seg_str->status = 0 ; 
    seg_str->size = 0 ; 
    seg_str->records_offset = 0 ; 
}

int insert(int page_id , int offset , char *data ){
    int len = strlen(data)  ;
    int data_putted  = 0 ;  
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
            seg_str->status = 1 ; 
            if (len >= default_segment_size - sizeof(segment_struct) ){
                seg_str->size = len - (default_segment_size - sizeof(segment_struct)  ) ; 

                seg_str->records_offset = seg_str->records_offset + seg_str->size ; 

            }
            else { 
                seg_str->size = len ; 
                records_struct record ; 
                record.crc = ; 
                record.type = 1  ; 
                record.insert ; 
                record.header.lsn = req_data->lsn_num  ; 
                req_data->lsn_num++ ; 
                record.header.type = 1 ; 
                record.header.transaction = req_data->transaction_num  ; 
                record.header.prev = -1 ; 
                record.header.lenght = len ; 
                record.insert.offset = offset ; 
                record.insert.page_id = page_id ; 
                record.insert.lenght = len ; 
                record.insert.inserted_data = memcpy( page_id ) ; 
                req_data->transaction_num++  ; 

                // write the thing in the file 
                seg_str->records_offset = seg_str->records_offset + seg_str->size ; 
            }
            
        }
        else { 

        }

    }
}
#include "Storage_engine/storage_engine.h"

int number_of_pages = 0;

page *get_page_from_file( int offset ) {
    FILE *file = fopen(data_filename, "rb");
    if (file == NULL) {
        return NULL ;
    }
    fseek(file, offset, SEEK_SET);
    page *ans = malloc(sizeof(page));
    if (ans == NULL) {
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread( ans , 1, sizeof(page), file);
    fclose(file);
    if ( bytes_read != sizeof(page)) {
        free(ans) ;
        return NULL ; }
    return ans;
}


int write_page_to_file(page *p, int offset, const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, data_filename);
    FILE *file = fopen(path, "r+b");
    if (file == NULL) {
        file = fopen(path, "w+b");
        if (file == NULL) {
            return -1;
        }
    }
    if (offset == -1) {
        fseek(file, 0, SEEK_END);
        offset = ftell(file);
    }
    fseek(file, offset, SEEK_SET);
    if (fwrite(p, 1, sizeof(page), file) != sizeof(page)) {
        offset = -1;
    }
    fclose(file);
    return offset;
}


int flatten_ans(char ** words , int num_of , char * ans , int num) {
    int lenght = 0 ;
    int out_of_bound = 0 ;
    for (int i = 0 ; i < num_of ; i++) {
        int len = strlen(words[i]) ;
        if (lenght + len > num - 1) {
            out_of_bound = 1 ;
        }
        else {
            memcpy(ans + lenght , words[i] , len );
            lenght += len ;
        }
        free(words[i]) ;
    }
    if (out_of_bound == 1 ){
        return -1 ;
    }
    ans[lenght] = '\0' ;
    return lenght ;
}



int find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(char *data, table_info *table, char **ans, float *pk, int *jane) {
    int size = 0;
    int i = 0;
    int j = 0;

    char *row = strdup(data);
    char *field[MAX_COLUMNS] = {0};
    int n = 0;
    char *somethin = row;
    field[n++] = somethin ;
    while (*somethin && n < MAX_COLUMNS) {
        if (*somethin == '\t') { 
            *somethin = '\0';
             field[n++] = somethin + 1; 
        }
        somethin++;
    }

    while (i < (int)table->num_columns){
        char *val = field[i] ? field[i] : "";
        if (table->columns[i].primary_key == true){
            *pk = (float)atof(val) ;
        }
        if (table->columns[i].col_type == INT || table->columns[i].col_type == BIGINT || table->columns[i].col_type == SMALLINT || table->columns[i].col_type == TINYINT) {
            size = size + (table->columns[i].type_bits_count / 8 );
            ans[j] = strdup(integer_num);
            j++ ;
            ans[j] = strdup(val) ;
            j++ ;
        }
        else if (table->columns[i].col_type == FLOAT ||table->columns[i].col_type == DOUBLE){
            size = size + (table->columns[i].type_bits_count /8 );
            ans[j] = strdup(real_num) ;
            j++;
            ans[j] = strdup(val) ;
            j++ ;
        }
        else if (table->columns[i].col_type == DECIMAL){
            size = size + strlen(val) ;
            ans[j] = strdup(real_num) ;   
            j++ ;
            ans[j] = strdup(val) ;
            j++ ; 
        }
        else if (table->columns[i].col_type == CHAR || table->columns[i].col_type == VARCHAR){
            if (strlen(val) > 1024){
                ans[j] = strdup(toast_num) ;
                j++;
                char *tp = toast_tables_insert(val , table );
                int cid = 0;
                if (tp != NULL) { 
                    memcpy(&cid, tp, sizeof(int)); 
                    free(tp);
                }
                char cid_txt[32];
                snprintf(cid_txt, sizeof(cid_txt), "%010d", cid);
                ans[j] = strdup(cid_txt) ;
                j++ ;
                size = size + strlen(ans[j-1]) ;
            }
            else {
                ans[j] = strdup(string_num) ;
                j++;
                char len_of_stuff[32];
                snprintf(len_of_stuff, sizeof(len_of_stuff), "%zu", strlen(val));
                ans[j] = strdup(len_of_stuff);
                j++;
                ans[j] = strdup(val);
                size = size + strlen(val);
                j++;
            }
        }
        else if (table->columns[i].col_type == DATE){
            size = size + (table->columns[i].type_bits_count/ 8);
            ans[j] = strdup(date_num);
            j++;
            ans[j] = strdup(val);
            j++;
        }
        else if (table->columns[i].col_type == TIMESTAMP){
            size = size +  (table->columns[i].type_bits_count /8) ;
            ans[j] = strdup(timestamp_num);
            j++ ;
            ans[j] = strdup(val) ;
            j++ ;
        }
        i++ ;
    }
    free(row);
    *jane = j ;
    return size;
}


pagezero *get_pagezero(char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, data_filename);
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }
    pagezero *ans = malloc(sizeof(pagezero));
    if (ans == NULL) {
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(ans, 1, sizeof(pagezero), file);
    fclose(file);
    if (bytes_read == 0) {
        free(ans);
        return NULL;
    }
    return ans;
}



int FSM_insert (char *insert_data, int page_num , int offset, table_info *table  ) {
    page *pg ;
    char *ans[table->num_columns * 3];
    char buf[PAGE_SIZE] ;
    int jane = 0 ;
    int size = 0 ;
    float primary_key = 0 ;
    int fr = -1 ;

    int slot_used = 0 ;
    (void)offset ;

    if (page_num > 0) {
        fr = insert_page_in_frame(bp, page_num , table->table_name );
        if (fr == -1) {
            return -1 ;
        }
        pg = (page *)bp->frames[fr];
        if (pg->header.page_num != (uint32_t)page_num){
            not_in_use_get_out(0, fr, bp) ;
            return -1 ;
        }
        size = find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(insert_data, table, ans, &primary_key, &jane);
        size = flatten_ans(ans, jane, buf, PAGE_SIZE);
        if (size < 0) {
            not_in_use_get_out(0, fr, bp);
            return -1;
        }
        size = size + sizeof(slots);
        if (size < pg->header.free_size){
            if (size <= pg->header.normal_free_size && pg->header.num < max_no_of_slots){
                slot_used = pg->header.num ;
                memcpy(pg->data + pg->header.current_offset , buf , size - sizeof(slots));
                pg->header.free_size = pg->header.free_size - size ;
                pg->header.normal_free_size = pg->header.normal_free_size - size ;
                pg->slot[pg->header.num].size = size - sizeof(slots);
                pg->slot[pg->header.num].offset = pg->header.current_offset;
                pg->slot[pg->header.num].pk = primary_key;
                pg->header.current_offset += size - sizeof(slots);
                int bit_pos = 7 - (pg->header.num % 8);
                pg->header.dead_slots[pg->header.num / 8] |= (1 << bit_pos);
                pg->header.num++;
                pg->header.page_type = normal;
            }
            else {
                int i = 0;
                int k = 0;
                int where_to_put = -1;
                int temp_slot_num = pg->header.num / 8;
                uint8_t temp = 0;
                while (i <= temp_slot_num) {
                    temp = pg->header.dead_slots[i];
                    for (int j = 7; j >= 0; j--) {
                        int slot_idx = (i * 8) + (7 - j);
                        if (slot_idx >= pg->header.num) {
                            break ;
                        }
                        int bit = (temp >> j) & 1;
                        if (bit == 0 && (int)(size - sizeof(slots)) <= pg->slot[slot_idx].size) {
                            where_to_put = slot_idx ;
                            k = i ;
                            break ;
                        }
                    }
                    if (where_to_put != -1) {
                        break;
                    }
                    i++;
                }
                if (where_to_put != -1) {
                    slot_used = where_to_put ;
                    pg->header.free_size = pg->header.free_size + pg->slot[where_to_put].size - (size - sizeof(slots));
                    pg->slot[where_to_put].size = size - sizeof(slots);
                    pg->slot[where_to_put].pk = primary_key;
                    memcpy(pg->data + pg->slot[where_to_put].offset, buf, size - sizeof(slots)) ;

                    int bit_pos = 7 - (where_to_put % 8);
                    temp |= (1 << bit_pos);
                    pg->header.dead_slots[k] = temp;
                    pg->header.page_type = normal;
                }
                else {
                    if (pg->header.num >= max_no_of_slots) {
                        not_in_use_get_out(0, fr, bp);
                        return -1;
                    }
                    slot_used = pg->header.num ;
                    int a = 0;
                    int temp_pointer = 0;
                    while (a < pg->header.num) {
                        int first = a / 8;
                        int second = 7 - (a % 8);
                        if (((pg->header.dead_slots[first] >> second) & 1) == 1){
                            if (pg->slot[a].offset != temp_pointer) {
                                memmove(pg->data + temp_pointer , pg->data + pg->slot[a].offset , pg->slot[a].size) ;
                                pg->slot[a].offset = temp_pointer ;
                            }
                            temp_pointer = temp_pointer + pg->slot[a].size;
                            a++;
                        }
                        else {
                            pg->slot[a].offset = temp_pointer ;
                            pg->slot[a].size = 0 ;
                            a++;
                            continue;
                        }
                    }
                    pg->header.current_offset = temp_pointer;
                    pg->header.normal_free_size = pg->header.free_size;
                    memcpy(pg->data + pg->header.current_offset , buf , size - sizeof(slots));
                    pg->slot[pg->header.num].size = size - sizeof(slots);
                    pg->slot[pg->header.num].offset = pg->header.current_offset;
                    pg->slot[pg->header.num].pk = primary_key;
                    pg->header.current_offset += size - sizeof(slots);
                    pg->header.free_size = pg->header.free_size - size;
                    pg->header.normal_free_size = pg->header.normal_free_size - size;
                    pg->header.dead_slots[pg->header.num / 8] |= (1 << (7 - (pg->header.num % 8)));
                    pg->header.num++;
                    pg->header.page_type = normal;
                }
            }
        }
        else {
            not_in_use_get_out(0, fr, bp);
            return -1 ;
        }
    }
    else {
        size = find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(insert_data , table , ans , &primary_key , &jane);
        size = flatten_ans(ans , jane , buf , PAGE_SIZE);
        if (size < 0 || size + sizeof(slots) > use_page_size) {
            return -1 ;
        }
        pg = calloc(1 , sizeof(page)) ;
        if (pg == NULL) {
            return -1;
        }
        number_of_pages++ ;
        pg->header.page_num = number_of_pages ;
        pg->header.page_type = normal ;
        pg->header.num = 0 ;
        pg->header.toast_table_num = 0  ;

        pg->header.current_offset = 0  ;
        pg->slot[pg->header.num].offset = 0 ;
        pg->slot[pg->header.num].size = size ;
        pg->slot[pg->header.num].pk = primary_key ;
        memcpy(pg->data , buf , size );
        pg->header.current_offset = size ;
        pg->header.dead_slots[0] |= (1 << 7);
        pg->header.free_size = use_page_size - (size + sizeof(slots)) ;
        pg->header.normal_free_size = use_page_size - (size + sizeof(slots)) ;
        pg->header.num++ ;
        pg->header.buffer_space = (int)data_size - use_page_size ;
    }
    int pnum = pg->header.page_num ;
    if (fr == -1){
        write_page_to_file(pg, pnum * PAGE_SIZE , table->table_name ) ;
        fr = insert_page_in_frame(bp, pnum , table->table_name  ) ;
        if (fr == -1){
            free(pg) ;
            return -1;
        }
        memcpy(bp->frames[fr], pg, PAGE_SIZE) ;
        free(pg) ;
    }
    int transaction_num = begin_transaction();
    if (transaction_num != -1){
        insert_wal(transaction_num, pnum, 0, buf , table->table_name);
        end_transaction(transaction_num);
    }
    tree_func_for_insert(get_root(table->table_name), (int)primary_key, buf, pnum, pnum * PAGE_SIZE, slot_used);
    not_in_use_get_out(1, fr, bp);
    return pnum ;
}


toast_table_file_header *get_toast_table_file_header(const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, toast_header_filename);
    FILE *file = fopen(path, "rb") ;
    toast_table_file_header *ans = calloc(1, sizeof(toast_table_file_header));
    if (ans == NULL) {
        if (file != NULL) fclose(file) ;
        return NULL ;
    }
    if (file == NULL) {
        ans->dead_space_available[0] |= (1 << 7);
        return ans ;
    }
    char dat[4096] ;
    size_t bytes_read = fread(dat, 1, 4096, file);
    fclose(file) ;
    if (bytes_read < sizeof(toast_table_file_header)) {
        ans->dead_space_available[0] |= (1 << 7);
        return ans ;
    }
    memcpy(ans, dat, sizeof(toast_table_file_header));
    ans->map_out_toast = NULL ;
    return ans ;
}

void write_toast_table_file_header(toast_table_file_header *h, const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, toast_header_filename);
    FILE *file = fopen(path, "wb");
    if (file == NULL){
        return ;
    }
    toast_table_file_header tmp = *h ;
    tmp.map_out_toast = NULL ;
    fwrite(&tmp, sizeof(tmp), 1, file);
    fclose(file);
}

void write_toast(toast * t, long offset, const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, toast_map_filename);
    FILE *file = fopen(path , "r+b");
    if (file == NULL){
         file = fopen(path, "w+b") ;
    }
    if (file == NULL) {
        return ;
    }
    fseek(file, offset, SEEK_SET);
    fwrite(t, sizeof(toast), 1, file);
    fclose(file);
}

void write_toast_page(long file_offset, page *pg, const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s", table, toast_filename);
    FILE *fp = fopen(path , "r+b");
    if (!fp) {
        fp = fopen(path , "w+b");
    }
    if (!fp ){
        return ;
    }
    fseek(fp, file_offset, SEEK_SET);
    fwrite(pg, sizeof(page), 1, fp);
    fclose(fp);
}

page *get_toast_page_from_file(long offset , const char *table) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s" , table , toast_filename);
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL ;
    }
    page *ans = malloc(sizeof(page));
    if (ans == NULL) {
        fclose(file);
        return NULL ;
    }
    fseek(file, offset, SEEK_SET);
    size_t bytes_read = fread(ans , 1 , sizeof(page) , file);
    fclose(file); 
    if (bytes_read != sizeof(page)) {
        free(ans) ;
        return NULL ;
    }
    return ans;
}

void put_the_chunk_in_the_list(toast_table_file_header * toast_header , int  chunk_id , int page_num , int slot_num , table_info * table ){
    toast_header->num_of_lines_in_map++ ;
    toast_header->map_out_toast[chunk_id].chunk_id = chunk_id ;
    toast_header->map_out_toast[chunk_id].offset_of_the_same = realloc(toast_header->map_out_toast[chunk_id].offset_of_the_same, (toast_header->map_out_toast[chunk_id].offset_number + 1) * sizeof(int));
    toast_header->map_out_toast[chunk_id].offset_of_the_same[ toast_header->map_out_toast[chunk_id].offset_number++ ] = toast_header->num_of_lines_in_map ;
    toast * tst = calloc(1 , sizeof(toast));
    tst->chunk_id = chunk_id ;
    tst->page = page_num ;
    tst->slot_number = slot_num ;
    write_toast( tst ,  (long)(toast_header->num_of_lines_in_map - 1) * sizeof(toast)  , table->table_name  ) ;
    free(tst);
}


char *toast_tables_insert(char *data ,  table_info *table) {
    toast_table_file_header *toast_header = get_toast_table_file_header(table->table_name) ;
    if (toast_header == NULL) {
        return NULL;
    }
    int a = 0 ;
    int size = strlen(data) ;
    int offset = 0 ;
    int chunk_id = toast_header->total_num_of_chunks + 1;
    toast_header->total_num_of_chunks = chunk_id;
    toast_header->map_out_toast = calloc(chunk_id + 1 , sizeof(map_out_toast));
    while (size > 0) {
        if (toast_header->dead_space > 0) {
            int page_num = -1;
            int first = 0;
            int second = 0;
            a = 0 ;
            while (a < 128) {
                uint8_t temp = toast_header->dead_space_available[a];
                for (int j = 7; j >= 0; j--) {
                    int bit = (temp >> j) & 1;
                    if (bit == 0) {
                        page_num = a * 8 + (7 - j);
                        first = a;
                        second = 7 - j;
                        break;
                    }
                }
                if (page_num != -1) {
                    break;
                }
                else {
                    a++;
                }
            }
            if (page_num > toast_header->pages_used){
                page_num = -1;
            }
            if (page_num == -1) {
                toast_header->dead_space = 0;
            }
            else {
                page *pg = get_toast_page_from_file(page_num * 4096L , table->table_name );
                if (pg == NULL) {
                    toast_header->dead_space_available[first] |= (1 << (7 - second)) ;
                    continue;
                }
                if (pg->header.num >= max_no_of_slots) {
                    toast_header->dead_space_available[first] |= (1 << (7 - second)) ;
                    free(pg);
                    continue;
                }
                int wonder = 0;
                int temp_slot_num = (pg->header.num + 7) / 8;
                uint8_t temp;
                for (int i = 0; i < temp_slot_num; i++) {
                    temp = pg->header.dead_slots[i];
                    for (int j = 7; j >= 0; j--) {
                        int bit = (temp >> j) & 1 ;
                        if (bit == 1) {
                            wonder = wonder + pg->slot[i * 8 + (7 - j)].size ;
                        }
                    }
                }
                int b = 0;
                int temp_pointer = 0 ;
                while (b < pg->header.num) {
                    int first = b / 8 ;
                    int second = 7 - (b % 8) ;
                    if (((pg->header.dead_slots[first] >> second) & 1) == 1) {
                        if (pg->slot[b].offset != temp_pointer) {
                            memmove(pg->data + temp_pointer , pg->data + pg->slot[b].offset, pg->slot[b].size);
                        }
                        pg->slot[b].offset = temp_pointer ;
                        temp_pointer = temp_pointer + pg->slot[b].size ;
                        b++ ;
                    }
                    else {
                        b++ ;
                        continue ;
                    }
                }
                pg->header.free_size = use_page_size - wonder;
                if (pg->header.free_size < 0) pg->header.free_size = 0;
                pg->header.num++;
                pg->slot[pg->header.num - 1].offset = wonder;

                if (size > pg->header.free_size && size - pg->header.free_size < 48 && wonder + size <= (int)data_size) {
                    memcpy(pg->data + wonder, data + offset, size);
                    pg->slot[pg->header.num - 1].size = size;
                    toast_header->dead_space = toast_header->dead_space - pg->header.free_size ;
                    pg->header.free_size = 0 ;
                    size = 0 ;
                    toast_header->dead_space_available[first] |= (1 << (7 - second)) ;
                    pg->header.dead_slots[(pg->header.num - 1) / 8] |= (1 << (7 - ((pg->header.num - 1) % 8)));
                    put_the_chunk_in_the_list(toast_header, chunk_id, page_num, pg->header.num - 1 , table );
                }
                else if (size > pg->header.free_size) {
                    memcpy(pg->data + wonder, data + offset, pg->header.free_size);
                    size = size - pg->header.free_size;
                    pg->slot[pg->header.num - 1].size = pg->header.free_size;
                    offset = offset + pg->header.free_size;
                    toast_header->dead_space = toast_header->dead_space - pg->header.free_size;
                    pg->header.free_size = 0;
                    int temo = 7 - second;
                    toast_header->dead_space_available[first] |= (1 << temo);
                    pg->header.dead_slots[(pg->header.num - 1) / 8] |= (1 << (7 - ((pg->header.num - 1) % 8))) ;
                    put_the_chunk_in_the_list(toast_header , chunk_id , page_num , pg->header.num - 1 , table ) ;
                }
                else {
                    memcpy(pg->data + wonder, data + offset, size);
                    pg->slot[pg->header.num - 1].size = size;
                    pg->header.free_size = pg->header.free_size - size;
                    toast_header->dead_space = toast_header->dead_space - size;
                    size = 0;
                    pg->header.dead_slots[(pg->header.num - 1) / 8] |= (1 << (7 - ((pg->header.num - 1) % 8)));
                    if (pg->header.free_size < 612) {
                        int bit_pos = 7 - second;
                        toast_header->dead_space_available[first] |= (1 << bit_pos);
                    }
                    put_the_chunk_in_the_list(toast_header, chunk_id, page_num, pg->header.num - 1 , table);
                }
                write_toast_page(page_num * 4096, pg , table->table_name);
                free(pg);
            }
        }
        else {
            toast_header->pages_used++;
            if (size > use_page_size) {
                page *pg = calloc(1, sizeof(page));
                pg->header.page_num = toast_header->pages_used;
                pg->header.page_type = toast_page;
                pg->header.num = 0;
                pg->slot[pg->header.num].offset = 0;
                pg->slot[pg->header.num].size = use_page_size;
                pg->header.num++;
                memcpy(pg->data, data + offset, use_page_size);
                offset = offset + use_page_size;
                size = size - use_page_size;
                pg->header.free_size = 0;
                int temp_slot_num = toast_header->pages_used / 8;
                toast_header->dead_space_available[temp_slot_num] |= (1 << (7 - (toast_header->pages_used % 8)));
                temp_slot_num = pg->header.num / 8;
                pg->header.dead_slots[temp_slot_num] |= (1 << (7 - ((pg->header.num - 1) % 8)));
                put_the_chunk_in_the_list(toast_header, chunk_id, pg->header.page_num, 0 , table );
                write_toast_page(toast_header->pages_used * 4096, pg , table->table_name);
                free(pg);
            }
            else {
                page *pg = calloc(1, sizeof(page));
                pg->header.page_num = toast_header->pages_used;
                pg->header.page_type = toast_page ;
                pg->header.num = 0;
                pg->slot[pg->header.num].offset = 0;
                pg->slot[pg->header.num].size = size ;
                pg->header.num++;
                memcpy(pg->data , data + offset , size) ;
                if (use_page_size - size >= 612) {
                    toast_header->dead_space = toast_header->dead_space + (use_page_size - size);
                    pg->header.free_size = pg->header.free_size + (use_page_size - size);
                    int temp_slot_num = toast_header->pages_used / 8;
                    toast_header->dead_space_available[temp_slot_num] &= ~(1 << (7 - (toast_header->pages_used % 8)));
                    temp_slot_num = pg->header.num / 8;
                    pg->header.dead_slots[temp_slot_num] |= (1 << (7 - ((pg->header.num - 1) % 8)));
                }
                else {
                    pg->header.free_size = pg->header.free_size + (use_page_size - size);
                    int temp_slot_num = toast_header->pages_used / 8;
                    toast_header->dead_space_available[temp_slot_num] |= (1 << (7 - (toast_header->pages_used % 8)));
                    temp_slot_num = pg->header.num / 8;
                    pg->header.dead_slots[temp_slot_num] |= (1 << (7 - ((pg->header.num - 1) % 8)));
                }
                size = 0;
                put_the_chunk_in_the_list(toast_header, chunk_id, pg->header.page_num, 0 , table );
                write_toast_page(toast_header->pages_used * 4096, pg , table->table_name );
                free(pg);
            }
        }
    }
    write_toast_table_file_header(toast_header , table->table_name);
    for (int i = 0; i <= chunk_id; i++){
        free(toast_header->map_out_toast[i].offset_of_the_same);
    }
    free(toast_header->map_out_toast);
    free(toast_header);
    char *ans = malloc(sizeof(int));
    memcpy(ans, &chunk_id, sizeof(int));
    return ans;
}
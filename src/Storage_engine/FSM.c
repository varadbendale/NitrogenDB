const int number_of_pages 0 ; 

page *get_page_from_file(int offset) {
    FILE *file = fopen(/*"name"*/, "rb");
    if (file == NULL) {
        return NULL ; 
    }
    fseek(file, offset, SEEK_SET);
    page *ans = malloc(sizeof(page));
    if (ans == NULL) { 
        fclose(file); 
        return NULL; 
    }
    size_t bytes_read = fread(ans, 1, sizeof(page), file);
    fclose(file);
    if (bytes_read == 0) { 
        free(ans) ; 
        return NULL ; }
    return ans;
}

void write_page_to_file(page *p, int offset ){
    FILE *file = fopen(filename, "r+b");
    if (file == NULL) {
        file = fopen(filename, "w+b");
        if (file == NULL){
            return ; 
        }
    }
    if (offset == -1 ){
        offset = lseek(fd, 0, SEEK_END);
    }
    fseek(file, offset, SEEK_SET);
    fwrite(p, 1, sizeof(page), file);
    fclose(file);
}

int find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(char *data, table_info *table, char **ans, float *pk) {
    int size = 0;
    int i = 0;
    int j = 0;

    while (i < table.num_columns) {
        if (table.columns[i].primary_key == true) {
            *pk = (float)atof(data);
        }
        if (table.columns[i].col_type == INT || table.columns[i].col_type == BIGINT || table.columns[i].col_type == SMALLINT || table.columns[i].col_type == TINYINT) {
            size = size + (table.columns[i].type_bits_count / 8 );
            ans[j] = strdup(integer_num);
            j++;
            ans[j] = strdup(table.columns[i].col_name);
            j++;
        }
        else if (table.columns[i].col_type == FLOAT || table.columns[i].col_type == DOUBLE) {
            size = size + (table.columns[i].type_bits_count / 8 );
            ans[j] = strdup(real_num);
            j++;
            ans[j] = strdup(table.columns[i].col_name);
            j++;
        }
        else if (table.columns[i].col_type == CHAR || table.columns[i].col_type == VARCHAR) {
            if (strlen(table.columns[i].col_name) > 1024) {
                ans[j] = strdup(toast);
                j++;
                ans[j] = strdup(toast_tables_insert(table.columns[i].col_name)) ; 
                j++ ; 
                size = size + sizeof(ans[j-1]);
            }
            else {
                ans[j] = strdup(string_num);
                j++;
                char len_of_stuff[32];
                snprintf(len_buf, sizeof(len_of_stuff), "%zu", strlen(table->columns[i].col_name));
                ans[j] = strdup(len_of_stuff);
                j++;
                ans[j] = strdup(table->columns[i].col_name);
                size = size + strlen(table->columns[i].col_name);
                j++;
            }
        }
        else if (table.columns[i].col_type == DATE) {
            size = size + (table.columns[i].type_bits_count / 8 );
            ans[j] = strdup(date_num);
            j++;
            ans[j] = strdup(table.columns[i].col_name);
            j++;
        }
        else if (table.columns[i].col_type == TIMESTAMP) {
            size = size +  (table.columns[i].type_bits_count / 8 );
            ans[j] = strdup(timestamp_num);
            j++;
            ans[j] = strdup(table.columns[i].col_name);
            j++;
        }
        i++;
    }
    return size;
}


pagezero *get_pagezero(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }
    char dat[4096];
    size_t bytes_read = fread(dat, 1, 4096, file);
    fclose(file);
    if (bytes_read == 0) {
        return NULL;
    }
    pagezero *ans = malloc(sizeof(pagezero));
    if (ans == NULL) {
        return NULL;
    }
    memcpy(ans, dat, sizeof(pagezero));
    return ans;
}


/*
    int slots_figured_out(page *pg, float pk) {
        int low = 0;
        int high = pg->slot_num;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (pk < pg->slot[mid].pk) {
                high = mid - 1;
            }
            else if (pk == pg->slot[mid].pk) {
                return NULL;
            }
            else {
                low = mid + 1;
            }
        }
        return low;
    }
*/


void FSM_insert (char *insert_data, int page_num, int offset, table_info *table  ) {
    page *pg;
    char *ans;
    int size = 0;
    float *primary_key = 0;
    int first = 0;
    uint8_t temp;
    if (offset > 0) {
        pg = get_page_from_file(offset);
        if (pg == NULL) {
            return;
        }
        if (pg->header.page_num != page_num) {
            return;
        }
        size = find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(data, table, &ans, &primary_key);
        size = size + sizeof(slots);
        if (size < pg->header.free_size) {
            if (size <= pg->header.normal_free_size) {
                memcpy(pg->data + pg->header.current_offset, ans, size - sizeof(slots));
                pg->header.free_size = pg->header.free_size - size;
                pg->header.normal_free_size = pg->header.normal_free_size - size;
                pg->slot[pg->header.num].size = size - sizeof(slots);
                pg->slot[pg->header.num].offset = pg->header.current_offset;
                pg->slot[pg->header.num].pk = *primary_key;
                pg->header.current_offset += size - sizeof(slots);
                int bit_pos = 7 - (pg->header.num % 8);
                pg->header.dead_slots[pg->header.num / 8] |= (1 << bit_pos);
                pg->header.num++;
                pg->header.page_type = normal;
            }
            else {
                int i = 0;
                int j = 0;
                int k = 0;
                int where_to_put = -1;
                int temp_slot_num = pg->header.num / 8;
                int temp_mod = pg->header.num % 8;
                while (i < temp_slot_num) {
                    temp = pg->header.dead_slots[i];
                    for (int j = 7; j >= 0; j--) {
                        int bit = (temp >> j) & 1;
                        if (bit == 0) {
                            where_to_put = (i * 8) + j;
                            k = i;
                            if (size - sizeof(slots) > pg->slot[where_to_put].size) {
                                where_to_put = -1;
                            }
                            break;
                        }
                    }
                    if (where_to_put != -1) {
                        break;
                    }
                    i++;
                }
                if (where_to_put != -1) {
                    pg->header.free_size = pg->header.free_size + pg->slot[where_to_put].size - (size - sizeof(slots));
                    pg->slot[where_to_put].size = size - sizeof(slots);
                    pg->slot[where_to_put].pk = primary_key;
                    memcpy(pg->data + pg->slot[where_to_put].offset, ans, size);
                    int index = temp_mod;
                    int bit_pos = 7 - index;
                    temp |= (1 << bit_pos);
                    pg->header.dead_slots[k] = temp;
                    pg->header.page_type = normal;
                }
                else {
                    int a = 0;
                    int temp_pointer = 0;
                    while (a < pg->header.num) {
                        int first = a / 8;
                        int second = 7 - (a % 8);
                        if ((pg->header.dead_slots[first] >> second) & 1 == 1) {
                            if (pg->slot[a].offset != temp_pointer) {
                                memmove(pg->data + temp_pointer, pg->data + pg->slot[a].offset, pg->slot[a].size);
                                pg->slot[a].offset = temp_pointer ; 
                            }
                            temp_pointer = temp_pointer + pg->slot[a].size;
                            a++;
                        }
                        else {
                            a++;
                            continue;
                        }
                    }
                    pg->header.current_offset = temp_pointer;
                    pg->header.normal_free_size = pg->header.free_size;
                    memcpy(pg->data + pg->header.current_offset, ans, size - sizeof(slots));
                    pg->slot[pg->header.num].size = size - sizeof(slots);
                    pg->slot[pg->header.num].offset = pg->header.current_offset;
                    pg->slot[pg->header.num].pk = *primary_key;
                    pg->header.current_offset += size - sizeof(slots);
                    pg->header.free_size = pg->header.free_size - size;
                    pg->header.normal_free_size = pg->header.normal_free_size - size;
                    int temp_slot_num = pg->header.num / 8;
                    pg->header.dead_slots[temp_slot_num] = (pg->header.dead_slots[temp_slot_num] << 1) | 1;
                    pg->header.num++;
                    pg->header.page_type = normal;
                }
            }
        }
    }
        else {
            number_of_pages++ ; 
            pg = malloc(sizeof(page)) ; 
            pg->header.page_num = number_of_pages ; 
            pg->header.page_type = normal ; 
            pg->header.num = 0 ; 
            pg->header.toast_table_num = 0  ; 
            pg->header.dead_slots[128] = {0} ;
            pg->header.current_offset = 0  ; 
            size = find_the_size_of_stuff_get_the_data_and_get_the_primary_key_num(data, table, &ans, &primary_key);
            pg->slot[pg->header.num].offset = 0 ; 
            pg->slot[pg->header.num].size = size ;  
            pg->slot[pg->header.num].pk = *primary_key ; 
            memcpy(pg->data , ans , size );
            pg->header.current_offset = size ;
            pg->header.dead_slots[0] |= (1 << 7);
            pg->header.free_size = use_page_size - (size + sizeof(slots)) ; 
            pg->header.normal_free_size = use_page_size - (size + sizeof(slots)) ; 
            pg->header.num++ ; 
            pg->buffer_space = PAGE_SIZE - use_page_size - sizeof(page) ; 
        }
        FILE *file = fopen(filename, "r+b");
        if (file != NULL) {
            fseek(file, offset, SEEK_SET);
            fwrite(pg, 1, sizeof(page), file);
            fclose(file);
        }
    free(pg);
}


void fsm_update(page *pg , char * update_data , int slot_num ){
    int req_space = sizeof(update_data) - pg->slot[slot_num].size ; 
    FILE *file = fopen("normal_file", "r+b");
    if (req_space < (pg->header.buffer_space + pg->header.free_size )){
        if (req_space < pg->header.buffer_space + pg->header.normal_free_size){
            memmove( pg->data + pg->slot[slot_num].offset + sizeof(update_data), pg->data + pg->slot[slot_num].offset + pg->slot[slot_num].size, pg->header.current_offset - (pg->slot[slot_num].offset + pg->slot[slot_num].size));
            memcpy( pg->data + pg->slot[slot_num].offset, update_data , sizeof(update_data) ) ; 
            pg->slot[slot_num].size  = sizeof(update_data) ; 
            if (slot_num != pg->header.num){
                for ( int l = slot_num + 1  ; l < pg->header.num ; l++ ){
                    pg->slot[l].offset = pg->slot[l].offset + req_space ; 
                }
            }
            if (req_space > pg->normal_free_size ){
                req_space = req_space - pg->header.normal_free_size ; 
                pg->header.free_size = pg->header.free_size - pg->header.normal_free_size ; 
                pg->header.normal_free_size  = 0 ; 
                pg->header.buffer_space = pg->header.buffer_space - req_space ; 
                pg->header.current_offset += req_space ;
                if (file != NULL) {
                    fseek(file, offset, SEEK_SET);
                    fwrite(pg, 1, sizeof(page), file);
                    fclose(file);
                }
                free(pg);
                return  ; 
            }
            else { 
                pg->header.normal_free_size = pg->header.normal_free_size - req_space  ; 
                pg->header.free_size = pg->header.free_size - req_space ; 
                pg->header.current_offset += req_space ;
                if (file != NULL) {
                    fseek(file, offset, SEEK_SET);
                    fwrite(pg, 1, sizeof(page), file);
                    fclose(file);
                }
            }
        }
        else {
            int a = 0;
            int temp_pointer = 0;
            if ( slot_num == pg->header.num ){
                while (a < pg->header.num) {
                    int first = a / 8;
                    int second = 7 - (a % 8);
                    if ((pg->header.dead_slots[first] >> second) & 1 == 1) {
                        if (pg->slot[a].offset != temp_pointer) {
                            memmove(pg->data + temp_pointer, pg->data + pg->slot[a].offset, pg->slot[a].size);
                            pg->slot[a].offset = temp_pointer ; 
                        }
                        temp_pointer = temp_pointer + pg->slot[a].size;
                        a++;
                    }
                    else {
                        a++;
                        continue;
                    }
                }
                pg->header.current_offset = temp_pointer;
                pg->header.normal_free_size = pg->header.free_size;
                memmove( pg->data + pg->slot[slot_num].offset + sizeof(update_data), pg->data + pg->slot[slot_num].offset + pg->slot[slot_num].size, pg->header.current_offset - (pg->slot[slot_num].offset + pg->slot[slot_num].size));
                memcpy( pg->data + pg->slot[slot_num].offset, update_data , sizeof(update_data) ) ; 
                pg->slot[slot_num].size  = sizeof(update_data) ; 
                if (req_space > pg->normal_free_size ){
                    req_space = req_space - pg->header.normal_free_size ; 
                    pg->header.free_size = pg->header.free_size - pg->header.normal_free_size ; 
                    pg->header.normal_free_size  = 0 ; 
                    pg->header.buffer_space = pg->header.buffer_space - req_space ; 
                    pg->header.current_offset += req_space ;
                    if (file != NULL) {
                        fseek(file, offset, SEEK_SET);
                        fwrite(pg, 1, sizeof(page), file);
                        fclose(file);
                    }
                    free(pg);
                    return  ; 
                }
                else {  
                    pg->header.normal_free_size = pg->header.normal_free_size - req_space  ; 
                    pg->header.free_size = pg->header.free_size - req_space ; 
                    pg->header.current_offset += req_space ;
                    if (file != NULL) {
                        fseek(file, offset, SEEK_SET);
                        fwrite(pg, 1, sizeof(page), file);
                        fclose(file);
                    }
                    free(pg);
                    return  ; 
                }

            }
            else { 
                int the_slot_we_want = -1 ; 
                int bummer = -1 ; 
                while (a < pg->header.num) {
                    int first = a / 8;
                    int second = 7 - (a % 8);
                    if ((pg->header.dead_slots[first] >> second) & 1 == 1) {
                        if (the_slot_we_want == -1 ){
                            if (pg->slot[a].offset != temp_pointer) {
                                memmove(pg->data + temp_pointer, pg->data + pg->slot[a].offset, pg->slot[a].size);
                                pg->slot[a].offset = temp_pointer ; 
                            }
                        }
                        else {
                            if (pg->slot[a].offset != temp_pointer) {
                                memmove(pg->data + temp_pointer + req_space , pg->data + pg->slot[a].offset, pg->slot[a].size);
                                bummer = 1 ; 
                                pg->slot[a].offset = temp_pointer ; 
                            }
                        }

                        if (a == slot_num){
                            the_slot_we_want = 1 ; 
                        }
                        temp_pointer = temp_pointer + pg->slot[a].size ;
                        a++;
                    }
                    else {
                        a++;
                        continue;
                    }
                }
                pg->header.current_offset = temp_pointer;
                pg->header.normal_free_size = pg->header.free_size;
                if (bummer == 1 ){
                    memcpy( pg->data + pg->slot[slot_num].offset, update_data , sizeof(update_data) ) ; 
                    pg->slot[slot_num].size  = sizeof(update_data) ; 
                    if (req_space > pg->normal_free_size ){
                        req_space = req_space - pg->header.normal_free_size ; 
                        pg->header.free_size = pg->header.free_size - pg->header.normal_free_size ; 
                        pg->header.normal_free_size  = 0 ; 
                        pg->header.buffer_space = pg->header.buffer_space - req_space ; 
                        pg->header.current_offset += req_space ;
                        if (file != NULL) {
                            fseek(file, offset, SEEK_SET);
                            fwrite(pg, 1, sizeof(page), file);
                            fclose(file);
                        }
                        free(pg);
                        return  ; 
                    }
                    else { 
                        memmove( pg->data + pg->slot[slot_num].offset + sizeof(update_data), pg->data + pg->slot[slot_num].offset + pg->slot[slot_num].size, pg->header.current_offset - (pg->slot[slot_num].offset + pg->slot[slot_num].size));
                        memcpy( pg->data + pg->slot[slot_num].offset, update_data , sizeof(update_data) ) ; 
                        pg->header.normal_free_size = pg->header.normal_free_size - req_space  ; 
                        pg->header.free_size = pg->header.free_size - req_space ; 
                        pg->header.current_offset += req_space ;
                        if (file != NULL) {
                            fseek(file, offset, SEEK_SET);
                            fwrite(pg, 1, sizeof(page), file);
                            fclose(file);
                        }
                    }
                }
            }
        }
    }
    else{
        number_of_pages++ ; 
        page * second_page = malloc(sizeof(page))
        second_page->header.page_num = number_of_pages ; 
        second_page->header.page_type = normal ; 
        second_page->header.num = 0 ; 
        second_page->header.toast_table_num = 0  ; 
        second_page->header.dead_slots[128] = {0} ;
        second_page->header.current_offset = 0  ; 
        second_page->header.free_size  = use_page_size ; 
        second_page->header.normal_free_size  = use_page_size ; 
        second_page->buffer_space = PAGE_SIZE - use_page_size - sizeof(page) ; 
        int temp  = pg->header.num / 2 ; 
        int for_me = 0 ; 
        int temp_offset = 0 ; 
        int size_reduced = 0 ; 
        for (int f = temp ; f < pg->header.num  ; f++  ){
            second_page->slot[for_me].size = pg->slot[f].size ; 
            second_page->slot[for_me].pk = pg->slot[f].pk ;
            second_page->slot[for_me].offset = temp_offset ; 
            memcpy(second_page->data + temp_offset , pg->data[pg->slot[f].offset] , second_page->slot[for_me].size  ) ; 
            temp_offset = temp_offset + pg->slot[f].size ; 
            second_page->header.dead_slots[for_me / 8] |= (1 << (7 - (for_me % 8)));
            second_page->header.num++ ; 
            int temp_size = pg->slot[f].size ; 
            second_page->header.free_size = second_page->header.free_size - temp_size ; 
            second_page->header.normal_free_size = second_page->header.normal_free_size - temp_size ; 
            second_page->header.current_offset = temp_offset ; 
            if (pg->header.buffer_space < PAGE_SIZE - use_page_size - sizeof(page) ){
                if (temp_size > ((PAGE_SIZE - use_page_size - sizeof(page) ) -  pg->header.buffer_space ) ){
                    pg->header.buffer_space = (PAGE_SIZE - use_page_size - sizeof(page) ) ; 
                    temp_size = temp_size - (((PAGE_SIZE - use_page_size - sizeof(page) ) -  pg->header.buffer_space ) ) ; 
                    pg->header.free_size = pg->header.free_size + (temp_size) ;
                }
                else {
                    pg->header.buffer_space = (PAGE_SIZE - use_page_size - sizeof(page) ) - temp_size  ; 
                }
            }
            else { 
                pg->header.free_size = pg->header.free_size + (temp_size) ;
            }
            int bit_pos = 7 - (f % 8 ); 
            pg->header.dead_slots[f / 8] |= (1 << bit_pos);
            for_me++ ; 
            size_reduced = size_reduced + pg->slot[f].size ; 
        }
        pg->header.normal_free_size = pg->header.normal_free_size + size_reduced ; 
        pg->header.current_offset = pg->slot[temp].offset ; 
        if (temp <= slot_num){
            int something  = slot_num - temp ; 
            memmove( second_page->data + second_page->slot[something].offset + sizeof(update_data), second_page->data + second_page->slot[something].offset + second_page->slot[something].size, second_page->header.current_offset - (second_page->slot[something].offset + second_page->slot[something].size));
            memcpy( second_page->data + second_page->slot[something].offset, update_data , sizeof(update_data) ) ; 
            second_page->slot[something].size  = sizeof(update_data) ; 
            if (req_space > second_page->normal_free_size ){
                req_space = req_space - second_page->header.normal_free_size ; 
                second_page->header.free_size = second_page->header.free_size - second_page->header.normal_free_size ; 
                second_page->header.normal_free_size  = 0 ; 
                second_page->header.buffer_space = second_page->header.buffer_space - req_space ; 
                second_page->header.current_offset += req_space ;
            }
            else {  
                second_page->header.normal_free_size = second_page->header.normal_free_size - req_space  ; 
                second_page->header.free_size = second_page->header.free_size - req_space ; 
                second_page->header.current_offset += req_space ;
            }
        }
        else {
            memmove( pg->data + pg->slot[slot_num].offset + sizeof(update_data), pg->data + pg->slot[slot_num].offset + pg->slot[slot_num].size, pg->header.current_offset - (pg->slot[slot_num].offset + pg->slot[slot_num].size));
            memcpy( pg->data + pg->slot[slot_num].offset, update_data , sizeof(update_data) ) ; 
            pg->slot[slot_num].size  = sizeof(update_data) ; 
            if (req_space > pg->normal_free_size ){
                req_space = req_space - pg->header.normal_free_size ; 
                pg->header.free_size = pg->header.free_size - pg->header.normal_free_size ; 
                pg->header.normal_free_size  = 0 ; 
                pg->header.buffer_space = pg->header.buffer_space - req_space ; 
                pg->header.current_offset += req_space ;
            }
            else {  
                pg->header.normal_free_size = pg->header.normal_free_size - req_space  ; 
                pg->header.free_size = pg->header.free_size - req_space ; 
                pg->header.current_offset += req_space ;
            }
        }
        if (file != NULL) {
            fseek(file, offset, SEEK_SET);
            fwrite(pg, 1, sizeof(page), file);
            fseek(file, 0, SEEK_END);
            fwrite(second_page, 1, sizeof(page), file);
            fclose(file);
        }

    }
}
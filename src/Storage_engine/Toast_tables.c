toast_table_file_header *get_toast_table_file_header() {
    FILE *file = fopen(/*filename*/, "rb");
    if (file == NULL) {
        return NULL;
    }
    char dat[4096];
    size_t bytes_read = fread(dat, 1, 4096, file);
    fclose(file);
    if (bytes_read == 0) {
        return NULL;
    }
    toast_table_file_header *ans = malloc(sizeof(toast_table_file_header));
    if (ans == NULL) {
        return NULL;
    }
    memcpy(ans, dat, sizeof(toast_table_file_header));
    return ans;
}


void write_toast(toast * t, long offset) {
    FILE *file = fopen(/*filename*/, "r+b");
    if (file == NULL) {
        return ;
    }
    fseek(file, offset, SEEK_SET);
    fwrite(t, sizeof(toast), 1, file);
    fclose(file); 
}

void write_toast_page(long file_offset, page *pg) {
    FILE *fp = fopen(/*filename*/, "r+b");
    if (!fp) fp = fopen(/*filename*/, "w+b");
    fseek(fp, file_offset, SEEK_SET);
    fwrite(pg, sizeof(page), 1, fp);
    fclose(fp);
}


void put_the_chunk_in_the_list(toast_table_file_header * toast_header , int  chunk_id , int page_num , int slot_num){
    toast_header->num_of_lines_in_map++ ; 
    toast_header->map_out_toast[chunk_id].chunk_id = chunk_id ; 
    toast_header->map_out_toast[chunk_id].offset_of_the_same[ toast_header->map_out_toast[chunk_id].offset_number++ ] = toast_header->num_of_lines_in_map ; 
    toast * tst = malloc(sizeof(toast)); 
    tst->chunk_id = chunk_id ; 
    tst->page = page_num ; 
    tst->slot_number = slot_num ; 
    write_toast( tst , toast_header->num_of_lines_in_map  ) ; 
    free(tst); 
}




char *toast_tables_insert(char *data) {
    toast_table_file_header *toast_header = get_toast_table_file_header() ;
    int a = 0 ;
    int size = strlen(data) ;
    int offset = 0 ;
    int chunk_id = toast_header->total_num_of_chunks + 1;
    while (size > 0) {
        if (toast_header->dead_space > 0) {
            int page_num = -1;
            int first = 0;
            int second = 0;
            while (a < 128) {
                uint8_t temp = toast_header->dead_slots[a];
                for (int j = 7; j >= 0; j--) {
                    int bit = (temp >> j) & 1;
                    if (bit == 0) {
                        page_num = a * 8 + j;
                        first = a;
                        second = j;
                        break;
                    }
                }
                if (page_num != -1) {
                    break;
                } else {
                    a++;
                }
            }
            if (page_num == -1) {
                // error
            } 
            else {
                page *pg = get_page_from_file(page_num * 4096);
                int wonder = 0;
                int temp_slot_num = pg->header.num / 8;
                uint8_t temp;
                for (int i = 0; i < temp_slot_num; i++) {
                    temp = pg->header.dead_slots[i];
                    for (int j = 7; j >= 0; j--) {
                        int bit = (temp >> j) & 1 ;
                        if (bit == 1) {
                            pg->slot[i * 8 + j].offset = wonder ;
                            wonder = wonder + pg->slot[i * 8 + j].size ;
                        }
                    }
                }
                int b = 0;
                int temp_pointer = 0 ;
                while (b < pg->header.num) {
                    int first = b / 8 ;
                    int second = 7 - (b % 8) ;
                    if ((pg->header.dead_slots[first] >> second) & 1 == 1) {
                        if (pg->slot[b].offset != temp_pointer) {
                            memmove(pg->data + temp_pointer, pg->data + pg->slot[b].offset, pg->slot[b].size);
                        }
                        temp_pointer = temp_pointer + pg->slot[b].size;
                        b++;
                    } else {
                        b++;
                        continue;
                    }
                }
                pg->header.free_size = pg->header.normal_free_size;
                pg->header.num++;
                pg->slot[pg->header.num - 1].offset = wonder;
                if (size + sizeof(slots) > pg->header.free_size) {
                    if (size + sizeof(slots) - pg->header.free_size < 48) {
                        memcpy(pg->data + wonder, data + offset, size);
                        pg->slot[pg->header.num - 1].size = size;
                        pg->header.free_size = pg->header.free_size - size;
                        toast_header->dead_space = toast_header->dead_space - size;
                        size = 0;
                        pg->header.dead_slots[(pg->header.num - 1) / 8] |= (1 << 7 - ((pg->header.num - 1) % 8));
                        if (pg->header.free_size < 612) {
                            int bit_pos = 7 - second;
                            toast_header->dead_space_available[first] |= (1 << bit_pos);
                        }
                        put_the_chunk_in_the_list(toast_header, chunk_id, page_num, pg->header.num - 1);
                    }
                    else {
                        memcpy(pg->data + wonder, data + offset, pg->header.free_size);
                        size = size - pg->header.free_size;
                        pg->slot[pg->header.num - 1].size = pg->header.free_size;
                        offset = offset + pg->header.free_size;
                        toast_header->dead_space = toast_header->dead_space - pg->header.free_size;
                        pg->header.free_size = 0;
                        int temo = 7 - second;
                        toast_header->dead_space_available[first] |= (1 << temo);
                        put_the_chunk_in_the_list(toast_header, chunk_id, page_num, pg->header.num);
                    }
                } 
                else {
                    memcpy(pg->data + wonder, data + offset, size);
                    pg->slot[pg->header.num - 1].size = size;
                    pg->header.free_size = pg->header.free_size - size;
                    toast_header->dead_space = toast_header->dead_space - size;
                    size = 0;
                    pg->header.dead_slots[(pg->header.num - 1) / 8] |= (1 << 7 - ((pg->header.num - 1) % 8));
                    if (pg->header.free_size < 612) {
                        int bit_pos = 7 - second;
                        toast_header->dead_space_available[first] |= (1 << bit_pos);
                    }
                    put_the_chunk_in_the_list(toast_header, chunk_id, page_num, pg->header.num - 1);
                }
                write_toast_page(page_num * 4096, pg);
                free(pg);
            }
        } 
        else {
            toast_header->pages_used++;
            if (size > use_page_size) {
                page *pg = malloc(sizeof(page));
                pg->header.page_num = toast_header->pages_used;
                pg->header.page_type = toast;
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
                put_the_chunk_in_the_list(toast_header, chunk_id, pg->header.page_num, 0);
                write_toast_page(toast_header->pages_used * 4096, pg);
                free(pg);
            }
            else {
                page *pg = malloc(sizeof(page));
                pg->header.page_num = toast_header->pages_used;
                pg->header.page_type = toast;
                pg->header.num = 0;
                pg->slot[pg->header.num].offset = 0;
                pg->slot[pg->header.num].size = 0;
                pg->header.num++;
                memcpy(pg->data, data + offset, size - offset);
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
                put_the_chunk_in_the_list(toast_header, chunk_id, pg->header.page_num, 0);
                write_toast_page(toast_header->pages_used * 4096, pg);
                free(pg);
            }
        }
    }
    char *ans = malloc(sizeof(char));
    memcpy(ans, chunk_id, sizeof(int));
    return ans;
}






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
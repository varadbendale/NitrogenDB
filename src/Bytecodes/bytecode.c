


int count = 0 ; 
sorter[op->p1]->array[count].key = get_data_sort( sorter[op->p1]->array[count] , sorter[op->p1].cols_to_look  ) ; 


int give_primary_number( reg * regist ){
    if ( regist->type == integer_op  ){
        return (float)regist->val.i ; 
    }
    else if (  regist->type == real_num   ){
        return regist->val.r ; 
    }
    else {
        int num = 0 ; 
        int i = 0 ; 
        while ( i < regist->lenght ){
            num = num + regist->val.s[i] ; 
            i++ ; 
        } 
        return float(num) ; 
    }
}


unsigned char * make_record_data(byte * byt , int start , int end ){
    unsigned char *ans[300] ; 
    int pos = 0 ; 
    reg temp = byt->regis[start] ; 
    for ( int i = start ; i <= end ; i++ ){
        temp = byt->regis[i] ; 
        memcpy( ans + pos  , temp->type  , 1   ) ; 
        pos = pos + 1 ; 
        if (temp->type == integer_num ){
            memcpy(ans + pos , temp->val.i , sizeof(int)) ; 
            pos  = pos + sizeof(int) ; 
        }
        else if (temp->type == real_num ){
            memcpy(ans + pos , temp->val.i , sizeof(float)) ; 
            pos  = pos + sizeof(float) ; 
        }
        else if (temp->type == string_num || temp->type == blob_num  ){
            memcpy( ans + pos , temp->lenght , sizeof(int)) ; 
            pos = pos + sizeof(int) ; 
            memcpy(ans + pos , temp->val.s , temp->lenght ) ; 
            pos = pos + temp->lenght  ; 
        }
    }
    return ans ; 
}


void *pager_get_page(Pager *pager, uint32_t page_num) {
    void *buf = malloc(PAGE_SIZE);
    fseek(pager->file, page_num * PAGE_SIZE, SEEK_SET);
    fread(buf, 1, PAGE_SIZE, pager->file);
    return buf;
}


uint32_t float_to_int_bits(float f) {
    if (f == 0.0f) f = 0.0f; 
    uint32_t bits;
    memcpy(&bits, &f, sizeof(f));
    return bits;
}


void hash_bin_which(reg check , int nume ){
    if (check->type == int_num  ){
        return check->val->i % nume
    }
    else if (check->type == real_num ){
        return float_to_int_bits(check->val->r) % nume  ; 
    }
    else if (check->type == blob_num && check->type == string_num  ){
        int i = 0 ; 
        int num = 0 ; 
        while ( i < check->lenght){
            num = num + check->val->s[i] - 'a' ; 
        }
        return num % nume  ; 
    }
}

uint32_t get_child_pointer(Pager * pager ,uint32_t page_num , uint16_t cell_index ){
    uint8_t *raw = pager_get_page(pager , page_num ) ; 
    uint16_t *cell_offset = sizeof(page_header) + ( cell_index * CELL_SIZE ) ; 
    uint32_t *addr  ; 
    memcpy(addr ,raw + cell_offset , sizeof(uint32_t) ) ; 
    return addr ; 
}

void *get_cell(void *node, uint32_t cell_num) {
    char *base = (char*)node;              
    return base + sizeof(page_header) + (cell_num * CELL_SIZE);
}


int bs( void * node , reg target  , int data_type  , int *index  , int (*function_name)(reg, void*, int)){
    int low = 0 ; 
    int high = node->num_cells - 1 ; 
    int cmp = -1  ;
    bool found = false;  
    while ( low <= high ){
        int mid = ( low + high )/ 2 ; 
        void * cell = get_cell(node , mid ) ; 
        reg cell_key = get_key_from_cell(cell);
        cmp = function_name(target , cell_key  , data_type  ) ; 
        if (cmp == 0 ){
            found = true ; 
            if (index != NULL){
                *index = mid ; 
            }
            high = mid -1 ; 
        }
        else { 
            low = mid + 1 ; 
        }
    }
    if ( found == true ){
        return 1 ; 
    }
    return 0 ; 

}

void agg_operation( aggregate *ans  , reg *num ,  char *  operation ){
    if (strcmp(operation , "SUM") == 0 ){
        if (num->type == integer_num ) {
        ans->total =  ans->total + num->val.i ; 
        ans->type = integer_num ; 
        }
        else if (num->type == real_num  ){
        ans->total =  ans->total + num->val.r ;         
        ans->type = real_num ;
        }
        else {
            return  ; 
        }
    }
    else if (strcmp(operation , "AVG") == 0){
        if (num->type == integer_num ) {
        ans->total =  ans->total + num->val.i ; 
        ans->type = integer_num ; 
        }
        else if (num->type == real_num  ){
        ans->total =  ans->total + num->val.r ;    
        ans->type = real_num  ; 
        } 
        else {
            return  ; 
        }
        ans->count = ans->count + 1 ; 
    }
    else if (strcmp(operation , "COUNT") == 0 ){
        ans->count = ans->count + 1 ; 
    }
    else if (strcmp(operation , "MIN") == 0  ){
    if (ans->have_value == false ){
            if (num->type == integer_num){
                ans->min.val.i = num->val.i ; 
                ans->have_value = true ; 
                ans->type = integer_num ; 
            }
            else if (num->type == real_num){
                ans->min.val.r = num->val.r ; 
                ans->have_value = true ; 
                ans->type = real_num ; 
            }
            else {
                return  ; 
            }
    } 
    else {
            if (num->type == integer_num){
                ans->type = integer_num ; 
                if (num->val.i < ans->min.val.i ){
                    ans->min.val.i = num->val.i ; 
                }
            }
            else if (num->type == real_num){
                ans->type = real_num ; 
                if (num->val.r < ans->min.val.r ){
                    ans->min.val.r = num->val.r ; 
                }
            }
            else {
                return  ; 
            }
    }
    }

    else if (strcmp(operation , "MAX") == 0  ){
    if (ans->have_value == false ){
            if (num->type == integer_num){
                ans->max.val.i = num->val.i ; 
                ans->have_value = true ; 
                ans->type = integer_num ; 
            }
            else if (num->type == real_num){
                ans->max.val.r = num->val.r ; 
                ans->have_value = true ; 
                ans->type = real_num ; 
            }
            else {
                return  ; 
            }
    } 
    else {
            if (num->type == integer_num){
                ans->type = integer_num ; 
                if (num->val.i > ans->max.val.i ){
                    ans->max.val.i = num->val.i ; 
                }
            }
            else if (num->type == real_num){
                ans->type = real_num ; 
                if (num->val.r > ans->max.val.r ){
                    ans->max.val.r = num->val.r ; 
                }
            }
            else {
                return  ; 
            }
    }
    }

    else if (strcmp(operation , "GROUP_CONCAT") == 0 ){
        if (num->type  != string_num ){
            return  ; 
        }
        ans->type = string_num ; 
        if ( ans->concat_string == NULL ){
            ans->concat_string = malloc(num->lenght) ; 
            ans->string_len = num->lenght ; 
            memcpy(&ans->concat_string  ,num->val.s  , num->lenght  ) ; 
        }
        else  {
            ans->concat_string = realloc(ans->concat_string  , ans->string_len + num->lenght ) ; 
            strcat(ans->concat_string , num->val.s ) ; 
        }

    }
    return ; 
}

void agg_final(aggregate *ans  , reg *num ,  char *  operation ){
        if (strcmp(operation , "SUM") == 0 ){
            if (ans->type == integer_num ) {
                num->type = integer_num ; 
                num->val.i = ans->total ; 
            }
            else if (ans->type == real_num  ){
                num->type = real_num ; 
                num->val.r = ans->total ;  
            }
            }

        else if (strcmp(operation , "AVG") == 0 ){
            num->type = real_num ; 
            num->val.r = ans->total/ans->count ; 
        }
        
        else if (strcmp(operation , "COUNT") == 0 ){
            num->val.i = ans->count ; 
        }
        else if (strcmp(operation , "MIN") == 0 ){
            if (ans->min.type == integer_num){
                num->val.i = ans->min.val.i ; 
            }
            else if (ans->min.type == real_num){
                num->val.r = ans->min.val.r ; 
            }
        }
        else if (strcmp(operation , "MAX") == 0 ){
            if (ans->max.type == integer_num){
                num->val.i = ans->max.val.i ; 
            }
            else if (ans->max.type == real_num){
                num->val.r = ans->max.val.r ; 
            }
        }
        else if (strcmp(operation , "GROUP_CONCAT") == 0 ){
            num->lenght = ans->string_len ; 
            num->val.s = ans->concat_string  ;    
        }
        return ; 
}

void bytcode(byte *byt){
    byt->prog_count = 0;
    while(1){
        type *op = &byt->prog[byt->prog_count];
        switch(op->operand){
            case begin_op:
                break;

            case goto_op:
                byt->pc = op->p2;
                continue;

            case halt:
                return;

            case check_negative : 
                if (byt->regis[op->p1].type == int_num   ){
                    if (byt->regis[op->p1].val.i < 0 || ){
                        trap_fired = 1 ; 
                    }
                    else { 
                        trap_fired = 0 ;     
                    }
                }
                if ( byt->regis[op->p1].type == real_num){
                    if (byt->regis[op->p1].val.r < 0 || ){
                        trap_fired = 1 ; 
                    }
                    else { 
                        trap_fired = 0 ;     
                    }
                }
                break ; 

            case integer_op:
                byt->regis[op->p2].type = int_num;
                byt->regis[op->p2].val.i = op->p1;
                break;

            case blob_op:
                byt->regis[op->p2].type = blob_num;
                byt->regis[op->p2].val.s = char*(op->p4);
                byt->regis[op->p2].lenght = op->p1;
                break;

            case string_op:
                byt->regis[op->p2].type = string_num;
                byt->regis[op->p2].lenght = op->p1;
                byt->regis[op->p2].val.s = char*(op->p4);
                break;


            case null_op:
                byt->regis[op->p2].type = null;
                break;

            case real_op:
                byt->regis[op->p2].type = real_num;
                byt->regis[op->p2].val.r = (float*)op->p4;
                break;

            case copy_op:
                byt->regis[op->p2].type = byt->regis[op->p1].type;
                byt->regis[op->p2].lenght = byt->regis[op->p1].lenght;

                if (byt->regis[op->p2].type == int_num){
                    byt->regis[op->p2].val.i = byt->regis[op->p1].val.i;
                }

                if (byt->regis[op->p2].type == real_op){
                    byt->regis[op->p2].val.r = byt->regis[op->p1].val.r;
                }

                if (byt->regis[op->p2].type == string_num || byt->regis[op->p2].type == blob_num){
                    byt->regis[op->p2].val.s = malloc(byt->regis[op->p2].lenght);
                    memcpy(byt->regis[op->p2].val.s, byt->regis[op->p1].val.s, byt->regis[op->p2].lenght);
                }
                break;

            case move_op:
                if (byt->regis[op->p1].type == string_num || byt->regis[op->p1].type == blob_num){
                    free(byt->regis[op->p1].val.s);
                }

                if (byt->regis[op->p1].type == integer_num || byt->regis[op->p1].type == real_num){
                    break;
                }

                byt->regis[op->p1].type = null;
                byt->regis[op->p2].val.s = byt->regis[op->p1].val.s;
                byt->regis[op->p1].val.s = NULL;
                break;



                
            case add_op:
                if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                    byt->regis[op->p3].type = integer_num ;
                    byt->regis[op->p3].val.i  = byt->regis[op->p2].val.i + byt->regis[op->p1].val.i  ; 
                }
                else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r + byt->regis[op->p1].val.r  ;   
                }

                else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p1].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r + num ;   
                }

                else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p2].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = num+ byt->regis[op->p1].val.r   ;   
                }

                break ; 


            case subs_op:
                if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                    byt->regis[op->p3].type = integer_num ; 
                    byt->regis[op->p3].val.i  = byt->regis[op->p1].val.i - byt->regis[op->p2].val.i  ; 
                }
                else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r - byt->regis[op->p2].val.r  ;   
                }

                else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p1].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = num - byt->regis[op->p2].val.r ;   
                }

                else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p2].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r - num ;    
                }

                break ; 



            case mul_op:
                if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                    byt->regis[op->p3].type = integer_num ;
                    byt->regis[op->p3].val.i  = byt->regis[op->p2].val.i * byt->regis[op->p1].val.i  ; 
                }
                else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r * byt->regis[op->p1].val.r  ;   
                }

                else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p1].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r * num ;   
                }

                else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p2].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = num *  byt->regis[op->p1].val.r   ;   
                }
                break ; 
            

            case divide_op:
                if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                    byt->regis[op->p3].type = integer_num ; 
                    byt->regis[op->p3].val.i  = byt->regis[op->p1].val.i  / byt->regis[op->p2].val.i  ; 
                }
                else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r  / byt->regis[op->p2].val.r  ;   
                }

                else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p1].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = num  /  byt->regis[op->p2].val.r ;   
                }

                else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                    float num  = (float)( byt->regis[op->p2].val.i ) ; 
                    byt->regis[op->p3].type = real_num ; 
                    byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r / num ;    
                }
                break ; 
            




            case eq_select_op : 
                eq_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ; 


            case ne_select_op : 
                ne_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ; 


            case gt_select_op : 
                gt_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ; 



            case lt_select_op : 
                lt_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ; 



            case le_select_op : 
                le_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ;  


            case ge_select_op : 
                ge_select_op_fn(op->p1 ,op->p2, op->p3 , op->p4 , &byt ) ; 
                break ;  


            case eq_op : 
                eq_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ;  


            case gt_op : 
                gt_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ;  


            case lt_op : 
                lt_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ;  


            case le_op : 
                le_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ;  


            case ge_op : 
                ge_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ;  

            case ne_op : 
                ne_op_fn( op->p1 , op->p2 , op->p3 , op->p4 , byt) ; 
                break ; 




            case open_read_op : 
                byt->btr[op->p1].Table  = lookup_table(db , op->p2) ; 
                byt->btr[op->p1].tree = read_the_tree(op->p2) ; 
                internal_node * temp_tree = byt->btr[op->p1].tree ; 
                while (temp_tree->children != NULL ){
                    temp_tree = temp_tree->children[0] ;  
                }
                byt->btr[op->p1].current_number = 0 ; 
                byt->btr[op->p1].current_node =  temp_tree ; 
                byt->btr[op->p1].data =  temp_tree->data[0] ; 
                byt->btr[op->p1].mode  = cursor_read ; 
                byt->btr[op->p1].end = false ; 
                break ; 
            
                
            case open_write_op : 
                byt->btr[op->p1].Table  = lookup_table(db , op->p2) ; 
                byt->btr[op->p1].tree = read_the_tree(op->p2) ; 
                internal_node * temp_tree = byt->btr[op->p1].tree ; 
                while (temp_tree->children != NULL ){
                    temp_tree = temp_tree->children[0] ;  
                }
                byt->btr[op->p1].current_number = 0 ; 
                byt->btr[op->p1].current_node =  temp_tree ; 
                byt->btr[op->p1].data =  temp_tree->data[0] ; 
                byt->btr[op->p1].mode  = cursor_write ; 
                byt->btr[op->p1].end = false ; 
                break ; 
            

            case close_cursor_op : 
                if (byt->btr[op->p1] ==  NULL){
                    free(byt->btr[op->p1]) ; 
                    byt->btr[op->p1] =  NULL
                }
                break ; 
            
            
            case next_cursor : 
                if (byt->btr[op->p1].end == true ){
                    break ; 
                }
                if (byt->btr[op->p1].current_number + 1 > byt->btr[op->p1].current_node.num_of_data_pushed ){
                    if ( byt->btr[op->p1].current_node.next  != NULL ){
                        byt->btr[op->p1].current_node = byt->btr[op->p1].current_node.next ; 
                        byt->btr[op->p1].current_number  = 0 ; 
                        byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[0] ; 
                        byt->pc = op->p2 ;
                        break ;    
                    }
                    else { 
                        byt->btr[op->p1].end  = true ; 
                        byt->pc++ ;
                        break ; 
                    }
                }
                byt->btr[op->p1].current_number = byt->btr[op->p1].current_number++ ; 
                byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[byt->btr[op->p1].current_number] ; 
                byt->pc = op->p2 ;
                break ; 



            case prev_cursor : 
                if (byt->btr[op->p1].start == true ){
                    break ; 
                }
                if (byt->btr[op->p1].current_number - 1 < 0 ){
                    if ( byt->btr[op->p1].current_node.prev  != NULL ){
                        byt->btr[op->p1].current_node = byt->btr[op->p1].current_node.prev ; 
                        byt->btr[op->p1].current_number  = byt->btr[op->p1].current_node.num_of_data_pushed ; 
                        byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[byt->btr[op->p1].current_number] ; 
                        break ;    
                    }
                    else { 
                        byt->btr[op->p1].start  = true ; 
                        break ; 
                    }
                }
                byt->btr[op->p1].current_number = byt->btr[op->p1].current_number-- ; 
                byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[byt->btr[op->p1].current_number] ; 
                break ; 


            case rewind_cursor : 
                while ( byt->btr[op->p1].current_node.prev != NULL ){
                    byt->btr[op->p1].current_node = byt->btr[op->p1].current_node.prev ; 
                }
                byt->btr[op->p1].current_number = 0 ;  
                byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[0] ;  
                byt->btr[op->p1].end = false ; 
                byt->btr[op->p1].start = true ;
                break ; 

            
            case cursor_end : 
                while ( byt->btr[op->p1].current_node.next != NULL ){
                    byt->btr[op->p1].current_node = byt->btr[op->p1].current_node.next ; 
                }
                byt->btr[op->p1].current_number = byt->btr[op->p1].current_node.num_of_data_pushed ;  
                byt->btr[op->p1].data = byt->btr[op->p1].current_node.data[byt->btr[op->p1].current_number] ;  
                byt->btr[op->p1].end = true ; 
                byt->btr[op->p1].start = false ;
                break ; 


            case seek_le : 
                internal_node tree  = byt->btr[op->p1].tree ; 
                int turner = 0 ; 
                int pk = give_primary_number(byt->regis[op->p3] ) ; 
                while ( tree->children != NULL ){
                    int low = 0 ;
                    int high = tree->num_of_leaf - 1 ;
                    int ans = -1 ; 
                    while (low <= high){
                        int mid = (low + high) / 2 ;
                        if (tree->primary_key[mid] <= pk ){
                            ans = mid ; 
                            low = mid + 1 ; 
                        }
                        else {
                            high = mid - 1 ;
                        }
                    }
                    if (ans == -1 ){
                        byt->pc = op->p2 ;
                        turner = 1 ; 
                        break ; 
                    }
                    tree = tree->children[ans] ; 
                }
                if ( turner == 1 ){
                    break ; 
                }
                byt->btr[op->p1].current_node = tree ; 
                int low = 0 ; 
                int high = tree->num_of_children - 1  ; 
                int ans = -1 ; 
                while ( low <= high ){
                    int mid  = ( low + high ) /2 ; 
                    if (tree->primary_key[mid] <= pk ){
                        ans = mid ; 
                        low = mid + 1 ; 
                    }
                    else {
                        high = mid - 1 ;
                    }
                }
                if ( ans == -1 ){
                    byt->pc = op->p2 ;
                    break ; 
                }
                byt->btr[op->p1].data = tree->data[ans] ; 
                byt->btr[op->p1].current_number = ans ; 
                break ; 


            case seek_lt : 
                internal_node tree  = byt->btr[op->p1].tree ; 
                int turner = 0 ; 
                int pk = give_primary_number(byt->regis[op->p3] ) ; 
                while ( tree->children != NULL ){
                    int low = 0 ;
                    int high = tree->num_of_leaf - 1 ;
                    int ans = -1 ; 
                    while (low <= high){
                        int mid = (low + high) / 2 ;
                        if (tree->primary_key[mid] < pk ){
                            ans = mid ; 
                            low = mid + 1 ; 
                        }
                        else {
                            high = mid - 1 ;
                        }
                    }
                    if (ans == -1 ){
                        byt->pc = op->p2 ;
                        turner = 1 ; 
                        break ; 
                    }
                    tree = tree->children[ans] ; 
                }
                if ( turner == 1 ){
                    break ; 
                }
                byt->btr[op->p1].current_node = tree ; 
                int low = 0 ; 
                int high = tree->num_of_children - 1  ; 
                int ans = -1 ; 
                while ( low <= high ){
                    int mid  = ( low + high ) /2 ; 
                    if (tree->primary_key[mid] < pk ){
                        ans = mid ; 
                        low = mid + 1 ; 
                    }
                    else {
                        high = mid - 1 ;
                    }
                }
                if ( ans == -1 ){
                    byt->pc = op->p2 ;
                    break ; 
                }
                byt->btr[op->p1].data = tree->data[ans] ; 
                byt->btr[op->p1].current_number = ans ; 
                break ; 


            case seek_ge : 
                internal_node tree  = byt->btr[op->p1].tree ; 
                int pk = give_primary_number(byt->regis[op->p3] ) ; 
                while ( tree->children != NULL ){
                    int low = 0 ;
                    int high = tree->num_of_leaf - 1 ;
                    int ans = -1 ; 
                    while (low <= high){
                        int mid = (low + high) / 2 ;
                        if (tree->primary_key[mid] <= pk ){
                            ans = mid ; 
                            low = mid + 1 ; 
                        }
                        else {
                            high = mid - 1 ;
                        }
                    }
                    if (ans == -1 ){
                        ans = 0 ; 
                    }
                    tree = tree->children[ans] ; 
                }
                byt->btr[op->p1].current_node = tree ; 
                int low = 0 ; 
                int high = tree->num_of_children - 1  ; 
                int ans = -1 ; 
                while ( low <= high ){
                    int mid  = ( low + high ) /2 ; 
                    if (tree->primary_key[mid] >= pk ){
                        ans = mid ; 
                        high = mid - 1 ; 
                    }
                    else {
                        low = mid + 1 ;
                    }
                }
                if ( ans == -1 ){
                    if ( tree->next == NULL ){
                        byt->pc = op->p2 ;
                        break ; 
                    }
                    tree = tree->next ; 
                    byt->btr[op->p1].current_node = tree ; 
                    ans = 0 ; 
                }
                byt->btr[op->p1].data = tree->data[ans] ; 
                byt->btr[op->p1].current_number = ans ; 
                break ; 

            case seek_gt : 
                internal_node tree  = byt->btr[op->p1].tree ; 
                int pk = give_primary_number(byt->regis[op->p3] ) ; 
                while ( tree->children != NULL ){
                    int low = 0 ;
                    int high = tree->num_of_leaf - 1 ;
                    int ans = -1 ; 
                    while (low <= high){
                        int mid = (low + high) / 2 ;
                        if (tree->primary_key[mid] <= pk ){
                            ans = mid ; 
                            low = mid + 1 ; 
                        }
                        else {
                            high = mid - 1 ;
                        }
                    }
                    if (ans == -1 ){
                        ans = 0 ; 
                    }
                    tree = tree->children[ans] ; 
                }
                byt->btr[op->p1].current_node = tree ; 
                int low = 0 ; 
                int high = tree->num_of_children - 1  ; 
                int ans = -1 ; 
                while ( low <= high ){
                    int mid  = ( low + high ) /2 ; 
                    if (tree->primary_key[mid] > pk ){
                        ans = mid ; 
                        high = mid - 1 ; 
                    }
                    else {
                        low = mid + 1 ;
                    }
                }
                if ( ans == -1 ){
                    if ( tree->next == NULL ){
                        byt->pc = op->p2 ;
                        break ; 
                    }
                    tree = tree->next ; 
                    byt->btr[op->p1].current_node = tree ; 
                    ans = 0 ; 
                }
                byt->btr[op->p1].data = tree->data[ans] ; 
                byt->btr[op->p1].current_number = ans ; 
                break ; 

            case if_op : 
                reg * temp = &byt->regis[op->p1];
                int check = 0 ; 
                    if (temp->type == integer_num ){
                        if (temp.val.i != 0 ){
                            check = 1 ; 
                        } 
                    }
                    else if (temp->type == real_num){
                        if (temp.val.r != 0.0 ){
                            check = 1 ; 
                        } 
                    }
                    else if(temp->type == string_num) {
                        if (temp->lenght > 0 ){
                            check = 1 ; 
                        }
                    }
                    if ( check == 1 ){
                        byt->pc = op->p2;
                    }
                    break ; 

            case is_null : 
                if (byt->regis[op->p1].type == null_op ){
                    byt->pc = op->p2;
                }
                break ; 

            case is_null_no_jump : 
                if (byt->regis[op->p1].type == null_op ){
                    byt->regis[op->p3].type = integer_op ; 
                    byt->regis[op->p3].val.i = 1 ;
                }
                else {
                    byt->regis[op->p3].type = integer_op ; 
                    byt->regis[op->p3].val.i = 0 ;
                }
                break ; 

            case is_not_null : 
                if (byt->regis[op->p1].type != null_op ){
                    byt->pc = op->p2;
                }
                break ; 

            case is_not_null_no_jump : 
                if (byt->regis[op->p1].type != null_op ){
                    byt->regis[op->p3].type = integer_op ; 
                    byt->regis[op->p3].val.i = 1 ;
                }
                else {
                    byt->regis[op->p3].type = integer_op ; 
                    byt->regis[op->p3].val.i = 0 ;
                }
                break ; 

            case and_op : 
                if (byt->regis[op->p1].type != string_op && byt->regis[op->p2].type != string_op  ){
                    byt->regis[op->p3].type = integer_num ; 
                    if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  && byt->regis[op->p2].val.i  ; 
                    }
                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r && byt->regis[op->p2].val.r ; 
                    }
                    else if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == real_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  && byt->regis[op->p2].val.r ; 
                    }
                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == integer_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r  && byt->regis[op->p2].val.i  ; 
                    }
                }
                break ; 
            
            case or_op : 
                if (byt->regis[op->p1].type != string_op && byt->regis[op->p2].type != string_op  ){
                    byt->regis[op->p3].type = integer_num ; 
                    if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  || byt->regis[op->p2].val.i  ; 
                    }
                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r || byt->regis[op->p2].val.r ; 
                    }
                    else if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == real_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  || byt->regis[op->p2].val.r ; 
                    }
                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == integer_num   ){
                        byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r  ||  byt->regis[op->p2].val.i  ; 
                    }
                }
                break ; 

            

            case if_not_op : 
                reg * temp = &byt->regis[op->p1];
                int check = 0 ; 
                    if (temp->type == integer_num ){
                        if (temp.val.i == 0 ){
                            check = 1 ; 
                        } 
                    }
                    else if (temp->type == real_num){
                        if (temp.val.r == 0.0 ){
                            check = 1 ; 
                        } 
                    }
                    else if(temp->type == string_num) {
                        if (temp->lenght == 0 ){
                            check = 1 ; 
                        }
                    }
                    if ( check == 1 ){
                        byt->pc = op->p2;
                    }
                    break ; 

            case aggregate_init : 
                byt->agg[op->p1].total = 0 ; 
                byt->agg[op->p1].have_value = false ; 
                byt->agg[op->p1].count = 0 ; 
                byt->agg[op->p1].concat_string = NULL ; 
                break  ; 

            case aggregate_reset:
                if (byt->agg[op->p1].concat_string != NULL) {
                    free(byt->agg[op->p1].concat_string);
                }
                byt->agg[op->p1].total = 0;
                byt->agg[op->p1].have_value = false;
                byt->agg[op->p1].count = 0;
                byt->agg[op->p1].concat_string = NULL;
                break;

                
            case aggregate_step : 
                char *  operation = *(char*)op->p4 ; 
                agg_operation(byt->agg[op->p1] , byt->regis[op->p2] , operation )  ; 
                break ; 
            
            case aggregate_final : 
                char * operation = *(char*)op->p4  ; 
                agg_final(byt->agg[op->p1] , byt->regis[op->p3] , operation) ; 
                break ; 

            case sorter_open : 
                byt->sort[op->p1].array = NULL ; 
                byt->sort[op->p1].row_count = 0 ; 
                byt->sort[op->p1].capacity = 0 ; 
                byt->sort[op->p1].cols_to_look = op->p2  ; 
                byt->sort[op->p1].keycols = op->p4  ; 
                byt->sort[op->p1].cursor = 0 ;  
                
            case sorter_insert : 
                if (byt->sort[op->p1].row_count == byt->sort[op->p1].capacity) {
                        if (byt->sort[op->p1].capacity == 0 ){
                            byt->sort[op->p1].capacity  = 4  ; 
                        }
                        else if (byt->sort[op->p1].capacity > 0 ){
                            byt->sort[op->p1].capacity  = byt->sort[op->p1].capacity*2   ; 
                        }
                        byt->sort[op->p1].capacity = byt->sort[op->p1].capacity == 0 ? 4 : byt->sort[op->p1].capacity * 2;  
                        sort_arr *tmp = realloc( byt->sort[op->p1].array, byt->sort[op->p1].capacity * sizeof(*byt->sort[op->p1].array)   );
                        if (tmp == NULL) { 
                            break; 
                        }/
                        byt->sort[op->p1].array = tmp;
                    }
                    sort_arr * temp  = malloc(sizeof(sort_arr)) ; 
                    temp->array = malloc(byt->regis[op->p2].lenght);
                    memcpy(temp->array, byt->regis[op->p2].val.s, byt->regis[op->p2].lenght ); 
                    temp.len = byt->regis[op->p2].lenght  ;     
                    temp->key = NULL;  
                    temp->key_type = byt->regis[op->p2].key_type;    
                    byt->sort[op->p1].array[byt->sort[op->p1].row_count++ ] = *temp ; 
                    free(temp) ; 
                    break ; 
                


                
            case sorter_sort:
                    qsort_r(byt->sort[op->p1].array,byt->sort[op->p1].row_count, sizeof(sort_arr), compare_sort, &byt->sort[op->p1]);             
                    break;

            case orderby_sort : 
                qsort_r(byt->sort[op->p1].array,byt->sort[op->p1].row_count, sizeof(sort_arr), orderby_campare_sort , &byt->sort[op->p1]);             
                    break; 

            case sorter_next : 
                if (byt->sort[op->p1].cursor < byt->sort[op->p1].row_count ){
                    byt->sort[op->p1].cursor++ ; 
                }
                else { 
                    byt->pc = op->p2; 
                }
                break ; 

            case sorter_data : 
                sort_arr *temp =  byt->sort[op->p1].array ; 
                byt->regis[op->p2].lenght = temp->len[ byt->sort[op->p1].cursor]  ; 
                byt->regis[op->p2].type =  blob_op ; 
                byt->regis[op->p2].val.s = temp->array[ byt->sort[op->p1].cursor]  ; 
                break ; 

            case gb_sorter_data:
                unsigned char * first = malloc(byt->regis[op->p1].lenght) ; 
                memcpy(first ,byt->regis[op->p1].val.s , byt->regis[op->p1].lenght ) ; 
                int key = op->p2 ; 
                int i  = 0 ; 
                int temp = 0 ; 
                int type = integer_num ; 
                int copy = 0 ; 
                unsigned char *tempo ; 
                unsigned char *blob = byt->regis[op->p1].val.s  ; 
                while (i < byt->regis[op->p1].lenght ){
                    type = blob[i];    
                    i  = i +  1;
                    if (temp < key ){
                        if (type == integer_num) {
                            i = i +  sizeof(int);     
                        }
                            else if (type == real_num) {
                            i = i +  sizeof(float);   
                        }
                            else if (type == string_num || type == blob_num) {
                            int len;
                            memcpy(&len, blob + i, sizeof(int));
                            i = i +  sizeof(int);     
                            i = i +  len;            
                        }
                    }
                    else { 
                        copy =  byt->regis[op->p1].lenght - i ; 
                        tempo = malloc(copy); 
                        memcpy(tempo , first+ i  , copy ) ; 
                        break ; 
                    }
                    temp++ ;    
                }
                byt->regis[op->p3].type = blob_num;
                byt->regis[op->p3].val.s = tempo ;
                byt->regis[op->p3].lenght = copy ;
                break ; 


            case gb_specific_column_op : 
                unsigned char * first = malloc(byt->regis[op->p1].lenght) ; 
                memcpy(first ,byt->regis[op->p1].val.s , byt->regis[op->p1].lenght ) ; 
                int pos = 0 ;
                unsigned char *thing = first; 
                unsigned char temp = thing[pos] ;  
                int key = op->p2 ; 
                reg * ans = &byt->regis[op->p3] ; 
                for ( int i =  0 ; i < key ; i++ ){
                    temp = thing[pos] ; 
                    pos = pos + 1;  
                    if (temp == integer_num){
                        pos = pos + sizeof(int) ; 
                    }
                    else if (temp == real_num){
                        pos = pos + sizeof(float ) ;   
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        pos = pos + sizeof(uint32_t) ; 
                        pos = pos + size ; 
                    }
                }

                int len = 0  ; 
                pos = pos + 1 ; 
                if (temp == integer_num){
                    len =  sizeof(int) ; 
                    ans->type = integer_num ; 
                }
                else if (temp == real_num){
                    len =  sizeof(float ) ; 
                    ans->type =  real_num ; 
                }
                else if (temp == string_num ){
                    uint32_t size ; 
                    memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                    len =  sizeof(uint32_t) ; 
                    len = len + size ; 
                    ans->type = string_num ; 
                }

                unsigned char * valli = malloc(len ) ; 
                memcpy(valli , thing + pos   , len ) ; 
                if (ans->type == integer_num ){
                    memcpy(&ans->val.i, valli, sizeof(int)) ;
                    free(valli) ; 
                }
                else if (ans->type == real_num ){
                    memcpy(&ans->val.r, valli, sizeof(float)) ; 
                    free(valli) ;   
                }
                else {
                    ans->val.s = valli ;     
                    ans->lenght = len ; 
                }
                free(first) ; 
                byt->btr[op->p1].null == false ; 
                break ; 
                

            case sorter_campare : 
                sort_arr *temp =  byt->sort[op->p1].array ; 
                if ( temp->key_type[byt->sort[op->p1].cursor] == integer_num && byt->regis[op->p3].type == integer_num  ){
                    int comp ; 
                    memcpy(&comp, temp->key[byt->sort[op->p1].cursor] , sizeof(int));
                    int comp = (int)temp->key[byt->sort[op->p1].cursor]  ; 
                    if (comp == byt->regis[op->p3].val.i ){
                        byt->pc = op->p2;
                    }
                }
                else if ( temp->key_type[byt->sort[op->p1].cursor]  == real_num && byt->regis[op->p3].type == real_num  ){
                    float comp  ;
                    memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                    if (comp == byt->regis[op->p3].val.r ){
                        byt->pc = op->p2;
                    }
                }

                else if (byt->regis[op->p3].type == real_num && temp->key_type[byt->sort[op->p1].cursor]  == integer_num   ){
                    float comp  ;
                    memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                    if (comp == byt->regis[op->p3].val.r ){
                        byt->pc = op->p2;
                    }
                }

                else if (temp->key_type[byt->sort[op->p1].cursor]  == real_num && byt->regis[op->p3].type == integer_num   ){
                    float comp  ;
                    memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                    float num  = (float)( byt->regis[op->p3].val.i ) ; 
                    if (comp == byt->regis[op->p3].val.r ){
                        byt->pc = op->p2;
                    }
                }
                else if (temp->key_type[byt->sort[op->p1].cursor] == string_num && byt->regis[op->p3].type == string_num ){
                    if (strcmp(temp->key[byt->sort[op->p1].cursor] ,byt->regis[op->p3].val.s ) == 0 ){
                        byt->pc = op->p2;
                    }
                }
                break ; 


            case register_get_stuff : 
                int pos = 0 ;
                unsigned char thing = byt->regis[op->p1]->val.s ; 
                unsigned char temp = thing[pos] ;  
                int key = op->p2 ; 
                reg * ans = byt->regis[op->p3] ; 
                for ( int i =  0 ; i < key ; i++ ){
                    temp = thing[pos] ; 
                    pos = pos + 1;  
                    if (temp == integer_num){
                        pos = pos + sizeof(long) ; 
                    }
                    else if (temp == real_num){
                        pos = pos + sizeof(float ) ;   
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        pos = pos + sizeof(uint32_t) ; 
                        pos = pos + size ; 
                    }
                }

                int len = 0  ; 
                pos = pos + 1 ; 
                if (temp == integer_num){
                    len =  sizeof(long) ; 
                    ans->type = integer_num ; 
                }
                else if (temp == real_num){
                    len =  sizeof(float ) ; 
                    ans->type =  real_num ; 
                }
                else if (temp == string_num ){
                    uint32_t size ; 
                    memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                    len =  sizeof(uint32_t) ; 
                    len = len + size ; 
                    ans->type = string_num ; 
                }

                unsigned char * valli = malloc(len ) ; 
                memcpy(valli , thing + pos   , len ) ; 
                if (ans->type == integer_num ){
                    ans->val.i = (int)valli ;  
                }
                else if (ans->type == real_num ){
                    ans->val.r = (float)valli ;     
                }
                else {
                    ans->val.s = valli ; 
                    ans->lenght = len ; 
                }
                byt->btr[op->p1].null == false ; 
                break ; 


            case column_op : 
                if (byt->btr[op->p1].null == true ){
                    break ; 
                }
                void * temp = pager_get_page( byt->pager , byt->btr[op->p1].stack[depth].page_num  ) ; 
                void * cell = get_cell(temp , byt->btr[op->p1].stack[depth].cell_num  ) ; 
                reg tempo = get_key_from_cell(cell) ; 
                int pos = 0 ;
                unsigned char thing = tempo->val.s ; 
                unsigned char temp = thing[pos] ;  
                int key = op->p2 ; 
                reg * ans = byt->regis[op->p3] ; 
                for ( int i =  0 ; i < key ; i++ ){
                    temp = thing[pos] ; 
                    pos = pos + 1;  
                    if (temp == integer_num){
                        pos = pos + sizeof(long) ; 
                    }
                    else if (temp == real_num){
                        pos = pos + sizeof(float ) ;   
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        pos = pos + sizeof(uint32_t) ; 
                        pos = pos + size ; 
                    }
                }

                int len = 0  ; 
                pos = pos + 1 ; 
                if (temp == integer_num){
                    len =  sizeof(long) ; 
                    ans->type = integer_num ; 
                }
                else if (temp == real_num){
                    len =  sizeof(float ) ; 
                    ans->type =  real_num ; 
                }
                else if (temp == string_num ){
                    uint32_t size ; 
                    memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                    len =  sizeof(uint32_t) ; 
                    len = len + size ; 
                    ans->type = string_num ; 
                }

                unsigned char * valli = malloc(len ) ; 
                memcpy(valli , thing + pos   , len ) ; 
                if (ans->type == integer_num ){
                    ans->val.i = (int)valli ;  
                }
                else if (ans->type == real_num ){
                    ans->val.r = (float)valli ;     
                }
                else {
                    ans->val.s = valli ; 
                    ans->lenght = len ; 
                }
                byt->btr[op->p1].null == false ; 
                break ; 
                
                
            case make_record : 
                byt->reg[op->p3].type = string_num ; 
                byt->reg[op->p3].val.s = make_record_data(byt , op->p1 , op->p1 + op->p2 ) ; 
                byt->reg[op->p3].lenght = strlen(byt->reg[op->p3].val.s) ; 
                break ; 
            
            case result_row : 
                byt->reg_start = op->p1 ; 
                byt->reg_end = byt->reg_start  + op->p2 ; 
                byt->reg_mode = end_num ;
                for ( int i = byt->reg_start ; i < byt->reg_end  ; i++ ){

                } 
                

            case limit_op : 
                if (byt->reg[op->p1].val.i > 0 ){
                    byt->reg[op->p1].val.i-- ; 
                    if (byt->reg[op->p1].val.i == 0  ){
                        byt->pc = op->p2;
                        continue ; 
                    }
                } break ; 

            case null_join_op : 
                byt->btr[op->p1].null = true ; 
                break ; 

            case push_to_hash:
                unsigned char  * blob = op->p4 ; 
                int temp = 0;
                int pk_1  ; 
                int str_len ; 
                int pk_2 ; 
                int hash_bin_nums ; 
                int row_num  = -1  ; 
                char * table_name  = NULL ; 

                memcpy(hash_bins_num, blob + temp, sizeof(int));
                temp += sizeof(int);

                memcpy(pk_1, blob + temp, sizeof(int));
                temp += sizeof(int);

                memcpy(pk_2, blob + temp, sizeof(int));
                temp += sizeof(int);

                if (blob + temp != NULL) {
                    memcpy(row_num, blob + temp, sizeof(int));
                    temp += sizeof(int);
                } 

                if (blob + temp != NULL ){
                    str_len = 0;
                    memcpy(str_len, blob + temp, sizeof(int));
                    temp += sizeof(int);
                    if (str_len > 0) {
                        table_name = (char *)malloc(str_len + 1);
                        memcpy(table_name, blob + temp, str_len);
                        table_name[str_len] = '\0';
                    }
                }
                byt->hash_bins[op->p1]->bn[hbw]->num_of_entires++ ; 
                int hbw = hash_bin_which( byt->regis[op->p2] , hash_bins_num ) ; 
                if (byt->regis[pk_1].type ==   integer_num  ){
                byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.i = byt->regis[pk_1].val.i ; 
                }
                else if (byt->regis[pk_1].type == real_num){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.r = byt->regis[pk_1].val.r ; 
                }
                else if (byt->regis[pk_1].type == string_num || byt->regis[pk_1].type == blob_num ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.s = malloc(byt->regis[pk_1].lenght)  ; 
                    memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.s  , byt->regis[pk_1].val.s , byt->regis[pk_1].lenght ) ; 
                }
                if (byt->regis[pk_2].type ==   integer_num  ){
                byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.i = byt->regis[pk_2].val.i ; 
                }
                else if (byt->regis[pk_2].type == real_num){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.r = byt->regis[pk_2].val.r ; 
                }
                else if (byt->regis[pk_2].type == string_num || byt->regis[pk_2].type == blob_num ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.s = malloc(byt->regis[pk_2].lenght)  ; 
                    memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.s  , byt->regis[pk_2].val.s , byt->regis[pk_2].lenght ) ; 
                }
                if (byt->regis[op->p2].type ==   integer_num  ){
                byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.i = byt->regis[op->p2].val.i ; 
                }
                else if (byt->regis[op->p2].type == real_num){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.r = byt->regis[op->p2].val.r ; 
                }
                else if (byt->regis[op->p2].type == string_num || byt->regis[op->p2].type == blob_num ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.s = malloc(byt->regis[op->p2].lenght)  ; 
                    memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.s  , byt->regis[op->p2].val.s , byt->regis[op->p2].lenght ) ;  
                }
                if (row_num != -1 ){
                    byt->hash_bins->bn[hbw].row_number = row_num ; 
                }
                if (table_name != NULL ){
                    byt->hash_bins->bn[hbw].tbl = malloc(str_len) ; 
                    memcpy(byt->hash_bins->bn[hbw].tbl , table_name , str_len) ; 
                }
                break  ; 


            
            
            }
        }
        
}




int like_campare(reg *a , reg *b ){
    int ans = 0 ; 
    if (a->type == string_num && b->type == string_num ){
        int first = 0 ; 
        int second = 0 ; 
        while ( first < a->len && second < b->len ){
            if (strcmp(a->val.s[first] , b->val.s[second] ) == 0 ){
                first++ ; 
                second++ ; 
            }
            else if (strcmp(a->val.s[first] , '%' ) == 0 ){
                int temp_fir = first ; 
                unsigned char temp ; 
                while (temp_fir < a->lenght &&  strcmp(a->val.s[temp_fir] , '%' ) != 0 ){
                    memcpy(temp , )
                }
            }
            else if ( strcmp(a->val.s[first] , "_" ) == 0 ){
                int first_temp = first ;  
            }
        }
    }
}




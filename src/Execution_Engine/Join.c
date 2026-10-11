#include "Engine.h"

int  table_num (tables_list * tab , char * table_name  ){
    for ( int i = 0 ; i < tab->num_of_tables ; i++ ){
        if ( strcmp( tab[i]->tables->name  , table_name  ) == 0 ) { 
            return i ; 
        }
    }
    return -1 ;  
}

char * table_name_from_num(tables_list * tab , int num ){
    return tab[i]->tables->name  ; 
}

select_join_info *join_expre(select_join_info *ans ,    compiler *c , tree * temp){
    int i = 0 ; 
    while (i < temp->num){
        if (strcmp(temp->children[i]->comp , "+")== 0 || strcmp(temp->children[i]->comp, "-")== 0 || strcmp(temp->children[i]->comp, "*")== 0 || strcmp(temp->children[i]->comp, "/")== 0 || strcmp(temp->children[i]->comp , "=")== 0 || strcmp(temp->children[i]->comp, "!=")== 0 || strcmp(temp->children[i]->comp , ">")== 0 || strcmp(temp->children[i]->comp , ">=")== 0 || strcmp(temp->children[i]->comp , "<")== 0 || strcmp(temp->children[i]->comp, "<=")== 0 || strcmp(temp->children[i]->comp , "GROUP_CONCAT")== 0 || strcmp(temp->children[i]->comp , "MAX") == 0 || strcmp(temp->children[i]->comp , "MIN") == 0 || strcmp(temp->children[i]->comp , "COUNT") == 0 || strcmp(temp->children[i]->comp, "AVG") == 0 || strcmp(temp->children[i]->comp , "SUM") == 0){
                ans->operator = temp->children[i]->comp;
            if (ans->left == NULL ){
                ans->left = malloc(sizeof(select_join_info))
                expre(ans->left, c, temp->children[i]);
            }
            else {
                ans->right = malloc(sizeof(select_join_info))
                expre(ans->right, c, temp->children[i]);
            }
        }
        else if (col_name_to_int_main(temp->children[i]->comp, c->select) != -1 ){
            if (ans->table_or_col_name != NULL ){
                    ans->extra_table_or_col_name = temp->children[i]->comp ;
            }
            else { 
                ans->table_or_col_name = temp->children[i]->comp ;
                }
            }
        else { 
            int check = data_type_check(temp->children[i]->comp);
            if (check == 0){
                ans->num_value = atoi(temp->children[i]->comp);
            }
            else if (check == 1){
                ans->float_val = (float)atof(temp->children[i]->comp);
            }
            else if (check == 2){
                ans->blob = temp->children[i]->comp ;
            }
            else {
                ans->char_value = temp->children[i]->comp ;
            }
        }
    i++;
    }
    return ans ; 
}

void from_parser_to_struct(compiler * c  , tree * select ){
    select_from_info **from  = c->select->from ; 
    int temp = 0   ; 
    while (temp < select->num && strcmp(select->children[temp]->comp , "FROM") != 0 ){
        temp++ ; 
    }
    select = select->children[temp] ; 
    int i = 0 ; 
    c->select->table_counter = 0 ; 
        while (  i < select->num && strcmp(select->children[i]->comp , "WHERE") != 0 ){
            if (strcmp(select->children[i]->comp , "JOIN") == 0 || strcmp(select->children[i]->comp , "INNER JOIN") == 0 || strcmp(select->children[i]->comp , "LEFT JOIN") == 0  || strcmp(select->children[i]->comp , "RIGHT JOIN") == 0  || strcmp(select->children[i]->comp , "CROSS JOIN") == 0  || strcmp(select->children[i]->comp , "FULL OUTER JOIN") == 0  ){
                int num = c->select->tables_counter - 1  ; 
                select_join_info * join = c->select->children[num]->join ; 
                if (elect->children[i]->children[0] != NULL) { 
                    join->table_or_col_name = select->children[i]->children[0]->comp ; 
                    if (select->children[i]->children[0]->as != NULL ){
                        join->as = select->children[i]->children[0]->as ; 
                    }
                }
                if (select->children[i]->children[1] != NULL ){
                    join->left = malloc(sizeof(select_join_info)) ; 
                    join->left = join_expre( join->left  , c ,select->children[i]->children[1]  ) ; 
                }
            }
            else { 
                from[c->select->tables_counter] = malloc(sizeof(select_from_info)) ; 
                from[c->select->tables_counter]->table_name = select->children[i]->comp ; 
                if (select->as != NULL ){
                    from[c->select->tables_counter]->as = select->as ; 
                }
                c->select->tables_counter++ ; 
            }

            i++ ; 
        }
}


void * just_for_extra_safety(compiler * c ){
    for ( int i = 0 ; i < 300 ; i++ ){
        c->join_select_hash_unique_table[i] = -1 ; 
    }
}

void * tables_and_their_hash_cursor_num(compiler * c  , select_select_info * from ){
    just_for_extra_safety(compiler * c ) ; 
    if (from->col_name ){
        int tab_num = table_num(c->tl ,table_thing(from->col) ) ; 
        int col_num = col_name_to_int(operand_thing(from->col) ,c->select ) ; 
        if ( c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->table_num != tab_num  ){
            c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->table_num = tab_num   ; 
            if ( c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->column_used[col_num] != col_num  ){
                c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->column_used[col_num] = col_num  ;
                c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->col_conter++ ; 
            }
            c->select->join[c->select->join_counter]->tables_occuring_number_of_times[tab_num]++ ; 
        }
    }
    else if (from->extra_col ){
        int tab_num = table_num(c->tl ,table_thing(from->extra_col)) ; 
        int col_num = col_name_to_int(operand_thing(from->extra_col) ,c->select ) ; 
        if ( c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num] != tab_num  ){
            c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num] = tab_num   ; 
            if ( c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->column_used[col_num] != col_num  ){
                    c->select->join[c->select->join_counter]->join_select_hash_unique_table[tab_num]->column_used[col_num] = col_num  ;
            }
            c->select->join[c->select->join_counter]->tables_occuring_number_of_times[tab_num]++ ; 
        }
    }
    else if(from->left != NULL ){
        tables_and_their_cursor_num( c , from->left ) ; 
    }
    else if(from->right != NULL ){
        tables_and_their_cursor_num( c , from->right ) ; 
    }
}

int tables_and_thier_cursor_num(compiler * c  , select_select_info *  from ){
    tables_and_their_hash_cursor_num(c , from ) ; 
    int counter = 0 ; 
    for (int i = 0 ; i < 300 ; i++ ){
        if (c->select->join[c->select->join_counter]->join_select_hash_unique_table[i] = i ){
            c->select->join[c->select->join_counter]->join_select_unique_table[counter]->table_num = i ; 
            for (int j = 0 ; j < c->select->join[c->select->join_counter]->join_select_hash_unique_table[i]->col_conter  ; j++ ){
                c->select->join[c->select->join_counter]->join_select_unique_table[counter]->column_used[j] = c->select->join[c->select->join_counter]->join_select_unique_table[i]->column_used[j]
            }
            c->select->join[c->select->join_counter]->tables_occuring_number_of_times[counter] = c->select->join[c->select->join_counter]->tables_occuring_number_of_times[i] ; 
            counter++ ; 
        }
    }
    return counter ; 
}

int primary_key_offset(dab*db , int table_num ){
    int i = 0 ; 
    while(i < db->master->entries[table_num]->num_columns ){
        if ( db->master->entries[table_num]->columns[i]->is_primary_key == true ){
            return i ; 
        }
        i++ ; 
    }
    return -1 ; 
}

int string_length(char * table_name){
    if (table_name == NULL ){
        return 0 ; 
    }
    int i = 0 ; 
    while ( table_name[i] != '\0'){
        i++ ; 
    }
    return i ; 
}

char * blob_of_data(compiler * c , int pk_1 , int pk_2  , int row_num , char * table_name){
    char * blob  ; 
    int temp = 0 ; 
    int hash_bins_num = (db->master->entries[table_num(table_name)]->num_rows / 100 )* 30  ; 
    memcpy(blob + temp , hash_bins_num , sizeof(int))  ; 
    temp += sizeof(int) ; 
    memcpy(blob + temp , pk_1 , sizeof(int))  ; 
    temp += sizeof(int) ; 
    memcpy(blob + temp , pk_2 , sizeof(int))  ; 
    temp += sizeof(int) ; 
    if (row_num != -1 ){
        memcpy(blob + temp , row_num , sizeof(int))  ; 
        temp += sizeof(int) ; 
    }
    if (table_name != NULL ){
        int str_len = string_length(table_name) ; 
        memcpy(blob + temp , str_len , sizeof(int))  ; 
        temp += sizeof(int) ; 
        memcpy(blob+temp , table_name , str_len) ; 
    }
    return blob ; 
}

void join_equal_clause( compiler * c  , select_select_info * from  , select_from_info * from_org ){
    hash_bins_str * hash_bins ; 
    int first_blob  ; 
    int second_blob ; 
    int hash_cursor  ; 
    int pirmary_key_first_loc ; 
    int primary_key_second_loc ; 
    c->select->join[c->select->join_counter]->join_table_counter = tables_and_thier_cursor_num(c , from ) ; 
    for ( int i = 0 ; i < num  ; i++ ){
        emit(c , open_read_op , c->cursor_num + i , c->tl->tables[i]->root_page_num ; ,  -1 , -1 , NULL    ) ; 
    }

    int loop_addr_hb = c->count ; 
        int first  ; 
        int first_table = table_name_from_num( c->tl , from_org->table_name)  ;  
        for ( int m = 0 ; m < c->select->join[c->select->join_counter]->join_table_counter ; m++ ){
            if (c->select->join[c->select->join_counter]->join_select_unique_table[m] == first  ){
                first = m ; 
                break ; 
            }
        }
        pirmary_key_first_loc = c->register_counter++ ; 
        emit(c , column_op , first , primary_key_offset( db , first_table ) , pirmary_key_first_loc , NULL) ; 

        int second_table = table_name_from_num( c->tl , from_org->join->table_or_col_name) ; 
        int second ;
        for ( int m = 0 ; m < c->select->join[c->select->join_counter]->join_table_counter ; m++ ){
            if (c->select->join[c->select->join_counter]->join_select_unique_table[m] == first  ){
                second = m ; 
                break ; 
            }
        }
        primary_key_second_loc = c->register_counter++ ; 
        emit(c , column_op , second , primary_key_offset( db , first_table ) ,primary_key_second_loc , NULL) ; 

    if (from->left != NULL ){
        int left_reg =  join_func(c , from->left) ; 
            hash_cursor = c->select->join[c->select->join_counter]->join_hash_counter++ ; 
        first_blob = blob_of_data( pirmary_key_first_loc, primary_key_second_loc, -1 , NULL ) ;
        emit(c , push_to_hash , hash_cursor ,  left_reg , -1 , first_blob )  ; 
    }
    if (from->right != NULL ){
        int right_reg =  join_func(c , from->right) ; 
            hash_cursor = c->select->join[c->select->join_counter]->join_hash_counter++ ; 
        second_blob = blob_of_data( pirmary_key_first_loc, primary_key_second_loc, -1 , NULL ) ; 
        emit(c , push_to_hash , hash_cursor ,  right_reg , -1 , second_blob )  ; 
    }
    if (from->col_name != NULL ){
            hash_cursor = c->select->join[c->select->join_counter]->join_hash_counter++ ; 
        char * needed_table_char = table_thing(from->col_name) ; 
        table * needed_table = lookup_table(needed_table_char) ; 
        int needed_col = col_name_to_int( operand_thing(from->col_name , needed_table ) )  ; 
        emit(c , column_op , table_num(needed_table_char) , needed_col ,  c->register_counter , NULL ) ; 
        c->register_counter++ ; 
        first_blob = blob_of_data(pirmary_key_first_loc , primary_key_second_loc , needed_col , needed_table_char ) ; 
        emit(c , push_to_hash , hash_cursor ,  c->register_counter -1  , -1 , first_blob )  ; 
    }
    if (from->extra_col != NULL  ){
            hash_cursor = c->select->join[c->select->join_counter]->join_hash_counter++ ; 
        char * needed_table_char = table_thing(from->extra_col) ; 
        table * needed_table = lookup_table(needed_table_char) ; 
        int needed_col = col_name_to_int( operand_thing(from->extra_col , needed_table ) )  ; 
        emit(c , column_op , table_num(needed_table_char) , needed_col ,  c->register_counter , NULL ) ; 
        c->register_counter++ ; 
        second_blob = blob_of_data( pirmary_key_first_loc, primary_key_second_loc , needed_col , needed_table_char ) ; 
        emit(c , push_to_hash , hash_cursor ,  c->register_counter -1  , -1 , second_blob )  ; 
    }

    for ( int i = 0 ; i < num  ; i++ ){
        if (i == num -1 ){
            emit(c , next_cursor , c->cursor_num + i  , loop_addr_hb   , -1 , NULL ) ; 
        }
        else { 
            emit(c , next_cursor , c->cursor_num + i  , c->count + 1    , -1 , NULL ) ;     
        }
    }

    hash_cursor ;
    hash_cursor - 1 ; 


}




void* get_the_max_and_min(compiler * c  ){
    c->select->join[c->select->join_counter]->join_select_unique_table ; 
    void * temp_max = "MAX" ; 
    void * temp_min = "MIN" ; 
    int init_register_counter = c->register_counter ; 
    for ( int i = 0 ; i < c->table_counter  ; i++ ){
        emit(c , open_read_op , c->cursor_num + i , c->tl->tables[i]->root_page_num ;  ,  -1 , -1 , NULL    ) ; 
        emit(c , aggregate_init , c->register_counter++  , -1 , -1 , NULL  ) ; 
        emit(c , aggregate_init , c->register_counter++ , -1 , -1 , NULL  ) ;  
    }
    int end_init_register_counter = c->register_counter -1 ;
    int loop_addrese = c->count ; 
    for ( int i = 0 ; i < c->table_counter ; i++ ){
        if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->col_conter > 0 ){
            for ( int j  = 0 ; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->col_conter ; j++  ){
                emit(c , column_op  , c->cursor_num + i , operand_thing(c->select->join[c->select->join_counter]->join_select_unique_table[i]->column_used[j] ) , c->register_counter++ , NULL ) ; 
                emit(c , aggregate_step , init_register_counter +  2*i  , c->register_counter - 1 , -1 , temp_max  ) ; 
                emit(c , aggregate_step , init_register_counter +  2*i + 1    , c->register_counter - 1 , -1 , temp_min  ) ; 
            }
        }
    }
    for ( int i = 0 ; i < c->table_counter  ; i++ ){
        if (i == c->table_counter -1 ){
                emit(c , next_cursor , c->current_num + i  , loop_addrese , -1 , NULL  ) ; 
        }
        else { 
                emit(c , next_cursor , c->current_num + i  , -1 , -1 , NULL  ) ; 
        }

    }

    for ( int i = 0 ; i < c->table_counter ; i++ ){
        if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->col_conter > 0 ){
            for ( int j  = 0 ; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->col_conter ; j++  ){
                emit(c , aggregate_final , init_register_counter +  2*i  , -1  , MAX - 6 , temp_max ) ; 
                emit(c , aggregate_final , init_register_counter +  2*i + 1  , -1  , MAX - 7 , temp_min ) ; 
                int * ans[2] = malloc(2 * sizeof(int)); ; 
                ans = {init_register_counter +  2*i , init_register_counter +  2*i + 1  } ; 
                c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter++]->range = ans ; 
                c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter++]->table_name = i ; 
                c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter++]->col_name = j ; 
            }
        }
    }
}


int * take_care_of_expression(compiler * c , int target , select_select_info * from   , int reg_1 , int reg_2  , int * done  , int * counts ){
    int *ans[3]  = malloc(3* sizeof(int)) ; 
    if (from->left != NULL ){
        ans = take_care_of_expression(c , target ,  from->left , reg_1  , reg_2 , done  , counts ) ; 
        if (*done){
            return ans; 
        } 

    }
    if (from->right != NULL ){
        ans = take_care_of_expression(c , target ,  from->right , reg_1 , reg_2 , done , counts  ) ; 
        if (*done){
            return ans; 
        } 
    }

    if (from->extra_col != NULL){
        int extra_num = table_num(table_thing(from->extra_col )) % c->select->join[c->select->join_counter]->join_table_counter ; 
        int extra_col_num = col_name_to_int(operand_thing(from->extra_col)) ; 
        if (1){
            if (extra_num  == target ){
                if (extra_num > 1 ){
                    c->select->join[c->select->join_counter]->tables_occuring_number_of_times[extra_num]-- ; 
                }
                else { 
                        *done = 1; 
                        int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
                        ans_for_func[0] = reg_1 ; 
                        ans_for_func[1] = reg_2 ; 
                        ans_for_func[2] = 1 ; 
                        return ans_for_func ; 
                }
            }
            else { 
                if (counts[num] > 0 ){
                    counts[num]--  ; 
                }
            }
        }
        if (from->col_name != NULL ){
            int num =  table_num(table_thing(from->col_name )) % c->select->join[c->select->join_counter]->join_table_counter ; 
            int col_num = col_name_to_int(operand_thing(from->col_name)) ; 
            if (num == target){
                if (num > 1 ){
                    counts[num]--  ; 
                }
                else { 
                        *done = 1; 
                        int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
                        ans_for_func[0] = reg_1 ; 
                        ans_for_func[1] = reg_2 ; 
                        ans_for_func[2] = 1  ; 
                        return ans_for_func ; 
                }
            }
            else { 
                if (counts[num] > 0 ){
                    counts[num]--  ; 
                }
            }
            int operator =  -1  ; 

            if (from->operator != NULL ){
                if (strcmp(from->operator , "+")== 0 ){
                    operator = add_op ; 
                }
                else if (strcmp(from->operator , "-")== 0 ){
                    operator = subs_op ; 
                }
                else if  (strcmp(from->operator , "*")== 0 ){
                    operator = mul_op ; 
                }   
                else if  (strcmp(from->operator , "/")== 0 ){
                    operator = divide_op ; 
                }
                else if (strcmp(from->operator , "IS NULL")== 0 ){
                    operator = is_null ; 
                }
                else if(strcmp(from->operator , "IS NOT NULL")== 0 ){
                    operator = is_not_null ; 
                }
                int extra_values = malloc(2* sizeof(int)) ; 
                for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                    if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->table_name == extra_num ){
                        for (int j = 0; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter; j++) {
                                if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->col_name == extra_col_num) {
                                    normal_val = c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->range;
                                    break;
                                }
                        }
                    }
                }

                int normal_val = malloc(2* sizeof(int)) ; 
                for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                    if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->table_name == num ){
                        for (int j = 0; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter; j++) {
                                if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->col_name == col_num) {
                                    normal_val = c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->range;
                                    break;
                                }
                        }
                    }
                }

                emit(c , operator  , extra_values[0]  ,  normal_val[0]  , reg_1 , NULL ) ;
                emit(c , operator  , extra_values[1] ,  normal_val[1]  , reg_2 , NULL ) ;
                int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
                ans_for_func[0] = reg_1 ; 
                ans_for_func[1] = reg_2 ; 
                ans_for_func[2] = 0  ; 
                return ans_for_func ; 
            }
            else { 
                // error ; 
            }
            
        }
        else { 
            // error  ; 
        }
    }


    if (from->col_name != NULL) {
        int num = table_num(table_thing(from->extra_col )) % c->select->join[c->select->join_counter]->join_table_counter ; 
        int col_num = col_name_to_int(operand_thing(from->col_name)) ; 
        if (1){
            if (num  == target ){
                if (num > 1 ){
                    counts[num]-- ; 
                }
                else { 
                        *done = 1; 
                        int * ans_for_func[3] = malloc(2*sizeof(int)) ; 
                        ans_for_func[0] = reg_1 ; 
                        ans_for_func[1] = reg_2 ; 
                        ans_for_func[2] = 0  ; 
                        return ans_for_func ; 
                } 
            }
            else {
                if (num > 0 ){
                    counts[num]-- ; 
                }
                else { 
                    //error ; 
                }
            }
        }
        if (from->operator != NULL ){
            if (strcmp(from->operator , "+")== 0 ){
                operator = add_op ; 
            }
            else if (strcmp(from->operator , "-")== 0 ){
                operator = subs_op ; 
            }
            else if  (strcmp(from->operator , "*")== 0 ){
                operator = mul_op ; 
            }   
            else if  (strcmp(from->operator , "/")== 0 ){
                operator = divide_op ; 
            }
            else if (strcmp(from->operator , "IS NULL")== 0 ){
                operator = is_null ; 
            }
            else if(strcmp(from->operator , "IS NOT NULL")== 0 ){
                operator = is_not_null ; 
            }
            if (ans == NULL ){
                if (from->num_value != NULL ){
                    emit(c , integer_op , *from->num_value , reg_1 , -1  , NULL  ) ;   
                    emit(c , integer_op , *from->num_value , reg_2 , -1  , NULL  ) ;   
                }
                else if (from->char_value != NULL ){
                    emit(c , string_op ,-1 , reg_1 , -1  , (void*)from->char_value   ) ;   
                    emit(c , string_op ,-1 , reg_2 , -1  , (void*)from->char_value   ) ; 
                }
                else if (from->float_val != NULL ){
                    emit(c , real_op , -1, reg_1 , -1  , (void*)from->float_val   ) ;   
                    emit(c , real_op , -1, reg_2 , -1  , (void*)from->float_val   ) ;   
                }
                else if (from->blob != NULL ){
                    emit(c , blob_op ,-1 , reg_1 , -1  , (void*)from->blob   ) ;   
                    emit(c , blob_op ,-1 , reg_2 , -1  , (void*)from->blob   ) ;   
                }
            }
            int normal_val = malloc(2* sizeof(int)) ; 
                for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                    if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->table_name == extra_num ){
                        for (int j = 0; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter; j++) {
                                if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->col_name == extra_col_num) {
                                    normal_val = c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->range;
                                    break;
                                }
                        }
                    }
                }       
                if (ans != NULL ){
                emit(c , operator  , ans[0] , normal_val[0] , reg_1 , NULL ) ;
                emit(c , operator  , ans[1] , normal_val[1] , reg_2 , NULL ) ;
            }
            else { 
                emit(c , operator  , reg_1 , normal_val[0] , reg_1 , NULL ) ;
                emit(c , operator  , reg_2 , normal_val[1] , reg_2 , NULL ) ;
            }
            int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
            ans_for_func[0] = reg_1 ; 
            ans_for_func[1] = reg_2 ; 
            ans_for_func[2] = 0  ; 
            return ans_for_func ; 
        }
        else { 
            if (ans != NULL ){
                return ans  ; 
            }
            else { 
                // error ; 
            }
        }

    }

    else {
        if (ans != NULL ){
            if (from->num_value != NULL ){
                emit(c , integer_op , *from->num_value , reg_1 , -1  , NULL  ) ;   
                emit(c , integer_op , *from->num_value , reg_2 , -1  , NULL  ) ;   
            }
            else if (from->char_value != NULL ){
                emit(c , string_op ,-1 , reg_1 , -1  , (void*)from->char_value   ) ;   
                emit(c , string_op ,-1 , reg_2 , -1  , (void*)from->char_value   ) ; 
            }
            else if (from->float_val != NULL ){
                emit(c , real_op , -1, reg_1 , -1  , (void*)from->float_val   ) ;   
                emit(c , real_op , -1, reg_2 , -1  , (void*)from->float_val   ) ;   
            }
            else if (from->blob != NULL ){
                emit(c , blob_op ,-1 , reg_1 , -1  , (void*)from->blob   ) ;   
                emit(c , blob_op ,-1 , reg_2 , -1  , (void*)from->blob   ) ;   
            }

            emit(c , operator  , reg_1 , normal_val[0] , reg_1 , NULL ) ;
            emit(c , operator  , reg_2 , normal_val[1] , reg_2 , NULL ) ;
            int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
            ans_for_func[0] = reg_1 ; 
            ans_for_func[1] = reg_2 ; 
            ans_for_func[2] = 0  ; 
            return ans_for_func ; 
        }
        else {
            //error ; 
        }
    }
    int * ans_for_func[3] = malloc(3*sizeof(int)) ; 
    ans_for_func[0] = reg_1 ; 
    ans_for_func[1] = reg_2 ; 
    ans_for_func[2] = 0  ; 
    return ans_for_func ; 
}

int  find_the_path_of_the_stuff_main(temp_info_for_path * tp , compiler * c   , int target   ,  select_select_info * tree  ){
    if (tree == NULL){
        return 0 ; 
    }
    if (tree->col_name != NULL ){
        if (table_num(table_thing(tree->col_name)) == target){
            return 1 ; 
        } 
    }
    if (tree->extra_col != NULL ){ 
        if ( table_num(table_thing(tree->extra_col)) == target) {
            return 1;
        }
    }
    if (tree->left != NULL ){
        int num = find_the_path_of_the_stuff( c  , target   , tree->left   ) ; 
        if (num == 1 ){
            tp->path[tp->counter++] = 0 ; 
            return 1 ; 
        }
    }
    if (tree->right != NULL ){
        int num = find_the_path_of_the_stuff( c  , target   , tree->right   ) ; 
        if (num == 1 ){
                tp->path[tp->counter++] = 1 ; 
                return 1 ; 
        }
    }
    return 0 ; 
}

void find_the_path_of_the_stuff(temp_info_for_path * tp , compiler * c , int target ,   select_select_info * tree  ){
    int k = find_the_path_of_the_stuff_main( tp , c   ,  target   ,  tree  )
    int i = 0 ; 
    int j = tp->counter -1  ; 
    while ( i < j  ){
        int temp = tp->path[i] ; 
        tp->path[i] = tp->path[j] ; 
        tp->path[j] = temp ; 
    }
}


void  check_if_the_stuff_negation_or_not(compiler *c  , int target , select_select_info * tree  ) {
    int * ans[3] = malloc(3*sizeof(int)) ; 
    int left_stuff = c->register_counter++   ; 
    int right_stuff  = c->register_counter++   ; 
    int *done = malloc(sizeof(int)) ; 
    *done = 0 ; 
    int counts[300];
    for (int i = 0 ; i < c->select->join[c->select->join_counter]->join_table_counter ; i++) {
        counts[i] = 300;
    }
    ans = take_care_of_expression(c , target , tree , left  , right , done  , counts ) ; 
    emit(c ,check_negative , ans[0] , -1 , -1, NULL ) ; 
    emit(c ,check_negative , ans[1] , -1 , -1 , NULL ) ; 
}

void  check_if_the_stuff_negation_or_not_without_solver(compiler *c  , int*ans  ) { 
    emit(c ,check_negative , ans[0] , -1 , -1, NULL ) ; 
    emit(c ,check_negative , ans[1] , -1 , -1 , NULL ) ; 
}


final_finished_equation * found_the_target_to_be_deplished_solve_it(compiler * c , int target , int  side , select_select_info * tree  , int * left_ans ){
    select_select_info * new_one  = malloc(sizeof(select_select_info)); 
    final_finished_equation * answer = malloc(sizeof(final_finished_equation)) ; 
    static int trap_fired = 0 ; 
    if (side == 0 ){
        if (tree->right != NULL ){
            int * right_ans[3] = malloc(3*sizeof(int)) ; 
            int left = c->register_counter++   ; 
            int right  = c->register_counter++   ; 
            int *done = malloc(sizeof(int)) ; 
            *done = 0 ; 
            int counts[300];
            for (int i = 0 ; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                counts[i] = c->select->join[c->select->join_counter]->tables_occuring_number_of_times[i];
            }
            right_ans = take_care_of_expression(c , target , tree->right , left  , right , done  , counts ) ; 
        }
        else {              
            int left = c->register_counter++   ; 
            int right  = c->register_counter++ ; 
                if (tree->extra_col != NULL ){
                    int num = table_num(table_thing(tree->extra_col)) ; 
                    int col_name = col_name_to_int(operand_thing(tree->extra_col)) ; 
                    int normal_val = malloc(2* sizeof(int)) ; 
                    for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                        if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->table_name == num ){
                            for (int j = 0; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter; j++) {
                                    if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->col_name == col_num) {
                                        normal_val = c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->range;
                                        break;
                                    }
                            }
                        }
                    }
                    right_ans[0]  = normal_val[0] ; 
                    right_ans[1] = normal_val[1] ; 
                    right_ans[2] = 0 ; 
                }
                else if ( tree->num_value ){
                    emit(c , integer_op , tree->num_value  , left  , -1 , NULL  ) ; 
                    emit(c , integer_op , tree->num_value  , right  , -1 , NULL  ) ; 
                    right_ans[0]  = left ; 
                    right_ans[1] = right ; 
                    right_ans[2] = 0 ; 
                }
                else if (tree->char_value != NULL ){
                    emit(c , string_op , -1  , left  , -1 , tree->char_value   ) ; 
                    emit(c , string_op , -1 , right  , -1 , tree->char_value    ) ; 
                    right_ans[0]  = left ; 
                    right_ans[1] = right ;
                    right_ans[2] = 0 ; 
                }
                else if (tree->blob != NULL  ){
                    emit(c , blob_op , -1  , left  , -1 , tree->blob   ) ; 
                    emit(c , blob_op , -1 , right  , -1 , tree->blob    ) ; 
                    right_ans[0]  = left ; 
                    right_ans[1] = right ;
                    right_ans[2] = 0 ; 
                }
                else if (tree->float_val ){
                    emit(c , real_op , tree->num_value  , left  , -1 , NULL  ) ; 
                    emit(c , real_op , tree->num_value  , right  , -1 , NULL  ) ; 
                    right_ans[0]  = left ; 
                    right_ans[1] = right ;
                    right_ans[2] = 0 ; 
                }
        }

        temp_info_for_path * tp ; 
        find_the_path_of_the_stuff( tp  , c ,target , tree ) ; 
        int i = 0 ; 
        select_select_info * tree_temp  = tree ; 
        if (tp->counter == 1 ){
            strcpy(answer->operator , tree_temp->operator ) ; 
            answer->range = malloc(3*sizeof(int)) ; 
            answer->range  = right_ans ; 
            answer->target_table = table ; 
            answer->target_column = col_name_to_int(operand_thing(tree_temp->col_name)) ;   
            return answer ; 
        }
        else { 
            while ( i < tp->counter ){
                int * temp_ans[3] = malloc(3*sizeof(int)) ; 
                int left_one = c->register_counter++   ; 
                int right_one  = c->register_counter++   ; 
                int first_num = 0 ;
                if (tp->path[i] == 0 ){
                    temp_ans = take_care_of_expression(c , target , tree_temp->left  , left_one ,right_one , done , counts   ) ; 
                    check_if_the_stuff_negation_or_not_without_solver(c , temp_ans) ; 
                    first_num = trap_fired; 
                }
                else if (tp->path[i] == 1 ){
                    temp_ans = take_care_of_expression(c , target , tree_temp->right  , left_one ,right_one , done , counts   ) ; 
                    check_if_the_stuff_negation_or_not_without_solver(c , temp_ans) ; 
                    first_num = trap_fired; 
                }
                if (tree_temp->operator != NULL ){
                    if (strcmp(tree_temp->operator  , "+" ) == 0 ){
                        emit(c , subs_op , right_ans[0] , temp_ans[0] , right_ans[0] , NULL) ; 
                        emit(c , subs_op , right_ans[1] , temp_ans[1] , right_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "-" ) == 0 ){
                        emit(c , add_op , right_ans[0] , temp_ans[0] , right_ans[0] , NULL) ; 
                        emit(c , add_op , right_ans[1] , temp_ans[1] , right_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "*" ) == 0 ){
                        int num = 0 ; 
                        if (tp->path[i] == 0 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->left ) ; 
                            second_num = trap_fired ; 
                        }
                        else if (tp->path[i] == 1 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->right ) ; 
                            second_num = trap_fired ; 
                        }
                        if ( ( first_num == 1 && second_num == 0 )  || ( first_num == 0 && second_num == 1 ) ){
                                if (strcmp(tree->operator , ">")== 0 ){
                                strcpy(tree->operator , "<") ; 
                            }
                            else if  (strcmp(tree->operator , ">=")== 0 ){
                                strcpy(tree->operator , "<=") ; 
                            }
                            else if  (strcmp(tree->operator , "<")== 0 ){
                                strcpy(tree->operator , ">") ; 
                            }
                            else if (strcmp(tree->operator , "<=")== 0 ){
                                strcpy(tree->operator , ">=") ; 
                            }
                        }
                        emit(c , divide_op , right_ans[0] , temp_ans[0] , right_ans[0] , NULL) ; 
                        emit(c , divide_op , right_ans[1] , temp_ans[1] , right_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "/" ) == 0 ){
                        int second_num = 0  ; 
                        if (tp->path[i] == 0 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->left ) ; 
                            second_num = trap_fired ; 
                        }
                        else if (tp->path[i] == 1 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->right ) ; 
                            second_num = trap_fired ;  
                        }
                        if ( ( first_num == 1 && second_num == 0 )  || ( first_num == 0 && second_num == 1 ) ){
                                if (strcmp(tree->operator , ">")== 0 ){
                                strcpy(tree->operator , "<") ; 
                            }
                            else if  (strcmp(tree->operator , ">=")== 0 ){
                                strcpy(tree->operator , "<=") ; 
                            }
                            else if  (strcmp(tree->operator , "<")== 0 ){
                                strcpy(tree->operator , ">") ; 
                            }
                            else if (strcmp(tree->operator , "<=")== 0 ){
                                strcpy(tree->operator , ">=") ; 
                            }
                        }
                        emit(c , mul_op , right_ans[0] , temp_ans[0] , right_ans[0] , NULL) ; 
                        emit(c , mul_op , right_ans[1] , temp_ans[1] , right_ans[1] , NULL) ; 
                    }
                }
                if (tp->path[i] == 0 ){
                    tree_temp = tree_temp->left ; 
                }
                else if (tp->path[i] == 1 ){
                    tree_temp = tree_temp->right ; 
                }
                i++ ; 
            }
            if (tp->path[tp->counter -1 ] == 1 ){
                strcpy(answer->operator , tree_temp->operator ) ; 
                answer->range = malloc(3*sizeof(int)) ; 
                answer->range  = right_ans ; 
                answer->target_table = table ; 
                answer->target_column = col_name_to_int(operand_thing(tree_temp->extra_col)) ;   
                return answer ;  
            }
            else { 
                strcpy(answer->operator , tree_temp->operator ) ; 
                answer->range = malloc(3*sizeof(int)) ; 
                answer->range  = right_ans ; 
                answer->target_table = table ; 
                answer->target_column = col_name_to_int(operand_thing(tree_temp->col_name)) ;   
                return answer ; 
            }
        }
    }
    else { 
        temp_info_for_path * tp ; 
        find_the_path_of_the_stuff( tp  , c ,target , tree ) ; 
        int i = 0 ; 
        select_select_info * tree_temp  = tree ; 
        if (tp->counter == 1 ){
            strcpy(answer->operator , tree_temp->operator ) ; 
            answer->range = malloc(3*sizeof(int)) ; 
            answer->range  = left_ans ; 
            answer->target_table = table ; 
            answer->target_column = col_name_to_int(operand_thing(tree_temp->extra_col)) ;   
            return answer ; 
        }
        else { 
            while ( i < tp->counter ){
                int * temp_ans[3] = malloc(3*sizeof(int)) ; 
                int left_one = c->register_counter++   ; 
                int right_one  = c->register_counter++   ; 
                int first_num = 0 ;
                if (tp->path[i] == 0 ){
                    temp_ans = take_care_of_expression(c , target , tree_temp->left  , left_one ,right_one , done , counts   ) ; 
                    check_if_the_stuff_negation_or_not_without_solver(c , temp_ans) ; 
                    first_num = trap_fired; 
                }
                else if (tp->path[i] == 1 ){
                    temp_ans = take_care_of_expression(c , target , tree_temp->right  , left_one ,right_one , done , counts   ) ; 
                    check_if_the_stuff_negation_or_not_without_solver(c , temp_ans) ; 
                    first_num = trap_fired; 
                }
                if (tree_temp->operator != NULL ){
                    if (strcmp(tree_temp->operator  , "+" ) == 0 ){
                        emit(c , subs_op , left_ans[0] , temp_ans[0] , left_ans[0] , NULL) ; 
                        emit(c , subs_op , left_ans[1] , temp_ans[1] , left_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "-" ) == 0 ){
                        emit(c , add_op , left_ans[0] , temp_ans[0] , left_ans[0] , NULL) ; 
                        emit(c , add_op , left_ans[1] , temp_ans[1] , left_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "*" ) == 0 ){
                        int num = 0 ; 
                        if (tp->path[i] == 0 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->left ) ; 
                            second_num = trap_fired ; 
                        }
                        else if (tp->path[i] == 1 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->right ) ; 
                            second_num = trap_fired ; 
                        }
                        if ( ( first_num == 1 && second_num == 0 )  || ( first_num == 0 && second_num == 1 ) ){
                                if (strcmp(tree->operator , ">")== 0 ){
                                strcpy(tree->operator , "<") ; 
                            }
                            else if  (strcmp(tree->operator , ">=")== 0 ){
                                strcpy(tree->operator , "<=") ; 
                            }
                            else if  (strcmp(tree->operator , "<")== 0 ){
                                strcpy(tree->operator , ">") ; 
                            }
                            else if (strcmp(tree->operator , "<=")== 0 ){
                                strcpy(tree->operator , ">=") ; 
                            }
                        }
                        emit(c , divide_op , left_ans[0] , temp_ans[0] , left_ans[0] , NULL) ; 
                        emit(c , divide_op , left_ans[1] , temp_ans[1] , left_ans[1] , NULL) ; 
                    }
                    else if (strcmp(tree_temp->operator  , "/" ) == 0 ){
                        int second_num = 0  ; 
                        if (tp->path[i] == 0 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->left ) ; 
                            second_num = trap_fired ; 
                        }
                        else if (tp->path[i] == 1 ){
                            check_if_the_stuff_negation_or_not(c , target ,tree_temp->right ) ; 
                            second_num = trap_fired ;  
                        }
                        if ( ( first_num == 1 && second_num == 0 )  || ( first_num == 0 && second_num == 1 ) ){
                                if (strcmp(tree->operator , ">")== 0 ){
                                strcpy(tree->operator , "<") ; 
                            }
                            else if  (strcmp(tree->operator , ">=")== 0 ){
                                strcpy(tree->operator , "<=") ; 
                            }
                            else if  (strcmp(tree->operator , "<")== 0 ){
                                strcpy(tree->operator , ">") ; 
                            }
                            else if (strcmp(tree->operator , "<=")== 0 ){
                                strcpy(tree->operator , ">=") ; 
                            }
                        }
                        emit(c , mul_op , left_ans[0] , temp_ans[0] , left_ans[0] , NULL) ; 
                        emit(c , mul_op , left_ans[1] , temp_ans[1] , left_ans[1] , NULL) ; 
                    }
                }
                if (tp->path[i] == 0 ){
                    tree_temp = tree_temp->left ; 
                }
                else if (tp->path[i] == 1 ){
                    tree_temp = tree_temp->right ; 
                }
                i++ ; 
            }
            if (tp->path[tp->counter -1 ] == 1 ){
                strcpy(answer->operator , tree_temp->operator ) ; 
                answer->range = malloc(3*sizeof(int)) ; 
                answer->range  = right_ans ; 
                answer->target_table = table ; 
                answer->target_column = col_name_to_int(operand_thing(tree_temp->extra_col)) ;   
                return answer ;  
            }
            else { 
                strcpy(answer->operator , tree_temp->operator ) ; 
                answer->range = malloc(3*sizeof(int)) ; 
                answer->range  = right_ans ; 
                answer->target_table = table ; 
                answer->target_column = col_name_to_int(operand_thing(tree_temp->col_name)) ;   
                return answer ; 
            }
        }
    }
    return answer ; 
}

void* join_inequality_clause(compiler * c, select_select_info * from  , select_from_info * from_org  ){
    c->select->join[c->select->join_counter]->join_table_counter = tables_and_thier_cursor(c , from ) ; 
    for ( int i = 0 ; i < c->table_counter  ; i++ ){
        emit(c , open_read_op , c->cursor_num + i , c->tl->tables[i]->root_page_num ;  ,  -1 , -1 , NULL    ) ; 
    }
    tables_and_thier_cursor_num(c , from) ; 
    get_the_max_and_min( c ) ; 
    int loop_addr_hb = c->count ; 
        int first  ; 
        int first_table = table_name_from_num( c->tl , from_org->table_name)  ;  
        for ( int m = 0 ; m < c->select->join[c->select->join_counter]->join_table_counter ; m++ ){
            if (c->select->join[c->select->join_counter]->join_select_unique_table[m] == first  ){
                first = m ; 
                break ; 
            }
        }
        pirmary_key_first_loc = c->register_counter++ ; 
        emit(c , column_op , first , primary_key_offset( db , first_table ) , pirmary_key_first_loc , NULL) ; 

        int second_table = table_name_from_num( c->tl , from_org->join->table_or_col_name) ; 
        int second ;
        for ( int m = 0 ; m < c->select->join[c->select->join_counter]->join_table_counter ; m++ ){
            if (c->select->join[c->select->join_counter]->join_select_unique_table[m] == first  ){
                second = m ; 
                break ; 
            }
        }
        primary_key_second_loc = c->register_counter++ ; 
        emit(c , column_op , second , primary_key_offset( db , first_table ) ,primary_key_second_loc , NULL) ;
    
    final_finished_equation * final_equation[2] = malloc(2* sizeof(final_finished_equation)) ; 
    int cursors[2]   ;  
    int cursor_type[2] ; 
    for ( int a = 0 ; a < 2 ; a++ ){
        final_finished_equation * ans ; 
        ans = sizeof(final_finished_equation) ; 
        int target  ; 
        if (a == 0 ){
            target = table_num(table_thing(from_org->table_name)) ; 
        }
        if (a == 1 ){
            target = table_num(table_thing(from_org->join->table_or_col_name)) ; 
        }
        if (from->left != NULL ){
            int left = c->register_counter++   ; 
            int right  = c->register_counter++   ; 
            int *done = malloc(sizeof(int)) ; 
            *done = 0 ; 
            final_finished_equation * ans ; 
            int * left_ans[3] = malloc(3*sizeof(int)) ; 
            int counts[300];
            for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                counts[i] = c->select->join[c->select->join_counter]->tables_occuring_number_of_times[i];
            }
            if (from->left){
                left_ans = take_care_of_expression(c , target , from->left , left  , right , done , counts  ) ; 
            }
            else {
                int left_aha = c->register_counter++   ; 
                int right_aha  = c->register_counter++ ; 
                if (tree->extra_col != NULL ){
                    int num = table_num(table_thing(tree->extra_col)) ; 
                    int col_name = col_name_to_int(operand_thing(tree->extra_col)) ; 
                    int normal_val = malloc(2* sizeof(int)) ; 
                    for (int i = 0; i < c->select->join[c->select->join_counter]->join_table_counter; i++) {
                        if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->table_name == num ){
                            for (int j = 0; j < c->select->join[c->select->join_counter]->join_select_unique_table[i]->range_counter; j++) {
                                    if (c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->col_name == col_num) {
                                        normal_val = c->select->join[c->select->join_counter]->join_select_unique_table[i]->range[j]->range;
                                        break;
                                    }
                            }
                        }
                    }
                    left_ans[0]  = normal_val[0] ; 
                    left_ans[1] = normal_val[1] ; 
                    left_ans[2] = 0 ; 
                }
                else if ( tree->num_value ){
                    emit(c , integer_op , tree->num_value  , left_aha  , -1 , NULL  ) ; 
                    emit(c , integer_op , tree->num_value  , right_aha  , -1 , NULL  ) ; 
                    left_ans[0]  = left_aha ; 
                    left_ans[1] = right_aha ; 
                    left_ans[2] = 0 ; 
                }
                else if (tree->char_value != NULL ){
                    emit(c , string_op , -1  , left_aha  , -1 , tree->char_value   ) ; 
                    emit(c , string_op , -1 , right_aha  , -1 , tree->char_value    ) ; 
                    left_ans[0]  = left_aha ; 
                    left_ans[1] = right_aha ;
                    left_ans[2] = 0 ; 
                }
                else if (tree->blob != NULL  ){
                    emit(c , blob_op , -1  , left_aha  , -1 , tree->blob   ) ; 
                    emit(c , blob_op , -1 , right_aha  , -1 , tree->blob    ) ; 
                    left_ans[0]  = left_aha ; 
                    left_ans[1] = right_aha ;
                    left_ans[2] = 0 ; 
                }
                else if (tree->float_val ){
                    emit(c , real_op , tree->num_value  , left_aha  , -1 , NULL  ) ; 
                    emit(c , real_op , tree->num_value  , right_aha  , -1 , NULL  ) ; 
                    left_ans[0]  = left_aha ; 
                    left_ans[1] = right_aha ;
                    left_ans[2] = 0 ; 
                }
            }
            if (left_ans[2] == 1 ){
                ans = found_the_target_to_be_deplished_solve_it(c , target , 0  , from  , left_ans )
            }


        }

        if (from->right != NULL ){
            int left = c->register_counter++ ; 
            int right  = c->register_counter++ ; 
            int *done = malloc(sizeof(int)) ; 
            *done = 0 ; 
            int counts[300];
            for (int i = 0; i < c->select->join[c->selec t->join_counter]->join_table_counter; i++) {
                counts[i] = c->select->join[c->select->join_counter]->tables_occuring_number_of_times[i];
            }
            if (from->right){
                take_care_of_expression(c , target , from->right , left  , right , done  , counts ) ;
            }
            if (left_ans[2] == 1 ){
                ans = found_the_target_to_be_deplisheedd_solve_it(c , target , 1  , from  , left_ans )
            } 
        }
        final_equation[a] = ans ; 
    }   

    for ( int a = 0 ; a < 2 ; a++ ){
        if (primary_key_offset( db , final_equation[a]->target_table  ) == final_equation[a]->target_column  ){
            cursor_type[a] = 0 ; 
            cursors[a] = final_equation[a]->target_table  ; 
        }
        else { 
            int cursor_for_sort = c->sorter_cursor++ ; 
            int cursor = c->cursor_num++ ; 
            cursor_type[a] = 1 ; 
            cursors[a] = cursor_for_sort ; 
            emit(c , open_read_op , cursor , c->tl->tables[table_num(table_thing(final_equation[a]->target_table))]->root_page_num ; ,  -1 , -1 , NULL    ) ; 
            emit(c , sorter_open , cursor_for_sort , final_equation[a]->target_column , -1 , NULL ) ; 
            int loop_count = c->count ; 
            int check = c->register_counter++ ; 
            int turn ; 
            emit(c , column_op , cursor , final_equation[a]->target_column  , check , NULL )  ; 
            if (strcmp(final_equation[a]->target_column , ">" )){
                emit(c , gt_op , final_equation[a]->range[0] , -1 , check , NULL  ) ; 
                turn = c->count ; 
            }
            else if (strcmp(final_equation[a]->target_column , ">=" )){
                emit(c , ge_op , final_equation[a]->range[0] , -1 , check , NULL  ) ; 
                turn = c->count ; 
            }
            else if (strcmp(final_equation[a]->target_column , "<=" )){
                emit(c , le_op , final_equation[a]->range[0] , -1 , check , NULL  ) ; 
                turn = c->count ; 
            }
            else if (strcmp(final_equation[a]->target_column , "<" )){
                emit(c , lt_op , final_equation[a]->range[0] , -1 , check , NULL  ) ; 
                turn = c->count ; 
            }
            int start = c->register_counter ; 
            int cur ; 
            for (int i = 0 ; i < c->tl->tables[table_num(table_thing(final_equation[a]->target_table))]->num_of_columns ; i++  ){
                cur = c->register_counter++ ; 
                emit(c , column_op , cursor , i , cur ,  NULL ) ; 
            }
            emit(c , make_record , start , cur , start , NULL ) ; 
            emit(c , sorter_insert , cursor_sort, start , -1  , NULL) ; 
            c->typ[turn]->p2 = c->count ; 
            emit(c , next_cursor , cursor ,loop_count , -1 , NULL ) ; 
            emit(c , sorter_sort , cursor_sort , -1 , -1 , NULL ) ; 
        }
    }

    int first = c->cursor_num++ ; 
    int second = c->cursor_num++ ; 
    if (cursor_type[0] == 0  ){
        emit(c , open_read_op ,first , c->tl->tables[table_num(table_thing(final_equation[a]->target_table))]->root_page_num , -1 , -1 , NULL  ) ; 
    }
    else { 
        
    }


    
    
}



int get_the_join_cursor(compiler * c  , char * operator){
    int tb_num =  table_num(c , operator  ) ; 
    for ( int i  = 0 ; i < c->select->join[c->select->join_counter]->join_table_counter ; i++ ){
        if (c->select->join[c->select->join_counter]->join_select_unique_table[i] == tb_num){
            return i ; 
        }
    }
    return -1 ; 
}

int join_func(compiler *c , select_select_info * node  ){
    int temp = c->register_counter ; 
    int reg = join_func_main(c , node) ; 
    c->register_counter = temp ; 
    return reg ; 
}

int join_func_main(compiler *c , select_select_info * node   ){
    int reg  = c->register_counter   ; 
    int operator ; 
    if (node->operator != NULL  ) {
        if (strcmp(node->operator , "+")== 0 ){
            operator = add_op ; 
        }
        else if (strcmp(node->operator , "-")== 0 ){
            operator = subs_op ; 
        }
        else if  (strcmp(node->operator , "*")== 0 ){
            operator = mul_op ; 
        }   
        else if  (strcmp(node->operator , "/")== 0 ){
            operator = divide_op ; 
        }
        else if (strcmp(node->operator , "IS NULL")== 0 ){
                operator = is_null ; 
        }
        else if(strcmp(node->operator , "IS NOT NULL")== 0 ){
                operator = is_not_null ; 
        }
        else { 
            return reg  ; 
        }
        int num = col_name_to_int_main( operand_thing(node->col_name), c->select   ) ; 

        int cursor = get_the_join_cursor(c , table_thing(node->col_name) ) ; 


        if (node->right == NULL && node->left == NULL  ){
            if (operator != is_null  && operator != is_not_null ){
                int reg_left = c->register_counter++ ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , column_op ,cursor , num , reg_left  , NULL  ) ;  
                    }
                    else {
                        if (node->num_value != NULL ){
                            emit(c , integer_op , *node->num_value , reg_left , -1  , NULL  ) ;   
                        }
                        else if (node->char_value != NULL ){
                            emit(c , string_op ,-1 , reg_left , -1  , (void*)node->char_value   ) ;   
                        }
                        else if (node->float_val != NULL ){
                            emit(c , real_op , -1, reg_left , -1  , (void*)node->float_val   ) ;   
                        }
                        else if (node->blob != NULL ){
                            emit(c , blob_op ,-1 , reg_left , -1  , (void*)node->blob   ) ;   
                        }
                    }
                }

                int reg_right =  c->register_counter++ ;    
                if (1){
                    if (node->extra_col != NULL ){
                        int extra_num = col_name_to_int_main( operand_thing(node->extra_col), c->select   ) ; 
                        int extra_cursor = get_the_join_cursor(c , table_thing(node->extra_col) ) ;  
                        emit(c , column_op ,extra_cursor , extra_num , reg_right  , NULL  ) ;  
                    }
                    else {
                        if (node->num_value != NULL ){
                            emit(c , integer_op , *node->num_value , reg_right , -1  , NULL  ) ;   
                        }
                        else if (node->char_value != NULL ){
                            emit(c , string_op ,-1 , reg_right , -1  , (void*)node->char_value   ) ;   
                        }
                        else if (node->float_val != NULL ){
                            emit(c , real_op , -1, reg_right , -1  , (void*)node->float_val   ) ;   
                        }
                        else if (node->blob != NULL ){
                            emit(c , blob_op ,-1 , reg_right , -1  , (void*)node->blob   ) ;   
                        }
                    }
                }
                reg = c->register_counter++  ; 
                emit(c , operator ,reg_left ,reg_right , reg , -1 , NULL ) ;    
        }
        else {
            int reg_temp = c->register_counter++  ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , column_op ,cursor , num , reg_left  , NULL  ) ;  
                    }
                    else {
                        if (node->num_value != NULL ){
                            emit(c , integer_op , *node->num_value , reg_temp , -1  , NULL  ) ;   
                        }
                        else if (node->char_value != NULL ){
                            emit(c , string_op ,-1 , reg_temp , -1  , (void*)node->char_value   ) ;   
                        }
                        else if (node->float_val != NULL ){
                            emit(c , real_op , -1, reg_temp , -1  , (void*)node->float_val   ) ;   
                        }
                        else if (node->blob != NULL ){
                            emit(c , blob_op ,-1 , reg_temp , -1  , (void*)node->blob   ) ;   
                        }
                    }
                }
                reg = c->register_counter++  ; 
                emit(c , operator ,reg_temp ,-1 , reg , -1 , NULL ) ;    
            }               
        }

        else if (node->right != NULL && node->left == NULL ) {
        if (operator != is_null  && operator != is_not_null ){
            int reg_right = func(c , node->right   ) ; 
            int reg_left =  c->register_counter++ ;  
                    if (1){
                        if (node->col_name != NULL ){
                            emit(c , column_op ,cursor , num , reg_left  , NULL  ) ;  
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , reg_left , NULL  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , reg_left , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, reg_left , N-1ULL  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){p
                                emit(c , blob_op ,-1 , reg_left , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
            reg = c->register_counter++  ; 
            emit(c , operator ,reg_left, reg_right , reg , -1 , NULL ) ;   
        }
        else  {
                int reg_temp = func(c , node->right  ) ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , column_op ,cursor , num , reg_temp  , NULL  ) ;  
                    }
                    else {
                        if (node->num_value != NULL ){
                            emit(c , integer_op , *node->num_value , reg_temp , -1  , NULL  ) ;   
                        }
                        else if (node->char_value != NULL ){
                            emit(c , string_op ,-1 , reg_temp , -1  , (void*)node->char_value   ) ;   
                        }
                        else if (node->float_val != NULL ){
                            emit(c , real_op , -1, reg_temp , -1  , (void*)node->float_val   ) ;   
                        }
                        else if (node->blob != NULL ){
                            emit(c , blob_op ,-1 , reg_temp , -1  , (void*)node->blob   ) ;   
                        }
                    }
                }
                reg = c->register_counter++  ; 
                emit(c , operator ,reg_temp ,-1 , reg , -1 , NULL ) ;    

        }    

        }

        else if(node->left != NULL && node->right == NULL ){
        if (operator != is_null  && operator != is_not_null ){
            int reg_left = func(c , node->left  ) ; 
            int reg_right =  c->register_counter++ ;  
                    if (1){
                        if (node->col_name != NULL ){
                            emit(c , column_op ,cursor , num , reg_right  , NULL  ) ;  
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , reg_right , -1  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , reg_right , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, reg_right , -1  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){
                                emit(c , blob_op ,-1 , reg_right , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
            reg = c->register_counter++  ; 
            emit(c , operator ,reg_left , reg_right  , reg , -1 , NULL ) ;  
        }
        else  {
                int reg_temp = func(c , node->left  ) ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , column_op ,cursor , num , reg_temp  , NULL  ) ;  
                    }
                    else {
                        if (node->num_value != NULL ){
                            emit(c , integer_op , *node->num_value , reg_temp , -1  , NULL  ) ;   
                        }
                        else if (node->char_value != NULL ){
                            emit(c , string_op ,-1 , reg_temp , -1  , (void*)node->char_value   ) ;   
                        }
                        else if (node->float_val != NULL ){
                            emit(c , real_op , -1, reg_temp , -1  , (void*)node->float_val   ) ;   
                        }
                        else if (node->blob != NULL ){
                            emit(c , blob_op ,-1 , reg_temp , -1  , (void*)node->blob   ) ;   
                        }
                    }
                }
                reg = c->register_counter++  ; 
                emit(c , operator ,reg_temp ,-1 , reg , -1 , NULL ) ;    

        }
        }
        else { 
            int reg_right = func(c , node->right   ) ; 
            int reg_left = func(c , node->left  ) ; 
            reg = c->register_counter++  ; 
            emit(c , operator ,reg_left , reg_right , reg , -1 , NULL ) ;  
        }
    }

    return reg ; 
}

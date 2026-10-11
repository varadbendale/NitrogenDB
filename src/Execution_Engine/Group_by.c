#include "Engine.h"

void groupby_parser_to_struct(compiler *c , tree * select ){
    int i = 0 ; 
    while (i < select->num ){
         if (strcmp(select->children[i]->comp  , "GROUP BY") == 0 ){
            tree * groupby = c->select->children[i] ; 
            int k = 0 ; 
            select_select_info *gb =  c->select->groupby ; 
                while (k < groupby->num && strcmp(groupby->children[k]->comp , "HAVING") != 0  ){
                if (strcmp(groupby->children[k]->comp, "+") == 0 || strcmp(groupby->children[k]->comp, "-") == 0 ||strcmp(groupby->children[k]->comp, "*") == 0 || strcmp(groupby->children[k]->comp, "/") == 0 || strcmp(groupby->children[k]->comp, "=") == 0 || strcmp(groupby->children[k]->comp, "!=") == 0 || strcmp(groupby->children[k]->comp, ">") == 0 || strcmp(groupby->children[k]->comp, ">=") == 0 || strcmp(groupby->children[k]->comp, "<") == 0 || strcmp(groupby->children[k]->comp, "<=") == 0 || strcmp(groupby->children[k]->comp, "GROUP_CONCAT") == 0 || strcmp(groupby->children[k]->comp, "MAX") == 0 || strcmp(groupby->children[k]->comp, "MIN") == 0 || strcmp(groupby->children[k]->comp, "COUNT") == 0 || strcmp(groupby->children[k]->comp, "AVG") == 0 || strcmp(groupby->children[k]->comp, "SUM") == 0) {
                    gb[c->select->groupby_counter] = malloc(sizeof(select_select_info));
                    gb[c->select->groupby_counter] = expre(gb[c->select->groupby_counter] , c, groupby->children[k]);
                    if (select->as != NULL) {
                        gb[c->select->groupby_counter]->as = select->as;
                    }
                    c->select.groupby_counter++;
                }
                else {
                    if (col_name_to_int_main(groupby->children[k]->comp, c->select) != -1) {
                        gb[c->select->groupby_counter] = malloc(sizeof(select_select_info));
                        gb[c->select->groupby_counter]->col_name = groupby->children[k]->comp;
                        gb[c->select->groupby_counter]->operator = NULL;
                        gb[c->select->groupby_counter]->left = NULL;
                        gb[c->select->groupby_counter]->right = NULL;
                        if (select->as != NULL) {
                            gb[c->select->groupby_counter]->as = select->as;
                        }
                        c->select->groupby_counter++;
                    }
                    else {
                        int check = data_type_check(groupby->children[k]->comp);
                        if (check == 0) {
                            gb[c->select->groupby_counter]  = malloc(sizeof(select_select_info));
                            gb[c->select->groupby_counter]->col_name = NULL;
                            gb[c->select->groupby_counter]->operator = NULL;
                            gb[c->select->groupby_counter]->left = NULL;
                            gb[c->select->groupby_counter]->right = NULL;
                            gb[c->select->groupby_counter]->num_value = atoi(groupby->children[k]->comp);
                            c->select->groupby_counter++;
                        }
                        else if (check == 1) {
                            gb[c->select->groupby_counter]  = malloc(sizeof(select_select_info));
                            gb[c->select->groupby_counter]->col_name = NULL;
                            gb[c->select->groupby_counter]->operator = NULL;
                            gb[c->select->groupby_counter]->left = NULL;
                            gb[c->select->groupby_counter]->right = NULL;
                            gb[c->select->groupby_counter]->float_val = (float)atof(groupby->children[k]->comp);
                            c->select->groupby_counter++;
                        }
                        else if (check == 2) {
                            gb[c->select->groupby_counter]  = malloc(sizeof(select_select_info));
                            gb[c->select->groupby_counter]->col_name = NULL;
                            gb[c->select->groupby_counter]->operator = NULL;
                            gb[c->select->groupby_counter]->left = NULL;
                            gb[c->select->groupby_counter]->right = NULL;
                            gb[c->select->groupby_counter]->blob =  groupby->children[k]->comp;
                            c->select->groupby_counter++;
                        }
                        else {
                            gb[c->select->groupby_counter]  = malloc(sizeof(select_select_info));
                            gb[c->select->groupby_counter]->col_name = NULL;
                            gb[c->select->groupby_counter]->operator = NULL;
                            gb[c->select->groupby_counter]->left = NULL;
                            gb[c->select->groupby_counter]->right = NULL;
                            gb[c->select->groupby_counter]->char_value = groupby->children[k]->comp;
                            c->select->groupby_counter++;
                        }
                    }
                }

                k++;
            }
            else if (strcmp(groupby->children[k]->comp , "HAVING") == 0  ){
                select_select_info *temp ; 
                c->select->having = expre(temp , c , groupby->children[k] ) ; 
            }
        }

    }
}


int groupby_func(compiler * c , select_select_info * temp  , bool final ){
    int addk = c->register_counter  ; 
    int ans = group_by_func(c ,temp , final ) ; 
    c->register_counter = addk  ; 
    return ans ; 
}


void get_all_select_stuff(compiler * c ){
    int i = 0 ; 
    while ( i < c->select->col_counter ){
            int num = col_name_to_int_main( c->select->sel[i]->col_name , c->select  )  ; 
        if (num != -1 ){
            if (c->select->sel[i]->operator ==  NULL ){
                if (c->select->hash[num] != num ){
                    c->select->hash[num] = num  ; 
                    c->select->gb_select_unique[c->select->sel_uni_counter++] = num ; 
                }
            }
            else {
                select_select_info *temp = c->select->sel[i] ; 
                if (temp->operator != NULL ){
                    get_the_data_tree(temp , c->select) ; 
                }
            }
        }
        i++ ;
    }
    return  ; 
}


void get_all_hash_covered_gb(compiler * c ){
    int i = 0 ; 
    int num ; 
    while ( i < c->select->groupby_counter){
        num = col_name_to_int_main( c->select->sel[i]->col_name  , c->select  )   ; 
        if (num != -1 ){
            if (c->select->groupby[i]->operator == NULL ){
                if ( c->select->hash[num] != num  ){
                    c->select->hash[num] = num  ; 
                }
            }
        }
        i++ ; 
    }
    return ; 
}

void get_the_tree_hash(select_select_info * temp , select_info *  sf ){
    int num = col_name_to_int_main( temp->col_name , sf  )  ; 
    int extra_num ; 
    if (num != -1 ){
        if (temp->left == NULL && temp->right == NULL  ){
                extra_num = col_name_to_int_main( temp->extra_col , sf  )   ; 
            if ( sf->hash[num] != num ){
                sf->hash[num] = num ; 
            }
            if (extra_num != -1 ){
                if ( sf->hash[extra_num] != extra_num ){
                    sf->hash[extra_num] = extra_num  ; 
                }
                }
            return  ; 
        }

        else { 
            if ( sf->hash[num] != num ){
                sf->hash[num] = num; 
            }
        }
    }

    if (temp->left != NULL ){
        get_the_tree_hash(temp->left , sf) ; 
    }
    if (temp->right != NULL ){
        get_the_tree_hash(temp->right , sf) ; 
    }

    return ; 
}

void get_the_data_tree(select_select_info * temp , select_info *  sf ){
    int num = col_name_to_int_main( temp->col_name , sf  )  ; 
    int extra_num ; 
    if (num != -1 ){
        if (temp->left == NULL && temp->right == NULL  ){
                extra_num = col_name_to_int_main( temp->extra_col , sf  )   ; 
            if ( sf->hash[num] != num ){
                sf->hash[num] = num  ; 
                sf->gb_select_unique[sf->sel_uni_counter++ ] = num ; 
            }
            if (extra_num != -1 ){
            if ( sf->hash[extra_num] != extra_num ){
                sf->hash[extra_num] = extra_num  ; 
                sf->gb_select_unique[sf->sel_uni_counter++ ] = extra_num ; 
            }
            }
            return  ; 
        }

        else { 
            if ( sf->hash[num] != num ){
                sf->hash[num] = num  ; 
                sf->gb_select_unique[sf->sel_uni_counter++ ] = num ; 
            }
        }
    }

    if (temp->left != NULL ){
        get_the_data_tree(temp->left , sf) ; 
    }
        if (temp->right != NULL ){
        get_the_data_tree(temp->right , sf) ; 
    }

    return ; 
}

void sort_groupby(compiler * c  ){
    int i = 0 ; 
    int start = c->register_start + c->register_counter ; 
    int cur = start ; 
    int norm_cursor = c->cursor_num++  ; 
    while ( i < c->select->sel_uni_counter){
        emit(c , column_op ,norm_cursor , c->select->gb_select_unique[i] , cur  , NULL  ) ;    
        i++ ; 
        cur++ ; 
    }
    i = 0 ; 
    if (c->select->orderby_counter > 0 ){
        while ( i < c->select->orderby_counter){
            int num = col_name_to_int_main( c->select->orderby[i]->ob_name , c->select   )   ; 
            if (num != -1   ){
                if (c->select->orderby[i]->operator != NULL ){
                    int temp = c->register_counter ; 
                    c->register_counter = cur ; 
                    c->register_counter = normal_func(c ,c->select->orderby[i]  , false  ) ; 
                    c->register_counter++ ; 
                    c->register_counter = temp ; 
                    cur++ ; 
                }
            }
            else { 
                break ; 
            }
            i++ ; 
        }
    }
    i = 0 ; 
    while ( i < c->select->groupby_counter){
        int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   )   ; 
        if (num != -1   ){
            if (c->select->groupby[i]->operator != NULL ){
                int temp = c->register_counter ; 
                c->register_counter = cur ; 
                c->register_counter = normal_func(c ,c->select->sel[i]  , false  ) ; 
                c->register_counter++ ; 
                c->register_counter = temp ; 
            }
            else { 
                emit(c , column_op ,norm_cursor , num , cur  , NULL  ) ;  
            }
        }
        else { 
            break ; 
        }
        i++ ; 
        cur++ ; 
    }
    emit(c , make_record , start , cur , start , NULL ) ; 
    emit(c , sorter_insert , cursor_sort, start , -1  , NULL) ; 
}

int  group_by_func(compiler *c , select_select_info * node  , bool final ){
    int reg = c->register_counter    ; 
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
        else if  (strcmp(node->operator , "=")== 0 ){
            operator = eq_select_op ; 
        }
        else if  (strcmp(node->operator , "!=")== 0 ){
                operator = ne_select_op ; 
        }
        else if (strcmp(node->operator , ">")== 0 ){
                operator = gt_select_op ; 
        }
        else if  (strcmp(node->operator , ">=")== 0 ){
                operator = ge_select_op ; 
        }
        else if  (strcmp(node->operator , "<")== 0 ){
                operator = lt_select_op ; 
        }
        else if (strcmp(node->operator , "<=")== 0 ){
                operator = le_select_op ; 
        }
        else if (strcmp(node->operator , "AND")== 0 ){
                operator = and_op ; 
        }
        else if (strcmp(node->operator , "OR")== 0 ){
                operator = or_op ; 
        }
        else if (strcmp(node->operator , "IS NULL")== 0 ){
                operator = is_null ; 
        }
        else if(strcmp(node->operator , "IS NOT NULL")== 0 ){
                operator = is_not_null ; 
        }
        else if (strcmp(node->operator , "GROUP_CONCAT")== 0 || strcmp(node->operator , "MAX") == 0   || strcmp(node->operator , "MIN") == 0 || strcmp(node->operator , "COUNT") == 0 || strcmp(node->operator , "AVG") == 0 || strcmp(node->operator , "SUM") == 0     ){
            if (node->acc_reg == -1 ){
                node->acc_reg = c->register_counter++   ; 
                emit(c ,aggregate_init ,node->acc_reg , -1 , -1 , NULL  ) ; 
            }
            aggregate_select(c , node ) ; 
            if (final == true ){
                emit(c ,aggregate_final ,node->acc_reg , -1 , node->acc_reg  , NULL  ) ; 
                emit(c ,aggregate_reset , node->acc_reg  , -1 , -1 , NULL ) ; 
            }
        }
        else { 
            return reg  ; 
        }
        int num = col_name_to_int_main( node->col_name , c->select   ) ; 
        int cursor = c->cursor_num ; 


        if (node->right == NULL && node->left == NULL  ){
            if (operator != is_null  && operator != is_not_null ){
                int reg_left = c->register_counter++ ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , gb_specific_column_op , MAX - 1 , num , reg_left  , NULL  ) ;  
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
                        int extra_num = col_name_to_int_main( node->extra_col , c->select   ) ; 
                        emit(c , gb_specific_column_op , MAX - 1 , extra_num , reg_right  , NULL  ) ;   
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
                            emit(c , gb_specific_column_op , MAX - 1 , num , reg_temp  , NULL  ) ; 
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
                int reg_right = group_by_func(c , node->right  , final ) ; 
                int reg_left =  c->register_counter++ ;  
                        if (1){
                            if (node->col_name != NULL ){
                                emit(c , gb_specific_column_op , MAX - 1 , num , reg_left  , NULL  ) ; 
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
                reg = c->register_counter++  ; 
                emit(c , operator ,reg_left, reg_right , reg , -1 , NULL ) ;   
            }
            else  {
                    int reg_temp =  group_by_func(c , node->right  , final ) ; 
                    if (1){
                        if (node->col_name != NULL ){
                            emit(c , gb_specific_column_op , MAX - 1 , num , reg_temp  , NULL  ) ; 
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
            int reg_left =  group_by_func(c , node->left  , final ) ; 
            int reg_right =  c->register_counter++ ;  
                    if (1){
                        if (node->col_name != NULL ){
                            emit(c , gb_specific_column_op , MAX - 1 , num , reg_right  , NULL  ) ; 
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
                int reg_temp =  group_by_func(c , node->left  , final ) ; 
                if (1){
                    if (node->col_name != NULL ){
                        emit(c , gb_specific_column_op , MAX - 1 , num , reg_temp  , NULL  ) ; 
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
        int reg_right =  group_by_func(c , node->right  , final ) ; 
        int reg_left =  group_by_func(c , node->left  , final ) ; 
        reg = c->register_counter++  ; 
        emit(c , operator ,reg_left , reg_right , reg , -1 , NULL ) ;  
    }
    }
    return reg ; 
}
#include "Engine.h"

select_ob_info *expre_order_by(select_ob_info *ans ,    compiler *c , tree * temp){
    int i = 0 ; 
    while (i < temp->num){
        if (strcmp(temp->children[i]->comp , "+")== 0 || strcmp(temp->children[i]->comp, "-")== 0 || strcmp(temp->children[i]->comp, "*")== 0 || strcmp(temp->children[i]->comp, "/")== 0 || strcmp(temp->children[i]->comp , "=")== 0 || strcmp(temp->children[i]->comp, "!=")== 0 || strcmp(temp->children[i]->comp , ">")== 0 || strcmp(temp->children[i]->comp , ">=")== 0 || strcmp(temp->children[i]->comp , "<")== 0 || strcmp(temp->children[i]->comp, "<=")== 0 || strcmp(temp->children[i]->comp , "GROUP_CONCAT")== 0 || strcmp(temp->children[i]->comp , "MAX") == 0 || strcmp(temp->children[i]->comp , "MIN") == 0 || strcmp(temp->children[i]->comp , "COUNT") == 0 || strcmp(temp->children[i]->comp, "AVG") == 0 || strcmp(temp->children[i]->comp , "SUM") == 0){
                ans->operator = temp->children[i]->comp;
            if (ans->left == NULL ){
                ans->left = malloc(sizeof(select_ob_info))
                expre_order_by(ans->left, c, temp->children[i]);
            }
            else {
                ans->right = malloc(sizeof(select_ob_info))
                expre_order_by(ans->right, c, temp->children[i]);
            }
        }
        else if (col_name_to_int_main(temp->children[i]->comp, c->select) != -1 ){
            if (ans->col_name != NULL ){
                    ans->extra_ob_name = temp->children[i]->comp ;
            }
            else { 
                ans->ob_name = temp->children[i]->comp ;
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

void where_parser_to_struct(compiler *c , tree * select ){
    int i = 0 ; 
    while (i < select->num ){
        if(strcmp(select->children[i]->comp  , "ORDER BY") == 0 ){
            tree * orderby = c->select->children[i] ;  
            select_ob_info *ob =  c->select->orderby ; 
            int k = 0 ; 
            while (  k < orderby->num ){
                if (c->select->groupby_counter > 0 ){
                if (orderby->children[k]->operator != NULL ){ 
                        ob[c->select->orderby_counter] = malloc(sizeof(select_select_info));
                        ob[c->select->orderby_counter]->ob_name = orderby->children[k] ; 
                        ob[c->select->orderby_counter]->operator = NULL;
                        ob[c->select->orderby_counter]->left = NULL;
                        ob[c->select->orderby_counter]->right = NULL;
                        if (orderby->children[k]->as != NULL) {
                            ob[c->select->orderby_counter]->as = orderby->children[k]->as;
                        }
                        if ( orderby->direction[k] ){
                                ob[c->select->orderby_counter]->dir = orderby->direction[k] ;
                        }
                        if (k+ 1 < orderby->num ){
                            if ( strcmp(orderby->children[k+1]->comp , "NULLS" ) == 0  ){
                                select_ob_info *ob_child = orderby->children[k] ; 
                                ob_child->extra_info_ob->nulls = malloc(sizeof(orderby->children[k+1]->comp)) ; 
                                memcpy(ob_child->extra_info_ob->nulls , orderby->children[k+1]->comp , sizeof(orderby->children[k+1]->comp) ) ; 
                                if (orderby->children[k+1]->num > 0 ){
                                    ob_child->extra_info_ob->order = malloc(sizeof(orderby->children[k+1]->children[0]->comp)) ; 
                                    memcpy(ob_child->extra_info_ob->order  , orderby->children[k+1]->children[0]->comp , sizeof(orderby->children[k+1]->children[0]->comp) ) ; 
                                }
                                else { 
                                    ob_child->extra_info_ob->order = "FIRST"; 
                                }
                                k++ ; 
                                continue  ; 
                            }
                        }

                        c->select->orderby_counter++;
                }
                else { 
                    if (strcmp(groupby->children[k]->comp, "+") == 0 || strcmp(groupby->children[k]->comp, "-") == 0 ||strcmp(groupby->children[k]->comp, "*") == 0 || strcmp(groupby->children[k]->comp, "/") == 0 || strcmp(groupby->children[k]->comp, "=") == 0 || strcmp(groupby->children[k]->comp, "!=") == 0 || strcmp(groupby->children[k]->comp, ">") == 0 || strcmp(groupby->children[k]->comp, ">=") == 0 || strcmp(groupby->children[k]->comp, "<") == 0 || strcmp(groupby->children[k]->comp, "<=") == 0 || strcmp(groupby->children[k]->comp, "GROUP_CONCAT") == 0 || strcmp(groupby->children[k]->comp, "MAX") == 0 || strcmp(groupby->children[k]->comp, "MIN") == 0 || strcmp(groupby->children[k]->comp, "COUNT") == 0 || strcmp(groupby->children[k]->comp, "AVG") == 0 || strcmp(groupby->children[k]->comp, "SUM") == 0) {
                        ob[c->select->orderby_counter] = malloc(sizeof(select_select_info));
                        ob[c->select->orderby_counter] = expre_order_by(ob[c->select->orderby_counter] , c , orderby->children[k] );
                            ob[c->select->orderby_counter]->operator = NULL;
                        ob[c->select->orderby_counter]->left = NULL;
                        ob[c->select->orderby_counter]->right = NULL;
                        if (orderby->children[k]->as != NULL) {
                            ob[c->select->orderby_counter]->as = orderby->children[k]->as;
                        }
                        if ( orderby->direction[k] ){
                                ob[c->select->orderby_counter]->dir = orderby->direction[k] ;
                        }
                        if (k+ 1 < orderby->num ){
                            if ( strcmp(orderby->children[k+1]->comp , "NULLS" ) == 0  ){
                                select_ob_info *ob_child = orderby->children[k] ; 
                                ob_child->extra_info_ob->nulls = malloc(sizeof(orderby->children[k+1]->comp)) ; 
                                memcpy(ob_child->extra_info_ob->nulls , orderby->children[k+1]->comp , sizeof(orderby->children[k+1]->comp) ) ; 
                                if (orderby->children[k+1]->num > 0 ){
                                    ob_child->extra_info_ob->order = malloc(sizeof(orderby->children[k+1]->children[0]->comp)) ; 
                                    memcpy(ob_child->extra_info_ob->order  , orderby->children[k+1]->children[0]->comp , sizeof(orderby->children[k+1]->children[0]->comp) ) ; 
                                }
                                else { 
                                    ob_child->extra_info_ob->order = "FIRST"; 
                                }
                                k++ ; 
                                continue  ; 
                            }
                        }

                        c->select->orderby_counter++;
                    }
                }
                }
                else { 
                if (orderby->children[k]->operator != NULL ){ 
                        ob[c->select->orderby_counter] = malloc(sizeof(select_select_info));
                        ob[c->select->orderby_counter]->ob_name = orderby->children[k] ; 
                        ob[c->select->orderby_counter]->operator = NULL;
                        ob[c->select->orderby_counter]->left = NULL;
                        ob[c->select->orderby_counter]->right = NULL;
                        if (orderby->children[k]->as != NULL) {
                            ob[c->select->orderby_counter]->as = orderby->children[k]->as;
                        }
                        if ( orderby->direction[k] ){
                                ob[c->select->orderby_counter]->dir = orderby->direction[k] ;
                        }
                        if (k+ 1 < orderby->num ){
                            if ( strcmp(orderby->children[k+1]->comp , "NULLS" ) == 0  ){
                                select_ob_info *ob_child = orderby->children[k] ; 
                                ob_child->extra_info_ob->nulls = malloc(sizeof(orderby->children[k+1]->comp)) ; 
                                memcpy(ob_child->extra_info_ob->nulls , orderby->children[k+1]->comp , sizeof(orderby->children[k+1]->comp) ) ; 
                                if (orderby->children[k+1]->num > 0 ){
                                    ob_child->extra_info_ob->order = malloc(sizeof(orderby->children[k+1]->children[0]->comp)) ; 
                                    memcpy(ob_child->extra_info_ob->order  , orderby->children[k+1]->children[0]->comp , sizeof(orderby->children[k+1]->children[0]->comp) ) ; 
                                }
                                else { 
                                    ob_child->extra_info_ob->order = "FIRST"; 
                                }
                                k++ ; 
                                continue  ; 
                            }
                        }
                        c->select->orderby_counter++;
                }
                }
                k++ ; 
            }
        }

    }
}


unsigned char *  sorter_orderby_init(compiler * c  , int groupby , int first , int first_table_num , int second , int second_table_num  ){
    orderby_key_cols * ob[300] ; 
    int ob_counter = 0 ; 
    int col = 0 ;
    for ( int i = 0 ; i < c->select->orderby_counter ; i++ ){
        if (c->select->orderby[i]->operator != NULL ){
            ob[i] = malloc(sizeof(orderby_key_cols));
            if (col == 0 ){
                if (groupby == 0 ){
                    col = c->select->col_counter++ ; 
                }
                else { 
                    col = c->select->sel_uni_counter++ ; 
                }

            }
            else if (col != 0 ) {
                col = col + 1 ; 
            }

            ob[ob_counter]->reg_num = col ; 
            if (strcmp(c->select->orderby[i]->dir , "ASC") == 0 ){
                ob[ob_counter]->direction = 0 ; 
            }
            else if (strcmp(c->select->orderby[i]->dir , "DSC") == 0 ) { 
                ob[ob_counter]->direction = 1 ; 
            }
            if (c->select->orderby[i]->ex != NULL){
                if (strcmp(c->select->orderby[i]->ex->order , "LAST") == 0 ){
                    ob[ob_counter]->nullslast = 1 ; 
                    ob[ob_counter]->nullsfirst = 0 ; 
                }
                else { 
                    ob[ob_counter]->nullsfirst = 1 ; 
                    ob[ob_counter]->nullslast = 0 ; 
                }
            }
            else if (c->select->orderby[i]->ex == NULL ){
                    ob[ob_counter]->nullsfirst = 0 ; 
                    ob[ob_counter]->nullslast = 0 ; 
            }
            if (ob_counter < 300 ){
                ob_counter++ ; 
            }

        }   
        else { 
            ob[i] = malloc(sizeof(orderby_key_cols));
            ob[ob_counter]->reg_num = col_name_to_int_main( c->select->orderby[i]->ob_name , c->select ) ; 
            if (col == 0 ){
                if (groupby == 0 ){
                    col = c->select->col_counter++ ; 
                }
                else { 
                    col = c->select->sel_uni_counter++ ; 
                }
            }
            else if (col != 0 ) {
                col = col + 1 ; 
            }
            ob[ob_counter]->reg_num = col ; 
            if (strcmp(c->select->orderby[i]->dir , "ASC") == 0 ){
                ob[ob_counter]->direction = 0 ; 
            }
            else if (strcmp(c->select->orderby[i]->dir , "DSC") == 0 ) { 
                ob[ob_counter]->direction = 1 ; 
            }
            if (c->select->orderby[i]->ex != NULL){
                if (strcmp(c->select->orderby[i]->ex->order , "LAST") == 0 ){
                    ob[ob_counter]->nullslast = 1 ; 
                    ob[ob_counter]->nullsfirst = 0 ; 
                }
                else { 
                    ob[ob_counter]->nullsfirst = 1 ; 
                    ob[ob_counter]->nullslast = 0 ; 
                }
            }
            else if (c->select->orderby[i]->ex == NULL ){
                    ob[ob_counter]->nullsfirst = 0 ; 
                    ob[ob_counter]->nullslast = 0 ; 
            }
            if (ob_counter < 300 ){
                ob_counter++ ; 
            }

        }
    }
    int struct_size = sizeof(orderby_key_cols);   
    int total_size = struct_size * ob_counter;
    unsigned char *buffer = malloc(total_size);
    int offset = 0;
    for (int i = 0; i < ob_counter ; i++) {
        memcpy(buffer + offset, ob[i], struct_size);
        offset += struct_size;
    }
    return buffer ; 
}


int orderby_func_main(compiler *c , select_ob_info * node ){
    int first_reg = c->register_counter ; 
    int ans = orderby_func(c , node) ; 
    c->register_counter = first_reg ; 
    return ans ;  
}

int orderby_func(compiler *c , select_ob_info * node , bool final , int first , int first_table_num , int second , int second_table_num ){
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

        int num ; 
        int table_number ; 
        if (second_table_num != -1 ){
            table_number = table_num(c->tl ,table_thing(node->ob_name)) ; 
            num = col_name_to_int( operand_thing(node->ob_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(node->ob_name))] )   ; 
        }
        else {
            num = col_name_to_int_main(node->ob_name  , c->select )   ; 
        }


        if (node->right == NULL && node->left == NULL  ){
            if (operator != is_null  && operator != is_not_null ){
                int reg_left = c->register_counter++ ; 
                if (1){
                    if (node->ob_name != NULL ){
                        if (num != -1 ){
                            int tum = 0 ; 
                            if (second_table_num != -1 ){
                                if (first_table_num != -1 && table_number == first_table_num ){
                                    tum = first ; 
                                }
                                else { 
                                    if (second_table_num != -1 ) {
                                        tum = second ;      
                                    }
                                    else { 
                                        //error ; 
                                    }
                                }
                            }
                            else { 
                                tum = first ; 
                            }
                            emit(c , register_get_stuff ,tum , num , reg_left  , NULL  ) ;  
                        }
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
                    if (node->extra_ob_name != NULL ){
                        int extra_num ; 
                        int extra_table_number ; 
                        if (second_table_num != -1 ){
                            extra_table_number = table_num(c->tl ,table_thing(node->extra_ob_name)) ; 
                            extra_num = col_name_to_int( operand_thing(node->extra_ob_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(node->extra_ob_name))] )   ; 
                        }
                        else {
                            extra_num = col_name_to_int_main(node->extra_ob_name  , c->select )   ; 
                        }
                        if (extra_num != -1 ){
                            int tum = 0 ; 
                            if (second_table_num != -1 ){
                                if (first_table_num != -1 && extra_table_number == first_table_num ){
                                    tum = first ; 
                                }
                                else { 
                                    if (second_table_num != -1 ) {
                                        tum = second ;      
                                    }
                                    else { 
                                        //error ; 
                                    }
                                }
                            }
                            else { 
                                tum = first ; 
                            }
                            emit(c , register_get_stuff ,tum , extra_num , reg_right  , NULL  ) ;  
                        }
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
                    if (node->ob_name != NULL ){
                        if (num != -1 ){
                            int tum = 0 ; 
                            if (second_table_num != -1 ){
                                if (first_table_num != -1 && table_number == first_table_num ){
                                    tum = first ; 
                                }
                                else { 
                                    if (second_table_num != -1 ) {
                                        tum = second ;      
                                    }
                                    else { 
                                        //error ; 
                                    }
                                }
                            }
                            else { 
                                tum = first ; 
                            }
                            emit(c , register_get_stuff ,tum , num , reg_temp  , NULL  ) ;  
                        }
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
            int reg_right = orderby_func(c , node->right ,final , first , first_table_num , second , second_table_num  ) ; 
            int reg_left =  c->register_counter++ ;  
                    if (1){
                        if (node->ob_name != NULL ){
                            if (num != -1 ){
                                int tum = 0 ; 
                                if (second_table_num != -1 ){
                                    if (first_table_num != -1 && table_number == first_table_num ){
                                        tum = first ; 
                                    }
                                    else { 
                                        if (second_table_num != -1 ) {
                                            tum = second ;      
                                        }
                                        else { 
                                            //error ; 
                                        }
                                    }
                                }
                                else { 
                                    tum = first ; 
                                }
                                emit(c , register_get_stuff ,tum , num , reg_left  , NULL  ) ;  
                            }
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
                int reg_temp = orderby_func(c , node->right ,final , first , first_table_num , second , second_table_num ) ; 
                if (1){
                    if (node->ob_name != NULL ){
                        if (num != -1 ){
                            int tum = 0 ; 
                            if (second_table_num != -1 ){
                                if (first_table_num != -1 && table_number == first_table_num ){
                                    tum = first ; 
                                }
                                else { 
                                    if (second_table_num != -1 ) {
                                        tum = second ;      
                                    }
                                    else { 
                                        //error ; 
                                    }
                                }
                            }
                            else { 
                                tum = first ; 
                            }
                            emit(c , register_get_stuff ,tum , num , reg_temp  , NULL  ) ;  
                        }
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
            int reg_left = orderby_func(c , node->left,final , first , first_table_num , second , second_table_num  ) ; 
            int reg_right =  c->register_counter++ ;  
                    if (1){
                        if (node->ob_name != NULL ){
                            if (num != -1 ){
                                int tum = 0 ; 
                                if (second_table_num != -1 ){
                                    if (first_table_num != -1 && table_number == first_table_num ){
                                        tum = first ; 
                                    }
                                    else { 
                                        if (second_table_num != -1 ) {
                                            tum = second ;      
                                        }
                                        else { 
                                            //error ; 
                                        }
                                    }
                                }
                                else { 
                                    tum = first ; 
                                }
                                emit(c , register_get_stuff ,tum , num , reg_right  , NULL  ) ;  
                            }
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
                int reg_temp = orderby_func(c , node->left ,final , first , first_table_num , second , second_table_num ) ; 
                if (1){
                    if (node->ob_name != NULL ){
                        if (num != -1 ){
                            int tum = 0 ; 
                            if (second_table_num != -1 ){
                                if (first_table_num != -1 && table_number == first_table_num ){
                                    tum = first ; 
                                }
                                else { 
                                    if (second_table_num != -1 ) {
                                        tum = second ;      
                                    }
                                    else { 
                                        //error ; 
                                    }
                                }
                            }
                            else { 
                                tum = first ; 
                            }
                            emit(c , register_get_stuff ,tum , num , reg_temp  , NULL  ) ;  
                        }
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
            int reg_right = orderby_func(c , node->right ,final , first , first_table_num , second , second_table_num  ) ; 
            int reg_left = orderby_func(c , node->left,final , first , first_table_num , second , second_table_num  ) ; 
            reg = c->register_counter++  ; 
            emit(c , operator ,reg_left , reg_right , reg , -1 , NULL ) ;  
        }
    }

    return reg ; 
}

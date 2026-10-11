#include "Engine.h"
#include <limits.h>
#define MAX 300
// what you need to do is just take up the stuff annd make the bytecode of it simple need to like get the the hash cursor and then the register where the stuff is prsemt and then one of the register which liek holds all the stuff in it the primary keys and all the info liek and then and then push it to the bytecode and then it process it and boom ohh god i reallty didnt wanted it to get extended but its alright i guess 


void emit(compiler * c , int op_code , int p1 , int p2 , int p3 , void * p4  ){
    if (c->count == c->capacity ){
        c->capactiy  = c->capacity * 2 ; 
        c->typ = realloc(c->typ , c->capacity * sizeof(type)) ; 
    }
    c->typ[c->count].operand =  op_code ; 
    c->typ[c->count].p1 = p1 ; 
    c->typ[c->count].p2 = p2 ; 
    c->typ[c->count].p3 = p3 ; 
    c->typ[c->count].p4 = p4 ; 
    c->count++ ; 

}

enum column_types{ 
    expression = 1 , 
    column_name , 
    aggregate_func  , 
    normal 
}



tree * select ; 
tree *from ; 

int is_blob_literal(char *temp) {
    if ((temp[0] == 'X' || temp[0] == 'x') && temp[1] == '\'') {
        return 1;
    }
    return 0;
}

int  data_type_check( char *str) {
    char *endptr;

    strtol(str, &endptr, 10);
    if (*endptr == '\0' && endptr != str){
            return 0 ;
    }

    strtod(str, &endptr);
    if (*endptr == '\0' && endptr != str){
        return 1 ; 
    } 

    if (is_blob_literal(str)){
        return 2 ; 
    }
    return 3 ;
}

select_select_info *expre(select_select_info *ans ,    compiler *c , tree * temp){
    int i = 0 ; 
    while (i < temp->num){
        if (strcmp(temp->children[i]->comp , "+")== 0 || strcmp(temp->children[i]->comp, "-")== 0 || strcmp(temp->children[i]->comp, "*")== 0 || strcmp(temp->children[i]->comp, "/")== 0 || strcmp(temp->children[i]->comp , "=")== 0 || strcmp(temp->children[i]->comp, "!=")== 0 || strcmp(temp->children[i]->comp , ">")== 0 || strcmp(temp->children[i]->comp , ">=")== 0 || strcmp(temp->children[i]->comp , "<")== 0 || strcmp(temp->children[i]->comp, "<=")== 0 || strcmp(temp->children[i]->comp , "GROUP_CONCAT")== 0 || strcmp(temp->children[i]->comp , "MAX") == 0 || strcmp(temp->children[i]->comp , "MIN") == 0 || strcmp(temp->children[i]->comp , "COUNT") == 0 || strcmp(temp->children[i]->comp, "AVG") == 0 || strcmp(temp->children[i]->comp , "SUM") == 0){
                ans->operator = temp->children[i]->comp;
            if (ans->left == NULL ){
                ans->left = malloc(sizeof(select_select_info))
                expre(ans->left, c, temp->children[i]);
            }
            else {
                ans->right = malloc(sizeof(select_select_info))
                expre(ans->right, c, temp->children[i]);
            }
        }
        else if (col_name_to_int_main(temp->children[i]->comp, c->select) != -1 ){
            if (ans->col_name != NULL ){
                    ans->extra_col = temp->children[i]->comp ;
            }
            else { 
                ans->col_name = temp->children[i]->comp ;
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


void select_parser_to_struct(  compiler *c , tree * select ){
    select_select_info *sel = c->select->sel ; 
    c->select.col_counter = 0 ; 
    int i = 0 ; 
    while ( i < select->num ){
        from_parser_to_struct(c , select ) ; 
        while(strcmp(select->children[i]->comp  , "FROM") != 0  ){
            if (1){
                if (strcmp(select->children[i]->comp  , "*") == 0 ){
                    for (int k = 0 ; k < c->tables_counter ; k++ ){
                        table * temp = lookup_table(c->tl , c->select->from[k] ) ; 
                        for ( int j = 0 ; j < temp->num_of_columns ; j++ ){
                            sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                            sel[c->select.col_counter ]->col_name = temp->col[j].name ; 
                            sel[c->select.col_counter ]->operator = NULL ; 
                            sel[c->select.col_counter ]->left = NULL ; 
                            sel[c->select.col_counter ]->right = NULL ; 
                            if (select->as != NULL ){
                                sel[i]->as = select->as ; 
                            }
                            c->select.col_counter ++ ; 
                        }
                    }
                }   
                else  {
                    if (strcmp(select->children[i]->comp , "+")== 0 || strcmp(select->children[i]->comp, "-")== 0 || strcmp(select->children[i]->comp , "*")== 0  || strcmp(select->children[i]->comp , "/")== 0 || strcmp(select->children[i]->comp, "=")== 0 || strcmp(select->children[i]->comp , "!=")== 0  || strcmp(select->children[i]->comp, ">")== 0 ||  strcmp(select->children[i]->comp , ">=")== 0  || strcmp(select->children[i]->comp , "<")== 0 ||  strcmp(select->children[i]->comp , "<=")== 0 || strcmp(select->children[i]->comp , "GROUP_CONCAT")== 0 || strcmp(select->children[i]->comp, "MAX") == 0 || strcmp(select->children[i]->comp , "MIN") == 0 || strcmp(select->children[i]->comp, "COUNT") == 0 || strcmp(select->children[i]->comp, "AVG") == 0 || strcmp(select->children[i]->comp , "SUM") == 0){
                            sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                            sel[c->select.col_counter ] = expre(sel[c->select.col_counter ] , c , select->children[i] )  ; 
                            if (select->as != NULL ){
                                sel[i]->as = select->as ; 
                            }
                            c->select.col_counter ++ ; 
                    }
                    else { 
                        if(col_name_to_int_main(select->children[i]->comp, c->select) != -1){
                            sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                            sel[c->select.col_counter ]->col_name = select->children[i]->comp ; 
                            sel[c->select.col_counter ]->operator = NULL ; 
                            sel[c->select.col_counter ]->left = NULL ; 
                            sel[c->select.col_counter ]->right = NULL ; 
                            if (select->as != NULL ){
                                sel[i]->as = select->as ; 
                            }
                            c->select.col_counter ++ ;  
                        }
                        else { 
                            int check = data_type_check(select->children[i]->comp);
                            if (check == 0){
                                sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                                sel[c->select.col_counter ]->col_name =  NULL ; 
                                sel[c->select.col_counter ]->operator = NULL ; 
                                sel[c->select.col_counter ]->left = NULL ; 
                                sel[c->select.col_counter ]->right = NULL ; 
                                sel[c->select.col_counter ]->num_value = atoi(select->children[i]->comp);
                                c->select.col_counter ++ ; 
                            }
                            else if (check == 1){
                                sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                                sel[nc->select.col_counter um]->col_name =  NULL ; 
                                sel[c->select.col_counter ]->operator = NULL ; 
                                sel[c->select.col_counter ]->left = NULL ; 
                                sel[c->select.col_counter ]->right = NULL ; 
                                sel[c->select.col_counter]->float_val = (float)atof(select->children[i]->comp);
                                c->select.col_counter ++ ; 

                            }
                            else if (check == 2){
                                sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                                sel[c->select.col_counter ]->col_name =  NULL ; 
                                sel[c->select.col_counter ]->operator = NULL ; 
                                sel[c->select.col_counter ]->left = NULL ; 
                                sel[c->select.col_counter ]->right = NULL ; 
                                sel[c->select.col_counter]->blob = select->children[i]->comp ;
                                c->select.col_counter ++ ; 

                            }
                            else {
                                sel[c->select.col_counter ] = malloc(sizeof(select_select_info)) ; 
                                sel[c->select.col_counter ]->col_name =  NULL ; 
                                sel[c->select.col_counter ]->operator = NULL ; 
                                sel[c->select.col_counter ]->left = NULL ; 
                                sel[c->select.col_counter ]->right = NULL ; 
                                sel[c->select.col_counter ]->char_value = select->children[i]->comp ;
                                c->select.col_counter++ ; 
                            }
                        }
                    }
                }
            }
        }
        if (strcmp(select->children[i]->comp  , "WHERE") == 0  ){
            where_parser_to_struct(c , select->children[i] ) ; 
        }


    }
}

void where_parser_to_struct(compiler *c , tree * select ){
    int i = 0 ; 
    while (i < select->num ){
        if (i == 0 ){
            select_select_info *temp ; 
            c->select->where = expre(temp , c , select->children[i] ) ; 
        }
        else if (strcmp(select->children[i]->comp  , "CASE") == 0 ){

        }
        else if (strcmp(select->children[i]->comp  , "LIMIT") == 0 || strcmp(select->children[i]->comp  , "OFFSET") == 0  ){

        }

    }
}


int col_name_to_int( char * column_name , table * t ){
    for (int i = 0 ; i < t->num_of_columns ; i++ ){
        if ( strcmp(t->col[i].name , column_name ) == 0 ){
            return i ; 
        }
    }
    return -1  ; 
}


int col_name_to_int_main( char * column_name , select_info *sf  ){
    int num = -1 ; 
    for ( int i = 0 ; i < sf->tab->num_of_tables ; i++ ){
        int number = col_name_to_int(column_name ,sf->from[i]->table_name ) ; 
        if ( number  != -1  ){
            if (num != -1 ){
                return -1 ;  
            }
            else {
                num = number ; 
            }
        }  
    }
    return num ; 
}


table * lookup_table( tables_list * tab , char * table_name ){
    for ( int i = 0 ; i < tab->tables->num_of_tables ; i++ ){
        if ( strcmp( tab[i]->name  , table_name  ) == 0 ) { 
            return tab[i] ; 
        }
    }
    return NULL ; 
}





// see where you need ot begin is the equal things in the join can be done easily byt the hash but for the rest of them the thing like suppose jsut read this prompt if you want in the nothing ( bhuiit) claude i cant really explain it so yeah work from here 
void compile_select (compiler *c ){
    emit(c , begin_op  , -1 , -1 , -1 , NULL ) ; 
    int cursor = c->cursor_num++ ; 
    int cursor_for_sort_orderby  = 0 ; 
    emit(c , open_read_op , cursor , sql_master->page_num ,  -1 , -1 , NULL    ) ; 
    emit(c , rewind_cursor , cursor , -1 , -1 , -1 , NULL  ) ; 
    int register_num = c->register_counter++ ; 
    c->register_start = register_num ; 
    if (c->select->groupby_counter == 0 ){
        if (c->select->orderby_counter > 0 ){
                cursor_for_sort_orderby = c->sorter_cursor++ ; 
            emit(c , sorter_open , cursor_for_sort_orderby , c->select->orderby_counter , -1 , sorter_orderby_init(c , 0 )  ) ; 
        }
        int loop_addr = c->count ; 
        emit(c , eq_op , where_func(c ,c->select->where , false  ) , -1  , MAX , "BINARY" ) ; 
        for ( int i = 0 ; i < c->select->col_counter ; i++  ){
            int num = col_name_to_int_main( c->select->sel[i]->col_name , c->select   ) ; 
                if (c->select->sel[i]->operator == NULL ){
                    register_num = c->register_counter++ ; 
                    if (1){
                        select_select_info *node = c->select->sel[i] ; 
                        if (node->col_name != NULL ){
                            if (num != -1 ){
                                emit(c , column_op ,cursor , num , register_num  , NULL  ) ;  
                            }
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){
                                emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
                }
                else { 
                    if ( num != -1 ){
                        c->register_counter = normal_func(c ,c->select->sel[i]  , false ) ; 
                        c->register_counter++ ; 
                    }
                }
        }
        if (c->select->orderby_counter > 0 ){
            int extra_depletion_record = 0 ; 
            for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                if (c->select->orderby[l]->operator != NULL){
                    c->register_counter = orderby_func_main(c , c->select->orderby[l] , false ) ; 
                    c->register_counter++ ; 
                    extra_depletion_record++ ; 
                }
            }
            emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
            emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
            c->register_counter = c->register_counter - extra_depletion_record ; 
        }
        if (c->select->orderby_counter == 0 ){
            if (c->select->select_agg != true ){
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            }
        }
        c->typ[loop_addr]->p2 = c->count ; 
        emit(c , next_cursor , cursor , loop_addr   , -1 , NULL ) ;
        if (c->select->orderby_counter == 0 ){
            if (c->select->select_agg != true ){
                for ( int z = 0 ; z < c->select->col_counter ;z++ ){
                    int extra_stupid = 0 ; 
                    if (c->select->sel[z]->acc_reg != -1 ){
                        emit(c ,aggregate_final , c->select->sel[z]->acc_reg , -1 , c->register_start + z , NULL  ) ;
                    }
                }
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            }
        }
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
            int orderby_sort_addr = c->count ; 
            int start_orderby_reg_counter = c->register_counter++ ; 
            emit(c , sorter_data , cursor_for_sort_orderby , c->register_counter , -1 , NULL ) ; 
            emit(c , result_row , start_orderby_reg_counter , start_orderby_reg_counter+1 , -1  , -1 , NULL ) ; 
            emit(c , sorter_next , cursor_for_sort_orderby , orderby_sort_addr , -1 , NULL ) ; 
        }
        emit(c, close_cursor_op , cursor, -1, -1, -1, NULL);
        emit(c, halt, -1, -1, -1, -1, NULL);
    }
    // so where you need to start is na like make the new campare function for the orderby as per ythe nulls and the direction and stuff make it not that hard 

    else {
        int loop_addr = c->count ; 
        emit(c , eq_op , where_func(c ,c->select->where , false  ) , -1  , MAX , "BINARY" ) ; 
        int cursor_for_sort_orderby  = 0 ;
        int cursor_sort = c->sorter_cursor++ ; 
        emit(c , sorter_open , cursor_sort,c->select->groupby_counter , -1 , { col_name_to_int_main(c->select->groupby[sel_uni_counter]->col_name   , c->select)} ) ; 
        get_all_select_stuff(c) ; 
        get_all_hash_covered_gb(c) ; 
        if (c->select->orderby_counter > 0 ){
            cursor_for_sort_orderby = c->sorter_cursor++   ; 
            emit(c , sorter_open , cursor_for_sort_orderby , c->select->orderby_counter , -1 , sorter_orderby_init(c , 1 )  ) ; 
        }
        int loop_addr_gb = c->count ; 
        sort_groupby(c) ;  
        emit(c , next_cursor , cursor , loop_addr_gb   , -1 , NULL ) ; 
        emit(c , rewind_cursor , cursor , -1 , -1 , -1 , NULL  ) ; 
        emit(c , sorter_sort , cursor_sort, -1 , -1 , NULL ) ; 
        emit(c , sorter_data , cursor_sort , MAX -1   , -1 , NULL ) ; 
        emit(c , gb_sorter_data , MAX - 1  , c->select->sel_uni_counter , MAX - 1 , NULL    ) ; 
        int sorter_next_jump = c->count ; 
        emit(c , sorter_next , cursor_sort, -1 , -1 , NULL ) ; 
        emit(c , sorter_data ,cursor_sort , MAX - 2    , -1 , NULL ) ; 
        emit(c ,gb_sorter_data , MAX - 2    , c->select->sel_uni_counter , MAX - 2  , NULL    ) ; 
        emit(c , ne_op , MAX - 2   , -1 ,  MAX - 1  , NULL  ) ; 
            int addrwe = c->count ; 
            for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
                int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                    if (c->select->groupby[i]->operator == NULL ){
                        register_num = c->register_counter++ ; 
                        if (1){
                            select_select_info *node = c->select->groupby[i] ; 
                            if (node->col_name != NULL ){
                                if (num != -1 ){
                                emit(c , gb_specific_column_op , MAX - 1 , num , register_num  , NULL  ) ;  
                                }
                            }
                            else {
                                if (node->num_value != NULL ){
                                    emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                                }
                                else if (node->char_value != NULL ){
                                    emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                                }
                                else if (node->float_val != NULL ){
                                    emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                                }
                                else if (node->blob != NULL ){
                                    emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                                }
                            }
                        }
                    }
                    else { 
                        if ( num != -1 ){
                                c->register_counter =  groupby_func(c ,c->select->groupby[i]  , true ) ; 
                                c->register_counter+ ; 
                        }
                    }
            }
            emit(c , copy_op , MAX - 2  , MAX -1  , -1 , NULL ) ; 
            int having   = 0 ; 
            if ( c->select->having != NULL  ){
                if (c->select->having->acc_reg != -1 ){
                    having = where_func(c ,c->select->having  , true   ) ; 
                }
                else { 
                    having = where_func(c ,c->select->having  , false    ) ;    
                }

            }
            emit(c , eq_op , having , -1 ,  MAX , NULL  ) ; 
            int gb_hav = c->count ; 
            if (c->select->orderby_counter > 0 ){
                int extra_depletion_record = 0 ; 
                for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                    if (c->select->orderby[l]->operator != NULL){
                        c->register_counter = orderby_func_main(c , c->select->orderby[l] , false ) ; 
                        c->register_counter++ ; 
                        extra_depletion_record++ ; 
                    }
                }
                emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
                emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
                c->register_counter = c->register_counter - extra_depletion_record ; 
            }
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            c->typ[gb_hav].p2 = c->count ; 
            emit(c ,goto_op , -1 ,  sorter_next_jump  , -1 , NULL ) ; 
            c->typ[addrwe].p2 = c->count ; 
            for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
                int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                    if (c->select->groupby[i]->operator != NULL ){
                        if ( num != -1 ){
                                int not_needed = groupby_func(c ,c->select->groupby[i]  , false ) ; 
                        }
                    }

            }
            emit(c , copy_op , MAX - 2  , MAX -1  , -1 , NULL ) ; 
            emit(c ,goto_op , -1 ,  sorter_next_jump  , -1 , NULL ) ; 

        c->typ[sorter_next_jump + 1].p2 = c->count ; 
        for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
            int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                if (c->select->groupby[i]->operator == NULL ){
                    register_num = c->register_counter++ ; 
                    if (1){
                        select_select_info *node = c->select->groupby[i] ; 
                        if (node->col_name != NULL ){
                            if (num != -1 ){
                            emit(c , gb_specific_column_op , MAX - 1 , num , register_num  , NULL  ) ;  
                            }
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){
                                emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
                }
                else { 
                    if ( num != -1 ){
                            c->register_counter =  groupby_func(c ,c->select->groupby[i]  , true ) ; 
                            c->register_counter++ ; 
                    }
                }
        }
        emit(c , eq_op , having , -1 ,  MAX , NULL  ) ; 
        int gb_hav_fin = c->count ; 
        if (c->select->orderby_counter > 0 ){
            int extra_depletion_record = 0 ; 
            for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                if (c->select->orderby[l]->operator != NULL){
                    c->register_counter = orderby_func_main(c , c->select->orderby[l] , false ) ; 
                    c->register_counter++ ; 
                    extra_depletion_record++ ; 
                }
            }
            emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
                emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
            c->register_counter = c->register_counter - extra_depletion_record ; 
        }
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
        }
        if (c->select->orderby_counter == 0 ){
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
        }
        c->typ[gb_hav_fin].p2 = c->count ; 
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
            int orderby_sort_addr = c->count ; 
            int start_orderby_reg_counter = c->register_counter++ ; 
            emit(c , sorter_data , cursor_for_sort_orderby , c->register_counter , -1 , NULL ) ; 
            emit(c , result_row , start_orderby_reg_counter , start_orderby_reg_counter+1 , -1  , -1 , NULL ) ; 
            emit(c , sorter_next , cursor_for_sort_orderby , orderby_sort_addr , -1 , NULL ) ; 
        }
        emit(c, close_cursor_op , cursor, -1, -1, -1, NULL) ;
        emit(c, halt, -1, -1, -1, -1, NULL) ;
        }
    
}

// one more boring stuff simply see liek here in the where na we have to check wheter the thing we have is 0 or 1 true or false if it is false you need to do the next command execute so for that one once you do the next command so now the thing is na we have the eq_op bytecode for the thing which like checks if the thing is  true or flase  then it like jumps to the next part the issue we dont know where the next_op thing will come in the execution so we simply put it as -1 and then we just updat ething thing when we find it simple as that 
// okay one of the most insane boring thing which happens here is see man like the loop occurs in the bytecodes itself so when we like put the register_counter like see we did the thing and as soo nas we hit the next_op it calls the bytecoders which we passed on earleir the earleir one okay only that gets called we are not calling anything in the compile_seelct getting ti it is complelty different thing got it 

// bug alert the aggregate init and the aggregate final pretty much break up everything in the thing itds just supported ffor the main stuff but for the function where the aggregate is pretty much used it breaks out there so yeah need to do it 
void aggregate_select(compiler *c , select_select_info * node , int first , int first_table_num , int second , int second_table_num   ){
    void * operation = node->operator ; 
    int reg ; 
    if (node->col_name != NULL) {      
        int num ; 
        int table_number ; 
        if (second_table_num != -1 ){
            table_number = table_num(c->tl ,table_thing(c->select->sel[i]->col_name)) ; 
            num = col_name_to_int( operand_thing(c->select->sel[i]->col_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(c->select->sel[i]->col_name))] )   ; 
        }
        else {
            num = col_name_to_int_main(c->select->sel[i]->col_name  , c->select )   ; 
        }
       
        reg = c->register_counter++;
            if (node->col_name != NULL ){
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
                    emit(c , register_get_stuff ,tum , num , register_num  , NULL  ) ;  
                }
            }
    }
    else if (node->left != NULL) {      
        c->register_counter = normal_func(c ,c->select->sel[i]  , false ) ; 
        c->register_counter++ ;        
        reg = func(c, node->left, cursor ,  first ,  first_table_num ,  second ,  second_table_num );
    }
    else {                         
        reg = -1;
    }
    emit(c , aggregate_step , node->acc_reg , reg , NULL , operation) ; 
}

int where_func(compiler *c , select_select_info * node , bool req ){
    int first_reg = c->register_counter ; 
    int ans = func(c , node , req   ,  first ,  first_table_num ,  second ,  second_table_num  ) ; 
    emit(c , integer_op , 0 , MAX , -1 , NULL ) ; 
    c->register_counter = first_reg ; 
    return ans ;  
}

int normal_func(compiler *c , select_select_info * node , bool req  , int first , int first_table_num , int second , int second_table_num   ){
    int first_reg = c->register_counter ; 
    int ans = func(c , node , req  ,   first ,  first_table_num ,  second ,  second_table_num ) ; 
    c->register_counter = first_reg ; 
    return ans ;  
}



    // bug alert the func needs a new attribute known as final check it out i think its quite broken so yeah 


char * operand_thing(char * name) {
    int i = 0;
    while (name[i] != '.' && name[i] != '\0') {
        i= i + 1 ;
    }
    if (name[i] == '\0') {
        return NULL;
    }
    i= i + 1 ;
    int len = strlen(name + i);
    char * col_name = malloc(len + 1);
    int j = 0 ; 
    int k = 0 ; 
    for ( j = i; j < len; j++) {
        col_name[k] = name[j];
        k++ ; 
    }
    col_name[len] = '\0';
    return col_name;
}

char * table_thing(char * name) {
    int i = 0;
    while (name[i] != '.' && name[i] != '\0') {
        i++;
    }
    char * col_name = malloc(i + 1);
    for (int j = 0; j < i; j++) {
        col_name[j] = name[j];
    }
    col_name[i] = '\0';

    return col_name;
}





int  func(compiler *c , select_select_info * node , bool final , int first , int first_table_num , int second , int second_table_num   ){
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
            c->select->select_agg = true ; 
            if (node->acc_reg == -1 ){
                node->acc_reg = c->register_counter++   ; 
                emit(c ,aggregate_init ,node->acc_reg , -1 , -1 , NULL  ) ; 
            }
            aggregate_select(c , node ,  first ,  first_table_num ,  second ,  second_table_num ) ; 
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
            table_number = table_num(c->tl ,table_thing(c->select->sel[i]->col_name)) ; 
            num = col_name_to_int( operand_thing(c->select->sel[i]->col_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(c->select->sel[i]->col_name))] )   ; 
        }
        else {
            num = col_name_to_int_main(c->select->sel[i]->col_name  , c->select )   ; 
        }


        if (node->right == NULL && node->left == NULL  ){
            if (operator != is_null  && operator != is_not_null ){
                int reg_left = c->register_counter++ ; 
                if (1){
                        if (node->col_name != NULL ){
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
                    if (node->extra_col != NULL ){
                        int extra_num ; 
                        int extra_table_number ; 
                        if (second_table_num != -1 ){
                            extra_table_number = table_num(c->tl ,table_thing(c->select->sel[i]->col_name)) ; 
                            extra_num = col_name_to_int( operand_thing(c->select->sel[i]->col_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(c->select->sel[i]->col_name))] )   ; 
                        }
                        else {
                            extra_num = col_name_to_int_main(c->select->sel[i]->col_name  , c->select )   ; 
                        }
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
                            if (node->col_name != NULL ){
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
                    int reg_right = func(c , node->right ,final ,   first ,  first_table_num ,  second ,  second_table_num   ) ; 
                    int reg_left =  c->register_counter++ ;  
                            if (1){
                                if (node->col_name != NULL ){
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
                            int reg_temp = func(c , node->right ,final ,  first ,  first_table_num ,  second ,  second_table_num  ) ; 
                            if (1){
                                if (node->col_name != NULL ){
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
                int reg_left = func(c , node->left,final  ,  first ,  first_table_num ,  second ,  second_table_num  ) ; 
                int reg_right =  c->register_counter++ ;  
                        if (1){
                            if (node->col_name != NULL ){
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
                    int reg_temp = func(c , node->left ,final  ,  first ,  first_table_num ,  second ,  second_table_num ) ; 
                    if (1){
                        if (node->col_name != NULL ){
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
            int reg_right = func(c , node->right ,final   ,  first ,  first_table_num ,  second ,  second_table_num ) ; 
            int reg_left = func(c , node->left,final  ,  first ,  first_table_num ,  second ,  second_table_num  ) ; 
            reg = c->register_counter++  ; 
            emit(c , operator ,reg_left , reg_right , reg , -1 , NULL ) ;  
        }
    }

    return reg ; 
}




void new_compile_select (compiler *c , int first , int first_table_num , int second , int second_table_num  ){
    emit(c , begin_op  , -1 , -1 , -1 , NULL ) ; 
    int cursor = c->cursor_num++ ; 
    int cursor_for_sort_orderby  = 0 ; 
    int register_num = c->register_counter++ ; 
    c->register_start = register_num ; 
    if (c->select->groupby_counter == 0 ){
        if (c->select->orderby_counter > 0 ){
                cursor_for_sort_orderby = c->sorter_cursor++ ; 
            emit(c , sorter_open , cursor_for_sort_orderby , c->select->orderby_counter , -1 , sorter_orderby_init(c , 0 )  ) ; 
        }
        int loop_addr = c->count ; 
        emit(c , eq_op , where_func(c ,c->select->where , false  ) , -1  , MAX , "BINARY" ) ; 
        for ( int i = 0 ; i < c->select->col_counter ; i++  ){
            int num ; 
            int table_number ; 
            if (second_table_num != -1 ){
                table_number = table_num(c->tl ,table_thing(c->select->sel[i]->col_name)) ; 
                num = col_name_to_int( operand_thing(c->select->sel[i]->col_name)  ,  c->tl->tables[table_num(c->tl ,table_thing(c->select->sel[i]->col_name))] )   ; 
            }
            else {
                num = col_name_to_int_main(c->select->sel[i]->col_name  , c->select )   ; 
            }

                if (c->select->sel[i]->operator == NULL ){
                    register_num = c->register_counter++ ; 
                    if (1){
                        select_select_info *node = c->select->sel[i] ; 
                        if (node->col_name != NULL ){
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
                                emit(c , register_get_stuff ,tum , num , register_num  , NULL  ) ;  
                            }
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){
                                emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
                }
                else { 
                    if ( num != -1 ){
                        c->register_counter = normal_func(c ,c->select->sel[i]  , false  ,  first ,  first_table_num ,  second ,  second_table_num ) ; 
                        c->register_counter++ ; 
                    }
                }
        }

        if (c->select->orderby_counter > 0 ){
            int extra_depletion_record = 0 ; 
            for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                if (c->select->orderby[l]->operator != NULL){
                    c->register_counter = orderby_func_main(c , c->select->orderby[l] , false  first ,  first_table_num ,  second ,  second_table_num ) ; 
                    c->register_counter++ ; 
                    extra_depletion_record++ ; 
                }
            }
            emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
            emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
            c->register_counter = c->register_counter - extra_depletion_record ; 
        }

        if (c->select->orderby_counter == 0 ){
            if (c->select->select_agg != true ){
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            }
        }

        c->typ[loop_addr]->p2 = c->count ; 



        emit(c , next_cursor , cursor , loop_addr   , -1 , NULL ) ;
        if (c->select->orderby_counter == 0 ){
            if (c->select->select_agg != true ){
                for ( int z = 0 ; z < c->select->col_counter ;z++ ){
                    int extra_stupid = 0 ; 
                    if (c->select->sel[z]->acc_reg != -1 ){
                        emit(c ,aggregate_final , c->select->sel[z]->acc_reg , -1 , c->register_start + z , NULL  ) ;
                    }
                }
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            }
        }
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
            int orderby_sort_addr = c->count ; 
            int start_orderby_reg_counter = c->register_counter++ ; 
            emit(c , sorter_data , cursor_for_sort_orderby , c->register_counter , -1 , NULL ) ; 
            emit(c , result_row , start_orderby_reg_counter , start_orderby_reg_counter+1 , -1  , -1 , NULL ) ; 
            emit(c , sorter_next , cursor_for_sort_orderby , orderby_sort_addr , -1 , NULL ) ; 
        }
        emit(c, close_cursor_op , cursor, -1, -1, -1, NULL);
        emit(c, halt, -1, -1, -1, -1, NULL);
    }
    // so where you need to start is na like make the new campare function for the orderby as per ythe nulls and the direction and stuff make it not that hard 




    else {
        int loop_addr = c->count ; 
        emit(c , eq_op , where_func(c ,c->select->where , false  ) , -1  , MAX , "BINARY" ) ; 
        int cursor_for_sort_orderby  = 0 ;
        int cursor_sort = c->sorter_cursor++ ; 
        emit(c , sorter_open , cursor_sort,c->select->groupby_counter , -1 , { col_name_to_int_main(c->select->groupby[sel_uni_counter]->col_name   , c->select)} ) ; 
        get_all_select_stuff(c) ; 
        get_all_hash_covered_gb(c) ; 
        if (c->select->orderby_counter > 0 ){
            cursor_for_sort_orderby = c->sorter_cursor++   ; 
            emit(c , sorter_open , cursor_for_sort_orderby , c->select->orderby_counter , -1 , sorter_orderby_init(c , 1 )  ) ; 
        }
        int loop_addr_gb = c->count ; 
        sort_groupby(c) ;  
        emit(c , next_cursor , cursor , loop_addr_gb   , -1 , NULL ) ; 
        emit(c , rewind_cursor , cursor , -1 , -1 , -1 , NULL  ) ; 
        emit(c , sorter_sort , cursor_sort, -1 , -1 , NULL ) ; 
        emit(c , sorter_data , cursor_sort , MAX -1   , -1 , NULL ) ; 
        emit(c , gb_sorter_data , MAX - 1  , c->select->sel_uni_counter , MAX - 1 , NULL    ) ; 
        int sorter_next_jump = c->count ; 
        emit(c , sorter_next , cursor_sort, -1 , -1 , NULL ) ; 
        emit(c , sorter_data ,cursor_sort , MAX - 2    , -1 , NULL ) ; 
        emit(c ,gb_sorter_data , MAX - 2    , c->select->sel_uni_counter , MAX - 2  , NULL    ) ; 
        emit(c , ne_op , MAX - 2   , -1 ,  MAX - 1  , NULL  ) ; 
            int addrwe = c->count ; 
            for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
                int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                    if (c->select->groupby[i]->operator == NULL ){
                        register_num = c->register_counter++ ; 
                        if (1){
                            select_select_info *node = c->select->groupby[i] ; 
                            if (node->col_name != NULL ){
                                if (num != -1 ){
                                emit(c , gb_specific_column_op , MAX - 1 , num , register_num  , NULL  ) ;  
                                }
                            }
                            else {
                                if (node->num_value != NULL ){
                                    emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                                }
                                else if (node->char_value != NULL ){
                                    emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                                }
                                else if (node->float_val != NULL ){
                                    emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                                }
                                else if (node->blob != NULL ){
                                    emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                                }
                            }
                        }
                    }
                    else { 
                        if ( num != -1 ){
                                c->register_counter =  groupby_func(c ,c->select->groupby[i]  , true ) ; 
                                c->register_counter+ ; 
                        }
                    }
            }
            emit(c , copy_op , MAX - 2  , MAX -1  , -1 , NULL ) ; 
            int having   = 0 ; 
            if ( c->select->having != NULL  ){
                if (c->select->having->acc_reg != -1 ){
                    having = where_func(c ,c->select->having  , true   ) ; 
                }
                else { 
                    having = where_func(c ,c->select->having  , false    ) ;    
                }

            }
            emit(c , eq_op , having , -1 ,  MAX , NULL  ) ; 
            int gb_hav = c->count ; 
            if (c->select->orderby_counter > 0 ){
                int extra_depletion_record = 0 ; 
                for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                    if (c->select->orderby[l]->operator != NULL){
                        c->register_counter = orderby_func_main(c , c->select->orderby[l] , false ) ; 
                        c->register_counter++ ; 
                        extra_depletion_record++ ; 
                    }
                }
                emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
                emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
                c->register_counter = c->register_counter - extra_depletion_record ; 
            }
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
            c->typ[gb_hav].p2 = c->count ; 
            emit(c ,goto_op , -1 ,  sorter_next_jump  , -1 , NULL ) ; 
            c->typ[addrwe].p2 = c->count ; 
            for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
                int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                    if (c->select->groupby[i]->operator != NULL ){
                        if ( num != -1 ){
                                int not_needed = groupby_func(c ,c->select->groupby[i]  , false ) ; 
                        }
                    }

            }
            emit(c , copy_op , MAX - 2  , MAX -1  , -1 , NULL ) ; 
            emit(c ,goto_op , -1 ,  sorter_next_jump  , -1 , NULL ) ; 

        c->typ[sorter_next_jump + 1].p2 = c->count ; 
        for ( int i = 0 ; i < c->select->groupby_counter ; i++  ){
            int num = col_name_to_int_main( c->select->groupby[i]->col_name , c->select   ) ; 
                if (c->select->groupby[i]->operator == NULL ){
                    register_num = c->register_counter++ ; 
                    if (1){
                        select_select_info *node = c->select->groupby[i] ; 
                        if (node->col_name != NULL ){
                            if (num != -1 ){
                            emit(c , gb_specific_column_op , MAX - 1 , num , register_num  , NULL  ) ;  
                            }
                        }
                        else {
                            if (node->num_value != NULL ){
                                emit(c , integer_op , *node->num_value , register_num , -1  , NULL  ) ;   
                            }
                            else if (node->char_value != NULL ){
                                emit(c , string_op ,-1 , register_num , -1  , (void*)node->char_value   ) ;   
                            }
                            else if (node->float_val != NULL ){
                                emit(c , real_op , -1, register_num , -1  , (void*)node->float_val   ) ;   
                            }
                            else if (node->blob != NULL ){
                                emit(c , blob_op ,-1 , register_num , -1  , (void*)node->blob   ) ;   
                            }
                        }
                    }
                }
                else { 
                    if ( num != -1 ){
                            c->register_counter =  groupby_func(c ,c->select->groupby[i]  , true ) ; 
                            c->register_counter++ ; 
                    }
                }
        }
        emit(c , eq_op , having , -1 ,  MAX , NULL  ) ; 
        int gb_hav_fin = c->count ; 
        if (c->select->orderby_counter > 0 ){
            int extra_depletion_record = 0 ; 
            for ( int l = 0 ; l < c->select->orderby_counter ; l++  ){
                if (c->select->orderby[l]->operator != NULL){
                    c->register_counter = orderby_func_main(c , c->select->orderby[l] , false ) ; 
                    c->register_counter++ ; 
                    extra_depletion_record++ ; 
                }
            }
            emit(c , make_record , c->register_start , c->register_counter , c->register_counter + 1 , NULL ) ; 
                emit(c , sorter_insert , cursor_for_sort_orderby ,  c->register_counter + 1  , -1  , NULL) ; 
            c->register_counter = c->register_counter - extra_depletion_record ; 
        }
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
        }
        if (c->select->orderby_counter == 0 ){
            emit(c , result_row ,c->register_start , c->register_start + c->register_counter , -1  , -1 , NULL ) ; 
        }
        c->typ[gb_hav_fin].p2 = c->count ; 
        if (c->select->orderby_counter > 0 ) {
            emit(c , orderby_sort , cursor_for_sort_orderby , -1 , -1 , NULL ) ; 
            int orderby_sort_addr = c->count ; 
            int start_orderby_reg_counter = c->register_counter++ ; 
            emit(c , sorter_data , cursor_for_sort_orderby , c->register_counter , -1 , NULL ) ; 
            emit(c , result_row , start_orderby_reg_counter , start_orderby_reg_counter+1 , -1  , -1 , NULL ) ; 
            emit(c , sorter_next , cursor_for_sort_orderby , orderby_sort_addr , -1 , NULL ) ; 
        }
        emit(c, close_cursor_op , cursor, -1, -1, -1, NULL) ;
        emit(c, halt, -1, -1, -1, -1, NULL) ;
        }
    
}










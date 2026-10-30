
#include "storage_engine.h"

void default_column_info(column_info * col){
    memset(col, 0, sizeof(column_info));
    strncpy(col->col_name, "none", sizeof(col->col_name) - 1);
    col->col_type = 0;
    col->type_bits_count = 0;
    col->first_decimal = 0;
    col->second_decimal  = 0;
    col->primary_key = false;
    col->foreign_key = false;
    col->default_stuff = false;
    col->not_null = false;
    col->unique = false;
    strncpy(col->default_val, "", sizeof(col->default_val)- 1);
    strncpy(col->foreign_key_table, "", sizeof(col->foreign_key_table)- 1);
    strncpy(col->foreign_key_column, "", sizeof(col->foreign_key_column)- 1);
}

void default_table_info(table_info * ti){
    memset(ti, 0, sizeof(table_info));
    strncpy(ti->table_name, "none", sizeof(ti->table_name) - 1);
    ti->num_columns = 0;
    ti->root_page   = 0;
    for (int i = 0; i < MAX_COLUMNS; i++){
        default_column_info(&ti->columns[i]);
    }
}

void default_pagezero(pagezero * pg){
    memset(pg, 0, sizeof(pagezero));
    pg->db_magic = 0xF1ACDB;
    pg->version = 1;
    pg->page_size = 4096;
    pg->num_tables = 0;
    for (int i = 0; i < MAX_TABLES; i++){
        default_table_info(&pg->tables[i]);
    }
}


int hash_num( char *str){
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;  
    }
    return hash % 307 ;
}

void prepared_the_intel_hash(info * intel){
    memset(intel, 0, sizeof(info));
}

bool is_the_node_primary_key(ctree * tree){
    bool what_is_it = tree->primary_key_counter >= 1 && tree->primary_key != NULL && tree->primary_key[0] != NULL ; 
    return what_is_it ;
}


bool is_pk_col(ctree * tree, const char * name){
    for (int a = 0; a < tree->num; a++){
        ctree * node = tree->children[a];
        if (is_the_node_primary_key(node)){                                  
            for (int b = 0; b < node->primary_key_counter; b++){
                if (strcmp(node->primary_key[b], name) == 0){
                    return true;
                }
            }
        }
    }
    return false;
}


int take_care_of_stuff(ctree * tree , info * intel){
    prepared_the_intel_hash(intel) ; 
    int num ; 
    int i = 0 ;
    while (i < tree->num ){
        if (tree->children[i] == NULL || tree->children[i]->comp == NULL ){
            return -1 ; 
        }
        if (is_the_node_primary_key(tree->children[i])){
            int j = 0 ; 
            while (j < tree->children[i]->primary_key_counter ){
                num =  hash_num(tree->children[i]->primary_key[j] ) ;  
                intel->primary_key[num] = 1 ; 
                intel->pri_num++ ; 
                j++ ;  
            }
        }
        else if (strcmp(tree->children[i]->comp , "FOREIGN KEY") == 0 ){
            if (tree->children[i]->num == 0 || tree->children[i]->children[tree->children[i]->num - 1]->num < tree->children[i]->foreign_key_counter ){
                return -1 ; 
            }
        }
        else { 
            int k = 0 ;
            while (k < i ){
                if (!is_the_node_primary_key(tree->children[k]) && strcmp(tree->children[k]->comp , "FOREIGN KEY") != 0 && strcmp(tree->children[k]->comp , tree->children[i]->comp) == 0 ){
                    return -1 ; 
                }
                k++ ; 
            }
            num =  hash_num(tree->children[i]->comp ) ; 
            intel->hash[num] = 1 ; 
            intel->num++ ; 
            if (intel->num > MAX_COLUMNS ){
                return -1 ; 
            }
        }
        i++ ; 
    }
    return 0 ; 
}

void do_the_foreign_key(ctree * tree, table_info * ti){
    int i = 0;
    while (i < tree->foreign_key_counter){
        column_info * col = NULL;
        for (int k = 0; k < (int)ti->num_columns; k++){
            if (strcmp(ti->columns[k].col_name, tree->foreign_key[i]) == 0){
                col = &ti->columns[k];
                break;
            }
        }
        if (col == NULL){
            if (ti->num_columns >= MAX_COLUMNS){
                return;
            }
            col = &ti->columns[ti->num_columns++];
            default_column_info(col);
            strncpy(col->col_name, tree->foreign_key[i], sizeof(col->col_name) - 1);
        }
        strncpy(col->foreign_key_table, tree->children[tree->num - 1]->comp, sizeof(col->foreign_key_table) - 1);
        strncpy(col->foreign_key_column, tree->children[tree->num - 1]->children[i]->comp, sizeof(col->foreign_key_column) - 1);
        col->foreign_key = true;
        i++;
    }
}


void main_create_parser(pagezero * pg){
    default_pagezero(pg);                                                                      
    ctree * tree = create_parser();
    if (tree == NULL || tree->num == 0){
        return ;
    }
    tree = tree->children[tree->num - 1];
    table_info ti;
    default_table_info(&ti);                                                              
    strncpy(ti.table_name, tree->comp, sizeof(ti.table_name) - 1);
    int i = 0;
    info intel_store ;  
    info * intel = &intel_store ;  
    int check_correct = take_care_of_stuff(tree, intel);
    if ( check_correct == -1 ){
        return  ; 
    }
    while (i < tree->num){
        ctree * child = tree->children[i]; 
        if (is_the_node_primary_key(child)){    
            i++ ;
            continue ;
        }
        if (strcmp(child->comp, "FOREIGN KEY") == 0){
            do_the_foreign_key(child , &ti) ;
            i++ ;
            continue ;
        }
        column_info col;
        default_column_info(&col);                                                  
        int num = hash_num(child->comp);
        if (intel->hash[num] == 1){
            strncpy(col.col_name, child->comp, sizeof(col.col_name) - 1);
        }
        if (child->primary_key_counter == 1){
            col.primary_key = true ;
        }
        if (is_pk_col(tree, child->comp)){
            col.primary_key = true ; 
        }
        if (child->not_null == true){
            col.not_null = true;
        }
        if (child->unique){
            col.unique = true;
        }
        if (1){
            if (child->type >= INT){
                col.col_type = child->type;
                col.type_bits_count = child->type_bits_count;
                if (child->type == DECIMAL){
                    col.first_decimal  = child->first_decimal;
                    col.second_decimal = child->second_decimal;
                }
            }
            else {
                return ; 
            }
        }
        if (child->default_val != NULL){
            col.default_stuff = true;
            if (col.unique == false ){
                strncpy(col.default_val, child->default_val, sizeof(col.default_val) - 1);
            }
            else { 
                return ; 
            }
        }
        if (ti.num_columns >= MAX_COLUMNS){
            return ;
        }
        ti.columns[ti.num_columns++] = col ;   
        i++;
    }

    if (pg->num_tables < MAX_TABLES){
        pg->tables[pg->num_tables++] = ti;
    }
}
#include "Parser/Parser.h"
#include "Storage_engine/storage_engine.h"
hash_bucket *hash_starter = NULL;
char insert_message[200] = "";

uint32_t hash_cal(const char *string) {
    uint32_t number = 5381;
    while (*string) {
        number = number * 33 + (uint8_t)*string;
        string++;
    }
    return number % HS_BUCKETS;
}


hash_bucket *hash_find(const char *table, uint32_t col, uint8_t create) {
    hash_bucket *current = hash_starter;
    while (current != NULL) {
        if (current->col == col && strcmp(current->table, table) == 0) {
            return current;
        }
        current = current->next;
    }
    if (!create) {
        return NULL;
    }
    hash_bucket *new_col = calloc(1, sizeof(hash_bucket));
    new_col->table = strdup(table);
    new_col->col = col;
    new_col->next = hash_starter;
    hash_starter = new_col;
    return new_col;
}


uint8_t check_hash(const char *val, uint32_t col, const char *table) {
    hash_bucket *current = hash_find(table, col, 0);
    if (current == NULL) {
        return 0;
    }
    uint32_t index = hash_cal(val);
    hash_node *node = current->bucket[index];
    while (node != NULL) {
        if (strcmp(node->val, val) == 0) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}


void add_hash(const char *table, uint32_t col, const char *val) {
    if (check_hash(val, col, table)) {
        return;
    }
    hash_bucket *current = hash_find(table, col, 1);
    uint32_t index = hash_cal(val);
    hash_node *new_node = malloc(sizeof(hash_node));
    new_node->val = strdup(val);
    new_node->next = current->bucket[index];
    current->bucket[index] = new_node;
}


void remove_hash(const char *table, uint32_t col, const char *val) {
    hash_bucket *current = hash_find(table, col, 0);
    if (current == NULL) {
        return;
    }
    uint32_t index = hash_cal(val);
    hash_node **node = &current->bucket[index];
    while (*node != NULL) {
        if (strcmp((*node)->val, val) == 0) {
            hash_node *temp = *node;
            *node = temp->next;
            free(temp->val);
            free(temp);
            return;
        }
        node = &(*node)->next;
    }
}

char *truncate_left(char *str, size_t max_bytes){
    size_t len = strlen(str);
    if (len <= max_bytes){
        return str;
    }
    return str + (len - max_bytes);
} 

char *to_decimal(const char *str, int first, int second){
    int i = 0;
    char sign = 0;
    if(str[i] == '-' || str[i] == '+'){
        sign = str[i] ;
        i++ ;
    }
    char right[256] = {0} ;
    char left[256] = {0} ;
    int dot = 0 ;
    int ans_counter = 0 ;
    int count = 0 ;
    for( ; str[i] != '\0'; i++){
        if (str[i] == '.'){ 
            dot = 1 ; 
            continue ;
        }
        if (dot == 0 ) { 
            left[ans_counter++] = str[i];
        }
        else {
            right[count++] = str[i];
        }
    }
    left[ans_counter] = '\0';
    right[count] = '\0';

    int left_len_max  = first - second;
    char *int_ptr = left;
    int left_len = strlen(left);
    if (left_len > left_len_max ){
        int_ptr = left + (left_len - left_len_max);
    }
    int len_of_something = (int)strlen(right) ; 
    if( len_of_something > second){
        right[second] = '\0';
    }
    int len = strlen(right);
    for (int j = len; j < second; j++){   
        right[j] = '0';
    }
    right[second] = '\0';
    char *result = malloc(first + 3);
    result[0] = '\0';
    if (sign){
        char sin[2] = {sign , '\0'};
        strcat(result, sin );
    }
    strcat(result, int_ptr);
    if (second > 0) {
        strcat(result , ".");
        strcat(result , right);
    }
    return result;
}



void struct_for_insert(itree *tree){
    char * table_name_we_need = tree->table ; 
    int error = 0;
    if (strcmp(tree->comp, "INSERT INTO") == 0){
        if (tree->ignore == 1) {
            error = 1;
        } else {
            error = 0;
        }
    } else if (strcmp(tree->comp, "REPLACE INTO") == 0){
        error = 2;
    }
    int u = 0;
    while (u < tree->num){
        if (strcmp(tree->children[u]->comp, "ON DUPLICATE KEY UPDATE") == 0){
            error = 3;
        }
        u++;
    }
    if (error == 2 || error == 3){
        return;
    }
    pagezero *header = get_pagezero(tree->table );
    if (header == NULL){
        return;
    }
    table_info *temp_table = NULL;
    int temp = 0;
    while (temp < (int)header->num_tables) {
        if (strcmp(header->tables[temp].table_name, tree->table) == 0){
            temp_table = &header->tables[temp];
            break;
        }
        temp++;
    }
    if (temp_table == NULL){
        free(header);
        return;
    }
    char **temp_column = calloc(128, sizeof(char *));
    int heck = 0;
    int hash[307] = {0} ;
    if (tree->table_col_num > 0){
        int temp_1 = 0;
        int temp_2 = 0;
        while (temp_2 < tree->table_col_num){
            int num = hash_num(tree->table_cols[temp_2]);
            hash[num] = num;
            temp_2++;
        }
        while (temp_1 < (int)temp_table->num_columns){
            int num = hash_num(temp_table->columns[temp_1].col_name);
            if (hash[num] != num){
                if (temp_table->columns[temp_1].default_stuff == true) {
                    char *def = temp_table->columns[temp_1].default_val;
                    if ( def != NULL ){
                        temp_column[heck++] =  strdup(def)  ; 
                    }
                }
                else {
                    if (temp_table->columns[temp_1].not_null == true) {
                        free(header);
                        return;
                    }
                    else {
                        heck++;
                    }
                }
            }
            temp_1++;
        }
    } 
    else {
        for (int i = 0; i < (int)temp_table->num_columns; i++) {
            int num = hash_num(temp_table->columns[i].col_name);
            hash[num] = num;
        }
    }
    int inserted_rows = 0; 
    int d = 0;
    while (d < tree->num){
        int counter = 0;
        char **col_temp = calloc(128, sizeof(char *));
        char *current_primary_key[MAX_COLUMNS] = {0};
        int skip = 0;
        if (strcmp(tree->children[d]->comp, "VALUE_ROW") == 0) {
            itree *temp_tree = tree->children[d];
            if (tree->table_col_num == 0 && temp_tree->insert_val_num != (int)temp_table->num_columns) {
                free(header);
                return;
            }
            if (tree->table_col_num > 0 && temp_tree->insert_val_num != tree->table_col_num) {  
                free(header);
                return;
            }
            int e = 0;
            int f = 0;
            while (f < (int)temp_table->num_columns){
                int temp_num = hash_num(temp_table->columns[f].col_name);
                if (hash[temp_num] == temp_num) {
                    if (tree->table_col_num > 0) {
                        for (int g = 0; g < tree->table_col_num; g++) {
                            if (strcmp(tree->table_cols[g], temp_table->columns[f].col_name) == 0) {
                                e = g;
                                break;
                            }
                        }
                    }
                    if (temp_table->columns[f].primary_key == true) {
                        if (check_hash(temp_tree->insert_values[e], f, tree->table) == 0) {
                            current_primary_key[f] = temp_tree->insert_values[e];
                            if (temp_table->columns[f].col_type == VARCHAR || temp_table->columns[f].col_type == CHAR) {
                                col_temp[f] = strdup( truncate_left( temp_tree->insert_values[e] , temp_table->columns[f].type_bits_count ) );
                                e++;
                            }
                            else if (temp_table->columns[f].col_type == DECIMAL) {
                                col_temp[f] = to_decimal( temp_tree->insert_values[e] , temp_table->columns[f].first_decimal , temp_table->columns[f].second_decimal );
                                e++;
                            }
                            else {
                                col_temp[f] = strdup(temp_tree->insert_values[e]);
                                e++;
                            }
                        } 
                        else {
                            if (error == 1) {
                                skip = 1;
                                break;
                            }
                            else {
                                free(header);
                                return;
                            }
                        }
                    } 
                    else {
                        if (temp_table->columns[f].col_type == VARCHAR || temp_table->columns[f].col_type == CHAR) {
                            col_temp[f] = strdup( truncate_left( temp_tree->insert_values[e], temp_table->columns[f].type_bits_count ) );
                            e++;
                        } 
                        else if (temp_table->columns[f].col_type == DECIMAL) {
                            col_temp[f] = to_decimal( temp_tree->insert_values[e], temp_table->columns[f].first_decimal, temp_table->columns[f].second_decimal );
                            e++;
                        } 
                        else {
                            col_temp[f] = strdup(temp_tree->insert_values[e]);
                            e++;
                        }
                    }
                } 
                else {
                    if (temp_table->columns[f].primary_key == false) {
                        if (temp_column[counter] != NULL) {
                            col_temp[f] = strdup(temp_column[counter]);
                        }
                        counter++;
                    } 
                    else {
                        free(header);
                        return;
                    }
                }

                f++;
            }

            if (skip == 0){
                char row_buf[PAGE_SIZE] = {0};
                for (int k = 0 ; k < (int)temp_table->num_columns ; k++) {
                    if (k > 0) {
                        strcat(row_buf, "\t");
                    }
                    if (col_temp[k] != NULL) {
                        strcat(row_buf , col_temp[k]);
                    }
                }
                int page_num = (int)temp_table->root_page;
                int where = FSM_insert(row_buf , page_num , 0 , temp_table);
                if (where < 0 && page_num > 0){
                    where = FSM_insert(row_buf , 0 , 0 , temp_table);
                }
                if (where < 0){
                    free(header);
                    return;
                }
                temp_table->root_page = where ; 
                inserted_rows++ ;
                for (int k = 0; k < (int)temp_table->num_columns; k++){
                    if (current_primary_key[k] != NULL) {
                        add_hash(tree->table , k , current_primary_key[k] );
                    }
                }
            }
        }
        for (int i = 0 ; i < (int)temp_table->num_columns ; i++){
            free(col_temp[i]);
        }
        free(col_temp);
        d++;
    }

    for (int k = 0 ; k < heck ; k++){ 
        free(temp_column[k]);
    }
    free(temp_column);
    update_pagezero(header , table_name_we_need) ; 
    free(header);
}
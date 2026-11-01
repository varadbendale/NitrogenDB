#include "Parser/Parser.h"
#include "Storage_engine/storage_engine.h"
hs_col *hs_head = NULL;

unsigned long hs_hash(char *string) {
    unsigned long number = 5381;
    while (*string) {
        number = number * 33 + (unsigned char)*string ;
        string++;
    }
    return number % HS_BUCKETS;
}


hs_col *hs_find(char *table, int col, int create) {
    hs_col *current = hs_head;
    while (current != NULL) {
        if (current->col == col && strcmp(current->table, table) == 0) {
            return current;
        }
        current = current->next;
    }
    if (!create) {
        return NULL;
    }
    hs_col *new_col = calloc(1, sizeof(hs_col));
    new_col->table = strdup(table);
    new_col->col = col;
    new_col->next = hs_head;
    hs_head = new_col;
    return new_col;
}


int check_hash(char *val, int col, char *table) {
    hs_col *current = hs_find(table, col, 0);
    if (current == NULL) {
        return 0;
    }
    unsigned long index = hs_hash(val);
    hs_node *node = current->bucket[index];
    while (node != NULL) {
        if (strcmp(node->val, val) == 0) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}


void add_hash(char *table, int col, char *val) {
    if (check_hash(val , col , table)) {
        return;
    }
    hs_col *current = hs_find(table, col, 1);
    unsigned long index = hs_hash(val);
    hs_node *new_node = malloc(sizeof(hs_node));
    new_node->val = strdup(val);
    new_node->next = current->bucket[index];
    current->bucket[index] = new_node;
}


void remove_hash(char *table, int col, char *val) {
    hs_col *current = hs_find(table, col, 0);
    if (current == NULL) {
        return;
    }
    unsigned long index = hs_hash(val);
    hs_node **node = &current->bucket[index];
    while (*node != NULL) {
        if (strcmp((*node)->val, val) == 0) {
            hs_node *temp = *node;
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
    if (len <= max_bytes)
        return str;
    return str + (len - max_bytes);
} 

char *to_decimal(const char *str, int precision, int scale){
    int i = 0;
    char sign = 0;
    if (str[i] == '-' || str[i] == '+'){
        sign = str[i];
        i++;
    }

    char integer_part[256] = {0};
    char frac_part[256] = {0};
    int dot = 0;
    int ii = 0;
    int fi = 0;
    for ( ; str[i] != '\0'; i++){
        if (str[i] == '.') { 
            dot = 1; 
            continue;
        }
        if (!dot) { 
            integer_part[ii++] = str[i];
        }
        else {
            frac_part[fi++] = str[i];
        }
    }
    integer_part[ii] = '\0';
    frac_part[fi] = '\0';

    int max_int_digits = precision - scale;
    char *int_ptr = integer_part;
    int int_len = strlen(integer_part);
    if (int_len > max_int_digits){
        int_ptr = integer_part + (int_len - max_int_digits);
    }

    int len_of_something = (int)strlen(frac_part) ; 
    if ( len_of_something > scale){
        frac_part[scale] = '\0';
    }
    int frac_len = strlen(frac_part);
    for (int j = frac_len; j < scale; j++){
        frac_part[j] = '0';
    }
    frac_part[scale] = '\0';

    char *result = malloc(precision + 3);
    result[0] = '\0';
    if (sign) {
        char s[2] = {sign, '\0'};
        strcat(result, s);
    }
    strcat(result, int_ptr);
    if (scale > 0) {
        strcat(result, ".");
        strcat(result, frac_part);
    }
    return result;
}


void struct_for_insert(itree *tree){
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
    pagezero *header = get_pagezero(DB_FILE);
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
    int hash_1[307] = {0} ;
    int hash_2[307] = {0} ;
    int position_org[307] ;
    int position_written[307];
    if (tree->table_col_num > 0){
        int temp_1 = 0;
        int temp_2 = 0;
        while (temp_1 < (int)temp_table->num_columns){
            int num = hash_num(temp_table->columns[temp_1].col_name);
            hash_1[num] = num;
            position_org[num] = temp_1;
            temp_1++;
        }
        while (temp_2 < tree->table_col_num){
            int num = hash_num(tree->table_cols[temp_2]);
            hash_2[num] = num;
            position_written[temp_2] = num;
            temp_2++;
        }
        temp_1 = 0;
        while (temp_1 < (int)temp_table->num_columns){
            int num = hash_num(temp_table->columns[temp_1].col_name);
            if (hash_2[num] != num){
                if (temp_table->columns[temp_1].default_stuff == true) {
                    char cleaned[32];
                    sscanf(temp_table->columns[temp_1].default_val, " %31[^\t\n ]", cleaned);
                    temp_column[heck++] = strdup(cleaned);
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
        for (int z = 0; z < (int)temp_table->num_columns; z++) {
            int num = hash_num(temp_table->columns[z].col_name);
            hash_2[num] = num;
        }
    }
    int d = 0;
    while (d < tree->num){
        int counter = 0;
        char **col_temp = calloc(128, sizeof(char *));
        char *pend[MAX_COLUMNS] = {0};
        int skip = 0;
        if (strcmp(tree->children[d]->comp, "VALUE_ROW") == 0) {
            itree *temp_tree = tree->children[d];
            if (tree->table_col_num == 0 && temp_tree->insert_val_num != (int)temp_table->num_columns) {
                free(header);
                return;
            }
            int e = 0;
            int f = 0;
            while (f < (int)temp_table->num_columns){
                int temp_num = hash_num(temp_table->columns[f].col_name);
                if (hash_2[temp_num] == temp_num) {
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
                            pend[f] = temp_tree->insert_values[e];
                            if (temp_table->columns[f].col_type == VARCHAR ||
                                temp_table->columns[f].col_type == CHAR) {
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
                    } else {
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
                for (int k = 0; k < (int)temp_table->num_columns; k++) {
                    if (k > 0) {
                        strcat(row_buf, "\t");
                    }
                    if (col_temp[k] != NULL) {
                        strcat(row_buf, col_temp[k]);
                    }
                }
                int page_num = (int)temp_table->root_page;
                int where = FSM_insert(row_buf, page_num, 0, temp_table);
                if (where < 0 && page_num > 0){
                    where = FSM_insert(row_buf, 0, 0, temp_table);
                }
                if (where < 0){
                    free(header);
                    return;
                }
                temp_table->root_page = where;
                for (int k = 0; k < (int)temp_table->num_columns; k++){
                    if (pend[k] != NULL) {
                        add_hash(tree->table, k, pend[k]);
                    }
                }
            }
        }
        for (int k = 0; k < (int)temp_table->num_columns; k++){
            free(col_temp[k]);
        }
        free(col_temp);
        d++;
    }

    write_pagezero(header, DB_FILE);
    free(header);
}
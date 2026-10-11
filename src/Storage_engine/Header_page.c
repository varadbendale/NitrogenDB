
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
    for (int i = 0; i < 16; i++){
        default_column_info(&ti->columns[i]);
    }
}

void default_pagezero(pagezero * pg){
    memset(pg, 0, sizeof(pagezero));
    pg->db_magic   = 0xF1ACDB;
    pg->version    = 1;
    pg->page_size  = 4096;
    pg->num_tables = 0;
    for (int i = 0; i < 300; i++){
        default_table_info(&pg->tables[i]);
    }
}


int hash_num( char *str){
    int hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;  
    }
    return hash % 307 ;
}

void prepared_the_intel_hash(info * intel){
    for (int i = 0 ; i < 307 ; i++ ){
        intel[i] = 0 ; 
        primary_key[i] = 0 ; 
    }
}

void take_care_of_stuff(ctree * tree , info * intel){
    prepared_the_intel_hash(intel) ; 
    int num ; 
    while (i < tree->num ){
        if (strcmp(tree->children[i]->comp , "PRIMARY KEY") == 0 ){
            int j = 0 ; 
            while (j < tree->children[i]->num ){
                num =  hash_num(tree->children[i]->children[j]->comp ) ;  
                intel->primary_key[num] = num ; 
                intel->pri_num++ ; 
            }
        }
        else { 
            num =  hash_num(tree->children[i]->comp ) ; 
            if (intel->hash[num] != num ){
                 intel->hash[num] = num ; 
            }
            else { 
                //error ; 
            }

            intel->num++ ; 
        }
    }
}


void do_the_foreign_key(ctree * tree, table_info * ti){
    int i = 0;
    while (i < tree->foreign_key_counter){
        column_info temp;
        default_column_info(&temp);                                                         
        strncpy(temp.col_name, tree->foreign_key[i], sizeof(temp.col_name) - 1);
        strncpy(temp.foreign_key_table, tree->children[tree->num - 1]->comp, sizeof(temp.foreign_key_table) - 1);
        strncpy(temp.foreign_key_column, tree->children[tree->num - 1]->children[i]->comp, sizeof(temp.foreign_key_column) - 1);
        temp.foreign_key = true;
        ti->columns[ti->num_columns++] = temp;
        i++;
    }
}


void main_create_parser(pagezero * pg){
    ctree * tree = create_parser();
    tree = tree->children[tree->num - 1];
    default_pagezero(pg);                                                                      
    table_info ti;
    default_table_info(&ti);                                                              
    strncpy(ti.table_name, tree->comp, sizeof(ti.table_name) - 1);
    int i = 0;
    info * intel;
    take_care_of_stuff(tree, intel);
    ti.num_columns = intel->num;
    int stuff = 0;
    while (i < tree->num){
        column_info col;
        default_column_info(&col);                                                  
        int num = hash_num(tree->children[tree->num - 1]->comp);
        if (intel->hash[num] == num){
            strncpy(col.col_name, tree->comp, sizeof(col.col_name) - 1);
        }
        if (tree->primary_key_counter == 1){
            col.primary_key = true;
        }
        if (intel->primary_key[num] == num){
            col.primary_key = true;
        }
        if (tree->not_null == true){
            col.not_null = true;
        }
        if (tree->unique){
            col.unique = true;
        }
        if (1){
            if (tree->type != -1){
                col.col_type        = tree->type;
                col.type_bits_count = tree->type_bits_count;
                if (tree->type == DECIMAL){
                    col.first_decimal  = tree->first_decimal;
                    col.second_decimal = tree->second_decimal;
                }
            }
            else {
                // error
            }
        }
        if (tree->default_val != NULL){
            col.default_stuff = true;
            if (col.unique == false ){
                strncpy(col.default_val, tree->default_val, sizeof(col.default_val) - 1);
            }

        }
        if (strcmp(tree->comp, "FOREIGN KEY") == 0){
            do_the_foreign_key(tree, &ti);
            stuff = 1;
        }
        if (stuff == 0){
            ti.columns[ti.num_columns++] = col;
        }

        i++;
    }

    pg->tables[pg->num_tables++] = ti;
}


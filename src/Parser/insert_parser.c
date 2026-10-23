insert_parser_tree *createNodeInsert(char *comp) {
    insert_parser_tree *node = malloc(sizeof(insert_parser_tree));  
    if (node == NULL) {
        return NULL;
    }
    if (comp != NULL ){
        node->comp = strdup(comp) ; 
    }
    else if (comp == NULL ){
        node->comp = NULL;   
    }
    node->table = NULL;
    node->table_cols = calloc(128, sizeof(char *));
    if (node->table_cols == NULL) {
        ree(node->comp) ; 
        free(node) ; 
        return NULL;
    }
    node->insert_values = calloc(128, sizeof(char *));
    if (node->insert_values == NULL) {
        free(node->table_cols); 
        free(node); 
        return NULL;
    }
    node->children = calloc(300, sizeof(insert_parser_tree *)); 
    if (node->children == NULL) {     
        free(node->insert_values) ; 
        free(node->table_cols) ;     
        free(node->comp) ;       
        free(node);
        return NULL;
    }
    node->num = 0;
    node->table_col_num = 0;
    node->insert_val_num = 0;
    node->ignore = 0;
    return node;
}


insert_parser_tree *make_insert_leaf(char *value, int row, int col) {
    (void)row ;
    (void)col ;
    return createNodeInsert(value) ;
}

insert_parser_tree* expression_insert(char ***buf, insert_parser_tree *node,  int i, int j, int end_row, int end_col) {
    char *operands[300];
    int start_row = i;
    int start_col = j;
    char *operators[300];
    char *string_temp;
    int operator_top = 0;
    int operand_top = 0;
    insert_parser_tree *prev_tree[300];
    int prev_tree_top = 0;
    int first = 0;
    int check = 0 ;   

    while (buf[i][j] != NULL && strcmp(buf[i][j], ";") != 0 && (i < end_row || (i == end_row && j < end_col)) ) {
        if (priority(buf[i][j]) != 0) {
            if (j + 1 <= end_col) {
                char merged[32];
                snprintf(merged , sizeof(merged) , "%s%s" , buf[i][j] , buf[i][j + 1]) ;  
                if (priority(merged) != 0) {
                    string_temp = strdup(merged);
                    j += 1;
                } else {
                    string_temp = strdup(buf[i][j]);
                }
            }
            else {
                string_temp = strdup(buf[i][j]);
            }
            if (first == 0 && j == start_col) {
                if (i > start_row) {
                    int size = 0;
                    for (int k = 0; buf[i-1][k] != NULL; k++) {
                        size++;
                    }
                    if (size > 0) {
                        operands[operand_top++] = buf[i-1][size - 1];
                    }
                }
                else if (i == start_row && start_col > 0) {
                    operands[operand_top++] = buf[i][start_col - 1];
                }
            }
            first = 1;

            if (operator_top == 0) {
                operators[operator_top++] = string_temp;
            }
            else {
                if (priority(string_temp) <= priority(operators[operator_top - 1])) {
                    while (operator_top > 0 && priority(string_temp) <= priority(operators[operator_top - 1])) {
                        char *oper = operators[operator_top - 1];
                        int num_temp = 2; 
                        if (operand_top < num_temp) {
                            operator_top--;
                            continue;
                        }
                        insert_parser_tree *new = createNodeInsert(oper);
                        operator_top--;
                        int pal = 0;
                        while (pal < num_temp) {
                            operand_top--;
                            if (operands[operand_top] == NULL) {
                                new->children[num_temp -1-pal] = prev_tree[--prev_tree_top];
                            } else {
                                new->children[num_temp-1-pal] = make_insert_leaf(operands[operand_top], i, j);
                            }
                            pal++;
                        }
                        new->num = num_temp;
                        prev_tree[prev_tree_top++] = new;
                        operands[operand_top] = NULL;
                        operand_top++;
                    }
                    operators[operator_top++] = string_temp;
                }
                else {
                    operators[operator_top++] = string_temp;
                }
            }
        }
        else {
            if (if_function(buf[i][j]) == true) {
                int func_start_row = i;
                int func_start_col = j;
                int commas = 0;
                bool present = false;

                while (i <= end_row) {
                    if (buf[i][j] == NULL) {
                        if (i + 1 <= end_row) {
                            i++ ; 
                            j = 0 ; 
                            continue ; 
                        }
                        break ;
                    }
                    if (strcmp(buf[i][j], "(") == 0) {
                        commas++ ;
                        present = true ;
                    }
                    else if (strcmp(buf[i][j], ")") == 0) {
                        commas-- ;
                    }
                    if (present && commas == 0){
                        break ;
                    } 
                    if (i == end_row && j >= end_col){
                        break ;
                    } 
                    j++ ;
                }

                insert_parser_tree *just_there;
                if (prev_tree_top == 0 || prev_tree[prev_tree_top - 1] == NULL) {
                    insert_parser_tree *func_parent = createNodeInsert(NULL);
                    just_there = function_insert(buf, func_parent, check, func_start_row, func_start_col, i, j);
                }
                else {
                    just_there = function_insert(buf, prev_tree[prev_tree_top - 1], check, func_start_row, func_start_col, i, j);
                }
                if (just_there != NULL) {
                    prev_tree[prev_tree_top++] = just_there;
                    operands[operand_top++] = NULL;
                }
            }
            else {
                operands[operand_top++] = buf[i][j];
            }
        }
        
        j++;
        if (buf[i][j] == NULL) {
            if (i + 1 < end_row) { 
                i++ ; 
                j = 0 ; 
            }
        }
    }

    while (operator_top > 0) {
        char *oper = operators[operator_top - 1];
        int num_temp = 2; 
        if (operand_top < num_temp) {
            operator_top--;
            continue;
        }
        insert_parser_tree *new = createNodeInsert(oper);
        operator_top--;
        int pal = 0;
        while (pal < num_temp) {
            operand_top--;
            if (operands[operand_top] == NULL) {
                new->children[num_temp - 1 - pal] = prev_tree[--prev_tree_top];
            } else {
                new->children[num_temp - 1 - pal] = make_insert_leaf(operands[operand_top], i, j);
            }
            pal++;
        }
        new->num = num_temp;
        prev_tree[prev_tree_top++] = new;
        operands[operand_top] = NULL;
        operand_top++;
    }

    insert_parser_tree *exp_tree = (prev_tree_top > 0) ? prev_tree[prev_tree_top - 1] : NULL;

    if (j - 1 >= start_col) {
        j = j - 1;
    }

    return exp_tree;
}


itree * insert_parser(){
    char ***buf = proper_data.query ; 
    int row = 0  ; 
    int col = 0 ; 
    int i = 0 ; 
    int j = 0  ; 
    int temp = 0 ; 
    int max = 0 ; 
    int end_row  ; 
    int end_col ;  
    while (buf[i][0] != NULL ){
        j = 0 ; 
        while (buf[i][j] != NULL ){
            j++ ; 
        }
        if (j > max ){
            max = j ; 
        }
        i++ ; 
    }
    end_row  = i - 1  ; 
    end_col = max ; 
    i = row ; 
    j = col ; 
    itree * start = NULL ; 
    itree * node = NULL ;
    if (buf[i][j] == NULL) {
        return NULL; 
    }

    if ( strcmp(buf[i][j], "INSERT")==0   ){
        int check = 0 ; 
        if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
        }   
        if ( strcmp(buf[i][j], "IGNORE")==0   ){ 
            temp = 1  ; 
            check = 1 ; 
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
            }   
        }
        if (strcmp(buf[i][j], "INTO")==0  ){
            node = createNodeInsert("INSERT INTO") ; 
            if (node == NULL) {                
                return NULL ; 
            }
            start = node ; 
        }
        else { 
            return NULL ; 
        }
        if (check == 1 && temp == 1 ){
            node->ignore = 1 ; 
        }

    }	



    else if (strcmp(buf[i][j], "REPLACE")==0  ){
        if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
        }   
        if (strcmp(buf[i][j], "INTO")==0  ){
            node = createNodeInsert("REPLACE INTO") ; 
            if (node == NULL) {                
                return NULL ; 
            }
            node->replace = 1 ; 
            start = node ; 
        }
        else { 
            return NULL ; 
        }
    }
    else { 
        return NULL ;   
    }



    if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
    else { 
        j++ ; 
    }  
    if (buf[i][j] != NULL ){
        node->table = strdup(buf[i][j] ) ; 
    }
    else { 
        return NULL ; 
    }

    if ( buf[i][j+1] == NULL ){
        if (i+1 <= end_row){
            i = i+ 1 ; 
            j = 0  ; 
        }
    }
    else { 
        j++ ; 
    }  

    if (strcmp(buf[i][j] , "(" ) == 0  ){
        if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
        else { 
            j++ ; 
        }
        while (buf[i][j] != NULL && strcmp(buf[i][j] , ")" ) == 0 && node->table_col_num < 128 ){
            if (strcmp(buf[i][j], ",") == 0) {
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){ 
                        i = i+1 ;
                         j = 0 ; 
                    }
                }
                else {
                    j++ ; 
                }
                continue;
            }
            node->table_cols[node->table_col_num++] = strdup(buf[i][j]) ; 
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
            }  
        }
        if ( buf[i][j] == NULL || strcmp(buf[i][j], ")") != 0 ) {  
            return NULL ; 
        }
        if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
        else { 
            j++ ; 
        }  
    }



    if ( strcmp(buf[i][j] , "SELECT" ) == 0 ){
        // select work needs to be done  ; 
    }

    if (strcmp(buf[i][j] , "VALUES" ) == 0  ){
        if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
        else { 
            j++ ; 
        }  
        if (strcmp(buf[i][j] , "SELECT" ) == 0  ){
            // need to put the select thing on it 
        }

        while (buf[i][j] != NULL && strcmp(buf[i][j] , "(" ) == 0 && node->num < 300  ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
            }  
            int kim = 0 ; 
            int terms = 128  ; 
            if (node->table_col_num  > 0 ){
                terms = node->table_col_num   ; 
            }
            insert_parser_tree * temp_value = createNodeInsert("VALUE_ROW");
            if (temp_value == NULL) { 
                break; 
            }

            while (buf[i][j] != NULL && strcmp(buf[i][j], ")") != 0 &&  kim < terms ){
                if (1){
                    if (strcmp(buf[i][j] , "(" ) == 0  ){
                        if (strcmp(buf[i][j] , "SELECT" ) == 0  ){
                            //work on it okay 
                            continue ; 
                        }
                        else { 
                            return NULL ; 
                        }
                    }
                    else { 
                        temp_value->insert_values[kim] = strdup(buf[i][j]) ;
                        temp_value->insert_val_num++;
                        kim++ ; 
                    }
                }
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }  

            }

            if (buf[i][j] == NULL || strcmp(buf[i][j] , ")") != 0 ){
                return NULL ; 
            }
            if (node->table_col_num > 0 && kim != node->table_col_num ){
                return NULL ; 
            }


            if (strcmp(buf[i][j] , ")") == 0 ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }  
            }
            node->children[node->num++] = temp_value ;
            if (strcmp(buf[i][j] , ",") == 0 ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }  
            }
        }
    }
    if (strcmp(buf[i][j] , "ON") == 0 ){
        if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
        else { 
            j++ ; 
        }  
        if (strcmp(buf[i][j] , "DUPLICATE") == 0 ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
            }   
            if (strcmp(buf[i][j] , "KEY") == 0 ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }  
                if (strcmp(buf[i][j] , "UPDATE") == 0 ){
                    if ( buf[i][j+1] == NULL ){
                        if (i+1 <= end_row){
                            i = i+ 1 ; 
                            j = 0  ; 
                        }
                    }
                    else { 
                        j++ ; 
                    }  
                    node->children[node->num++] = createNodeInsert("ON DUPLICATE KEY UPDATE") ; 
                    insert_parser_tree *temporary = (insert_parser_tree *)node->children[node->num - 1];
                    while ( strcmp(buf[i][j] , ";") != 0 && strcmp(buf[i][j] , "RETURNING") != 0 ){
                        int e = i ; 
                        int f = j ; 
                        int depth = 0 ;   
                        while ( strcmp(buf[i][j] , ",") != 0  && strcmp(buf[i][j] , "RETURNING") != 0 ){
                            if (strcmp(buf[i][j], "(") == 0) {
                                depth++ ;
                            }
                            else if (strcmp(buf[i][j], ")") == 0) {
                                depth-- ;
                            }
                            if ( buf[i][j+1] == NULL ){
                                if (i+1 <= end_row){
                                    i = i+ 1 ; 
                                    j = 0  ; 
                                }
                            }
                            else { 
                                j++ ; 
                            }   
                        }

                        insert_parser_tree *expr =  expression_insert(buf , temporary ,  e , f , i , j) ; 
                        if (expr != NULL && temporary->num < 300){
                            temporary->children[temporary->num++] = expr;
                        } 
                        if (buf[i][j] != NULL && strcmp(buf[i][j], ",") == 0){
                            if ( buf[i][j+1] == NULL ){
                                if (i+1 <= end_row){
                                    i = i+1 ;
                                    j = 0 ;
                                }
                            } 
                            else { 
                                j++ ; 
                            }
                        }
                    }
                }
                else { 
                    return NULL  ; 
                }
            }
            else { 
                return NULL ; 
            }
        }
        else { 
            return NULL ; 
        }
    }



    else if (strcmp(buf[i][j] , "RETURNING") == 0 ){
        node->children[node->num++] = createNodeInsert("RETURNING") ; 
        node = node->children[node->num -1 ] ; 
        if ( buf[i][j+1] == NULL ){
            if (i+1 <= end_row){
                i = i+ 1 ; 
                j = 0  ; 
            }
        }
        else { 
            j++ ; 
        }  
        while (strcmp(buf[i][j] , ";") != 0 && node->num < 300 ){
            if (strcmp(buf[i][j], ",") == 0) {
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){ 
                        i = i+1 ;
                        j = 0 ;
                    }
                } 
                else {
                    j++ ; 
                }
                continue;
            }
            node->children[node->num++] = createNodeInsert(buf[i][j]) ; 
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ; 
                    j = 0  ; 
                }
            }
            else { 
                j++ ; 
            }  
        }

    }
    return start ; 

}

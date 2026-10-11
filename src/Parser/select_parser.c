#include Parser.h 

char *SQL_functions[] = {
  "COUNT", "SUM", "AVG", "MIN", "MAX", "STDDEV", "VARIANCE",
  "UPPER", "LOWER", "LENGTH", "TRIM", "LTRIM", "RTRIM", "SUBSTRING", "REPLACE",
  "CONCAT", "CONCAT_WS", "INSTR", "LEFT", "RIGHT", "LPAD", "RPAD", "REVERSE",
  "REPEAT", "SPACE", "ASCII", "CHAR", "CHARINDEX", "POSITION", "STRCMP",
  "NOW", "CURDATE", "CURTIME", "DATE", "TIME", "YEAR", "MONTH", "DAY",
  "HOUR", "MINUTE", "SECOND", "DATEDIFF", "DATEADD", "DATE_ADD", "DATE_SUB",
  "DATE_FORMAT", "STR_TO_DATE", "TIMESTAMPDIFF", "EXTRACT", "AGE",
  "TO_DATE", "TO_TIMESTAMP",
  "ABS", "CEIL", "FLOOR", "ROUND", "TRUNCATE", "MOD", "POWER", "SQRT",
  "EXP", "LOG", "LOG2", "LOG10", "PI", "RAND", "SIGN", "GREATEST", "LEAST",
  "IF", "IIF", "COALESCE", "NULLIF", "ISNULL", "IFNULL", "NVL",
  "CAST", "CONVERT", "TO_CHAR", "TO_NUMBER",
  "ROW_NUMBER", "RANK", "DENSE_RANK", "NTILE", "LAG", "LEAD",
  "FIRST_VALUE", "LAST_VALUE", "NTH_VALUE", "CUME_DIST", "PERCENT_RANK",
  "JSON_EXTRACT", "JSON_OBJECT", "JSON_ARRAY", "JSON_SET", "JSON_REMOVE",
  "JSON_CONTAINS", "JSON_LENGTH",
  NULL
};


char *SQLITE_HL_keywords[] = {
  "CREATE", "TABLE", "VIEW", "INDEX", "TRIGGER", "VIRTUAL", "DROP", "ALTER",
	"ADD", "COLUMN", "RENAME", "ATTACH", "DETACH", "DATABASE",
	"SELECT", "INSERT", "UPDATE", "DELETE", "REPLACE", "INTO", "VALUES", "SET",
	"FROM", "WHERE", "RETURNING", "UPSERT",
	"JOIN", "INNER", "LEFT", "RIGHT", "FULL", "OUTER", "CROSS", "NATURAL", "ON",
	"USING", "GROUP", "BY", "HAVING", "ORDER", "ASC", "DESC", "LIMIT", "OFFSET",
	"UNION", "INTERSECT", "EXCEPT", "ALL", "DISTINCT", "WITH", "RECURSIVE", "AS",
	"WINDOW", "OVER", "PARTITION", "ROWS", "RANGE", "GROUPS", "BETWEEN",
	"UNBOUNDED", "CURRENT", "ROW", "PRECEDING", "FOLLOWING",
	"PRIMARY", "KEY", "FOREIGN", "REFERENCES", "UNIQUE", "NOT", "NULL", "DEFAULT",
	"CHECK", "CONSTRAINT", "WITHOUT", "ROWID", "STRICT", "AUTOINCREMENT",
	"CONFLICT", "ROLLBACK", "ABORT", "FAIL", "IGNORE", "CASCADE", "RESTRICT",
	"NO", "ACTION", "DEFERRABLE", "DEFERRED", "INITIALLY", "IMMEDIATE",
	"BEGIN", "COMMIT", "END", "TRANSACTION", "SAVEPOINT", "RELEASE",
	"EXCLUSIVE", "CONCURRENT",
	"AND", "OR", "IN", "LIKE", "GLOB", "REGEXP", "MATCH", "IS", "ISNULL",
	"NOTNULL", "EXISTS", "CASE", "WHEN", "THEN", "ELSE", "CAST", "COLLATE",
	"RAISE", "INDEXED", "UNINDEXED",
	"PRAGMA", "VACUUM", "ANALYZE", "REINDEX", "EXPLAIN", "QUERY", "PLAN",
	"TEMP", "TEMPORARY", "IF", "EACH", "FOR", "OF", "NEW", "OLD",
	"BEFORE", "AFTER", "INSTEAD" , 
	"TRUE", "FALSE", "CURRENT_TIME", "CURRENT_DATE", "CURRENT_TIMESTAMP",
	"INTEGER", "INT", "REAL", "TEXT", "BLOB", "NUMERIC", "BOOLEAN",
	"DATE", "DATETIME", "FLOAT", "DOUBLE", "CHAR", "VARCHAR", "CLOB", NULL 
};



bool if_function( char* word ){
	for ( int i = 0 ; SQL_functions[i] != NULL ; i++  ){
	if (strcmp( word , SQL_functions[i]  ) == 0 ){
		return true ; 
	}  ; 
	}	
	return false ; 
}

bool if_sql_syntax( char* word ){
	for ( int i = 0 ; SQLITE_HL_keywords[i] != NULL ; i++  ){
	if (strcmp( word , SQLITE_HL_keywords[i]  ) == 0 ){
		return true ; 
	}  ; 
	}	
	return false ; 
}



tree* createNode( char* comp) {
    tree* node = malloc(sizeof(tree));
    if (node == NULL) {
        return NULL;   
    }
    if( comp != NULL  ){
        node->comp = strdup(comp) ; 
    }
    else { 
        node->comp = NULL ; 
    }
    node->as = NULL ; 
    node->children = calloc(300, sizeof(tree*));
    
    node->direction = NULL ;  
    node->num = 0  ; 
    node->col = 0 ; 
    node->line = 0 ; 
    return node;
}



int priority(char *string){
    if ( strcmp("CASE" , string ) == 0 ||  strcmp("ON" , string ) == 0   ){
        return 1 ; 
    }
    else if (strcmp("OR" , string ) == 0  ){
        return 2 ; 
    }
    else if (strcmp("AND" , string ) == 0  ){
        return 3 ; 
    }
    else if (strcmp("NOT" , string ) == 0  ){
        return 4 ; 
    }
    else if (strcmp("BETWEEN", string) == 0 || strcmp("IN", string) == 0 || strcmp("LIKE", string) == 0 || strcmp("IS", string) == 0 || strcmp("EXISTS", string) == 0 || strcmp("IS NULL", string) == 0 || strcmp("IS NOT NULL", string) == 0 || strcmp("NOT IN", string) == 0 || strcmp("NOT LIKE", string) == 0 || strcmp("NOT BETWEEN", string) == 0){
         return 5;
    }
    else if (  strcmp("=" , string ) == 0  || strcmp("!=" , string ) == 0  || strcmp("<>" , string ) == 0  || strcmp(">" , string ) == 0  || strcmp("<" , string ) == 0  || strcmp(">=" , string ) == 0  || strcmp("<=" , string ) == 0  ){
        return 6 ; 
    }
    else if (strcmp("||" , string ) == 0   ){
        return 7 ; 
    }
    else if ( strcmp("+" , string ) == 0   || strcmp("-" , string ) == 0  ){
        return 8 ; 
    }
    else if (strcmp("*" , string ) == 0   || strcmp("/" , string ) == 0  || strcmp("%" , string ) == 0   ) {
        return 9 ; 
    }
    return 0  ; 
}


tree* expression(char ***buf , tree * node , int check , int i , int j  , int end_row , int end_col  ){
    if ( check == 1 ){
        return NULL ; 
    }
    char string_temp_buf[600];
    char * operands[300] ; 
    int start_row =  i  ; 
    int start_col = j ; 
    char * operators[300] ; 
    char *string_temp; 
    int operator_top = 0 ; 
    int operand_top = 0 ; 
    tree * prev_tree[300]  ; 
    int prev_tree_top = 0 ; 
    int first = 0 ; 

    while ( buf[i][j] != NULL && strcmp(buf[i][j], ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){

        if (priority(buf[i][j]) != 0) {    
            if ( j+1 <= end_col &&  priority(buf[i][j]) != 0  && strcmp(buf[i][j], "IS") == 0 && strcmp(buf[i][j+1], "NULL") == 0   ){
                string_temp = "IS NULL" ; 
                j = j +1 ; 
            }
            else if (j+1 <= end_col &&  priority(buf[i][j]) != 0  && strcmp(buf[i][j], "IS") == 0 && strcmp(buf[i][j+1], "NOT") == 0  && strcmp(buf[i][j+1], "NULL") == 0  ){
                string_temp = "IS NOT NULL" ; 
                j = j+2 ; 
            }
            else if (j+1 <= end_col &&  priority(buf[i][j]) != 0  && strcmp(buf[i][j], "NOT") == 0 && strcmp(buf[i][j+1], "IN") == 0   ){
                string_temp = "NOT IN" ; 
                j = j +1 ; 
            }
            else if ( j+1 <= end_col && priority(buf[i][j]) != 0  && strcmp(buf[i][j], "NOT") == 0 && strcmp(buf[i][j+1], "LIKE") == 0   ){
                string_temp = "NOT LIKE" ; 
                    j = j +1 ; 
            }
            else if (j+1 <= end_col &&  priority(buf[i][j]) != 0  && strcmp(buf[i][j], "NOT") == 0 && strcmp(buf[i][j+1], "BETWEEN") == 0   ){
                string_temp = "NOT BETWEEN" ; 
                j = j +1 ; 
            }
            else if (j + 1 <= end_col ){
                char merged[10] ; 
                strcpy(merged, buf[i][j]);
                strcat(merged, buf[i][j + 1]);
                if ( priority(merged) != 0 ){
                    string_temp = strdup(merged) ; 
                    j += 1  ; 
                }
                else {
                    string_temp = strdup(buf[i][j]);  
                }
            }
            else { 
                   string_temp = strdup(buf[i][j]);
            }

             if  (first == 0 && j == start_col ){
                 if ( i > start_row ){
                        int size = 0  ; 
                        for ( int k = 0 ; buf[i - 1][k] != NULL ; k++ ){
                            size++ ; 
                        }
                        if ( size > 0 ){
                          operands[operand_top++] = buf[i-1][size - 1 ] ; 
                        }
                    }
                        else if ( i == start_row && start_col > 0 ){
                          operands[operand_top++] = buf[i][start_col - 1] ;
                        }
                }
            first = 1 ; 

            if ( operator_top == 0 ){
                operators[operator_top++] = string_temp ;                                            
            }
            else { 
                if  ( priority(string_temp) <= priority(operators[operator_top - 1 ]) ){
                    while ( operator_top > 0 && priority(string_temp) <= priority(operators[operator_top - 1 ]  )){
                        char *oper = operators[operator_top - 1] ; 
                        int num_temp = 2  ; 
                        if ( strcmp(oper, "NOT") == 0 ||strcmp(oper, "IS NULL") == 0 || strcmp(oper, "IS NOT NULL") == 0 )  {
                            num_temp = 1 ; 
                        }
                        if ( operand_top < num_temp ){
                            operator_top-- ;
                            continue ;
                        }
                        tree * new  = createNode(oper) ; 
                        operator_top-- ; 
                        int pal = 0 ; 
                        while ( pal < num_temp  ){
                            operand_top-- ;     
                            if ( operands[operand_top] ==  NULL){
                                new->children[num_temp - 1 - pal]  = prev_tree[--prev_tree_top] ; 
                            }
                            else { 
                                new->children[num_temp - 1 - pal] = make_leaf(operands[operand_top] , i  ,  j ) ; 
                            }
                            pal++ ; 
                        }
                        new->num = num_temp;
                        prev_tree[prev_tree_top++] = new;
                        operands[operand_top] = NULL;
                        operand_top++;
                    }
                    operators[operator_top++] = string_temp ; 
                }
                else  {
                    operators[operator_top++] = string_temp ; 
                }
            }
        }
        else { 
                if ( if_function( buf[i][j] ) == true ){
                        int start_row = i ; 
                        int start_col = j ; 
                        int commas = 0 ; 
                        bool present = false ;
                        while (i<=end_row){
                            while (j <= end_col){
                                if ( strcmp(buf[i][j] , "(") == 0  ){
                                    commas++ ;
                                    present = true ; 
                                }
                                else if ( strcmp(buf[i][j] , ")") == 0 ){
                                    commas-- ; 
                                }
                                if ( present &&  commas == 0 ){
                                    break ; 
                                }
                                if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                            }
                            if ( present &&  commas == 0 ){
                                break ; 
                            }                               
                             if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }


                        }

                         tree * just_there;
                        if ( prev_tree_top == 0 || prev_tree[prev_tree_top - 1] == NULL ){
                            tree *func_parent = createNode(NULL);
                            just_there = function( buf , func_parent , check , start_row , start_col , i , j );
                        }
                        else {
                            just_there = function( buf , prev_tree[prev_tree_top - 1] , check , start_row , start_col , i , j );
                        }
                        if (just_there != NULL  ){
                                prev_tree[prev_tree_top++] = just_there ;
                              operands[operand_top++] = NULL ;
                        } 
                }
                else if ( strcmp(buf[i][j] , "(") == 0  ){
                    if ( j-1 > 0 && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = j+1 ; 
                        int end_row_dec  = i ; 
                        int braces = 1 ; 
                        for ( int m = i ; m <= end_row ; m++  ){
                            for ( int n = j+1 ; n <= end_col ; n++  ){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                            }
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec , pain  ); 
                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                }
                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                } 
                                just_there->as = strdup(buf[i][j] )  ; 
                            }   
                            else { 
                                check = 1 ; 
                                return NULL ; 
                            }
                        }
                        if (just_there != NULL  ){
                                prev_tree[prev_tree_top++] = just_there ;
                              operands[operand_top++] = NULL ;
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        operands[operand_top] = buf[i][j] ; 
                        operand_top++ ; 
                    }
                }
                else {
                    operands[operand_top] = buf[i][j] ; 
                    operand_top++ ; 
                }
        }
            if ( buf[i][j] == NULL){
                if ( i +1 < end_row){
                i++ ; 
                j = 0 ; 
                }
                else { 
                    break ; 
                }
            }
            else { 
                j++ ; 
            }
    }


    while (operator_top > 0) {
        char *oper = operators[operator_top - 1] ; 
        int num_temp = 2   ; 
        if ( strcmp(oper, "NOT") == 0 ||strcmp(oper, "IS NULL") == 0 || strcmp(oper, "IS NOT NULL") == 0 )  {
            num_temp = 1 ; 
        }

        if ( operand_top < num_temp ){
            operator_top-- ;
            continue ;
        }

        tree * new  = createNode(oper) ; 
        operator_top-- ; 
        int pal = 0 ; 
        while ( pal < num_temp  ){
            operand_top-- ;     
            if ( operands[operand_top] ==  NULL){
                new->children[num_temp - 1 - pal]  = prev_tree[--prev_tree_top] ; 
            }
            else { 
                new->children[num_temp - 1 - pal ] = make_leaf(operands[operand_top] , i  ,  j ) ; 
            }

            pal++ ; 
        }
         new->num = num_temp;
        prev_tree[prev_tree_top++] = new;
        operands[operand_top] = NULL;
        operand_top++;
    }
    tree *exp_tree ; 
    if (prev_tree_top > 0){
        exp_tree = prev_tree[prev_tree_top - 1] ; 
    }
    else {
        exp_tree = NULL ; 
    }
    if ( j-1 >= start_col ){
        j = j - 1 ;
    }
    return exp_tree ; 
}






tree *function( char ***buf , tree * node ,int check  ,  int i , int j  , int end_row , int end_col ){
        if ( check == 1 ){
            return NULL ; 
        }
         tree* temp = node ; 
        temp->children[temp->num++]  = createNode(buf[i][j]) ; 
        temp = temp->children[temp->num- 1 ] ; 
        tree *start = temp ; 
        int pal = 0 ; 
        while ( pal < 2 ){
        if ( buf[i][j+1] != NULL ){
            j++ ; 
        }
        pal++ ; 
         }


        while ( strcmp(buf[i][j] , ")") != 0 ){
             if ( strcmp(buf[i][j] , "(") == 0  ){
                    if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = j+1 ; 
                        int end_row_dec  = i ; 
                        int braces = 1 ; 
                        for ( int m = i ; m <= end_row ; m++  ){
                            for ( int n = j+1 ; n <= end_col ; n++  ){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                            }
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain  ); 
                            if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                }
                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                } 
                                just_there->as = strdup(buf[i][j] )  ; 
                            }   
                            else { 
                                check = 1 ; 
                                return NULL ; 
                            }
                        }
                        if (just_there != NULL  ){
                              temp->children[temp->num++]  = just_there ; 
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        while ( strcmp(buf[i][j] , ")") != 0 ){
                                if ( priority(buf[i][j]) != 0  ){
                                int temp_row = i ; 
                                int temp_col = j ; 
                                while ( buf[i][j] != NULL && strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+ 1 <= end_row){
                                        i++ ; 
                                        j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                    }
                                    else {
                                        j++ ;
                                    }
                                }
                                tree *  just_there =  expression(buf , temp ,check ,   temp_row ,  temp_col  ,  i ,  j ) ; 
                                if (just_there != NULL ){
                                temp->children[temp->num++ ]  = just_there ; 
                                }
                                }
                                else  {
                                if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if ( buf[i][j] == NULL){
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } else {
                                                break ;
                                            }
                                        }
                                        else {
                                            if (j +1 <= end_col){
                                            j++ ; 
                                            }
                                            else {
                                                break ; 
                                            }
                                        }
                                                continue ;
                                        }
                                        temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                    }
                                    else { 
                                        check = 1 ; 
                                            return NULL ;   
                                    }
                            } 
                        }
                    }
            }

            else { 
                    if ( priority(buf[i][j]) != 0  ){
                        int temp_row = i ; 
                        int temp_col = j ; 
                        while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                            if ( buf[i][j] == NULL ){
                                if ( i+ 1 <= end_row){
                                i++ ; 
                                j = 0 ; 
                            }
                            else { 
                                break ; 
                            }
                            }
                           else {
                                if (buf[i][j+1] != NULL){
                                j++ ; 
                                }
                                else {
                                    break ; 
                                }
                            }
                        }
                        tree*  just_there =  expression(buf , temp ,check ,   temp_row ,  temp_col  ,  i ,  j ) ; 
                        if (just_there != NULL ){
                        temp->children[temp->num++ ]  = just_there ; 
                        }
                        continue ; 
                    }         
                    else {
                        if (if_syntax(buf[i][j]) == false ){
                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                    if (  buf[i][j] == NULL  ) {
                                         if ( buf[i+1][0] != NULL ){
                                        i++ ;
                                        j = 0 ;
                                    } 
                                }
                                else { 
                                    j++ ; 
                                }
                                        continue ;
                                }
                                 if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                    int temp_row = i ; 
                                    int temp_col = j ; 
                                    while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                        if ( buf[i][j] == NULL ){
                                            if ( i+ 1 <= end_row){
                                            i++ ; 
                                            j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                        }
                                        else {
                                            j++ ;
                                        }
                                    }
                                    tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                    if (just_there != NULL ){
                                    temp->children[temp->num++ ]  = just_there ; 
                                    }
                                    continue ;  
                                }
                                else {
                                       temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                }
                            }
                            else { 
                                check = 1 ; 
                                return NULL ;   
                            }
                    }                    
            }
            if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }
    }
    return start ; 
}



tree *case_expr(char ***buf , int check ,  int i , int j , int end_row , int end_col ){
    tree*temp = createNode("CASE") ; 
    tree * start = temp ; 
    if (buf[i][j+1] != NULL) {
        j++ ;
    }
    else if (buf[i][j+1] == NULL) {
        if (buf[i+1][0] != NULL) {
            i++ ;
            j = 0 ;
        }
        else {
            break ;
        }
    }

    int first = 0 ; 
    tree *prev_where[300] = {0} ; 
    int prev_where_count = 0  ; 

    while ( i <= end_row && j <= end_col ){
        if (  buf[i][j] != NULL &&  (strcmp(buf[i][j] , "WHEN") == 0 ||  strcmp(buf[i][j] , "ELSE") == 0 )){
            temp = start ; 
            if ( first > 0 ){
            temp->num++ ; 
            }
            else { 
                first = 1 ; 
            }
            if ( buf[i][j] != NULL &&  strcmp(buf[i][j] , "WHEN") == 0 ){
                temp->children[temp->num] = createNode("WHEN") ; 
                temp = temp->children[temp->num] ; 
            }
            else{
                temp->children[temp->num] = createNode("ELSE") ; 
                temp = temp->children[temp->num] ; 
            }
            prev_where[prev_where_count++] = temp ; 
        }





        else if ( strcmp(buf[i][j] , "CASE") == 0){
           int cases = 1 ; 
            if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }
            int start_row = i ; 
            int start_col = j   ; 
            while (  i <= end_row && j<= end_col && cases != 0 &&  strcmp(buf[i][j] , "END") != 0  ){
                if ( strcmp(buf[i][j] , "CASE") == 0 ){
                    cases++ ; 
                }
                else if ( cases > 0  && strcmp(buf[i][j] , "END") == 0 ){
                    cases-- ; 
                }
                if (buf[i][j+1] != NULL) {
                    j++ ;
                }
                else {
                    if (buf[i+1][0] != NULL) {
                        i++ ;
                        j = 0 ;
                    }
                    else {
                        break ;
                    }
                }
            }
            temp->children[temp->num++] = case_expr(buf  , check , start_row , start_col , i , j ) ; 
        }


        else { 
            if (buf[i][j] != NULL &&   strcmp(buf[i][j] , "THEN") == 0 ){
                if (  prev_where[prev_where_count -1 ]  == NULL ){
                    check = 1 ; 
                    return NULL ;  
                }
                temp->children[temp->num++] = createNode("THEN") ; 
                temp = temp->children[temp->num - 1 ] ; 
                if (buf[i][j+1] != NULL) {
                    j++ ;
                }
                else {
                    if (buf[i+1][0] != NULL) {
                        i++ ;
                        j = 0 ;
                    }
                    else {
                        break ;
                    }
                    continue ; 
                }

            }

             if ( buf[i][j] != NULL &&  strcmp(buf[i][j] , "(") == 0  ){
                    if ( j+1 <=  end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = j+1 ; 
                        int end_row_dec  = i ; 
                        int braces = 1 ; 
                        int thing = j+1 ;   ; 
                        for ( int m = i ; m <= end_row ; m++  ){
                            for ( int n = thing; n <= end_col ; n++  ){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                            }
                            thing = 0 ; 
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec , pain   ); 
                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                }
                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                } 
                                just_there->as = strdup(buf[i][j] )  ; 
                            }   
                            else { 
                                check = 1 ; 
                                return NULL ; 
                            }
                        }
                        if (just_there != NULL  ){
                              temp->children[temp->num++]  = just_there ; 
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        while ( buf[i][j] != NULL &&  strcmp(buf[i][j] , ")") != 0 ){
                                if ( priority(buf[i][j]) != 0  ){
                                int temp_row = i ; 
                                int temp_col = j ; 
                                while ( buf[i][j] != NULL && strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+ 1 <= end_row){
                                        i++ ; 
                                        j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                    }
                                    else {
                                        j++ ;
                                    }
                                }
                                tree *  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                if (just_there != NULL ){
                                temp->children[temp->num++ ]  = just_there ; 
                                }
                                }
                                else  {
                                if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if ( buf[i][j] == NULL){
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } else {
                                                break ;
                                            }
                                        }
                                        else {
                                            j++ ; 
                                        }
                                                continue ;
                                        }
                                        temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                    }
                                    else { 
                                        check = 1 ; 
                                            return NULL ;   
                                    }
                            } 
                        }
                    }
            }




            else {
                if (if_syntax(buf[i][j]) == false ){
                        if ( strcmp(buf[i][j] , ",") == 0 ){
                            if (  buf[i][j] == NULL  ) {
                                if ( buf[i+1][0] != NULL ){
                                i++ ;
                                j = 0 ;
                            } 
                        }
                        else { 
                            j++ ; 
                        }
                                continue ;
                        }
                            if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                            int temp_row = i ; 
                            int temp_col = j ; 
                            while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                if ( buf[i][j] == NULL ){
                                    if ( i+ 1 <= end_row){
                                    i++ ; 
                                    j = 0 ; 
                                }
                                else { 
                                    break ; 
                                }
                                }
                                else {
                                    j++ ;
                                }
                            }
                            tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                            if (just_there != NULL ){
                            temp->children[temp->num++ ]  = just_there ; 
                            }
                        }
                        else {
                                temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                        }
                    }
                    else { 
                        check = 1 ; 
                        return NULL ;   
                    }
            }
    }
        if (buf[i][j+1] != NULL) {
            j++ ;
        }
        else {
            if (buf[i+1][0] != NULL) {
                i++ ;
                j = 0 ;
            }
            else {
                break ;
            }
        }

    }
    return start ; 
}




tree *comp_1(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col ){
    int start_row = i ; 
    int start_col = j ; 
    tree * start = node ; 
    tree * temp  = node ; 
    int check = 0 ; 
    int pain  = 0  ;
    while ( i <= end_row){
       while ( j <= end_col ) {

            if (  strcmp(buf[i][j] ,  "FROM"  ) == 0  ) {
                return start ; 
            }
           else { 

                if ( if_function( buf[i][j] ) == true ){
                    int start_row = i ; 
                    int start_col = j ; 
                    int commas = 0 ; 
                    bool present = false ; 
                          while (i<=end_row){
                            while (j <= end_col){
                                if ( strcmp(buf[i][j] , "(") == 0  ){
                                    commas++ ;
                                    present = true ; 
                                }
                                else if ( strcmp(buf[i][j] , ")") == 0 ){
                                    commas-- ; 
                                }
                                if ( present && commas == 0 ){
                                    break ; 
                                }
                                if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                            }
                            if ( present &&  commas == 0 ){
                                break ; 
                            }                               
                             if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }


                        }
                temp->children[temp->num++] = function( buf , temp,check ,   i ,  j , end_row , end_col ) ;
            }
                

            else  if ( strcmp(buf[i][j] , "(") == 0  ){
                    if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = j+1 ; 
                        int end_row_dec  = i ; 
                        int braces = 1 ; 
                        int m = i ; 
                        int n = j+1 ; 
                        while (m<= end_row){
                            while (n <= end_col){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                                if ( buf[m][n] == NULL){
                                    if ( m +1 < end_row){
                                    m++ ; 
                                    n = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    n++ ; 
                                }
                            }
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain ); 
                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                }
                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                } 
                                just_there->as = strdup(buf[i][j] )  ; 
                            }   
                            else { 
                                check = 1 ; 
                                return NULL ; 
                            }
                        }
                        if (just_there != NULL  ){
                              temp->children[temp->num++]  = just_there ; 
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        while ( strcmp(buf[i][j] , ")") != 0 ){
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    if (buf[i][j+1] != NULL ){
                                        j++ ; 
                                    }   
                                        else {
                                            break ;
                                        }
                                }
                               if ( priority(buf[i][j]) != 0 ){
                                int temp_row = i ;
                                int temp_col = j ;
                                int depth = 0 ;

                                while (i <= end_row ){
                                    while (j <= end_col ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+1 <= end_row ){ 
                                            i++ ;
                                            j = 0 ;
                                             continue ; }
                                        else { 
                                            break ;
                                         }
                                    }

                                    if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                        break ;
                                    }

                                    if ( strcmp(buf[i][j], "(") == 0 ){
                                        depth++ ;
                                    }
                                    else if ( strcmp(buf[i][j], ")") == 0 ){
                                        if ( depth == 0 ){
                                            break ;  
                                        }
                                        depth-- ;
                                    }
                                    if ( buf[i][j] == NULL){
                                        if ( i +1 < end_row){
                                        i++ ; 
                                        j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                    }
                                    else { 
                                        j++ ; 
                                    }   
                                    if ( i > end_row ){
                                        break ;   
                                    }
                                }   
                                }

                                tree * just_there = expression( buf , temp , check , temp_row , temp_col , i , j ) ;
                                if ( just_there != NULL ){
                                    temp->children[temp->num++] = just_there ;
                                }
                            }
                                else  {
                                    if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if (  buf[i][j] == NULL  ) {
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } 
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                                continue ;
                                        }
                                        else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                            int temp_row = i ; 
                                            int temp_col = j ; 
                                            while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                if ( buf[i][j] == NULL ){
                                                    if ( i+ 1 <= end_row){
                                                    i++ ; 
                                                    j = 0 ; 
                                                }
                                                else { 
                                                    break ; 
                                                }
                                                }
                                                else {
                                                    j++ ;
                                                }
                                            }
                                            tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                            if (just_there != NULL ){
                                            temp->children[temp->num++ ]  = just_there ; 
                                            }
                                        }
                                        else {
                                            temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                        }
                                    }
                                    else { 
                                        check = 1 ; 
                                            return NULL ;   
                                    }
                            } 
                        }
                    }
            }




              else { 
                 if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                        temp->children[temp->num++] = make_leaf("*", i, j) ;
                    }
                    else if ( priority(buf[i][j]) != 0  ){
                        int temp_row = i ; 
                        int temp_col = j ; 
                        while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                            if ( buf[i][j] == NULL ){
                                if ( i+ 1 <= end_row){
                                i++ ; 
                                j = 0 ; 
                            }
                            else { 
                                break ; 
                            }
                            }
                            else {
                                j++ ;
                            }
                        }
                        tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                        if (just_there != NULL ){
                        temp->children[temp->num++ ]  = just_there ; 
                        }
                    }         
                    else {
                        if (if_syntax(buf[i][j]) == false ){
                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                    if (  buf[i][j] == NULL  ) {
                                         if ( buf[i+1][0] != NULL ){
                                        i++ ;
                                        j = 0 ;
                                    } 
                                }
                                else { 
                                    j++ ; 
                                }
                                        continue ;
                                }
                                 if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                    int temp_row = i ; 
                                    int temp_col = j ; 
                                    while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                        if ( buf[i][j] == NULL ){
                                            if ( i+ 1 <= end_row){
                                            i++ ; 
                                            j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                        }
                                        else {
                                            j++ ;
                                        }
                                    }
                                    tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                    if (just_there != NULL ){
                                    temp->children[temp->num++ ]  = just_there ; 
                                    }
                                }
                                else {
                                       temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                }
                            }
                            else { 
                                check = 1 ; 
                                return NULL ;   
                            }
                    }                    
            }
        }


             if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }

            }
            break;
    }
    return start  ; 

}



tree *comp_2(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col ){
    int start_row = i ; 
    int start_col = j ; 
    tree * start = node ; 
    tree * temp  = node ; 
    int check = 0 ; 
    int pain  = 0  ;
    while ( i <= end_row){
       while ( j <= end_col ) {

            if (  strcmp(buf[i][j] ,  "WHERE"  ) == 0  ) {
                return start ; 
            }
            else if (  strcmp(buf[i][j], "JOIN") == 0  || 
                    ( j+1 <= end_col && strcmp(buf[i][j], "INNER") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) || 
                    (j+1 <= end_col && strcmp(buf[i][j], "LEFT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                    (j+1 <= end_col && strcmp(buf[i][j], "RIGHT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                    (j+1 <= end_col && strcmp(buf[i][j], "CROSS") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                    (j+2 <= end_col && strcmp(buf[i][j], "FULL") == 0 && strcmp(buf[i][j+1], "OUTER") == 0 && strcmp(buf[i][j+2], "JOIN") == 0 )){

                int on = 0 ; 
                tree* starter  ; 

                if (strcmp(buf[i][j], "INNER") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                    starter = createNode("INNER JOIN");
                    j = j + 2;
                }
                else if (strcmp(buf[i][j], "JOIN") == 0 ){
                    starter = createNode(buf[i][j]);
                    j++ ; 
                }
                else if (strcmp(buf[i][j], "LEFT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                    starter = createNode("LEFT JOIN");
                    j = j + 2;
                }
                else if (strcmp(buf[i][j], "RIGHT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                    starter =  createNode("RIGHT JOIN");
                    j = j + 2;
                }
                else if (strcmp(buf[i][j], "CROSS") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                    starter =  createNode("CROSS JOIN");
                    j = j + 2;
                }
                else if (strcmp(buf[i][j], "FULL") == 0 && strcmp(buf[i][j+1], "OUTER") == 0 && strcmp(buf[i][j+2], "JOIN") == 0 ){
                   starter = createNode("FULL OUTER JOIN");
                    j = j + 3;
                }
                else {
                    check = 1 ; 
                    return NULL  ; 
                }

                tree *temp_word = starter ; 

                while ( strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 && strcmp(buf[i][j] , "WHERE") != 0   && strcmp(buf[i][j], "GROUP") != 0 &&  strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "FETCH") != 0 &&  strcmp(buf[i][j], "LIMIT") != 0 &&strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "UNION") != 0 && strcmp(buf[i][j], "INTERSECT") != 0 && strcmp(buf[i][j], "EXCEPT") != 0  && strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 && buf[i][j] != NULL){

                        if ( if_function( buf[i][j] ) == true ){
                            int start_row = i ; 
                            int start_col = j ; 
                            int commas = 0 ; 
                            bool present = false ; 
                              while (i<=end_row){
                            while (j <= end_col){
                                if ( strcmp(buf[i][j] , "(") == 0  ){
                                    commas++ ;
                                    present = true ; 
                                }
                                else if ( strcmp(buf[i][j] , ")") == 0 ){
                                    commas-- ; 
                                }
                                if (present && commas == 0 ){
                                    break ; 
                                }
                                if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                            }
                            if ( present &&  commas == 0 ){
                                break ; 
                            }                               
                             if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }


                        }
                        temp_word->children[temp_word->num++] = function( buf , temp_word,check ,   start_row ,  start_col , i , j ) ;
                    }
                        

                    else  if ( strcmp(buf[i][j] , "(") == 0  ){
                            if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                                int end_col_dec  = j+1 ; 
                                int end_row_dec  = i ; 
                                int braces = 1 ; 
                                int m = i ; 
                                int n = j+1 ; 
                                while (m<= end_row){
                                    while (n <= end_col){
                                        if ( strcmp(buf[m][n] , "(") == 0  ){
                                            braces++ ; 
                                        }
                                        else if ( strcmp(buf[m][n] , ")") == 0 ){
                                            braces-- ; 
                                        }
                                        if ( braces == 0 ){
                                            end_row_dec = m ; 
                                            end_col_dec = n ; 
                                            break ; 
                                        }
                                        if ( buf[m][n] == NULL){
                                            if ( m +1 < end_row){
                                            m++ ; 
                                            n = 0 ; 
                                            }
                                            else { 
                                                break ; 
                                            }
                                        }
                                        else { 
                                            n++ ; 
                                        }
                                    }
                                        if ( braces == 0 ){
                                            break ; 
                                        }

                                }
                                tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec , pain   ); 
                                    if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                            if (buf[i][j+1] != NULL) {
                                                j++ ;
                                            }
                                            else if (buf[i][j+1] == NULL) {
                                                if (buf[i+1][0] != NULL) {
                                                    i++ ;
                                                    j = 0 ;
                                                }
                                                else {
                                                    break ;
                                                }
                                            }
                                        if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                            if (buf[i][j+1] != NULL) {
                                                j++ ;
                                            }
                                            else if (buf[i][j+1] == NULL) {
                                                if (buf[i+1][0] != NULL) {
                                                    i++ ;
                                                    j = 0 ;
                                                }
                                                else {
                                                    break ;
                                                }
                                            } 
                                            just_there->as = strdup(buf[i][j] )  ; 
                                        }   
                                        else { 
                                            check = 1 ; 
                                            return NULL ; 
                                        }
                                    }
                                if (just_there != NULL  ){
                                    temp_word->children[temp_word->num++]  = just_there ; 
                                }
                                i = end_row_dec ; 
                                j = end_col_dec ; 
                            }
                            else {
                                while ( strcmp(buf[i][j] , ")") != 0 ){
                                        if ( buf[i][j] == NULL){
                                            if ( i +1 < end_row){
                                            i++ ; 
                                            j = 0 ; 
                                            }
                                            else { 
                                                break ; 
                                            }
                                        }
                                        else { 
                                            if (buf[i][j+1] != NULL ){
                                                j++ ; 
                                            }   
                                             else {
                                                    break ;
                                                }
                                        }
                                        if (buf[i][j] != NULL && strcmp(buf[i][j] , "ON") == 0   ){
                                        if (strcmp(starter->comp, "CROSS MODEL") == 0) {
                                                check = 1;
                                                return NULL;
                                            }
                                            j++;
                                            continue;
                                    }
                                    if ( priority(buf[i][j]) != 0 ){
                                        int temp_row = i ;
                                        int temp_col = j ;
                                        int depth = 0 ;

                                        while (i <= end_row ){
                                            while (j <= end_col ){
                                            if ( buf[i][j] == NULL ){
                                                if ( i+1 <= end_row ){ 
                                                    i++ ;
                                                    j = 0 ;
                                                    continue ; }
                                                else { 
                                                    break ;
                                                }
                                            }

                                            if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                                break ;
                                            }

                                            if ( strcmp(buf[i][j], "(") == 0 ){
                                                depth++ ;
                                            }
                                            else if ( strcmp(buf[i][j], ")") == 0 ){
                                                if ( depth == 0 ){
                                                    break ;  
                                                }
                                                depth-- ;
                                            }
                                            if ( buf[i][j] == NULL){
                                                if ( i +1 < end_row){
                                                i++ ; 
                                                j = 0 ; 
                                                }
                                                else { 
                                                    break ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }   
                                            if ( i > end_row ){
                                                break ;   
                                            }
                                        }   
                                        }

                                        tree * just_there = expression( buf , temp_word , check , temp_row , temp_col , i , j ) ;
                                        if ( just_there != NULL ){
                                            temp_word->children[temp_word->num++] = just_there ;
                                        }
                                    }
                                        else  {
                                            if (if_syntax(buf[i][j]) == false ){
                                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                                    if (  buf[i][j] == NULL  ) {
                                                        if ( buf[i+1][0] != NULL ){
                                                        i++ ;
                                                        j = 0 ;
                                                    } 
                                                }
                                                else { 
                                                    j++ ; 
                                                }
                                                        continue ;
                                                }
                                                else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                    int temp_row = i ; 
                                                    int temp_col = j ; 
                                                    while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                        if ( buf[i][j] == NULL ){
                                                            if ( i+ 1 <= end_row){
                                                            i++ ; 
                                                            j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                        }
                                                        else {
                                                            j++ ;
                                                        }
                                                    }
                                                    tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                    if (just_there != NULL ){
                                                    temp_word->children[temp_word->num++ ]  = just_there ; 
                                                    }
                                                }
                                                else {
                                                    tree * leaf_node = make_leaf( buf[i][j]  , i , j ) ;
                                                    if ( ( j+1 <= end_col && buf[i][j+1] != NULL && strcmp(buf[i][j+1], "AS") == 0 ) ||
                                                         ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL && strcmp(buf[i+1][0], "AS") == 0 ) ){
                                                        if (buf[i][j+1] != NULL) {
                                                            j++ ;
                                                        }
                                                        else {
                                                            i++ ;
                                                            j = 0 ;
                                                        }
                                                        if ( ( j+1 <= end_col && buf[i][j+1] != NULL ) ||
                                                             ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL ) ){
                                                            if (buf[i][j+1] != NULL) {
                                                                j++ ;
                                                            }
                                                            else {
                                                                i++ ;
                                                                j = 0 ;
                                                            }
                                                            leaf_node->as = strdup( buf[i][j] ) ;
                                                        }
                                                    }
                                                    temp_word->children[temp_word->num++] = leaf_node ;
                                                }
                                            }
                                            else { 
                                                check = 1 ; 
                                                    return NULL ;   
                                            }
                                    } 
                                }
                            }
                    }




                    else { 
                            if (buf[i][j] != NULL && strcmp(buf[i][j] , "ON") == 0   ){
                                 if (strcmp(starter->comp, "CROSS MODEL") == 0) {
                                        check = 1;
                                        return NULL;
                                    }
                                    j++;
                                    continue;
                            }
                             if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                                    temp->children[temp->num++] = make_leaf("*", i, j) ;
                                }
                            if ( priority(buf[i][j]) != 0  ){
                                int temp_row = i ; 
                                int temp_col = j ; 
                                while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+ 1 <= end_row){
                                        i++ ; 
                                        j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                    }
                                    else {
                                        j++ ;
                                    }
                                }
                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                if (just_there != NULL ){
                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                }
                            }         
                            else {
                                if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if (  buf[i][j] == NULL  ) {
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } 
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                                continue ;
                                        }
                                        if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                            int temp_row = i ; 
                                            int temp_col = j ; 
                                            while ( strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                if ( buf[i][j] == NULL ){
                                                    if ( i+ 1 <= end_row){
                                                    i++ ; 
                                                    j = 0 ; 
                                                }
                                                else { 
                                                    break ; 
                                                }
                                                }
                                                else {
                                                    j++ ;
                                                }
                                            }
                                            tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                            if (just_there != NULL ){
                                            temp_word->children[temp_word->num++ ]  = just_there ; 
                                            }
                                        }
                                        else {
                                            tree * leaf_node = make_leaf( buf[i][j]  , i , j ) ;
                                            if ( ( j+1 <= end_col && buf[i][j+1] != NULL && strcmp(buf[i][j+1], "AS") == 0 ) ||
                                                 ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL && strcmp(buf[i+1][0], "AS") == 0 ) ){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else {
                                                    i++ ;
                                                    j = 0 ;
                                                }
                                                if ( ( j+1 <= end_col && buf[i][j+1] != NULL ) ||
                                                     ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL ) ){
                                                    if (buf[i][j+1] != NULL) {
                                                        j++ ;
                                                    }
                                                    else {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    leaf_node->as = strdup( buf[i][j] ) ;
                                                }
                                            }
                                            temp_word->children[temp_word->num++] = leaf_node ;
                                        }
                                    }
                                    else { 
                                        check = 1 ; 
                                        return NULL ;   
                                    }
                            }                    
                    }

                    if (buf[i][j+1] != NULL) {
                        j++ ;
                    }
                    else {
                        if (buf[i+1][0] != NULL) {
                            i++ ;
                            j = 0 ;
                        }
                        else {
                            break ;
                        }
                    }

                }


                if ( j-1 > 0 ){
                    j = j-1 ; 
                }
                else {
                    int temp_row ; 
                    int temp_col ; 
                    if ( i- 1 > start_row ){
                        temp_row = i -1 ; 
                        temp_col = start_col ; 
                        for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                        }
                        j = temp_col -1 ; 
                        i = temp_row ; 
                    }
                  
          
                      else {
                        i = start_row ; 
                        j = start_col ; 
                    }
                }


                if ( pain == 0 ){
                    temp->children[temp->num++] = starter ; 
                }
                else { 
                    return starter ; 
                }

                }



            else { 

                if ( if_function( buf[i][j] ) == true ){
                    int start_row = i ; 
                    int start_col = j ; 
                    int commas = 0 ; 
                    bool present = false ; 
                          while (i<=end_row){
                            while (j <= end_col){
                                if ( strcmp(buf[i][j] , "(") == 0  ){
                                    commas++ ;
                                    present = true ; 
                                }
                                else if ( strcmp(buf[i][j] , ")") == 0 ){
                                    commas-- ; 
                                }
                                if ( present && commas == 0 ){
                                    break ; 
                                }
                                if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                            }
                            if ( present &&  commas == 0 ){
                                break ; 
                            }                               
                             if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }


                        }
                temp->children[temp->num++] = function( buf , temp,check ,   i ,  j , end_row , end_col ) ;
            }
                

            else  if ( strcmp(buf[i][j] , "(") == 0  ){
                    if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = 0 ; 
                        int end_row_dec  = 0 ; 
                        int braces = 1 ; 
                        int m = i ; 
                        int n = j+1 ; 
                        while (m<= end_row){
                            while (n <= end_col){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                                if ( buf[m][n] == NULL){
                                    if ( m +1 < end_row){
                                    m++ ; 
                                    n = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    n++ ; 
                                }
                            }
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain  ); 
                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                }
                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                if (buf[i][j+1] != NULL) {
                                    j++ ;
                                }
                                else if (buf[i][j+1] == NULL) {
                                    if (buf[i+1][0] != NULL) {
                                        i++ ;
                                        j = 0 ;
                                    }
                                    else {
                                        break ;
                                    }
                                } 
                                just_there->as = strdup(buf[i][j] )  ; 
                            }   
                            else { 
                                check = 1 ; 
                                return NULL ; 
                            }
                        }
                        if (just_there != NULL  ){
                              temp->children[temp->num++]  = just_there ; 
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        while ( strcmp(buf[i][j] , ")") != 0 ){
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    if (buf[i][j+1] != NULL ){
                                        j++ ; 
                                    }   
                                        else {
                                            break ;
                                        }
                                }
                               if ( priority(buf[i][j]) != 0 ){
                                int temp_row = i ;
                                int temp_col = j ;
                                int depth = 0 ;

                                while (i <= end_row ){
                                    while (j <= end_col ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+1 <= end_row ){ 
                                            i++ ;
                                            j = 0 ;
                                             continue ; }
                                        else { 
                                            break ;
                                         }
                                    }

                                    if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                        break ;
                                    }

                                    if ( strcmp(buf[i][j], "(") == 0 ){
                                        depth++ ;
                                    }
                                    else if ( strcmp(buf[i][j], ")") == 0 ){
                                        if ( depth == 0 ){
                                            break ;  
                                        }
                                        depth-- ;
                                    }
                                    if ( buf[i][j] == NULL){
                                        if ( i +1 < end_row){
                                        i++ ; 
                                        j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                    }
                                    else { 
                                        j++ ; 
                                    }   
                                    if ( i > end_row ){
                                        break ;   
                                    }
                                }   
                                }

                                tree * just_there = expression( buf , temp , check , temp_row , temp_col , i , j ) ;
                                if ( just_there != NULL ){
                                    temp->children[temp->num++] = just_there ;
                                }
                            }
                                else  {
                                    if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if (  buf[i][j] == NULL  ) {
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } 
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                                continue ;
                                        }
                                        else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                            int temp_row = i ; 
                                            int temp_col = j ; 
                                            while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                if ( buf[i][j] == NULL ){
                                                    if ( i+ 1 <= end_row){
                                                    i++ ; 
                                                    j = 0 ; 
                                                }
                                                else { 
                                                    break ; 
                                                }
                                                }
                                                else {
                                                    j++ ;
                                                }
                                            }
                                            tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                            if (just_there != NULL ){
                                            temp->children[temp->num++ ]  = just_there ; 
                                            }
                                        }
                                        else {
                                            tree * leaf_node = make_leaf( buf[i][j]  , i , j ) ;
                                            if ( ( j+1 <= end_col && buf[i][j+1] != NULL && strcmp(buf[i][j+1], "AS") == 0 ) ||
                                                 ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL && strcmp(buf[i+1][0], "AS") == 0 ) ){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else {
                                                    i++ ;
                                                    j = 0 ;
                                                }
                                                if ( ( j+1 <= end_col && buf[i][j+1] != NULL ) ||
                                                     ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL ) ){
                                                    if (buf[i][j+1] != NULL) {
                                                        j++ ;
                                                    }
                                                    else {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    leaf_node->as = strdup( buf[i][j] ) ;
                                                }
                                            }
                                            temp->children[temp->num++] = leaf_node ;
                                        }
                                    }
                                    else { 
                                        check = 1 ; 
                                            return NULL ;   
                                    }
                            } 
                        }
                    }
            }




              else { 
                 if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                        temp->children[temp->num++] = make_leaf("*", i, j) ;
                    }
                    else if ( priority(buf[i][j]) != 0  ){
                        int temp_row = i ; 
                        int temp_col = j ; 
                        while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                            if ( buf[i][j] == NULL ){
                                if ( i+ 1 <= end_row){
                                i++ ; 
                                j = 0 ; 
                            }
                            else { 
                                break ; 
                            }
                            }
                            else {
                                j++ ;
                            }
                        }
                        tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                        if (just_there != NULL ){
                        temp->children[temp->num++ ]  = just_there ; 
                        }
                    }         
                    else {
                        if (if_syntax(buf[i][j]) == false ){
                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                    if (  buf[i][j] == NULL  ) {
                                         if ( buf[i+1][0] != NULL ){
                                        i++ ;
                                        j = 0 ;
                                    } 
                                }
                                else { 
                                    j++ ; 
                                }
                                        continue ;
                                }
                                 if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                    int temp_row = i ; 
                                    int temp_col = j ; 
                                    while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                        if ( buf[i][j] == NULL ){
                                            if ( i+ 1 <= end_row){
                                            i++ ; 
                                            j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                        }
                                        else {
                                            j++ ;
                                        }
                                    }
                                    tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                    if (just_there != NULL ){
                                    temp->children[temp->num++ ]  = just_there ; 
                                    }
                                }
                                else {
                                       tree * leaf_node = make_leaf( buf[i][j]  , i , j ) ;
                                       if ( ( j+1 <= end_col && buf[i][j+1] != NULL && strcmp(buf[i][j+1], "AS") == 0 ) ||
                                            ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL && strcmp(buf[i+1][0], "AS") == 0 ) ){
                                           if (buf[i][j+1] != NULL) {
                                               j++ ;
                                           }
                                           else {
                                               i++ ;
                                               j = 0 ;
                                           }
                                           if ( ( j+1 <= end_col && buf[i][j+1] != NULL ) ||
                                                ( buf[i][j+1] == NULL && i+1 <= end_row && buf[i+1][0] != NULL ) ){
                                               if (buf[i][j+1] != NULL) {
                                                   j++ ;
                                               }
                                               else {
                                                   i++ ;
                                                   j = 0 ;
                                               }
                                               leaf_node->as = strdup( buf[i][j] ) ;
                                           }
                                       }
                                       temp->children[temp->num++] = leaf_node ;
                                }
                            }
                            else { 
                                check = 1 ; 
                                return NULL ;   
                            }
                    }                    
            }
        }


             if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }

            }
            break;
    }
    return start  ; 

}



tree *comp_3(char*** buf , tree * node  ,  int i , int j , int end_row , int end_col  , int pain  , int having ){
    int start_row = i ; 
    int start_col = j ; 
    tree * start = node ; 
    tree * temp  = node ; 
    int check = 0 ; 
    while ( i <= end_row){
       while ( j <= end_col ) {



                if (  strcmp(buf[i][j], "JOIN") == 0  || 
                            ( j+1 <= end_col && strcmp(buf[i][j], "INNER") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) || 
                            (j+1 <= end_col && strcmp(buf[i][j], "LEFT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                            (j+1 <= end_col && strcmp(buf[i][j], "RIGHT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                            (j+1 <= end_col && strcmp(buf[i][j], "CROSS") == 0 && strcmp(buf[i][j+1], "JOIN") == 0  ) ||  
                            (j+2 <= end_col && strcmp(buf[i][j], "FULL") == 0 && strcmp(buf[i][j+1], "OUTER") == 0 && strcmp(buf[i][j+2], "JOIN") == 0 )){

                        int on = 0 ; 
                        tree* starter  ; 

                        if (strcmp(buf[i][j], "INNER") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                            starter = createNode("INNER JOIN");
                            j = j + 2;
                        }
                        else if (strcmp(buf[i][j], "JOIN") == 0 ){
                            starter = createNode(buf[i][j]);
                            j++ ; 
                        }
                        else if (strcmp(buf[i][j], "LEFT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                            starter = createNode("LEFT JOIN");
                            j = j + 2;
                        }
                        else if (strcmp(buf[i][j], "RIGHT") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                            starter =  createNode("RIGHT JOIN");
                            j = j + 2;
                        }
                        else if (strcmp(buf[i][j], "CROSS") == 0 && strcmp(buf[i][j+1], "JOIN") == 0 ){
                            starter =  createNode("CROSS JOIN");
                            j = j + 2;
                        }
                        else if (strcmp(buf[i][j], "FULL") == 0 && strcmp(buf[i][j+1], "OUTER") == 0 && strcmp(buf[i][j+2], "JOIN") == 0 ){
                        starter = createNode("FULL OUTER JOIN");
                            j = j + 3;
                        }
                        else {
                            check = 1 ; 
                            return NULL  ; 
                        }

                        tree *temp_word = starter ; 

                        while ( strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 && strcmp(buf[i][j] , "WHERE") != 0   && strcmp(buf[i][j], "GROUP") != 0 &&  strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "FETCH") != 0 &&  strcmp(buf[i][j], "LIMIT") != 0 &&strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "UNION") != 0 && strcmp(buf[i][j], "INTERSECT") != 0 && strcmp(buf[i][j], "EXCEPT") != 0  && strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 && buf[i][j] != NULL){

                                if ( if_function( buf[i][j] ) == true ){
                                    int start_row = i ; 
                                    int start_col = j ; 
                                    int commas = 0 ; 
                                    bool present = false ; 
                                    while (i<=end_row){
                                    while (j <= end_col){
                                        if ( strcmp(buf[i][j] , "(") == 0  ){
                                            commas++ ;
                                            present = true ; 
                                        }
                                        else if ( strcmp(buf[i][j] , ")") == 0 ){
                                            commas-- ; 
                                        }
                                        if (present && commas == 0 ){
                                            break ; 
                                        }
                                        if ( i >= end_row && j > end_col ){ 
                                            break ; 
                                        }
                                        if ( buf[i][j] == NULL){
                                            if ( i +1 < end_row){
                                            i++ ; 
                                            j = 0 ; 
                                            }
                                            else { 
                                                break ; 
                                            }
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                    }
                                    if ( present &&  commas == 0 ){
                                        break ; 
                                    }                               
                                    if ( i >= end_row && j > end_col ){ 
                                            break ; 
                                        }


                                }
                                temp_word->children[temp_word->num++] = function( buf , temp_word,check ,   start_row ,  start_col , i , j ) ;
                            }
                                

                            else  if ( strcmp(buf[i][j] , "(") == 0  ){
                                    if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                                        int end_col_dec  = j+1 ; 
                                        int end_row_dec  = i ; 
                                        int braces = 1 ; 
                                        int m = i ; 
                                        int n = j+1 ; 
                                        while (m<= end_row){
                                            while (n <= end_col){
                                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                                    braces++ ; 
                                                }
                                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                                    braces-- ; 
                                                }
                                                if ( braces == 0 ){
                                                    end_row_dec = m ; 
                                                    end_col_dec = n ; 
                                                    break ; 
                                                }
                                                if ( buf[m][n] == NULL){
                                                    if ( m +1 < end_row){
                                                    m++ ; 
                                                    n = 0 ; 
                                                    }
                                                    else { 
                                                        break ; 
                                                    }
                                                }
                                                else { 
                                                    n++ ; 
                                                }
                                            }
                                                if ( braces == 0 ){
                                                    break ; 
                                                }

                                        }
                                        if ( braces != 0 ){
                                            check = 1 ; 
                                            return NULL  ; 
                                        }
                                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec , pain  ); 
                                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                }
                                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                } 
                                                just_there->as = strdup(buf[i][j] )  ; 
                                            }   
                                            else { 
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                        }
                                        if (just_there != NULL  ){
                                            temp_word->children[temp_word->num++]  = just_there ; 
                                        }
                                        i = end_row_dec ; 
                                        j = end_col_dec ; 
                                    }
                                    else {
                                        while ( strcmp(buf[i][j] , ")") != 0 ){
                                                if ( buf[i][j] == NULL){
                                                    if ( i +1 < end_row){
                                                    i++ ; 
                                                    j = 0 ; 
                                                    }
                                                    else { 
                                                        break ; 
                                                    }
                                                }
                                                else { 
                                                    if (buf[i][j+1] != NULL ){
                                                        j++ ; 
                                                    }   
                                                    else {
                                                            break ;
                                                        }
                                                }
                                                if (buf[i][j] != NULL && strcmp(buf[i][j] , "ON") == 0   ){
                                                if (strcmp(starter->comp, "CROSS MODEL") == 0) {
                                                        check = 1;
                                                        return NULL;
                                                    }
                                                    j++;
                                                    continue;
                                            }
                                            if ( priority(buf[i][j]) != 0 ){
                                                int temp_row = i ;
                                                int temp_col = j ;
                                                int depth = 0 ;

                                                while (i <= end_row ){
                                                    while (j <= end_col ){
                                                    if ( buf[i][j] == NULL ){
                                                        if ( i+1 <= end_row ){ 
                                                            i++ ;
                                                            j = 0 ;
                                                            continue ; }
                                                        else { 
                                                            break ;
                                                        }
                                                    }

                                                    if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                                        break ;
                                                    }

                                                    if ( strcmp(buf[i][j], "(") == 0 ){
                                                        depth++ ;
                                                    }
                                                    else if ( strcmp(buf[i][j], ")") == 0 ){
                                                        if ( depth == 0 ){
                                                            break ;  
                                                        }
                                                        depth-- ;
                                                    }
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        j++ ; 
                                                    }   
                                                    if ( i > end_row ){
                                                        break ;   
                                                    }
                                                }   
                                                }

                                                tree * just_there = expression( buf , temp_word , check , temp_row , temp_col , i , j ) ;
                                                if ( just_there != NULL ){
                                                    temp_word->children[temp_word->num++] = just_there ;
                                                }
                                            }
                                                else  {
                                                    if (if_syntax(buf[i][j]) == false ){
                                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                                            if (  buf[i][j+1] == NULL  ) {
                                                                if ( buf[i+1][0] != NULL ){
                                                                i++ ;
                                                                j = 0 ;
                                                            } 
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }
                                                                continue ;
                                                        }
                                                        else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                            int temp_row = i ; 
                                                            int temp_col = j ; 
                                                            while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                if ( buf[i][j] == NULL ){
                                                                    if ( i+ 1 <= end_row){
                                                                    i++ ; 
                                                                    j = 0 ; 
                                                                }
                                                                else { 
                                                                    break ; 
                                                                }
                                                                }
                                                                else {
                                                                    j++ ;
                                                                }
                                                            }
                                                            tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                            if (just_there != NULL ){
                                                            temp_word->children[temp_word->num++ ]  = just_there ; 
                                                            }
                                                        }
                                                        else {
                                                            temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                        }
                                                    }
                                                    else { 
                                                        check = 1 ; 
                                                            return NULL ;   
                                                    }
                                            } 
                                        }
                                    }
                            }




                            else { 
                                    if (buf[i][j] != NULL && strcmp(buf[i][j] , "ON") == 0   ){
                                        if (strcmp(starter->comp, "CROSS MODEL") == 0) {
                                                check = 1;
                                                return NULL;
                                            }
                                            j++;
                                            continue;
                                    }
                                    if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                                            temp->children[temp->num++] = make_leaf("*", i, j) ;
                                        }
                                else  if ( priority(buf[i][j]) != 0  ){
                                        int temp_row = i ; 
                                        int temp_col = j ; 
                                        while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                            if ( buf[i][j] == NULL ){
                                                if ( i+ 1 <= end_row){
                                                i++ ; 
                                                j = 0 ; 
                                            }
                                            else { 
                                                break ; 
                                            }
                                            }
                                            else {
                                                j++ ;
                                            }
                                        }
                                        tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                        if (just_there != NULL ){
                                        temp_word->children[temp_word->num++ ]  = just_there ; 
                                        }
                                    }         
                                    else {
                                        if (if_syntax(buf[i][j]) == false ){
                                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                                    if (  buf[i][j+1] == NULL  ) {
                                                        if ( buf[i+1][0] != NULL ){
                                                        i++ ;
                                                        j = 0 ;
                                                    } 
                                                }
                                                else { 
                                                    j++ ; 
                                                }
                                                        continue ;
                                                }
                                                if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                    int temp_row = i ; 
                                                    int temp_col = j ; 
                                                    while ( strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                        if ( buf[i][j] == NULL ){
                                                            if ( i+ 1 <= end_row){
                                                            i++ ; 
                                                            j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                        }
                                                        else {
                                                            j++ ;
                                                        }
                                                    }
                                                    tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                    if (just_there != NULL ){
                                                    temp_word->children[temp_word->num++ ]  = just_there ; 
                                                    }
                                                }
                                                else {
                                                    temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                }
                                            }
                                            else { 
                                                check = 1 ; 
                                                return NULL ;   
                                            }
                                    }                    
                            }

                            if (buf[i][j+1] != NULL) {
                                j++ ;
                            }
                            else {
                                if (buf[i+1][0] != NULL) {
                                    i++ ;
                                    j = 0 ;
                                }
                                else {
                                    break ;
                                }
                            }

                        }


                        if ( j-1 >= 0 ){
                            j = j-1 ; 
                        }
                        else {
                            int temp_row ; 
                            int temp_col ; 
                            if ( i- 1 > start_row ){
                                temp_row = i -1 ; 
                                temp_col = start_col ; 
                                for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                                }
                                if (temp_col > 0 ){
                                    j = temp_col -1 ; 
                                }
                                else { 
                                    j = 0 ; 
                                }
                                i = temp_row ; 
                            }
                            

                                else {
                                i = start_row ; 
                                j = start_col ; 
                            }
                        }


                        if ( pain == 0 ){
                            temp->children[temp->num++] = starter ; 
                        }
                        else { 
                            return starter ; 
                        }

                }


                else if (strcmp( buf[i][j] , "ORDER") == 0 ){
                            if (buf[i][j+1] != NULL ){
                                if (strcmp( buf[i][j+1] , "BY") == 0 ){
                                    j++ ; 
                                    tree * temp_word = createNode("ORDER BY") ; 
                                    tree *starter = temp_word ; 



                                    while ( strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 && strcmp(buf[i][j], "NULLS") != 0 &&  strcmp(buf[i][j], "ASC") != 0 && strcmp(buf[i][j], "DSC") != 0 && strcmp(buf[i][j], ",") != 0 &&  strcmp(buf[i][j], "FIRST") != 0 && strcmp(buf[i][j], "LAST") != 0 &&   strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "FETCH") != 0  &&  strcmp(buf[i][j], "LIMIT") != 0 &&strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "UNION") != 0 && strcmp(buf[i][j], "INTERSECT") != 0 && strcmp(buf[i][j], "EXCEPT") != 0 ){
                                            if ( if_function( buf[i][j] ) == true ){
                                                int start_row = i ; 
                                                int start_col = j ; 
                                                int commas = 0 ; 
                                                bool present = false ; 
                                                while (i<=end_row){
                                                while (j <= end_col){
                                                    if ( strcmp(buf[i][j] , "(") == 0  ){
                                                        commas++ ;
                                                        present = true ; 
                                                    }
                                                    else if ( strcmp(buf[i][j] , ")") == 0 ){
                                                        commas-- ; 
                                                    }
                                                    if (present && commas == 0 ){
                                                        break ; 
                                                    }
                                                    if ( i >= end_row && j > end_col ){ 
                                                        break ; 
                                                    }
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        j++ ; 
                                                    }
                                                }
                                                if ( present &&  commas == 0 ){
                                                    break ; 
                                                }                               
                                                if ( i >= end_row && j > end_col ){ 
                                                        break ; 
                                                    }


                                            }
                                            temp_word->children[temp_word->num++] = function( buf , temp_word,check ,   start_row ,  start_col , i , j ) ;
                                        }


                                else  if ( strcmp(buf[i][j] , "(") == 0  ){
                                        if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                                            int end_col_dec  = j+1 ; 
                                            int end_row_dec  = i ; 
                                            int braces = 1 ; 
                                            int m = i ; 
                                            int n = j+1 ; 
                                            while (m<= end_row){
                                                while (n <= end_col){
                                                    if ( strcmp(buf[m][n] , "(") == 0  ){
                                                        braces++ ; 
                                                    }
                                                    else if ( strcmp(buf[m][n] , ")") == 0 ){
                                                        braces-- ; 
                                                    }
                                                    if ( braces == 0 ){
                                                        end_row_dec = m ; 
                                                        end_col_dec = n ; 
                                                        break ; 
                                                    }
                                                    if ( buf[m][n] == NULL){
                                                        if ( m +1 < end_row){
                                                        m++ ; 
                                                        n = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        n++ ; 
                                                    }
                                                }
                                                    if ( braces == 0 ){
                                                        break ; 
                                                    }

                                            }
                                            if (braces != 0 ){
                                                check = 1 ; 
                                                return NULL 
                                            }
                                            tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain  ); 
                                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                }
                                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                } 
                                                just_there->as = strdup(buf[i][j] )  ; 
                                            }   
                                            else { 
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                        }                                            
                                            if (just_there != NULL  ){
                                                temp_word->children[temp_word->num++]  = just_there ; 
                                            }
                                            i = end_row_dec ; 
                                            j = end_col_dec ; 
                                        }
                                        else {
                                            while ( strcmp(buf[i][j] , ")") != 0 ){
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        if (buf[i][j+1] != NULL ){
                                                            j++ ; 
                                                        }   
                                                        else {
                                                                break ;
                                                            }
                                                    }
                                                    if (buf[i][j] != NULL && strcmp(buf[i][j] , "ON") == 0   ){
                                                    if (strcmp(starter->comp, "CROSS MODEL") == 0) {
                                                            check = 1;
                                                            return NULL;
                                                        }
                                                        j++;
                                                        continue;
                                                }
                                                if ( priority(buf[i][j]) != 0 ){
                                                    int temp_row = i ;
                                                    int temp_col = j ;
                                                    int depth = 0 ;

                                                    while (i <= end_row ){
                                                        while (j <= end_col ){
                                                        if ( buf[i][j] == NULL ){
                                                            if ( i+1 <= end_row ){ 
                                                                i++ ;
                                                                j = 0 ;
                                                                continue ; }
                                                            else { 
                                                                break ;
                                                            }
                                                        }

                                                        if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "NULLS") != 0 ||  strcmp(buf[i][j], "ASC") != 0 ||  strcmp(buf[i][j], "DSC") != 0 ||  strcmp(buf[i][j], ",") != 0 ||   strcmp(buf[i][j], "FIRST") != 0 ||  strcmp(buf[i][j], "LAST") != 0 ||  strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                                            break ;
                                                        }

                                                        if ( strcmp(buf[i][j], "(") == 0 ){
                                                            depth++ ;
                                                        }
                                                        else if ( strcmp(buf[i][j], ")") == 0 ){
                                                            if ( depth == 0 ){
                                                                break ;  
                                                            }
                                                            depth-- ;
                                                        }
                                                        if ( buf[i][j] == NULL){
                                                            if ( i +1 < end_row){
                                                            i++ ; 
                                                            j = 0 ; 
                                                            }
                                                            else { 
                                                                break ; 
                                                            }
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }   
                                                        if ( i > end_row ){
                                                            break ;   
                                                        }
                                                    }   
                                                    }

                                                    tree * just_there = expression( buf , temp_word , check , temp_row , temp_col , i , j ) ;
                                                    if ( just_there != NULL ){
                                                        temp_word->children[temp_word->num++] = just_there ;
                                                    }
                                                }
                                                    else  {
                                                        if (if_syntax(buf[i][j]) == false ){
                                                            if ( strcmp(buf[i][j] , ",") == 0 ){
                                                                if (  buf[i][j+1] == NULL  ) {
                                                                    if ( buf[i+1][0] != NULL ){
                                                                    i++ ;
                                                                    j = 0 ;
                                                                } 
                                                            }
                                                            else { 
                                                                j++ ; 
                                                            }
                                                                    continue ;
                                                            }
                                                            else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                                int temp_row = i ; 
                                                                int temp_col = j ; 
                                                                while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                    if ( buf[i][j] == NULL ){
                                                                        if ( i+ 1 <= end_row){
                                                                        i++ ; 
                                                                        j = 0 ; 
                                                                    }
                                                                    else { 
                                                                        break ; 
                                                                    }
                                                                    }
                                                                    else {
                                                                        j++ ;
                                                                    }
                                                                }
                                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                                if (just_there != NULL ){
                                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                                }
                                                            }
                                                            else {
                                                                temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                            }
                                                        }
                                                        else { 
                                                            check = 1 ; 
                                                                return NULL ;   
                                                        }
                                                } 
                                            }
                                        }
                                }



                                    else { 
                                                if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                                                    temp_word->children[temp_word->num++] = make_leaf("*", i, j) ;
                                                }
                                            else if ( priority(buf[i][j]) != 0  ){
                                                int temp_row = i ; 
                                                int temp_col = j ; 
                                                while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                    if ( buf[i][j] == NULL ){
                                                        if ( i+ 1 <= end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                    }
                                                    else { 
                                                        break ; 
                                                    }
                                                    }
                                                    else {
                                                        j++ ;
                                                    }
                                                }
                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                if (just_there != NULL ){
                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                }
                                            }         
                                            else {
                                                if (if_syntax(buf[i][j]) == false ){
                                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                                            if (  buf[i][j+1] == NULL  ) {
                                                                if ( buf[i+1][0] != NULL ){
                                                                i++ ;
                                                                j = 0 ;
                                                            } 
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }
                                                            temp_word->direction[temp_word->num - 1 ] = "ASC" ; 
                                                            continue ;
                                                        }
                                                        if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                            int temp_row = i ; 
                                                            int temp_col = j ; 
                                                            while ( strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                if ( buf[i][j] == NULL ){
                                                                    if ( i+ 1 <= end_row){
                                                                    i++ ; 
                                                                    j = 0 ; 
                                                                }
                                                                else { 
                                                                    break ; 
                                                                }
                                                                }
                                                                else {
                                                                    j++ ;
                                                                }
                                                            }
                                                            tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                            if (just_there != NULL ){
                                                            temp_word->children[temp_word->num++ ]  = just_there ; 
                                                            }
                                                        }
                                                        else {
                                                            temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                        }
                                                    }
                                                    else if ( strcmp(buf[i][j]  , "ASC"  ) == 0 ||  strcmp(buf[i][j]  , "DSC"  ) == 0 || strcmp(buf[i][j]  , "NULLS"  ) == 0   ){
                                                            int dir = 0 ; 
                                                            int first = 0 ; 
                                                            tree * cat  ; 
                                                            tree * startcat ; 
                                                            while (  strcmp(buf[i][j] , ",") != 0 &&  strcmp(buf[i][j] , ";") != 0   ){
                                                                if (dir == 0 ) {
                                                                if (strcmp(buf[i][j]  , "NULLS"  ) == 0 || strcmp(buf[i][j]  , "ASC"  ) == 0  ){
                                                                    temp_word->direction[temp_word->num -1 ] = "ASC" ; 
                                                                }
                                                                else if ( strcmp(buf[i][j]  , "DSC"  ) == 0 ){
                                                                    temp_word->direction[temp_word->num - 1 ] = "DSC" ; 
                                                                }
                                                                dir = 1 ; 
                                                                }
                                                                if (  strcmp(buf[i][j]  , "NULLS"  ) == 0 ||  strcmp(buf[i][j]  , "LAST"  ) == 0  ||  strcmp(buf[i][j]  , "FIRST"  ) == 0 ){
                                                                    if ( first == 0  && strcmp(buf[i][j]  , "NULLS"  ) == 0  ){
                                                                        cat = createNode(buf[i][j] ); 
                                                                        startcat = cat ; 
                                                                        first = 1 ; 
                                                                    }
                                                                    else {
                                                                        cat->children[cat->num++ ]  = createNode(buf[i][j] ); 
                                                                        cat = cat->children[cat->num - 1 ] ;  
                                                                    }
                                                                }
                                                                if ( buf[i][j] == NULL ){
                                                                    if ( i+ 1 <= end_row){
                                                                    i++ ; 
                                                                    j = 0 ; 
                                                                }
                                                                else { 
                                                                    break ; 
                                                                }
                                                                }
                                                                else {
                                                                    j++ ;
                                                                }
                                                            }
                                                            if (startcat != NULL ){
                                                                temp_word->children[temp_word->num++ ] = startcat ; 
                                                            }
                                                    }
                                                    else { 
                                                        check = 1 ; 
                                                        return NULL ;   
                                                    }
                                            }                    
                                    }

                                    if (buf[i][j+1] != NULL) {
                                        j++ ;
                                    }
                                    else {
                                        if (buf[i+1][0] != NULL) {
                                            i++ ;
                                            j = 0 ;
                                        }
                                        else {
                                            break ;
                                        }
                                    }

                                }


                                if ( j-1 >= 0 ){
                                    j = j-1 ; 
                                }
                                    else {
                                    int temp_row ; 
                                    int temp_col ; 
                                    if ( i- 1 > start_row ){
                                        temp_row = i -1 ; 
                                        temp_col = start_col ; 
                                        for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                                        }
                                        if (temp_col > 0 ){
                                            j = temp_col -1 ; 
                                        }
                                        else { 
                                            j = 0 ; 
                                        }
                                        i = temp_row ; 
                                    }
                                    

                                        else {
                                        i = start_row ; 
                                        j = start_col ; 
                                    }
                                }

                                if ( pain == 0 ){
                                    temp->children[temp->num++] = starter ; 
                                }
                                else { 
                                    return starter ; 
                                }

                                }
                                }
                        else {
                        check = 1 ; 
                        return NULL  ; 
                        }
                }

            
                else if ( strcmp(buf[i][j] , "CASE" ) == 0 ){
                    int cases = 1 ; 
                    int start_row = i ; 
                    int start_col = j ; 
                        if (buf[i][j+1] != NULL) {
                        j++ ;
                    }
                    else {
                        if (buf[i+1][0] != NULL) {
                            i++ ;
                            j = 0 ;
                        }
                        else {
                            break ;
                        }
                    }

                    while ( i <= end_row ){
                        while ( j <= end_col){
                        if (strcmp(buf[i][j] , "CASE" ) == 0){
                            cases++ ; 
                        }
                        else if (  strcmp(buf[i][j] , "END" ) == 0  ){
                            if ( cases > 0 ){
                                    cases-- ; 
                                    if ( cases == 0 ){
                                    break ; 
                                    }
                            }
                            else { 
                                break ; 
                            }
                        }
                        if ( buf[i][j] == NULL){
                            if ( i + 1 <=  end_row){
                                i = i +1 ; 
                                j = 0 ; 
                            }
                        }
                        else { 
                            j++ ; 
                        }
                    }
                    if (cases == 0 ){
                        break ; 
                    }
                    }
                    temp->children[temp->num++] = case_expr(buf  ,check , start_row , start_col , i , j ) ; 
                }


            
 
                else if (strcmp( buf[i][j] , "GROUP") == 0 ){
                            if (buf[i][j+1] != NULL ){
                                if (strcmp( buf[i][j+1] , "BY") == 0 ){
                                    j++ ; 
                                    tree * temp_word = createNode("GROUP BY") ; 
                                    tree *starter = temp_word ; 


                                    while ( strcmp(buf[i][j], ")") != 0 &&  strcmp(buf[i][j], ";") != 0 &&  strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "FETCH") != 0  &&  strcmp(buf[i][j], "LIMIT") != 0 &&strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "UNION") != 0 && strcmp(buf[i][j], "INTERSECT") != 0 && strcmp(buf[i][j], "EXCEPT") != 0 ){
                                            if ( if_function( buf[i][j] ) == true ){
                                                int start_row = i ; 
                                                int start_col = j ; 
                                                int commas = 0 ; 
                                                bool present = false ; 
                                                while (i<=end_row){
                                                while (j <= end_col){
                                                    if ( strcmp(buf[i][j] , "(") == 0  ){
                                                        commas++ ;
                                                        present = true ; 
                                                    }
                                                    else if ( strcmp(buf[i][j] , ")") == 0 ){
                                                        commas-- ; 
                                                    }
                                                    if (present && commas == 0 ){
                                                        break ; 
                                                    }
                                                    if ( i >= end_row && j > end_col ){ 
                                                        break ; 
                                                    }
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        j++ ; 
                                                    }
                                                }
                                                if ( present &&  commas == 0 ){
                                                    break ; 
                                                }                               
                                                if ( i >= end_row && j > end_col ){ 
                                                        break ; 
                                                    }


                                            }
                                            temp_word->children[temp_word->num++] = function( buf , temp_word,check ,   start_row ,  start_col , i , j ) ;
                                        }


                                else if  (strcmp(buf[i][j]  , "HAVING") == 0 ){
                                    int temp_start_row = i ; 
                                    int temp_start_col = j ; 
                                    while ( strcmp(buf[i][j], ")") != 0 && strcmp(buf[i][j], ";") != 0  && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "UNION") != 0 && strcmp(buf[i][j], "INTERSECT")!= 0 && strcmp(buf[i][j], "EXCEPT") != 0 ) { 
                                            
                                            if ( buf[i][j] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                                else {
                                                    break ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }
                                    }
                                    tree * cat = createNode("HAVING") ; 
                                    tree * just_there  =   comp_3( buf ,  cat , temp_start_row, temp_start_col,  i ,  j ,  pain  , 1 ) ;
                                    if (just_there != NULL  ){
                                        temp_word->children[temp_word->num++]  = just_there ; 
                                    }
                                    continue ; 
                                }


                                else  if ( strcmp(buf[i][j] , "(") == 0  ){
                                        if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                                            int end_col_dec  = j+1  ; 
                                            int end_row_dec  = i ; 
                                            int braces = 1 ; 
                                            int m = i ; 
                                            int n = j+1 ; 
                                            while (m<= end_row){
                                                while (n <= end_col){
                                                    if ( strcmp(buf[m][n] , "(") == 0  ){
                                                        braces++ ; 
                                                    }
                                                    else if ( strcmp(buf[m][n] , ")") == 0 ){
                                                        braces-- ; 
                                                    }
                                                    if ( braces == 0 ){
                                                        end_row_dec = m ; 
                                                        end_col_dec = n ; 
                                                        break ; 
                                                    }
                                                    if ( buf[m][n] == NULL){
                                                        if ( m +1 < end_row){
                                                        m++ ; 
                                                        n = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        n++ ; 
                                                    }
                                                }
                                                    if ( braces == 0 ){
                                                        break ; 
                                                    }

                                            }
                                            if ( braces != 0 ){
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                            tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain  ); 
                                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                }
                                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                } 
                                                just_there->as = strdup(buf[i][j] )  ; 
                                            }   
                                            else { 
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                        }                                            
                                            if (just_there != NULL  ){
                                                temp_word->children[temp_word->num++]  = just_there ; 
                                            }
                                            i = end_row_dec ; 
                                            j = end_col_dec ; 
                                        }
                                        else {
                                            while ( strcmp(buf[i][j] , ")") != 0 ){
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        if (buf[i][j+1] != NULL ){
                                                            j++ ; 
                                                        }   
                                                        else {
                                                                break ;
                                                            }
                                                    }

                                                if ( priority(buf[i][j]) != 0 ){
                                                    int temp_row = i ;
                                                    int temp_col = j ;
                                                    int depth = 0 ;

                                                    while (i <= end_row ){
                                                        while (j <= end_col ){
                                                        if ( buf[i][j] == NULL ){
                                                            if ( i+1 <= end_row ){ 
                                                                i++ ;
                                                                j = 0 ;
                                                                continue ; }
                                                            else { 
                                                                break ;
                                                            }
                                                        }

                                                        if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                                            break ;
                                                        }

                                                        if ( strcmp(buf[i][j], "(") == 0 ){
                                                            depth++ ;
                                                        }
                                                        else if ( strcmp(buf[i][j], ")") == 0 ){
                                                            if ( depth == 0 ){
                                                                break ;  
                                                            }
                                                            depth-- ;
                                                        }
                                                        if ( buf[i][j] == NULL){
                                                            if ( i +1 < end_row){
                                                            i++ ; 
                                                            j = 0 ; 
                                                            }
                                                            else { 
                                                                break ; 
                                                            }
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }   
                                                        if ( i > end_row ){
                                                            break ;   
                                                        }
                                                    }   
                                                    }

                                                    tree * just_there = expression( buf , temp_word , check , temp_row , temp_col , i , j ) ;
                                                    if ( just_there != NULL ){
                                                        temp_word->children[temp_word->num++] = just_there ;
                                                    }
                                                }
                                                    else  {
                                                        if (if_syntax(buf[i][j]) == false ){
                                                            if ( strcmp(buf[i][j] , ",") == 0 ){
                                                                if (  buf[i][j+1] == NULL  ) {
                                                                    if ( buf[i+1][0] != NULL ){
                                                                    i++ ;
                                                                    j = 0 ;
                                                                } 
                                                            }
                                                            else { 
                                                                j++ ; 
                                                            }
                                                                    continue ;
                                                            }
                                                            else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                                int temp_row = i ; 
                                                                int temp_col = j ; 
                                                                while (  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                    if ( buf[i][j] == NULL ){
                                                                        if ( i+ 1 <= end_row){
                                                                        i++ ; 
                                                                        j = 0 ; 
                                                                    }
                                                                    else { 
                                                                        break ; 
                                                                    }
                                                                    }
                                                                    else {
                                                                        j++ ;
                                                                    }
                                                                }
                                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                                if (just_there != NULL ){
                                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                                }
                                                            }
                                                            else {
                                                                temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                            }
                                                        }
                                                        else { 
                                                            check = 1 ; 
                                                                return NULL ;   
                                                        }
                                                } 
                                            }
                                        }
                                }



                                    else { 
                                                if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                                                    temp_word->children[temp_word->num++] = make_leaf("*", i, j) ;
                                                }
                                            else if ( priority(buf[i][j]) != 0  ){
                                                int temp_row = i ; 
                                                int temp_col = j ; 
                                                while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                    if ( buf[i][j] == NULL ){
                                                        if ( i+ 1 <= end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                    }
                                                    else { 
                                                        break ; 
                                                    }
                                                    }
                                                    else {
                                                        j++ ;
                                                    }
                                                }
                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                if (just_there != NULL ){
                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                }
                                            }         
                                            else {
                                                if (if_syntax(buf[i][j]) == false ){
                                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                                            if (  buf[i][j+1] == NULL  ) {
                                                                if ( buf[i+1][0] != NULL ){
                                                                i++ ;
                                                                j = 0 ;
                                                            } 
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }
                                                            continue ;
                                                        }
                                                        if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                            int temp_row = i ; 
                                                            int temp_col = j ; 
                                                            while ( strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                if ( buf[i][j] == NULL ){
                                                                    if ( i+ 1 <= end_row){
                                                                    i++ ; 
                                                                    j = 0 ; 
                                                                }
                                                                else { 
                                                                    break ; 
                                                                }
                                                                }
                                                                else {
                                                                    j++ ;
                                                                }
                                                            }
                                                            tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                            if (just_there != NULL ){
                                                            temp_word->children[temp_word->num++ ]  = just_there ; 
                                                            }
                                                        }
                                                        else {
                                                            temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                        }
                                                    }
                    
                                                    else { 
                                                        check = 1 ; 
                                                        return NULL ;   
                                                    }
                                            }                    
                                    }

                                    if (buf[i][j+1] != NULL) {
                                        j++ ;
                                    }
                                    else {
                                        if (buf[i+1][0] != NULL) {
                                            i++ ;
                                            j = 0 ;
                                        }
                                        else {
                                            break ;
                                        }
                                    }

                                }


                                if ( j-1 >= 0 ){
                                    j = j-1 ; 
                                }
                                else {
                                    int temp_row ; 
                                    int temp_col ; 
                                    if ( i- 1 > start_row ){
                                        temp_row = i -1 ; 
                                        temp_col = start_col ; 
                                        for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                                        }
                                        if (temp_col > 0 ){
                                            j = temp_col -1 ; 
                                        }
                                        else { 
                                            j = 0 ; 
                                        }
                                        i = temp_row ; 
                                    }
                                    

                                        else {
                                        i = start_row ; 
                                        j = start_col ; 
                                    }
                                }

                                if ( pain == 0 ){
                                    temp->children[temp->num++] = starter ; 
                                }
                                else { 
                                    return starter ; 
                                }

                                }
                                        else {
                                        check = 1 ; 
                                        return NULL  ; 
                                        }
                                }
                        else {
                        check = 1 ; 
                        return NULL  ; 
                        }
                }


      


                    else if( strcmp(buf[i][j] , "LIMIT") == 0 || strcmp(buf[i][j] ,"OFFSET") == 0 ){
                                tree * temp_word ; 
                                if (strcmp(buf[i][j] ,"LIMIT") == 0 ){
                                     temp_word = createNode("LIMIT") ; 
                                }
                                else if (strcmp(buf[i][j] ,"OFFSET") == 0) {
                                    temp_word = createNode("OFFSET") ; 
                                }
                                tree *starter = temp_word ; 
                                j++ ; 

                        while ( strcmp(buf[i][j], ";") != 0  && strcmp(buf[i][j], ")") != 0 ){

                            if ( strcmp(buf[i][j] , "OFFSET") == 0 ){
                                if (strcmp(starter->comp , "LIMIT") == 0 ){
                                 if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                                continue ; 
                                }
                                else {
                                    return NULL ; 
                                }
                            }

                            else if ( strcmp(buf[i][j], "(") == 0 ){

                                        if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                                            int end_col_dec  = j+1  ; 
                                            int end_row_dec  = i ; 
                                            int braces = 1 ; 
                                            int m = i ; 
                                            int n = j+1 ; 
                                            while (m<= end_row){
                                                while (n <= end_col){
                                                    if ( strcmp(buf[m][n] , "(") == 0  ){
                                                        braces++ ; 
                                                    }
                                                    else if ( strcmp(buf[m][n] , ")") == 0 ){
                                                        braces-- ; 
                                                    }
                                                    if ( braces == 0 ){
                                                        end_row_dec = m ; 
                                                        end_col_dec = n ; 
                                                        break ; 
                                                    }
                                                    if ( buf[m][n] == NULL){
                                                        if ( m +1 < end_row){
                                                        m++ ; 
                                                        n = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        n++ ; 
                                                    }
                                                }
                                                    if ( braces == 0 ){
                                                        break ; 
                                                    }

                                            }
                                            if ( braces != 0 ){
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                            tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec  , pain  ); 
                                        if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                }
                                            if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                                if (buf[i][j+1] != NULL) {
                                                    j++ ;
                                                }
                                                else if (buf[i][j+1] == NULL) {
                                                    if (buf[i+1][0] != NULL) {
                                                        i++ ;
                                                        j = 0 ;
                                                    }
                                                    else {
                                                        break ;
                                                    }
                                                } 
                                                just_there->as = strdup(buf[i][j] )  ; 
                                            }   
                                            else { 
                                                check = 1 ; 
                                                return NULL ; 
                                            }
                                        }                                            
                                            if (just_there != NULL  ){
                                                temp_word->children[temp_word->num++]  = just_there ; 
                                            }
                                            i = end_row_dec ; 
                                            j = end_col_dec ; 
                                        }


                                                     else {
                                            while ( strcmp(buf[i][j] , ")") != 0 ){
                                                    if ( buf[i][j] == NULL){
                                                        if ( i +1 < end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                        }
                                                        else { 
                                                            break ; 
                                                        }
                                                    }
                                                    else { 
                                                        if (buf[i][j+1] != NULL ){
                                                            j++ ; 
                                                        }   
                                                        else {
                                                                break ;
                                                            }
                                                    }

                                                if ( priority(buf[i][j]) != 0 ){
                                                    int temp_row = i ;
                                                    int temp_col = j ;
                                                    int depth = 0 ;

                                                    while (i <= end_row ){
                                                        while (j <= end_col ){
                                                        if ( buf[i][j] == NULL ){
                                                            if ( i+1 <= end_row ){ 
                                                                i++ ;
                                                                j = 0 ;
                                                                continue ; }
                                                            else { 
                                                                break ;
                                                            }
                                                        }

                                                        if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                                            break ;
                                                        }

                                                        if ( strcmp(buf[i][j], "(") == 0 ){
                                                            depth++ ;
                                                        }
                                                        else if ( strcmp(buf[i][j], ")") == 0 ){
                                                            if ( depth == 0 ){
                                                                break ;  
                                                            }
                                                            depth-- ;
                                                        }
                                                        if ( buf[i][j] == NULL){
                                                            if ( i +1 < end_row){
                                                            i++ ; 
                                                            j = 0 ; 
                                                            }
                                                            else { 
                                                                break ; 
                                                            }
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }   
                                                        if ( i > end_row ){
                                                            break ;   
                                                        }
                                                    }   
                                                    }

                                                    tree * just_there = expression( buf , temp_word , check , temp_row , temp_col , i , j ) ;
                                                    if ( just_there != NULL ){
                                                        temp_word->children[temp_word->num++] = just_there ;
                                                    }
                                                }
                                                    else  {
                                                        if (if_syntax(buf[i][j]) == false ){
                                                            if ( strcmp(buf[i][j] , ",") == 0 ){
                                                                if (  buf[i][j+1] == NULL  ) {
                                                                    if ( buf[i+1][0] != NULL ){
                                                                    i++ ;
                                                                    j = 0 ;
                                                                } 
                                                            }
                                                            else { 
                                                                j++ ; 
                                                            }
                                                                    continue ;
                                                            }
                                                            else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                                int temp_row = i ; 
                                                                int temp_col = j ; 
                                                                while (  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                    if ( buf[i][j] == NULL ){
                                                                        if ( i+ 1 <= end_row){
                                                                        i++ ; 
                                                                        j = 0 ; 
                                                                    }
                                                                    else { 
                                                                        break ; 
                                                                    }
                                                                    }
                                                                    else {
                                                                        j++ ;
                                                                    }
                                                                }
                                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                                if (just_there != NULL ){
                                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                                }
                                                            }
                                                            else {
                                                                temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                            }
                                                        }
                                                        else { 
                                                            check = 1 ; 
                                                                return NULL ;   
                                                        }
                                                } 
                                            }
                                        }
                                }

                                    else { 
                                                if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                                                    temp_word->children[temp_word->num++] = make_leaf("*", i, j) ;
                                                }
                                            else if ( priority(buf[i][j]) != 0  ){
                                                int temp_row = i ; 
                                                int temp_col = j ; 
                                                while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                    if ( buf[i][j] == NULL ){
                                                        if ( i+ 1 <= end_row){
                                                        i++ ; 
                                                        j = 0 ; 
                                                    }
                                                    else { 
                                                        break ; 
                                                    }
                                                    }
                                                    else {
                                                        j++ ;
                                                    }
                                                }
                                                tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                if (just_there != NULL ){
                                                temp_word->children[temp_word->num++ ]  = just_there ; 
                                                }
                                            }         
                                            else {
                                                if (if_syntax(buf[i][j]) == false ){
                                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                                            if (  buf[i][j+1] == NULL  ) {
                                                                if ( buf[i+1][0] != NULL ){
                                                                i++ ;
                                                                j = 0 ;
                                                            } 
                                                        }
                                                        else { 
                                                            j++ ; 
                                                        }
                                                            continue ;
                                                        }
                                                        if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                                            int temp_row = i ; 
                                                            int temp_col = j ; 
                                                            while ( strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                                if ( buf[i][j] == NULL ){
                                                                    if ( i+ 1 <= end_row){
                                                                    i++ ; 
                                                                    j = 0 ; 
                                                                }
                                                                else { 
                                                                    break ; 
                                                                }
                                                                }
                                                                else {
                                                                    j++ ;
                                                                }
                                                            }
                                                            tree*  just_there =  expression(buf , temp_word , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                                            if (just_there != NULL ){
                                                            temp_word->children[temp_word->num++ ]  = just_there ; 
                                                            }
                                                        }
                                                        else {
                                                            temp_word->children[temp_word->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                                        }
                                                    }
                    
                                                    else { 
                                                        check = 1 ; 
                                                        return NULL ;   
                                                    }
                                            }                    
                                    }

                                    if (buf[i][j+1] != NULL) {
                                        j++ ;
                                    }
                                    else {
                                        if (buf[i+1][0] != NULL) {
                                            i++ ;
                                            j = 0 ;
                                        }
                                        else {
                                            break ;
                                        }
                                    }

                                }


                        if ( j-1 >= 0 ){
                            j = j-1 ; 
                        }
                        else {
                            int temp_row ; 
                            int temp_col ; 
                            if ( i- 1 > start_row ){
                                temp_row = i -1 ; 
                                temp_col = start_col ; 
                                for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                                }
                                if (temp_col > 0 ){
                                      j = temp_col -1 ; 
                                }
                                else { 
                                    j = 0 ; 
                                }
                                i = temp_row ; 
                            }
                            

                                else {
                                i = start_row ; 
                                j = start_col ; 
                            }
                        }

                        if ( pain == 0 ){
                            temp->children[temp->num++] = starter ; 
                        }
                        else { 
                            return starter ; 
                        }

                        }





      



            else { 

                if ( if_function( buf[i][j] ) == true ){
                    int start_row = i ; 
                    int start_col = j ; 
                    int commas = 0 ; 
                    bool present = false ; 
                          while (i<=end_row){
                            while (j <= end_col){
                                if ( strcmp(buf[i][j] , "(") == 0  ){
                                    commas++ ;
                                    present = true ; 
                                }
                                else if ( strcmp(buf[i][j] , ")") == 0 ){
                                    commas-- ; 
                                }
                                if ( present && commas == 0 ){
                                    break ; 
                                }
                                if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    j++ ; 
                                }
                            }
                            if ( present &&  commas == 0 ){
                                break ; 
                            }                               
                             if ( i >= end_row && j > end_col ){ 
                                    break ; 
                                }


                        }
                temp->children[temp->num++] = function( buf , temp,check ,   i ,  j , end_row , end_col ) ;
            }
                

            else  if ( strcmp(buf[i][j] , "(") == 0  ){
                    if ( j+1 <= end_col && strcmp(buf[i][j+1] , "SELECT" ) == 0  ){
                        int end_col_dec  = j+1 ; 
                        int end_row_dec  = i ; 
                        int braces = 1 ; 
                        int m = i ; 
                        int n = j+1 ; 
                        while (m<= end_row){
                            while (n <= end_col){
                                if ( strcmp(buf[m][n] , "(") == 0  ){
                                    braces++ ; 
                                }
                                else if ( strcmp(buf[m][n] , ")") == 0 ){
                                    braces-- ; 
                                }
                                if ( braces == 0 ){
                                    end_row_dec = m ; 
                                    end_col_dec = n ; 
                                    break ; 
                                }
                                if ( buf[m][n] == NULL){
                                    if ( m +1 < end_row){
                                    m++ ; 
                                    n = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    n++ ; 
                                }
                            }
                                if ( braces == 0 ){
                                    break ; 
                                }

                        }
                        tree * just_there  =  select_query( i  , j+1 , check ,  end_row_dec ,  end_col_dec , pain   ); 
                            if ( (j+1 <= end_col && strcmp(buf[i][j+1] , "AS") == 0 ) || (i+1 <= end_row && strcmp(buf[i+1][0] , "AS") == 0   )){
                                    if (buf[i][j+1] != NULL) {
                                        j++ ;
                                    }
                                    else if (buf[i][j+1] == NULL) {
                                        if (buf[i+1][0] != NULL) {
                                            i++ ;
                                            j = 0 ;
                                        }
                                        else {
                                            break ;
                                        }
                                    }
                                if ( ( just_there != NULL && j+1 <= end_col && if_sql_syntax(buf[i][j+1]) == false   ) || (just_there != NULL && i+1 <= end_col && if_sql_syntax(buf[i+1][0]) == false   )){
                                    if (buf[i][j+1] != NULL) {
                                        j++ ;
                                    }
                                    else if (buf[i][j+1] == NULL) {
                                        if (buf[i+1][0] != NULL) {
                                            i++ ;
                                            j = 0 ;
                                        }
                                        else {
                                            break ;
                                        }
                                    } 
                                    just_there->as = strdup(buf[i][j] )  ; 
                                }   
                                else { 
                                    check = 1 ; 
                                    return NULL ; 
                                }
                            }
                        if (just_there != NULL  ){
                              temp->children[temp->num++]  = just_there ; 
                        }
                        i = end_row_dec ; 
                        j = end_col_dec ; 
                    }
                    else {
                        while ( strcmp(buf[i][j] , ")") != 0 ){
                                if ( buf[i][j] == NULL){
                                    if ( i +1 < end_row){
                                    i++ ; 
                                    j = 0 ; 
                                    }
                                    else { 
                                        break ; 
                                    }
                                }
                                else { 
                                    if (buf[i][j+1] != NULL ){
                                        j++ ; 
                                    }   
                                        else {
                                            break ;
                                        }
                                }
                               if ( priority(buf[i][j]) != 0 ){
                                int temp_row = i ;
                                int temp_col = j ;
                                int depth = 0 ;

                                while (i <= end_row ){
                                    while (j <= end_col ){
                                    if ( buf[i][j] == NULL ){
                                        if ( i+1 <= end_row ){ 
                                            i++ ;
                                            j = 0 ;
                                             continue ; }
                                        else { 
                                            break ;
                                         }
                                    }

                                    if ( strcmp(buf[i][j], ";") == 0 || strcmp(buf[i][j], "ON") == 0 || strcmp(buf[i][j], "OFFSET") == 0 || strcmp(buf[i][j], "LIMIT") == 0 || strcmp(buf[i][j], "ORDER") == 0 || strcmp(buf[i][j], "HAVING") == 0 || strcmp(buf[i][j], "GROUP") == 0 || strcmp(buf[i][j], "WHERE") == 0 || strcmp(buf[i][j], "FROM") == 0 ){
                                        break ;
                                    }

                                    if ( strcmp(buf[i][j], "(") == 0 ){
                                        depth++ ;
                                    }
                                    else if ( strcmp(buf[i][j], ")") == 0 ){
                                        if ( depth == 0 ){
                                            break ;  
                                        }
                                        depth-- ;
                                    }
                                    if ( buf[i][j] == NULL){
                                        if ( i +1 < end_row){
                                        i++ ; 
                                        j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                    }
                                    else { 
                                        j++ ; 
                                    }   
                                    if ( i > end_row ){
                                        break ;   
                                    }
                                }   
                                }

                                tree * just_there = expression( buf , temp , check , temp_row , temp_col , i , j ) ;
                                if ( just_there != NULL ){
                                    temp->children[temp->num++] = just_there ;
                                }
                            }
                                else  {
                                    if (if_syntax(buf[i][j]) == false ){
                                        if ( strcmp(buf[i][j] , ",") == 0 ){
                                            if (  buf[i][j+1] == NULL  ) {
                                                if ( buf[i+1][0] != NULL ){
                                                i++ ;
                                                j = 0 ;
                                            } 
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                                continue ;
                                        }
                                        else if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                            int temp_row = i ; 
                                            int temp_col = j ; 
                                            while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                                if ( buf[i][j] == NULL ){
                                                    if ( i+ 1 <= end_row){
                                                    i++ ; 
                                                    j = 0 ; 
                                                }
                                                else { 
                                                    break ; 
                                                }
                                                }
                                                else {
                                                    j++ ;
                                                }
                                            }
                                            tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                            if (just_there != NULL ){
                                            temp->children[temp->num++ ]  = just_there ; 
                                            }
                                        }
                                        else {
                                            temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                        }
                                    }
                                    else { 
                                        check = 1 ; 
                                            return NULL ;   
                                    }
                            } 
                        }
                    }
            }




              else { 
                 if ( strcmp(buf[i][j], "*") == 0 && (j == 0 || strcmp(buf[i][j-1], "SELECT") == 0 || strcmp(buf[i][j-1], ",") == 0) ){
                        temp->children[temp->num++] = make_leaf("*", i, j) ;
                    }
                    else if ( priority(buf[i][j]) != 0  ){
                        int temp_row = i ; 
                        int temp_col = j ; 
                        while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                            if ( buf[i][j] == NULL ){
                                if ( i+ 1 <= end_row){
                                i++ ; 
                                j = 0 ; 
                            }
                            else { 
                                break ; 
                            }
                            }
                            else {
                                j++ ;
                            }
                        }
                        tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                        if (just_there != NULL ){
                        temp->children[temp->num++ ]  = just_there ; 
                        }
                    }         
                    else {
                        if (if_syntax(buf[i][j]) == false ){
                                if ( strcmp(buf[i][j] , ",") == 0 ){
                                    if (  buf[i][j+1] == NULL  ) {
                                         if ( buf[i+1][0] != NULL ){
                                        i++ ;
                                        j = 0 ;
                                    } 
                                }
                                else { 
                                    j++ ; 
                                }
                                        continue ;
                                }
                                 if (buf[i][j+1] != NULL && priority(buf[i][j+1]) != 0) {
                                    int temp_row = i ; 
                                    int temp_col = j ; 
                                    while (buf[i][j] != NULL &&  strcmp(buf[i][j] , ";") != 0  && strcmp(buf[i][j], "ON") != 0 && strcmp(buf[i][j], "OFFSET") != 0 && strcmp(buf[i][j], "LIMIT") != 0 && strcmp(buf[i][j], "ORDER") != 0 && strcmp(buf[i][j], "HAVING") != 0 && strcmp(buf[i][j], "GROUP") != 0&& strcmp(buf[i][j], "WHERE") != 0 && strcmp(buf[i][j], "FROM") != 0 && strcmp(buf[i][j], ")") != 0 ){
                                        if ( buf[i][j] == NULL ){
                                            if ( i+ 1 <= end_row){
                                            i++ ; 
                                            j = 0 ; 
                                        }
                                        else { 
                                            break ; 
                                        }
                                        }
                                        else {
                                            j++ ;
                                        }
                                    }
                                    tree*  just_there =  expression(buf , temp , check ,  temp_row ,  temp_col  ,  i ,  j ) ; 
                                    if (just_there != NULL ){
                                    temp->children[temp->num++ ]  = just_there ; 
                                    }
                                }
                                else {
                                       temp->children[temp->num++] = make_leaf( buf[i][j]  , i , j ) ; 
                                }
                            }
                            else { 
                                check = 1 ; 
                                return NULL ;   
                            }
                    }                    
            }
         }


             if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }

            }
            break;
    }
    return start  ; 

}
 
 

tree * select_query( int row , int col , int check , int end_row , int end_col , int pain){
         int compulsion = 0 ; 
        tree* start_of_tree = NULL  ; 
        char ***buf = proper_data.query ; 
        int i = row ; 
        int j = col ; 
        if ( check == 1 ){
            return NULL ; 
        }
        tree * node = NULL ; 
        while ( i <= end_row){
            while ( j<= end_col ){
                if ( check == 1 ){
                    return NULL ; 
                }
                if ( i == row && j == col ){
                    if (  strcmp(buf[i][j], "SELECT")==0  ){
                        node =  createNode("SELECT") ; 
                        start_of_tree = node ; 
                        compulsion++ ; 
                        if (buf[i][j+1] != NULL) {
                            j++ ;
                        }
                        else {
                            if (buf[i+1][0] != NULL) {
                                i++ ;
                                j = 0 ;
                            }
                            else {
                                break ;
                            }
                        } 
                        continue ; 
                    }
                }

                else if ( strcmp(buf[i][j], ";")==0 ){
                    return start_of_tree   ; 
                }

                else if (compulsion == 1){
                    int start_row = i ; 
                    int start_col = j ; 
                    while ( strcmp(buf[i][j]  , "FROM ") != 0 ){
                        if ( buf[i][j] == NULL ){
                            if (i+1 <= end_row){
                                i = i+ 1 ; 
                                j = 0  ; 
                            }
                        }
                        else { 
                            j++ ; 
                        }
                    }
                    node->children[node->num++] = comp_1(buf , node , start_row , start_col , i , j) ; 
                    compulsion++ ; 
                    if (strcmp(buf[i][j]  , "FROM ") == 0 ){
                        node->children[node->num++]= createNode("FROM") ; 
                        node = node->children[node->num - 1 ] ;  
                    }
                        if ( buf[i][j] == NULL ){
                            if (i+1 <= end_row){
                                i = i+ 1 ; 
                                j = 0  ; 
                            }
                        }
                        else { 
                            j++ ; 
                        }
                    continue ; 

                }

            else if ( strcmp(buf[i][j] , "UNION") == 0 || strcmp(buf[i][j] , "EXCEPT") == 0 || strcmp(buf[i][j] , "INTERSECT") == 0  ){
                pain = 1 ; 
                tree* superior ; 
                if ( (j+1 <= end_col && strcmp(buf[i][j+1]  , "ALL") == 0  ) || (i+1 <= end_row && strcmp(buf[i+1][0]  , "ALL") == 0 )){
                    if (strcmp(buf[i][j] , "UNION") == 0 ){
                        superior = createNode("UNION ALL") ;      
                    }
                    else if (strcmp(buf[i][j] , "EXCEPT") == 0 ){
                        superior = createNode("EXCEPT ALL") ;      
                    }
                    else if (strcmp(buf[i][j] , "INTERSECT") == 0 ){
                        superior = createNode("INTERSECT ALL") ;      
                    }
                    if ( buf[i][j] == NULL ){
                        if (i+1 <= end_row){
                            i = i+ 1 ; 
                            j = 0  ; 
                        }
                    }
                    else { 
                        j++ ; 
                    }
                }
                else { 
                    if (strcmp(buf[i][j] , "UNION") == 0 ){
                        superior = createNode("UNION ") ;      
                    }
                    else  if (strcmp(buf[i][j] , "EXCEPT") == 0 ){
                        superior = createNode("EXCEPT ") ;      
                    }
                    else if (strcmp(buf[i][j] , "INTERSECT") == 0 ){
                        superior = createNode("INTERSECT ") ;      
                    }
                }
                if ( buf[i][j] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else if (buf[i][j] != NULL   ) { 
                    j++ ; 
                }
                superior->children[superior->num++] = start_of_tree ; 
                start_of_tree = superior ; 

             start_of_tree->children[start_of_tree->num++] =  select_query( i  ,  j ,  check  ,  end_row ,  end_col ,  pain   ) ; 
                return start_of_tree ; 
            }


            else if (compulsion == 2){
                int start_row = i ; 
                int start_col = j ; 
                while (strcmp(buf[i][j]  , "WHERE ") != 0 && strcmp(buf[i][j]  , "UNION") != 0 && strcmp(buf[i][j]  , "EXCEPT") != 0  && strcmp(buf[i][j]  , "INTERSECT") != 0   &&strcmp(buf[i][j]  , ";") != 0  ){
                    if ( buf[i][j] == NULL ){
                        if (i+1 <= end_row){
                            i = i+ 1 ; 
                            j = 0  ; 
                        }
                    }
                    else { 
                        j++ ; 
                    }
                }
                if ( pain == 0 ){
                    node->children[node->num++] = comp_2(buf , node , start_row , start_col , i , j , pain ) ; 
                    compulsion++ ; 
                    if (strcmp(buf[i][j]  , "WHERE ") == 0 ){
                        node->children[node->num++]= createNode("WHERE") ; 
                        node = node->children[node->num - 1 ] ;  
                    }
                    }
                else if ( pain == 1 ) { 
                        start_of_tree->children[start_of_tree->num++] = comp_2(buf , node , start_row , start_col , i , j , pain )  ; 
                }

                if (strcmp(buf[i][j]  , ";") == 0 ) {
                    if ( j-1 > 0 ){
                        j = j-1 ; 
                    }
                    else {
                        int temp_row  = 0 ; 
                        int temp_col ; 
                        if ( i- 1 > 0 ){
                        temp_row = i -1 ; 
                        }
                        else {
                            i = 0 ; 
                            j = 0 ; 
                        }
                        temp_col = 0 ; 
                        for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                        }
                        j = temp_col -1 ; 
                        i = temp_row ; 
                    }
                }
                else {
                if ( buf[i][j] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }
                }
                continue ; 

            }



            else if (compulsion == 3){
                int start_row = i ; 
                int start_col = j ; 
                while (strcmp(buf[i][j]  , ";") != 0  && strcmp(buf[i][j]  , "UNION") != 0 && strcmp(buf[i][j]  , "EXCEPT") != 0  && strcmp(buf[i][j]  , "INTERSECT") != 0    ){
                    if ( buf[i][j] == NULL ){
                        if (i+1 <= end_row){
                            i = i+ 1 ; 
                            j = 0  ; 
                        }
                    }
                    else { 
                        j++ ; 
                    }
                }

                if ( pain == 0 ){
                    node->children[node->num++] = comp_3(buf , node , start_row , start_col , i , j , pain  , 0 ) ; 
                    compulsion++ ; 
                }
                else { 
                    start_of_tree->children[start_of_tree->num++] =  comp_3(buf , node , start_row , start_col , i , j , pain  , 0 ) ; 
                }

                if (strcmp(buf[i][j]  , ";") == 0 ) {
                    if ( j-1 > 0 ){
                        j = j-1 ; 
                    }
                    else {
                        int temp_row  = 0 ; 
                        int temp_col ; 
                        if ( i- 1 > 0 ){
                        temp_row = i -1 ; 
                        }
                        else {
                            i = 0 ; 
                            j = 0 ; 
                        }
                        temp_col = 0 ; 
                        for ( temp_col= 0 ; buf[temp_row][temp_col] != NULL ; temp_col++ ){
                        }
                        j = temp_col -1 ; 
                        i = temp_row ; 
                    }
                }
                else {
                if ( buf[i][j] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ; 
                        j = 0  ; 
                    }
                }
                else { 
                    j++ ; 
                }
                }
                continue ; 

            }

            if (buf[i][j+1] != NULL) {
                j++ ;
            }
            else {
                if (buf[i+1][0] != NULL) {
                    i++ ;
                    j = 0 ;
                }
                else {
                    break ;
                }
            }



            }
        }
}





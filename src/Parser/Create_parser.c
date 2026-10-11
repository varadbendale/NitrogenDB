#include Parser.h 

ctree* createNode( char* comp) {
    ctree* node = malloc(sizeof(ctree));
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
    node->children = calloc(300, sizeof(ctree*));
    node->references = 0 ; 
    node->num = 0  ; 
    node->col = 0 ; 
    node->line = 0 ; 
    node->type = 0 ; 
    node->type_bits_count = 0 ; 
    node->first_decimal = 0 ; 
    node->second_decimal = 0 ; 
    node->primary_key = calloc(64, sizeof(char*)) ;  ; 
    node->foreign_key_counter = 0 ; 
    node->primary_key_counter = 0 ; 
    node->foreign_key = calloc(64, sizeof(char*)) ;   ; 
    node->not_null = false  ; 
    node->default_val = NULL  ; 
    node->unique = false  ; 
    return node;
}

bool validate_default(char *default_val, int type) {
    switch (type) {
        case INT:
        case BIGINT:
        case SMALLINT:
        case TINYINT: {
            int start = (default_val[0] == '-') ? 1 : 0;
            for (int i = start; i < strlen(default_val); i++) {
                if (!isdigit(default_val[i])){
                    return false;
                } 
            }
            return true;
        }

        case FLOAT:
        case DOUBLE: {
            int dots = 0;
            int start = (default_val[0] == '-') ? 1 : 0;
            for (int i = start; i < strlen(default_val); i++) {
                if (default_val[i] == '.') {
                    dots++;
                    if (dots > 1) {
                        return false;
                    }
                }
                else if (!isdigit(default_val[i])) {
                    return false;
                }
            }
            return true;
        }

        case BOOL: {
            return (strcmp(default_val, "true") == 0  ||strcmp(default_val, "false") == 0 ||strcmp(default_val, "1") == 0     || strcmp(default_val, "0") == 0);
        }

        case CHAR:
        case VARCHAR:
        case DATE:
        case TIMESTAMP:
            return true;

        default:
            return false;
    }
}


ctree* create_parser(){
    char ***buf = proper_data.query ; 
    int row = 0  ; 
    int col = 0 ; 
    int i = 0 ; 
    int j = 0  ; 
    int check = 0 ; 
    int max = 0 ; 
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
    end_row  = i ; 
    end_col = j ; 
    i = row ; 
    j = col ; 
    ctree * start = NULL ; 
    ctree * node = NULL ;
    while ( i <= end_row ){
        while ( j <= end_col){
            if (i == row && j == col ){
                if ( strcmp(buf[i][j], "CREATE")==0   ){
                    if ( buf[i][j+1] == NULL ){
                            if (i+1 <= end_row){
                                i = i+ 1 ; 
                                j = 0  ; 
                            }
                        }
                        else { 
                            j++ ; 
                    }   
                    if (strcmp(buf[i][j], "TABLE")==0 ){
                        node =  createNode("CREATE TABLE") ; 
                        start = node ; 
                        if (1){
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
                        if (1) {               
                            if (buf[i][j] != NULL ){
                                node->children[node->num++] = createNode(buf[i][j]) ; 
                                node = node->children[node->num - 1 ] ; 
                            }
                            else { 
                                return NULL ; 
                            }
                        }
                        if (1){
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
                    else {
                        return NULL ; 
                    }
                }
            }

            else if ( strcmp(buf[i][j], ";")==0 ){
                    return start   ; 
            }

            if (strcmp(buf[i][j] , "AS ") != 0 ){
                int braces = 0  ; 
                if (strcmp(buf[i][j]  , "(") == 0 ){
                    if (1){
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

                    braces++ ; 
                    while (braces > 0 ){
                        if ( buf[i][j] == NULL ) {
                            break ; 
                        } ; 
                        ctree * tree = createNode(NULL) ; 
                        int present = 0 ; 
                        if (strcmp(buf[i][j] , ")") == 0 ){
                            braces-- ; 
                            if (braces == 0 ){
                                break ; 
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
                            continue ; 
                        }

                        if (strcmp(buf[i][j] , "(")){
                            braces++ ; 
                        }
                        int tems = 0; 
                        while (buf[i][j] != NULL && strcmp(buf[i][j]  , ",") != 0 ){
                            tems = 1 ; 

                            if (strcmp(buf[i][j] , ";") == 0 ){
                                braces = 0 ; 
                                break ; 
                            }

                            if (strcmp(buf[i][j] , ")") == 0 ){
                                braces-- ; 
                                if (braces == 0 ){
                                    break ; 
                                }
                            }

                            if (buf[i][j] == NULL ) {
                                break ; 
                            } ; 

                            if (buf[i][j] != NULL  ){
                                if (strcmp(buf[i][j] , "DECIMAL" ) == 0 || strcmp(buf[i][j] , "INT" ) == 0 ||strcmp(buf[i][j] , "BIGINT" ) == 0 ||strcmp(buf[i][j] , "SMALLINT" ) == 0 ||strcmp(buf[i][j] , "FLOAT" ) == 0 ||strcmp(buf[i][j] , "TINYINT" ) == 0 ||strcmp(buf[i][j] , "DOUBLE" ) == 0 ||strcmp(buf[i][j] , "CHAR" ) == 0 ||strcmp(buf[i][j] , "VARCHAR" ) == 0 ||strcmp(buf[i][j] , "BOOL" ) == 0 || strcmp(buf[i][j] , "TIMESTAMP" ) == 0 || strcmp(buf[i][j] , "DATE" ) == 0 ){

                                    if (strcmp(buf[i][j] , "INT" ) == 0 ){
                                        tree->type = INT ; 
                                        tree->type_bits_count = 4 ; 
                                    }
                                    if (strcmp(buf[i][j] , "BIGINT" ) == 0 ){
                                        tree->type = BIGINT ; 
                                        tree->type_bits_count = 8 ;     
                                    }
                                    if(strcmp(buf[i][j] , "SMALLINT" ) == 0 ){
                                        tree->type = SMALLINT ; 
                                        tree->type_bits_count = 2 ; 
                                    }
                                    if (strcmp(buf[i][j] , "FLOAT" ) == 0 ){
                                        tree->type = FLOAT ; 
                                        tree->type_bits_count = 4 ; 
                                    }
                                    if (strcmp(buf[i][j] , "TINYINT" ) == 0 ){
                                        tree->type = TINYINT ; 
                                        tree->type_bits_count = 1 ; 

                                    }
                                    if (strcmp(buf[i][j] , "DOUBLE" ) == 0 ){
                                        tree->type = DOUBLE ; 
                                        tree->type_bits_count = 8 ; 
                                    }
                                    if (strcmp(buf[i][j] , "DECIMAL" ) == 0 ){
                                        if ( buf[i][j+1] == NULL ){
                                            if (i+1 <= end_row){
                                                i = i+ 1 ; 
                                                j = 0  ; 
                                            }
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                        if (strcmp(buf[i][j] , ",") != 0 ){
                                            if ( buf[i][j+1] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }
                                            tree->first_decimal = atoi(buf[i][j]) ; 
                                            if ( buf[i][j+1] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }
                                            tree->second_decimal = atoi(buf[i][j]) ; 
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

                                    if (strcmp(buf[i][j] , "CHAR" ) == 0 ){
                                        tree->type = CHAR ; 
                                        if ( buf[i][j+1] == NULL ){
                                            if (i+1 <= end_row){
                                                i = i+ 1 ; 
                                                j = 0  ; 
                                            }
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                        int num = 0 ; 
                                        if (buf[i][j] != NULL ){
                                            num = atoi(string(buf[i][j])) ; 
                                        }
                                        tree->type_bits_count = num  ; 
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
                                    if (strcmp(buf[i][j] , "VARCHAR" ) == 0 ){
                                        tree->type = VARCHAR ; 
                                        if ( buf[i][j+1] == NULL ){
                                            if (i+1 <= end_row){
                                                i = i+ 1 ; 
                                                j = 0  ; 
                                            }
                                        }
                                        else { 
                                            j++ ; 
                                        }
                                        int num = 0 ; 
                                        if (buf[i][j] != NULL ){
                                            num = atoi(string(buf[i][j])) ; 
                                        }
                                        tree->type_bits_count = num  ; 
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
                                    if(strcmp(buf[i][j] , "BOOL" ) == 0 ){
                                        tree->type = BOOL ; 
                                        tree->type_bits_count = 1 ; 
                                    }
                                    if (strcmp(buf[i][j] , "DATE" ) == 0 ){
                                        tree->type = DATE ; 
                                        tree->type_bits_count = 4 ; 
                                    }
                                    if (strcmp(buf[i][j] , "TIMESTAMP" ) == 0 ){
                                        tree->type = TIMESTAMP ; 
                                        tree->type_bits_count = 8 ; 
                                    }
                                }
                                
                                else if (strcmp(buf[i][j] , PRIMARY ) == 0 || strcmp(buf[i][j] , UNIQUE ) == 0  || strcmp(buf[i][j] , NOT ) == 0 || strcmp(buf[i][j] , DEFAULT ) == 0  ||  strcmp(buf[i][j] , FOREIGN ) == 0  ){
                                    if (strcmp(buf[i][j] , PRIMARY ) == 0 ){
                                        if (present == 1 ){
                                            tree->primary_key_counter = 1 ; 
                                        }
                                        else { 
                                            int a = 0 ; 
                                            tree->primary_key_counter = 0 ; 
                                            while ( a < 3 ){
                                                if ( buf[i][j+1] == NULL ){
                                                    if (i+1 <= end_row){
                                                        i = i+ 1 ; 
                                                        j = 0  ; 
                                                    }
                                                }
                                                else { 
                                                    j++ ; 
                                                }   
                                                a++ ;
                                            }
                                            while ( buf[i][j] != NULL &&  strcmp(buf[i][j] , ')') != 0 ){
                                                tree->primary_key[tree->primary_key_counter++] = strdup(buf[i][j]) ;  
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
                                            if (tree->primary_key_counter == 1 ){
                                                if (tree->primary_key[0] != NULL ){
                                                    tree->comp = strdup(tree->primary_key[0] ) ; 
                                                }
                                            }
                                            else if (tree->primary_key_counter > 1 ){
                                                tree->comp = "PRIMARY KEY" ; 
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
                                    }


                                else  if(strcmp(buf[i][j] , "UNIQUE" ) == 0  ){
                                    if (present == 1 ){
                                        tree->unique = true ; 
                                    }
                                }

                                else if(strcmp(buf[i][j] , "NOT" )== 0 ){
                                    if ( buf[i][j+1] == NULL ){
                                        if (i+1 <= end_row){
                                            i = i+ 1 ; 
                                            j = 0  ; 
                                        }
                                    }
                                    else { 
                                        j++ ; 
                                    } 
                                    if (strcmp(buf[i][j] , "NULL" )== 0){
                                        if (present == 1 ){
                                            tree->not_null = true ; 
                                        }

                                    }
                                }

                                else if (strcmp(buf[i][j] , "DEFAULT" ) == 0 ){
                                    if ( buf[i][j+1] == NULL ){
                                        if (i+1 <= end_row){
                                            i = i+ 1 ; 
                                            j = 0  ; 
                                        }
                                    }
                                    else { 
                                        j++ ; 
                                    }
                                    
                                    if (buf[i][j] != NULL && validate_default(buf[i][j] , tree->type)  == true ){
                                        tree->default_val = strdup(buf[i][j] ) ; 
                                    }

                                }

                                else if (strcmp(buf[i][j] , "FOREIGN") == 0 ){
                                        int a = 0 ; 
                                        tree->foreign_key_counter = 0 ; 
                                        while ( a < 3 ){
                                            if ( buf[i][j+1] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }   
                                            a++ ; 
                                        }
                                        while ( buf[i][j] != NULL &&  strcmp(buf[i][j] , ")") != 0 ){
                                            tree->foreign_key[tree->foreign_key_counter++] = strdup(buf[i][j]) ;  
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
                                        if (tree->foreign_key_counter > 0 ){
                                            tree->comp = "FOREIGN KEY" ; 
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

                                        if (buf[i][j] != NULL && strcmp(buf[i][j] , "REFERENCES") == 0 ){
                                            if ( buf[i][j+1] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            }  
                                            tree->references = 1  ; 
                                            ctree * temp = createNode(buf[i][j]) ; 

                                            if ( buf[i][j+1] == NULL ){
                                                if (i+1 <= end_row){
                                                    i = i+ 1 ; 
                                                    j = 0  ; 
                                                }
                                            }
                                            else { 
                                                j++ ; 
                                            } 

                                            if (strcmp(buf[i][j] , "(") == 0 ){
                                                if ( buf[i][j+1] == NULL ){
                                                    if (i+1 <= end_row){
                                                        i = i+ 1 ; 
                                                        j = 0  ; 
                                                    }
                                                }
                                                else { 
                                                    j++ ; 
                                                } 

                                                while ( buf[i][j] != NULL && strcmp(buf[i][j] , ")") != 0 ){
                                                    ctree * ref_col = createNode(buf[i][j]) ; 
                                                    temp->children[temp->num++] = ref_col ; 
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

                                                if ( buf[i][j+1] == NULL ){
                                                    if (i+1 <= end_row){
                                                        i = i+ 1 ; 
                                                        j = 0  ; 
                                                    }
                                                }
                                                else { 
                                                    j++ ; 
                                                } 

                                                tree->children[tree->num++] = temp ; 

                                            }
                                        }

                                }


                                }

                                else  {
                                    tree->comp = strdup(buf[i][j]) ; 
                                    present = 1 ; 
                                }
                            }



                            if (1){
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
                        if (tems == 1 ){
                            node->children[node->num++] = tree ; 
                        }

                        if ( buf[i][j] != NULL && strcmp(buf[i][j] , ",") == 0 ){
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

            }
            else { 
                node->as = select_query( i , j , end_row , end_col , pain ) ; 
            }




            j++ ; 
        }
        i++ ; 
        j = 0 ; 
    }
    return start ; 
}
































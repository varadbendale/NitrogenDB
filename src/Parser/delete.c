#include "Parser.h"

dtree* createNodeDelete( char* comp) {
    dtree* node = malloc(sizeof(dtree));
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
    node->sub = NULL ;
    node->children = calloc(300, sizeof(dtree*));
    node->num = 0 ;
    node->line = 0 ;
    node->col = 0 ;
    return node;
}

dtree* d_or( char ***buf , int *i , int *j ) ;
dtree* d_and( char ***buf , int *i , int *j ) ;
dtree* d_not( char ***buf , int *i , int *j ) ;
dtree* d_cmp( char ***buf , int *i , int *j ) ;
dtree* d_sum( char ***buf , int *i , int *j ) ;
dtree* d_mul( char ***buf , int *i , int *j ) ;
dtree* d_unary( char ***buf , int *i , int *j ) ;
dtree* d_primary( char ***buf , int *i , int *j ) ;


dtree* d_bin( char *op , dtree *left , dtree *right ){
    dtree * node = NULL ;
    node = createNodeDelete(op) ;
    node->children[node->num++] = left ;
    node->children[node->num++] = right ;
    return node ;
}


dtree* d_subquery( char ***buf , int *i , int *j ){
    int check = 0 ;
    int pain = 0 ;
    int depth = 0 ;
    dtree * node = NULL ;
    node = createNodeDelete("SUBQUERY") ;
    node->line = *i ;
    node->col = *j ;
    node->sub = select_query( *i , *j , check , end_row , end_col , pain ) ;
    if ( node->sub == NULL ){
        return NULL ;
    }
    depth = 1 ;
    while ( buf[*i][*j] != NULL ){
        if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "(") == 0 ){
            depth++ ;
        }
        else if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ){
            depth-- ;
            if ( depth == 0 ){
                break ;
            }
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
    }
    if ( depth != 0 ){
        return NULL ;
    }
    return node ;
}


dtree* d_or( char ***buf , int *i , int *j ){
    dtree * left = NULL ;
    dtree * right = NULL ;
    left = d_and(buf , i , j) ;
    if ( left == NULL ){
        return NULL ;
    }
    while ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "OR") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        right = d_and(buf , i , j) ;
        if ( right == NULL ){
            return NULL ;
        }
        left = d_bin("OR" , left , right) ;
    }
    return left ;
}

dtree* d_and( char ***buf , int *i , int *j ){
    dtree * left = NULL ;
    dtree * right = NULL ;
    left = d_not(buf , i , j) ;
    if ( left == NULL ){
        return NULL ;
    }
    while ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "AND") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        right = d_not(buf , i , j) ;
        if ( right == NULL ){
            return NULL ;
        }
        left = d_bin("AND" , left , right) ;
    }
    return left ;
}

dtree* d_not( char ***buf , int *i , int *j ){
    dtree * child = NULL ;
    dtree * node = NULL ;
    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "NOT") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        child = d_not(buf , i , j) ;
        if ( child == NULL ){
            return NULL ;
        }
        node = createNodeDelete("NOT") ;
        node->children[node->num++] = child ;
        return node ;
    }
    return d_cmp(buf , i , j) ;
}


dtree* d_cmp( char ***buf , int *i , int *j ){
    dtree * sub = NULL ;
    dtree * left = NULL ;
    dtree * right = NULL ;
    dtree * node = NULL ;
    dtree * low = NULL ;
    dtree * high = NULL ;
    dtree * pattern = NULL ;
    dtree * item = NULL ;
    char * t = NULL ;
    char * op = NULL ;
    int neg = 0 ;

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "EXISTS") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "(") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "SELECT") == 0 ) ){
            return NULL ;
        }
        sub = d_subquery(buf , i , j) ;
        if ( sub == NULL ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        node = createNodeDelete("EXISTS") ;
        node->children[node->num++] = sub ;
        return node ;
    }

    left = d_sum(buf , i , j) ;
    if ( left == NULL ){
        return NULL ;
    }
    if ( buf[*i][*j] == NULL ){
        return left ;
    }

    t = buf[*i][*j] ;

    if ( strcmp(t , "=") == 0 || strcmp(t , "!=") == 0 || strcmp(t , "<>") == 0 || strcmp(t , "<") == 0 || strcmp(t , ">") == 0 || strcmp(t , "<=") == 0 || strcmp(t , ">=") == 0 ){
        op = strdup(t) ;
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        right = d_sum(buf , i , j) ;
        if ( right == NULL ){
            return NULL ;
        }
        return d_bin(op , left , right) ;
    }

    if ( strcmp(t , "IS") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        neg = 0 ;
        if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "NOT") == 0 ){
            neg = 1 ;
            if ( buf[*i][*j] != NULL ){
                if ( buf[*i][*j+1] == NULL ){
                    if (*i+1 <= end_row){
                        *i = *i + 1 ;
                        *j = 0 ;
                    }
                }
                else {
                    *j = *j + 1 ;
                }
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "NULL") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        node = createNodeDelete( neg ? "IS NOT NULL" : "IS NULL" ) ;
        node->children[node->num++] = left ;
        return node ;
    }

    neg = 0 ;
    if ( strcmp(t , "NOT") == 0 ){
        neg = 1 ;
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "IN") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "(") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        node = createNodeDelete( neg ? "NOT IN" : "IN" ) ;
        node->children[node->num++] = left ;
        if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "SELECT") == 0 ){
            sub = d_subquery(buf , i , j) ;
            if ( sub == NULL ){
                return NULL ;
            }
            node->children[node->num++] = sub ;
        }
        else {
            while ( buf[*i][*j] != NULL ){
                item = d_sum(buf , i , j) ;
                if ( item == NULL ){
                    return NULL ;
                }
                node->children[node->num++] = item ;
                if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ",") == 0 ){
                    if ( buf[*i][*j] != NULL ){
                        if ( buf[*i][*j+1] == NULL ){
                            if (*i+1 <= end_row){
                                *i = *i + 1 ;
                                *j = 0 ;
                            }
                        }
                        else {
                            *j = *j + 1 ;
                        }
                    }
                }
                else {
                    break ;
                }
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        return node ;
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "BETWEEN") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        low = d_sum(buf , i , j) ;
        if ( low == NULL || !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "AND") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        high = d_sum(buf , i , j) ;
        if ( high == NULL ){
            return NULL ;
        }
        node = createNodeDelete( neg ? "NOT BETWEEN" : "BETWEEN" ) ;
        node->children[node->num++] = left ;
        node->children[node->num++] = low ;
        node->children[node->num++] = high ;
        return node ;
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "LIKE") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        pattern = d_sum(buf , i , j) ;
        if ( pattern == NULL ){
            return NULL ;
        }
        return d_bin( neg ? "NOT LIKE" : "LIKE" , left , pattern ) ;
    }

    if ( neg ){
        return NULL ;
    }
    return left ;
}


dtree* d_sum( char ***buf , int *i , int *j ){
    dtree * left = NULL ;
    dtree * right = NULL ;
    char * op = NULL ;
    left = d_mul(buf , i , j) ;
    if ( left == NULL ){
        return NULL ;
    }
    while ( ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "+") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "-") == 0 ) ){
        op = strdup(buf[*i][*j]) ;
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        right = d_mul(buf , i , j) ;
        if ( right == NULL ){
            return NULL ;
        }
        left = d_bin(op , left , right) ;
    }
    return left ;
}

dtree* d_mul( char ***buf , int *i , int *j ){
    dtree * left = NULL ;
    dtree * right = NULL ;
    char * op = NULL ;
    left = d_unary(buf , i , j) ;
    if ( left == NULL ){
        return NULL ;
    }
    while ( ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "*") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "/") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "%") == 0 ) ){
        op = strdup(buf[*i][*j]) ;
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        right = d_unary(buf , i , j) ;
        if ( right == NULL ){
            return NULL ;
        }
        left = d_bin(op , left , right) ;
    }
    return left ;
}

dtree* d_unary( char ***buf , int *i , int *j ){
    dtree * child = NULL ;
    dtree * node = NULL ;
    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "-") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        child = d_unary(buf , i , j) ;
        if ( child == NULL ){
            return NULL ;
        }
        node = createNodeDelete("NEG") ;
        node->children[node->num++] = child ;
        return node ;
    }
    return d_primary(buf , i , j) ;
}


dtree* d_primary( char ***buf , int *i , int *j ){
    dtree * sub = NULL ;
    dtree * inner = NULL ;
    dtree * node = NULL ;
    dtree * arg = NULL ;
    char * full = NULL ;

    if ( buf[*i][*j] == NULL ){
        return NULL ;
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "(") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "SELECT") == 0 ){
            sub = d_subquery(buf , i , j) ;
            if ( sub == NULL ){
                return NULL ;
            }
            if ( buf[*i][*j] != NULL ){
                if ( buf[*i][*j+1] == NULL ){
                    if (*i+1 <= end_row){
                        *i = *i + 1 ;
                        *j = 0 ;
                    }
                }
                else {
                    *j = *j + 1 ;
                }
            }
            return sub ;
        }
        inner = d_or(buf , i , j) ;
        if ( inner == NULL || !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        return inner ;
    }

    if ( ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ",") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ";") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "WHERE") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "ORDER") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "LIMIT") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "AND") == 0 ) || ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "OR") == 0 ) ){
        return NULL ;
    }

    node = createNodeDelete(buf[*i][*j]) ;
    node->line = *i ;
    node->col = *j ;
    if ( buf[*i][*j] != NULL ){
        if ( buf[*i][*j+1] == NULL ){
            if (*i+1 <= end_row){
                *i = *i + 1 ;
                *j = 0 ;
            }
        }
        else {
            *j = *j + 1 ;
        }
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ".") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        if ( buf[*i][*j] == NULL ){
            return NULL ;
        }
        full = malloc(strlen(node->comp) + strlen(buf[*i][*j]) + 2) ;
        sprintf(full , "%s.%s" , node->comp , buf[*i][*j]) ;
        node->comp = full ;
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
    }

    if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , "(") == 0 ){
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
        while ( buf[*i][*j] != NULL && !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ) ){
            arg = d_or(buf , i , j) ;
            if ( arg == NULL ){
                return NULL ;
            }
            node->children[node->num++] = arg ;
            if ( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ",") == 0 ){
                if ( buf[*i][*j] != NULL ){
                    if ( buf[*i][*j+1] == NULL ){
                        if (*i+1 <= end_row){
                            *i = *i + 1 ;
                            *j = 0 ;
                        }
                    }
                    else {
                        *j = *j + 1 ;
                    }
                }
            }
        }
        if ( !( buf[*i][*j] != NULL && strcmp(buf[*i][*j] , ")") == 0 ) ){
            return NULL ;
        }
        if ( buf[*i][*j] != NULL ){
            if ( buf[*i][*j+1] == NULL ){
                if (*i+1 <= end_row){
                    *i = *i + 1 ;
                    *j = 0 ;
                }
            }
            else {
                *j = *j + 1 ;
            }
        }
    }
    return node ;
}


dtree* delete_parser(){
    char ***buf = proper_data.query ;
    int i = 0 ;
    int j = 0 ;
    int k = 0 ;
    int names = 0 ;
    int *pi = &i ;
    int *pj = &j ;
    dtree * start = NULL ;
    dtree * table = NULL ;
    dtree * where = NULL ;
    dtree * cond = NULL ;
    dtree * order = NULL ;
    dtree * col = NULL ;
    dtree * dir = NULL ;
    dtree * limit = NULL ;

    while (buf[i][0] != NULL ){
        i++ ;
    }
    end_row = i ;
    i = 0 ;
    j = 0 ;

    if ( buf[i][j] == NULL ){
        return NULL ;
    }

    if ( strcmp(buf[i][j] , "DELETE") == 0 ){
        start = createNodeDelete("DELETE") ;
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }

        while ( ( buf[i][j] != NULL && strcmp(buf[i][j] , "LOW_PRIORITY") == 0 ) || ( buf[i][j] != NULL && strcmp(buf[i][j] , "QUICK") == 0 ) || ( buf[i][j] != NULL && strcmp(buf[i][j] , "IGNORE") == 0 ) ){
            start->children[start->num++] = createNodeDelete(buf[i][j]) ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }

        if ( !( buf[i][j] != NULL && strcmp(buf[i][j] , "FROM") == 0 ) ){
            return NULL ;
        }
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }

        if ( buf[i][j] == NULL || ( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            return NULL ;
        }
        table = createNodeDelete(buf[i][j]) ;
        table->line = i ;
        table->col = j ;
        start->children[start->num++] = table ;
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }

        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "AS") == 0 ){
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            if ( buf[i][j] == NULL ){
                return NULL ;
            }
            table->as = strdup(buf[i][j]) ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }
        else if ( buf[i][j] != NULL && !( buf[i][j] != NULL && strcmp(buf[i][j] , "WHERE") == 0 ) && !( buf[i][j] != NULL && strcmp(buf[i][j] , "ORDER") == 0 ) && !( buf[i][j] != NULL && strcmp(buf[i][j] , "LIMIT") == 0 ) && !( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            table->as = strdup(buf[i][j]) ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }

        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "WHERE") == 0 ){
            where = createNodeDelete("WHERE") ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            cond = d_or(buf , pi , pj) ;
            if ( cond == NULL ){
                return NULL ;
            }
            where->children[where->num++] = cond ;
            start->children[start->num++] = where ;
        }

        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "ORDER") == 0 ){
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            if ( !( buf[i][j] != NULL && strcmp(buf[i][j] , "BY") == 0 ) ){
                return NULL ;
            }
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            order = createNodeDelete("ORDER BY") ;
            while ( buf[i][j] != NULL && !( buf[i][j] != NULL && strcmp(buf[i][j] , "LIMIT") == 0 ) && !( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
                col = d_sum(buf , pi , pj) ;
                if ( col == NULL ){
                    return NULL ;
                }
                if ( ( buf[i][j] != NULL && strcmp(buf[i][j] , "ASC") == 0 ) || ( buf[i][j] != NULL && strcmp(buf[i][j] , "DESC") == 0 ) ){
                    dir = createNodeDelete(buf[i][j]) ;
                    dir->children[dir->num++] = col ;
                    col = dir ;
                    if ( buf[i][j] != NULL ){
                        if ( buf[i][j+1] == NULL ){
                            if (i+1 <= end_row){
                                i = i+ 1 ;
                                j = 0 ;
                            }
                        }
                        else {
                            j++ ;
                        }
                    }
                }
                order->children[order->num++] = col ;
                if ( buf[i][j] != NULL && strcmp(buf[i][j] , ",") == 0 ){
                    if ( buf[i][j] != NULL ){
                        if ( buf[i][j+1] == NULL ){
                            if (i+1 <= end_row){
                                i = i+ 1 ;
                                j = 0 ;
                            }
                        }
                        else {
                            j++ ;
                        }
                    }
                }
            }
            if ( order->num == 0 ){
                return NULL ;
            }
            start->children[start->num++] = order ;
        }

        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "LIMIT") == 0 ){
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            if ( buf[i][j] == NULL ){
                return NULL ;
            }
            k = 0 ;
            while ( k < strlen(buf[i][j]) ){
                if ( !isdigit(buf[i][j][k]) ){
                    return NULL ;
                }
                k++ ;
            }
            limit = createNodeDelete("LIMIT") ;
            limit->children[limit->num++] = createNodeDelete(buf[i][j]) ;
            start->children[start->num++] = limit ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }

        if ( buf[i][j] != NULL && !( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            return NULL ;
        }
        return start ;
    }

    if ( strcmp(buf[i][j] , "TRUNCATE") == 0 ){
        start = createNodeDelete("TRUNCATE TABLE") ;
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }
        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "TABLE") == 0 ){
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }
        if ( buf[i][j] == NULL || ( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            return NULL ;
        }
        start->children[start->num++] = createNodeDelete(buf[i][j]) ;
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }
        if ( buf[i][j] != NULL && !( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            return NULL ;
        }
        return start ;
    }

    if ( strcmp(buf[i][j] , "DROP") == 0 ){
        start = createNodeDelete("DROP TABLE") ;
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }
        if ( !( buf[i][j] != NULL && strcmp(buf[i][j] , "TABLE") == 0 ) ){
            return NULL ;
        }
        if ( buf[i][j] != NULL ){
            if ( buf[i][j+1] == NULL ){
                if (i+1 <= end_row){
                    i = i+ 1 ;
                    j = 0 ;
                }
            }
            else {
                j++ ;
            }
        }
        if ( buf[i][j] != NULL && strcmp(buf[i][j] , "IF") == 0 ){
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
            if ( !( buf[i][j] != NULL && strcmp(buf[i][j] , "EXISTS") == 0 ) ){
                return NULL ;
            }
            start->children[start->num++] = createNodeDelete("IF EXISTS") ;
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }
        names = 0 ;
        while ( buf[i][j] != NULL && !( buf[i][j] != NULL && strcmp(buf[i][j] , ";") == 0 ) ){
            if ( !( buf[i][j] != NULL && strcmp(buf[i][j] , ",") == 0 ) ){
                start->children[start->num++] = createNodeDelete(buf[i][j]) ;
                names++ ;
            }
            if ( buf[i][j] != NULL ){
                if ( buf[i][j+1] == NULL ){
                    if (i+1 <= end_row){
                        i = i+ 1 ;
                        j = 0 ;
                    }
                }
                else {
                    j++ ;
                }
            }
        }
        if ( names == 0 ){
            return NULL ;
        }
        return start ;
    }

    return NULL ;
}
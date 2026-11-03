    unsigned char * get_data_sort(sort_arr thing , int key , int * lenght , int * type ){
        thing->array + key + 1  ; 
        int pos = 0 ; 
        unsigned char * temp = thing[pos] ; 
        for ( int i =  0 ; i < key ; i++ ){
            temp = thing[pos] ; 
            pos = pos + 1 ; 
            if (temp == integer_num){
                pos = pos + sizeof(long) ; 
            }
            else if (temp == real_num){
                pos = pos + sizeof(float ) ;   
            }
            else if (temp == string_num ){
                uint32_t size ; 
                memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                pos = pos + sizeof(uint32_t) ; 
                pos = pos + size ; 
            }
        }
        temp = thing[pos] ; 
        int len ; 
        pos = pos + 1 ; 
        if (temp == integer_num){
            len =  sizeof(long) ; 
            type = integer_num ; 
        }
        else if (temp == real_num){
            len =  sizeof(float ) ; 
            type =  real_num ; 
        }
        else if (temp == string_num ){
            uint32_t size ; 
            memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
            len =  sizeof(uint32_t) ; 
            len = len + size ; 
            type = string_num ; 
        }

        unsigned char * ans = malloc(len ) ; 
        memcpy(ans , thing + pos   , len ) ; 
        *lenght = len ; 
        return ans  ; 
    }




    int compare_sort(const void * a, const void * b , void * sorter_e  ){
        sorter sort = (sorter *)sorter_e ; 
        sort_arr * first = (sort_arr * )a ; 
        first->key = get_data_sort(first , sort->cols_to_look ,first->len , first->key_type  ) ;  
        sort_arr * second = (sort_arr * )b ; 
        second->key = get_data_sort(second , sort->cols_to_look ,second->len , second->key_type  ) ;  
        int result = 0 ; 
        if ( first->key_type == integer_num && second->key_type == integer_num  ){
            long first_num ;
            memcpy(&first_num, first->key, sizeof(long));
            long second_num  ;
            memcpy(&second_num, second->key, sizeof(long));
            if (first_num < second_num){
                if (sort->keyinfo->dir== DESC  ){
                    result =  1 ; 
                }
                else { 
                    result = -1 ; 
                }
            }
            else if (first_num > second_num){
                if (sort->keyinfo->dir== DESC  ){
                    result =  -1 ; 
                }
                else { 
                    result = 1 ; 
                }
            }
            else {
                if (sort->cols_to_look + 1 < sort->capacity ){
                    sorter *temp = malloc(sizeof(sorter));
                    *temp = *sort;                    
                    temp->cols_to_look = sort->cols_to_look + 1;
                    result = campare_sort(a , b , (void*)temp ) ; 

                }
                else { 
                     result =  0 ; 
                }
            }
        }
        else if ( ( first->key_type == real_num && second->key_type == real_num )  ){
            float first_num ;
            memcpy(&first_num, first->key, sizeof(float));
            float second_num  ;
            memcpy(&second_num, second->key, sizeof(float));
            if (first_num < second_num){
                if (sort->keyinfo->dir== DESC  ){
                    result =  1 ; 
                }
                else { 
                    result =  -1 ; 
                }
            }
            else if (first_num > second_num){
                if (sort->keyinfo->dir== DESC  ){
                    result =  -1 ; 
                }
                else { 
                result =  1 ; 
                }
            }
            else {
                result =  0 ; 
            }
        }

        else if ( ( first->key_type == real_num && second->key_type == integer_num ) || ( first->key_type == integer_num && second->key_type == real_num )    ){
            float first_num;
            if (first->key_type == real_num) {
                memcpy(&first_num, first->key, sizeof(float));
            } 
            else if (first->key_type == integer_num) {
                long tmp;
                memcpy(&tmp, first->key, sizeof(long));
                first_num = (float)tmp;
            }

            float second_num;
            if (second->key_type == real_num) {
                memcpy(&second_num, second->key, sizeof(float));
            } 
            else if (second->key_type == integer_num) {
                long tmp;
                memcpy(&tmp, second->key, sizeof(long));
                second_num = (float)tmp;
            }
            if (first_num < second_num){
                if (sort->keyinfo->dir == DESC  ){
                    result =  1 ; 
                }
                else { 
                    result = -1 ; 
                }
            }
            else if (first_num > second_num){
                if (sort->keyinfo->dir == DESC  ){
                    result =  -1 ; 
                }
                else {
                    result = 1 ; 
                }
            }
            else {
                if (sort->cols_to_look + 1 < sort->capacity ){
                    sorter *temp = malloc(sizeof(sorter));
                    *temp = *sort;                    
                    temp->cols_to_look = sort->cols_to_look + 1;
                    result = campare_sort(a , b , (void*)temp ) ; 

                }
                else { 
                     result =  0 ; 
                }
            }
        }



        
        else if (first->key_type  == string_num && second->key_type == string_num ){
            if (sort->keyinfo.coll == BINARY  ){
                int num_1 = 0 ; 
                int num_2 = 0 ;
                int m = 0 ; 
                int n = 0  ; 
                while (  num_1 == num_2  ) {
                    if (m < first->len ){
                        num_1 = num_1 + (int)first->key[i] ; 
                        m++ ; 
                    } 
                    if ( n < second->len  ){
                        num_2 = num_2 + (int)second->key[j] ;        
                        n++ ;  
                    }      
                    if ( m >= first->len &&  n >= second->len   ) {
                        break ; 
                    }
                }
                if (num_2 > num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  1 ; 
                    }
                    else {
                        result = -1 ; 
                    }
                }
                else if (num_2 < num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else {
                if (sort->cols_to_look + 1 < sort->capacity ){
                    sorter *temp = malloc(sizeof(sorter));
                    *temp = *sort;                    
                    temp->cols_to_look = sort->cols_to_look + 1;
                    result = campare_sort(a , b , (void*)temp ) ; 

                }
                else { 
                     result =  0 ; 
                }
                }
            }    

            else if (sort->keyinfo.coll == NOCASE  ){
                int num_1 = 0 ; 
                int num_2 = 0 ;
                int m = 0 ; 
                int n = 0  ; 
                while (  num_1 == num_2  ) {
                    if (m < first->len ){
                        num_1 = num_1 + (int)first->key[i] ; 
                        m++ ; 
                    } 
                    if ( n < second->len  ){
                        num_2 = num_2 + (int)second->key[j] ;        
                        n++ ;  
                    }      
                    if ( m >= first->len &&  n >= second->len   ) {
                        break ; 
                    }
                }
                if (num_2 > num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  1 ; 
                    }
                    else { 
                        result = -1 ; 
                    }
                }
                else if (num_2 < num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else {
                if (sort->cols_to_look + 1 < sort->capacity ){
                    sorter *temp = malloc(sizeof(sorter));
                    *temp = *sort;                    
                    temp->cols_to_look = sort->cols_to_look + 1;
                    result = campare_sort(a , b , (void*)temp ) ; 

                }
                else { 
                     result =  0 ; 
                }
                }
            }   

            else if (sort->keyinfo.coll == RTRIM  ){
                int num_1 = 0 ; 
                int num_2 = 0 ;
                int m = 0 ; 
                int n = 0  ; 
                int len_1  = first->len -  1   ; 
                while( len_1 >= 0 && first->key[len_1] == ' '){
                    len_1-- ; 
                }
                len_1++ ; 

                int len_2  = second->len - 1   ; 
                while( len_2 >= 0 && second->key[len_2] == ' '){
                    len_2-- ; 
                }
                len_2++ ; 

                while (  num_1 == num_2  ) {
                    if (m <len_1 ){
                        num_1 = num_1 + (int)first->key[i]; 
                        m++ ; 
                    } 
                    if ( n < len_2 ){
                        num_2 = num_2 + (int)second->key[j]  ;        
                        n++ ;  
                    }      
                    if ( m >= len_1 &&  n >= len_2   ) {
                        break ; 
                    }
                }
                if (num_2 > num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  1 ; 
                    }
                    else { 
                        result = -1 ; 
                    }
                }
                else if (num_2 < num_1){
                    if (sort->keyinfo->dir== DESC  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else {
                    if (sort->cols_to_look + 1 < sort->capacity ){
                        sorter *temp = malloc(sizeof(sorter));
                        *temp = *sort;                    
                        temp->cols_to_look = sort->cols_to_look + 1;
                        result = campare_sort(a , b , (void*)temp ) ; 

                    }
                    else { 
                        result =  0 ; 
                    }
                }
            }   

        }
        free(first->key) ; 
        free(second->key) ; 
        return result ; 
    }


    int orderby_campare_sort(const void * a, const void * b , void * sorter_e){         
        sorter sort = (sorter *)sorter_e ;     
        int ob_counter = sort->row_count  / sizeof(orderby_key_cols) ; 
        orderby_key_cols *ob[300];

        for (int i = 0; i < ob_counter; i++) {
            ob[i] = malloc(sizeof(orderby_key_cols));
            memcpy(ob[i], buffer + (i * sizeof(orderby_key_cols)), sizeof(orderby_key_cols));
        }
        sort_arr * first = (sort_arr * )a ; 
        sort_arr * second = (sort_arr * )b ; 
        int result = 0 ; 
        for ( int i = 0 ; i < ob_counter ; i++ ){
            if (result != 0 ){
                break ; 
            }

            if (first->key != NULL) { 
                free(first->key); 
            }     

            if (second->key != NULL) { 
            free(second->key); 
            }  


            first->key = get_data_sort(first , ob[i]->reg_num ,first->len , first->key_type  ) ;  
            second->key = get_data_sort(second , ob[i]->reg_num  ,second->len , second->key_type  ) ;  
            if ( first->key_type == integer_num && second->key_type == integer_num  ){
                long first_num ;
                memcpy(&first_num, first->key, sizeof(long));
                long second_num  ;
                memcpy(&second_num, second->key, sizeof(long));
                if (first_num < second_num){
                    if (ob[i]->direction == 1  ){
                        result =  1 ; 
                    }
                    else { 
                        result = -1 ; 
                    }
                }
                else if (first_num > second_num) {
                    if (ob[i]->direction == 1  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else { 
                    result = 0 ; 
                }

            }
            else if ( ( first->key_type == real_num && second->key_type == real_num )  ){
                float first_num ;
                memcpy(&first_num, first->key, sizeof(float));
                float second_num  ;
                memcpy(&second_num, second->key, sizeof(float));
                if (first_num < second_num){
                    if (ob[i]->direction == 1  ){
                        result =  1 ; 
                    }
                    else { 
                        result = -1 ; 
                    }
                }
                else if (first_num > second_num) {
                    if (ob[i]->direction == 1  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else { 
                    result = 0 ; 
                }
            }
            else if ( ( first->key_type == real_num && second->key_type == integer_num ) || ( first->key_type == integer_num && second->key_type == real_num )    ){
                float first_num;
                if (first->key_type == real_num) {
                    memcpy(&first_num, first->key, sizeof(float));
                } 
                else if (first->key_type == integer_num) {
                    long tmp;
                    memcpy(&tmp, first->key, sizeof(long));
                    first_num = (float)tmp;
                }

                float second_num;
                if (second->key_type == real_num) {
                    memcpy(&second_num, second->key, sizeof(float));
                } 
                else if (second->key_type == integer_num) {
                    long tmp;
                    memcpy(&tmp, second->key, sizeof(long));
                    second_num = (float)tmp;
                }

                if (first_num < second_num){
                    if (ob[i]->direction == 1  ){
                        result =  1 ; 
                    }
                    else { 
                        result = -1 ; 
                    }
                }
                else if (first_num > second_num) {
                    if (ob[i]->direction == 1  ){
                        result =  -1 ; 
                    }
                    else { 
                        result = 1 ; 
                    }
                }
                else { 
                    result = 0 ; 
                }
            }      
            else if (first->key_type  == string_num && second->key_type == string_num ){
                if (sort->keyinfo.coll == BINARY  ){
                    int num_1 = 0 ; 
                    int num_2 = 0 ;
                    int m = 0 ; 
                    int n = 0  ; 
                    while (  num_1 == num_2  ) {
                        if (m < first->len ){
                            num_1 = num_1 + (int)first->key[i] ; 
                            m++ ; 
                        } 
                        if ( n < second->len  ){
                            num_2 = num_2 + (int)second->key[j] ;        
                            n++ ;  
                        }      
                        if ( m >= first->len &&  n >= second->len   ) {
                            break ; 
                        }
                    }
                        if (num_1 < num_2){
                            if (ob[i]->direction == 1  ){
                                result =  1 ; 
                            }
                            else { 
                                result = -1 ; 
                            }
                        }
                        else if (num_1 > num_2) {
                            if (ob[i]->direction == 1  ){
                                result =  -1 ; 
                            }
                            else { 
                                result = 1 ; 
                            }
                        }
                        else { 
                            result = 0 ; 
                        }

                }    

                else if (sort->keyinfo.coll == NOCASE  ){
                    int num_1 = 0 ; 
                    int num_2 = 0 ;
                    int m = 0 ; 
                    int n = 0  ; 
                    while (  num_1 == num_2  ) {
                        if (m < first->len ){
                            num_1 = num_1 + (int)to_lowercase(first->key[i]) ; 
                            m++ ; 
                        } 
                        if ( n < second->len  ){
                            num_2 = num_2 + (int)to_lowercase(second->key[j] )  ;        
                            n++ ;  
                        }      
                        if ( m >= first->len &&  n >= second->len   ) {
                            break ; 
                        }
                    }
                        if (num_1 < num_2){
                            if (ob[i]->direction == 1  ){
                                result =  1 ; 
                            }
                            else { 
                                result = -1 ; 
                            }
                        }
                        else if (num_1 > num_2) {
                            if (ob[i]->direction == 1  ){
                                result =  -1 ; 
                            }
                            else { 
                                result = 1 ; 
                            }
                        }
                        else { 
                            result = 0 ; 
                        }
                }   

                else if (sort->keyinfo.coll == RTRIM  ){
                    int num_1 = 0 ; 
                    int num_2 = 0 ;
                    int m = 0 ; 
                    int n = 0  ; 
                    int len_1  = first->len -  1   ; 
                    while( len_1 >= 0 && first->key[len_1] == ' '){
                        len_1-- ; 
                    }
                    len_1++ ; 

                    int len_2  = second->len - 1   ; 
                    while( len_2 >= 0 && second->key[len_2] == ' '){
                        len_2-- ; 
                    }
                    len_2++ ; 

                    while (  num_1 == num_2  ) {
                        if (m <len_1 ){
                            num_1 = num_1 + (int)first->key[i]; 
                            m++ ; 
                        } 
                        if ( n < len_2 ){
                            num_2 = num_2 + (int)second->key[j]  ;        
                            n++ ;  
                        }      
                        if ( m >= len_1 &&  n >= len_2   ) {
                            break ; 
                        }
                    }
                        if (num_1 < num_2){
                            if (ob[i]->direction == 1  ){
                                result =  1 ; 
                            }
                            else { 
                                result = -1 ; 
                            }
                        }
                        else if (num_1 > num_2) {
                            if (ob[i]->direction == 1  ){
                                result =  -1 ; 
                            }
                            else { 
                                result = 1 ; 
                            }
                        }
                        else { 
                            result = 0 ; 
                        }
                }   

            }
            else if (first->key_type == null && second->key_type != null  ){
                if (ob[i]->nullsfirst == 1 && ob[i]->nullslast == 0 ){
                    result = -1 ; 
                }
                else if (ob[i]->nullsfirst == 0 && ob[i]->nullslast == 1 ){
                    result = 1 ; 
                }
                else { 
                    result = 0  ; 
                }
            }
            else if (first->key_type != null && second->key_type == null  ){
                if (ob[i]->nullsfirst == 1 && ob[i]->nullslast == 0 ){
                    result = 1 ; 
                }
                else if (ob[i]->nullsfirst == 0 && ob[i]->nullslast == 1 ){
                    result = -1 ; 
                }
                else { 
                    result = 0  ; 
                }
            }
            else if (first->key_type == null && second->key_type == null  ){
                result = 0 ; 
                continue ; 
            }
    
        }
        free(first->key) ; 
        free(second->key) ; 
        return result ; 

}

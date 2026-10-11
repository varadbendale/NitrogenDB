#include<strings.h>
define PAGE_SIZE 4096
bytecode { 
    enum { 
        int_num = 1 , 
        blob_num , 
        string_num , 
        null , 
        integer_op   , 
        begin_op , 
        next_op , 
        print_op , 
        halt  , 
        blob_op , 
        string_op , 
        null_op , 
        real_op ,
        real_num , 
        add_op , 
        subs_op , 
        mul_op , 
        divide_op , 
        eq_op , 
        ne_op , 
        gt_op
        lt_op , 
        le_op , 
        ge_op , 
        end_num , 
        cursor_read , 
        boolean_op , 
        cursor_write  , 
        open_write_op , 
        open_read_op
    }

    typedef struct data_bin{
        union{
            long i ; 
            char *s ; 
            float r ; 
        }compare ; 
        union{
            long i ; 
            char *s ; 
            float r ; 
        }primary_key_first ; 
        union{
            long i ; 
            char *s ; 
            float r ; 
        }primary_key_second ; 
    }

    typedef struct bins{
        data_bin data[300] ; 
        int row_number ; 
        char * tbl ; 
        int num_of_entires ; 
    }

    typedef struct hash_bins_str{
        bins bn[300] ;
        int num_of_bins ; 
    }

    typedef enum collation { 
        BINARY = 1 ,
        NOCASE,
        RTRIM 
    } ;

    typedef enum direction { 
        ASC = 1 ,
        DESC 
    } ;

    typedef struct Pager{
        FILE *file ; 
    }

    typedef struct type{
        int operand ; 
        int p1 ; 
        int p2 ; 
        int p3 
        void * p4 ; 
        int p4_type ; 
    } ; 


    typedef struct reg{
        int type ; 
        int lenght ; 
        union{
            long i ; 
            char *s ; 
            float r ; 
        }val ; 
    }

    typedef struct path_entry{
        uint32_t page_num     ; 
        uint16_t  cell_index     ; 
    }


    typedef struct btree{
        table * Table ; 
        uint32_t start_root_num ; 
        uint32_t page_num ; 
        uint32_t row_num ; 
        bool start ; 
        int data_type ; 
        bool end ; 
        int total ; 
        int mode ; 
        int depth ; 
        path_entry stack[300] ; 
        bool null ; 
    }

    typedef struct aggregate{
        reg min ; 
        reg max ; 
        double total ; 
        int type ; 
        char *concat_string ; 
        int * string_len ; 
        bool have_value ; 
        int count ; 
    }

    typedef struct sort_arr{
        unsigned char * key ; 
        int key_type ; 
        unsigned char * array ; 
        int len ;  
    }

    typedef struct Key_info{
        int feild ; 
        collation * coll  ; 
        direction * dir  ; 
    }

    typedef struct sorter{
        int row_count ; 
        sort_arr * array ; 
        int capacity ;
        int keycols;  
        int cursor ; 
        int  cols_to_look ;  
        Key_info * keyinfo ; 
    }


    int count = 0 ; 
    sorter[op->p1]->array[count].key = get_data_sort( sorter[op->p1]->array[count] , sorter[op->p1].cols_to_look  ) ; 



    unsigned char * make_record_data(byte * byt , int start , int end ){
        unsigned char *ans[300] ; 
        int pos = 0 ; 
        reg temp = byt->regis[start] ; 
        for ( int i = start ; i <= end ; i++ ){
            temp = byt->regis[i] ; 
            memcpy( ans + pos  , temp->type  , 1   ) ; 
            pos = pos + 1 ; 
            if (temp->type == integer_num ){
                memcpy(ans + pos , temp->val.i , sizeof(int)) ; 
                pos  = pos + sizeof(int) ; 
            }
            else if (temp->type == real_num ){
                memcpy(ans + pos , temp->val.i , sizeof(float)) ; 
                pos  = pos + sizeof(float) ; 
            }
            else if (temp->type == string_num || temp->type == blob_num  ){
                memcpy( ans + pos , temp->lenght , sizeof(int)) ; 
                pos = pos + sizeof(int) ; 
                memcpy(ans + pos , temp->val.s , temp->lenght ) ; 
                pos = pos + temp->lenght  ; 
            }
        }
        return ans ; 
    }

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

    typedef struct byte{
        type prog[300] ; 
        int prog_count ; 
        int ins_count ; 
        reg regis[300] ; 
        int reg_start ; 
        int reg_end ; 
        int reg_mode ; 
        hash_bins_str * hash_bins ; 
        aggregate agg[300] ; 
        btree btr[300] ; 
        table *db ; 
        Pager *pager ; 
        sorter * sort ; 
    }

    typedef struct page_header{
        uint8_t page_type ; 
        uint16_t free_space ; 
        uint16_t num_cells  ; 
    }__attribute__((packed)) 


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
    void *pager_get_page(Pager *pager, uint32_t page_num) {
        void *buf = malloc(PAGE_SIZE);
        fseek(pager->file, page_num * PAGE_SIZE, SEEK_SET);
        fread(buf, 1, PAGE_SIZE, pager->file);
        return buf;
    }
    uint32_t float_to_int_bits(float f) {
        if (f == 0.0f) f = 0.0f; 
        uint32_t bits;
        memcpy(&bits, &f, sizeof(f));
        return bits;
    }

    void hash_bin_which(reg check , int nume ){
        if (check->type == int_num  ){
            return check->val->i % nume
        }
        else if (check->type == real_num ){
            return float_to_int_bits(check->val->r) % nume  ; 
        }
        else if (check->type == blob_num && check->type == string_num  ){
            int i = 0 ; 
            int num = 0 ; 
            while ( i < check->lenght){
                num = num + check->val->s[i] - 'a' ; 
            }
            return num % nume  ; 
        }
    }

    uint32_t get_child_pointer(Pager * pager ,uint32_t page_num , uint16_t cell_index ){
        uint8_t *raw = pager_get_page(pager , page_num ) ; 
        uint16_t *cell_offset = sizeof(page_header) + ( cell_index * CELL_SIZE ) ; 
        uint32_t *addr  ; 
        memcpy(addr ,raw + cell_offset , sizeof(uint32_t) ) ; 
        return addr ; 
    }

    void *get_cell(void *node, uint32_t cell_num) {
        char *base = (char*)node;              
        return base + sizeof(page_header) + (cell_num * CELL_SIZE);
    }

    int campare_ge(reg target , void * cell_key , int  data_type ){
      if (target->type == integer_num){
            if (data_type == 1 ){
                uint32_t num = *(uint32_t*)cell_key ; 
                if (num >= target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                int num_temp = int(num) ; 
                if (num_temp >= target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else {
                return -1 ; 
            }
        }
        else if ( target->type == real_num ){
            if (data_type == 1 ){
                uint32_t num_temp = *(uint32_t*)cell_key ; 
                float num = (float)num_temp ; 
                if (num >= target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                if (num >= target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else { 
                return -1 ; 
            }
        }
        else { 
            if (data_type == 3 ){
                char *str_temp = (char *)cell_key; 
                int len = strlen(str_temp) ; 
                    if (strlen(target->val.s) >= (size_t)len ? strncmp(target->val.s, str_temp, len) >= 0 : strncmp(target->val.s, str_temp, strlen(target->val.s)) > 0){
                        return 0 ; 
                    }
                    else { 
                        return -1 
                    } 
            }
            else { 
                return -1 ; 
            }
        }
    }

    int campare_gt(reg target , void * cell_key , int  data_type ){
     if (target->type == integer_num){
            if (data_type == 1 ){
                uint32_t num = *(uint32_t*)cell_key ; 
                if (num > target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                int num_temp = int(num) ; 
                if (num_temp > target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else {
                return -1 ; 
            }
        }
        else if ( target->type == real_num ){
            if (data_type == 1 ){
                uint32_t num_temp = *(uint32_t*)cell_key ; 
                float num = (float)num_temp ; 
                if (num > target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                if (num > target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else { 
                return -1 ; 
            }
        }
        else { 
            if (data_type == 3 ){
                char *str_temp = (char *)cell_key; 
                int len = strlen(str_temp) ; 
                    if (strlen(target->val.s) > (size_t)len ? strncmp(target->val.s, str_temp, len) >= 0 : strncmp(target->val.s, str_temp, strlen(target->val.s)) > 0){
                        return 0 ; 
                    }
                    else { 
                        return -1 
                    } 
            }
            else { 
                return -1 ; 
            }
        }
    }

    int campare_lt(reg target , void * cell_key , int  data_type ){
     if (target->type == integer_num){
            if (data_type == 1 ){
                uint32_t num = *(uint32_t*)cell_key ; 
                if (num < target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                int num_temp = int(num) ; 
                if (num_temp < target->val.i){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else {
                return -1 ; 
            }
        }
        else if ( target->type == real_num ){
            if (data_type == 1 ){
                uint32_t num_temp = *(uint32_t*)cell_key ; 
                float num = (float)num_temp ; 
                if (num < target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            if (data_type == 2 ){
                float num = *(float*)cell_key ; 
                if (num < target->val.r){
                    return 0 ; 
                }
                else {
                    return -1 ; 
                }
            }
            else { 
                return -1 ; 
            }
        }
        else { 
            if (data_type == 3 ){
                char *str_temp = (char *)cell_key; 
                int len = strlen(str_temp) ; 
                    if (strlen(target->val.s) < (size_t)len ? strncmp(target->val.s, str_temp, len) >= 0 : strncmp(target->val.s, str_temp, strlen(target->val.s)) > 0){
                        return 0 ; 
                    }
                    else { 
                        return -1 
                    } 
            }
            else { 
                return -1 ; 
            }
        }
    }

    int campare_le(reg target , void * cell_key , int  data_type ){
        if (target->type == integer_num){
                if (data_type == 1 ){
                    uint32_t num = *(uint32_t*)cell_key ; 
                    if (num <= target->val.i){
                        return 0 ; 
                    }
                    else {
                        return -1 ; 
                    }
                }
                else if (data_type == 2 ){
                    float num = *(float*)cell_key ; 
                    int num_temp = int(num) ; 
                    if (num_temp <= target->val.i){
                        return 0 ; 
                    }
                    else {
                        return -1 ; 
                    }
                }
                else {
                    return -1 ; 
                }
            }
            else if ( target->type == real_num ){
                if (data_type == 1 ){
                    uint32_t num_temp = *(uint32_t*)cell_key ; 
                    float num = (float)num_temp ; 
                    if (num <= target->val.r){
                        return 0 ; 
                    }
                    else {
                        return -1 ; 
                    }
                }
                if (data_type == 2 ){
                    float num = *(float*)cell_key ; 
                    if (num <= target->val.r){
                        return 0 ; 
                    }
                    else {
                        return -1 ; 
                    }
                }
                else { 
                    return -1 ; 
                }
            }
            else { 
                if (data_type == 3 ){
                    char *str_temp = (char *)cell_key; 
                    int len = strlen(str_temp) ; 
                        if (strlen(target->val.s) <= (size_t)len ? strncmp(target->val.s, str_temp, len) >= 0 : strncmp(target->val.s, str_temp, strlen(target->val.s)) > 0){
                            return 0 ; 
                        }
                        else { 
                            return -1 
                        } 
                }
                else { 
                    return -1 ; 
                }
            }
    }

    int bs( void * node , reg target  , int data_type  , int *index  , int (*function_name)(reg, void*, int)){
        int low = 0 ; 
        int high = node->num_cells - 1 ; 
        int cmp = -1  ;
        bool found = false;  
        while ( low <= high ){
            int mid = ( low + high )/ 2 ; 
            void * cell = get_cell(node , mid ) ; 
            reg cell_key = get_key_from_cell(cell);
            cmp = function_name(target , cell_key  , data_type  ) ; 
            if (cmp == 0 ){
                found = true ; 
                if (index != NULL){
                    *index = mid ; 
                }
                high = mid -1 ; 
            }
            else { 
                low = mid + 1 ; 
            }
        }
        if ( found == true ){
            return 1 ; 
        }
        return 0 ; 

    }

    void agg_operation( aggregate *ans  , reg *num ,  char *  operation ){
        if (strcmp(operation , "SUM") == 0 ){
            if (num->type == integer_num ) {
            ans->total =  ans->total + num->val.i ; 
            ans->type = integer_num ; 
            }
            else if (num->type == real_num  ){
            ans->total =  ans->total + num->val.r ;         
            ans->type = real_num ;
            }
            else {
                return  ; 
            }
        }
        else if (strcmp(operation , "AVG") == 0){
            if (num->type == integer_num ) {
            ans->total =  ans->total + num->val.i ; 
            ans->type = integer_num ; 
            }
            else if (num->type == real_num  ){
            ans->total =  ans->total + num->val.r ;    
            ans->type = real_num  ; 
            } 
            else {
                return  ; 
            }
            ans->count = ans->count + 1 ; 
        }
        else if (strcmp(operation , "COUNT") == 0 ){
            ans->count = ans->count + 1 ; 
        }
        else if (strcmp(operation , "MIN") == 0  ){
        if (ans->have_value == false ){
                if (num->type == integer_num){
                    ans->min.val.i = num->val.i ; 
                    ans->have_value = true ; 
                    ans->type = integer_num ; 
                }
                else if (num->type == real_num){
                    ans->min.val.r = num->val.r ; 
                    ans->have_value = true ; 
                    ans->type = real_num ; 
                }
                else {
                    return  ; 
                }
        } 
        else {
                if (num->type == integer_num){
                    ans->type = integer_num ; 
                    if (num->val.i < ans->min.val.i ){
                        ans->min.val.i = num->val.i ; 
                    }
                }
                else if (num->type == real_num){
                    ans->type = real_num ; 
                    if (num->val.r < ans->min.val.r ){
                        ans->min.val.r = num->val.r ; 
                    }
                }
                else {
                    return  ; 
                }
        }
        }

        else if (strcmp(operation , "MAX") == 0  ){
        if (ans->have_value == false ){
                if (num->type == integer_num){
                    ans->max.val.i = num->val.i ; 
                    ans->have_value = true ; 
                    ans->type = integer_num ; 
                }
                else if (num->type == real_num){
                    ans->max.val.r = num->val.r ; 
                    ans->have_value = true ; 
                    ans->type = real_num ; 
                }
                else {
                    return  ; 
                }
        } 
        else {
                if (num->type == integer_num){
                    ans->type = integer_num ; 
                    if (num->val.i > ans->max.val.i ){
                        ans->max.val.i = num->val.i ; 
                    }
                }
                else if (num->type == real_num){
                    ans->type = real_num ; 
                    if (num->val.r > ans->max.val.r ){
                        ans->max.val.r = num->val.r ; 
                    }
                }
                else {
                    return  ; 
                }
        }
        }

        else if (strcmp(operation , "GROUP_CONCAT") == 0 ){
            if (num->type  != string_num ){
                return  ; 
            }
            ans->type = string_num ; 
            if ( ans->concat_string == NULL ){
                ans->concat_string = malloc(num->lenght) ; 
                ans->string_len = num->lenght ; 
                memcpy(&ans->concat_string  ,num->val.s  , num->lenght  ) ; 
            }
            else  {
                ans->concat_string = realloc(ans->concat_string  , ans->string_len + num->lenght ) ; 
                strcat(ans->concat_string , num->val.s ) ; 
            }

        }
        return ; 
    }

    void agg_final(aggregate *ans  , reg *num ,  char *  operation ){
            if (strcmp(operation , "SUM") == 0 ){
                if (ans->type == integer_num ) {
                    num->type = integer_num ; 
                    num->val.i = ans->total ; 
                }
                else if (ans->type == real_num  ){
                    num->type = real_num ; 
                    num->val.r = ans->total ;  
                }
                }

            else if (strcmp(operation , "AVG") == 0 ){
                num->type = real_num ; 
                num->val.r = ans->total/ans->count ; 
            }
            
            else if (strcmp(operation , "COUNT") == 0 ){
                num->val.i = ans->count ; 
            }
            else if (strcmp(operation , "MIN") == 0 ){
                if (ans->min.type == integer_num){
                    num->val.i = ans->min.val.i ; 
                }
                else if (ans->min.type == real_num){
                    num->val.r = ans->min.val.r ; 
                }
            }
            else if (strcmp(operation , "MAX") == 0 ){
                if (ans->max.type == integer_num){
                    num->val.i = ans->max.val.i ; 
                }
                else if (ans->max.type == real_num){
                    num->val.r = ans->max.val.r ; 
                }
            }
            else if (strcmp(operation , "GROUP_CONCAT") == 0 ){
                num->lenght = ans->string_len ; 
                num->val.s = ans->concat_string  ;    
            }
            return ; 
    }

    void bytcode(byte *byt){
        byt->prog_count = 0;
        while(1){
            type *op = &byt->prog[byt->prog_count];
            switch(op->operand){
                case begin_op:
                    break;

                case goto_op:
                    byt->pc = op->p2;
                    continue;

                case halt:
                    return;

                case check_negative : 
                    if (byt->regis[op->p1].type == int_num   ){
                        if (byt->regis[op->p1].val.i < 0 || ){
                            trap_fired = 1 ; 
                        }
                        else { 
                            trap_fired = 0 ;     
                        }
                    }
                    if ( byt->regis[op->p1].type == real_num){
                        if (byt->regis[op->p1].val.r < 0 || ){
                            trap_fired = 1 ; 
                        }
                        else { 
                            trap_fired = 0 ;     
                        }
                    }
                    break ; 

                case integer_op:
                    byt->regis[op->p2].type = int_num;
                    byt->regis[op->p2].val.i = op->p1;
                    break;

                case blob_op:
                    byt->regis[op->p2].type = blob_num;
                    byt->regis[op->p2].val.s = char*(op->p4);
                    byt->regis[op->p2].lenght = op->p1;
                    break;

                case string_op:
                    byt->regis[op->p2].type = string_num;
                    byt->regis[op->p2].lenght = op->p1;
                    byt->regis[op->p2].val.s = char*(op->p4);
                    break;


                case null_op:
                    byt->regis[op->p2].type = null;
                    break;

                case real_op:
                    byt->regis[op->p2].type = real_num;
                    byt->regis[op->p2].val.r = (float*)op->p4;
                    break;

                case copy_op:
                    byt->regis[op->p2].type = byt->regis[op->p1].type;
                    byt->regis[op->p2].lenght = byt->regis[op->p1].lenght;

                    if (byt->regis[op->p2].type == int_num){
                        byt->regis[op->p2].val.i = byt->regis[op->p1].val.i;
                    }

                    if (byt->regis[op->p2].type == real_op){
                        byt->regis[op->p2].val.r = byt->regis[op->p1].val.r;
                    }

                    if (byt->regis[op->p2].type == string_num || byt->regis[op->p2].type == blob_num){
                        byt->regis[op->p2].val.s = malloc(byt->regis[op->p2].lenght);
                        memcpy(byt->regis[op->p2].val.s, byt->regis[op->p1].val.s, byt->regis[op->p2].lenght);
                    }
                    break;

                case move_op:
                    if (byt->regis[op->p1].type == string_num || byt->regis[op->p1].type == blob_num){
                        free(byt->regis[op->p1].val.s);
                    }

                    if (byt->regis[op->p1].type == integer_num || byt->regis[op->p1].type == real_num){
                        break;
                    }

                    byt->regis[op->p1].type = null;
                    byt->regis[op->p2].val.s = byt->regis[op->p1].val.s;
                    byt->regis[op->p1].val.s = NULL;
                    break;



                    
                case add_op:
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                        byt->regis[op->p3].type = integer_num ;
                        byt->regis[op->p3].val.i  = byt->regis[op->p2].val.i + byt->regis[op->p1].val.i  ; 
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r + byt->regis[op->p1].val.r  ;   
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r + num ;   
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p2].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = num+ byt->regis[op->p1].val.r   ;   
                    }

                    break ; 


                case subs_op:
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                        byt->regis[op->p3].type = integer_num ; 
                        byt->regis[op->p3].val.i  = byt->regis[op->p1].val.i - byt->regis[op->p2].val.i  ; 
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r - byt->regis[op->p2].val.r  ;   
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = num - byt->regis[op->p2].val.r ;   
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p2].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r - num ;    
                    }

                    break ; 



                case mul_op:
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                        byt->regis[op->p3].type = integer_num ;
                        byt->regis[op->p3].val.i  = byt->regis[op->p2].val.i * byt->regis[op->p1].val.i  ; 
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r * byt->regis[op->p1].val.r  ;   
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p2].val.r * num ;   
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p2].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = num *  byt->regis[op->p1].val.r   ;   
                    }
                    break ; 
                

                case divide_op:
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num  ){
                        byt->regis[op->p3].type = integer_num ; 
                        byt->regis[op->p3].val.i  = byt->regis[op->p1].val.i  / byt->regis[op->p2].val.i  ; 
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num  ){
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r  / byt->regis[op->p2].val.r  ;   
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = num  /  byt->regis[op->p2].val.r ;   
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p2].val.i ) ; 
                        byt->regis[op->p3].type = real_num ; 
                        byt->regis[op->p3].val.r  = byt->regis[op->p1].val.r / num ;    
                    }
                    break ; 
                





                case eq_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i == byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r == byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num ==  byt->regis[op->p3].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num ==  byt->regis[op->p1].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            if (strcmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) == 0 ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    break ; 


                case ne_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i != byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                           byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r != byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num !=  byt->regis[op->p3].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num !=  byt->regis[op->p1].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i= 0  ;
                        }
                    }
                    
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            if (strcmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) != 0 ){
                            byt->regis[op->p2].val.i= 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i= 0  ;
                        }
                    }
                    break ; 




                case gt_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i < byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r < byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num <  byt->regis[op->p3].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num <  byt->regis[op->p1].val.r){
                             byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 > num_1){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                      }
                      break ; 



                case lt_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i > byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r > byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num >  byt->regis[op->p3].val.r){
                             byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num > byt->regis[op->p1].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    



                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 < num_1){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    break ; 



                case le_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i >= byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r >= byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num >=  byt->regis[op->p3].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num >= byt->regis[op->p1].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    



                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 <= num_1){
                                byt->regis[op->p2].val.i = 1  ;
                            }
                            else { 
                                byt->regis[op->p2].val.i = 0  ;
                            }
                    }
                    break ; 




                case ge_select_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i <= byt->regis[op->p3].val.i ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r <= byt->regis[op->p3].val.r ){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num <=  byt->regis[op->p3].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num <=  byt->regis[op->p1].val.r){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                    }
                    
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 >= num_1){
                            byt->regis[op->p2].val.i = 1  ;
                        }
                        else { 
                            byt->regis[op->p2].val.i = 0  ;
                        }
                      }
                      break ; 



                case eq_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i == byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r == byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num ==  byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num ==  byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            if (strcmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) == 0 ){
                                byt->pc = op->p2;
                            }
                        }
                        else  if (strcmp(op->p4 , "NOCASE") == 0 ){
                            if (strcasecmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) == 0 ){
                                byt->pc = op->p2;
                            }
                        }
                        else if (strcmp(op->p4 , "RTRIM") == 0 ){
                            int num ; 
                            int max_num ; 
                            char * temp ; 
                            if (byt->regis[op->p3].lenght < byt->regis[op->p1].lenght){
                                num = byt->regis[op->p3].lenght  ; 
                                max = byt->regis[op->p1].lenght  ; 
                                temp = ,byt->regis[op->p3].val.s   ; 
                            }
                            else { 
                                num = byt->regis[op->p1].lenght   ; 
                                max = byt->regis[op->p3].lenght  ; 
                                temp = ,byt->regis[op->p1].val.s   ; 
                            }
                            int take = 0 ; 
                            for ( int i = 0 ; i < num ; i++ ){
                                if (byt->regis[op->p1].val.s[i] == byt->regis[op->p3].val.s[i] ){
                                    continue ; 
                                }
                                else { 
                                    take = 1 ; 
                                    break ; 
                                }
                            }
                            if ( take == 1 ){
                                break ; 
                            }
                            else { 
                            if (num < max_num ){
                            for ( int i = num ; i < max_num ; i++ ){
                                if (temp[i] == ' '){
                                    continue ; 
                                }
                                else { 
                                    take = 1 ; 
                                    break ; 
                                }
                            }
                            }
                        }
                            if ( take == 0 ){
                                byt->pc = op->p2;
                            }
                        }
                    }
                    break ; 





                case gt_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i < byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r < byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num <  byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num <  byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 > num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }                
                        
                        else if (strcmp(op->p4 , "NOCASE") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)to_lowercase(byt->regis[op->p1].val.s[i]); 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)to_lowercase(byt->regis[op->p3].val.s[i]);        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 > num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }

                        else if (strcmp(op->p4 , "RTRIM") == 0){
                            int len_1  = byt->regis[op->p1].lenght -  1   ; 
                            while( len_1 >= 0 && byt->regis[op->p1].val.s[len_1] == ' '){
                                len_1-- ; 
                            }
                            len_1++ ; 

                            int len_2  = byt->regis[op->p3].lenght - 1   ; 
                            while( len_2 >= 0 && byt->regis[op->p3].val.s[len_2] == ' '){
                                len_2-- ; 
                            }
                            len_2++ ; 

                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < len_1 ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i]; 
                                    i++ ; 
                                } 
                                if ( j < len_2  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[i]; 
                                    j++ ;  
                                }      
                                if ( i >= len_1 &&  j >= len_2  ) {
                                    break ; 
                                }
                            }
                            if (num_2 > num_1){
                                byt->pc = op->p2;
                            }
                            break ; 

                        }


                    }
                    break ; 







                case lt_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i > byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r > byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num >  byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num > byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 < num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }                
                        
                        else if (strcmp(op->p4 , "NOCASE") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)to_lowercase(byt->regis[op->p1].val.s[i]); 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)to_lowercase(byt->regis[op->p3].val.s[i]);        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 < num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }

                        else if (strcmp(op->p4 , "RTRIM") == 0){
                            int len_1  = byt->regis[op->p1].lenght -  1   ; 
                            while( len_1 >= 0 && byt->regis[op->p1].val.s[len_1] == ' '){
                                len_1-- ; 
                            }
                            len_1++ ; 

                            int len_2  = byt->regis[op->p3].lenght - 1   ; 
                            while( len_2 >= 0 && byt->regis[op->p3].val.s[len_2] == ' '){
                                len_2-- ; 
                            }
                            len_2++ ; 

                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < len_1 ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i]; 
                                    i++ ; 
                                } 
                                if ( j < len_2  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[i]; 
                                    j++ ;  
                                }      
                                if ( i >= len_1 &&  j >= len_2  ) {
                                    break ; 
                                }
                            }
                            if (num_2  < num_1){
                                byt->pc = op->p2;
                            }
                            break ; 

                        }


                    }
                    break ; 




                case le_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i >= byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r >= byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num >=  byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num >= byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 <= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }                
                        
                        else if (strcmp(op->p4 , "NOCASE") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)to_lowercase(byt->regis[op->p1].val.s[i]); 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)to_lowercase(byt->regis[op->p3].val.s[i]);        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 <= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }

                        else if (strcmp(op->p4 , "RTRIM") == 0){
                            int len_1  = byt->regis[op->p1].lenght -  1   ; 
                            while( len_1 >= 0 && byt->regis[op->p1].val.s[len_1] == ' '){
                                len_1-- ; 
                            }
                            len_1++ ; 

                            int len_2  = byt->regis[op->p3].lenght - 1   ; 
                            while( len_2 >= 0 && byt->regis[op->p3].val.s[len_2] == ' '){
                                len_2-- ; 
                            }
                            len_2++ ; 

                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < len_1 ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i]; 
                                    i++ ; 
                                } 
                                if ( j < len_2  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[i]; 
                                    j++ ;  
                                }      
                                if ( i >= len_1 &&  j >= len_2  ) {
                                    break ; 
                                }
                            }
                            if (num_2  <= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 

                        }


                    }
                    break ; 






                case ge_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i <= byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r <= byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num <= byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num <=  byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i] ; 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[j] ;        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 >= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }                
                        
                        else if (strcmp(op->p4 , "NOCASE") == 0 ){
                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < byt->regis[op->p1].lenght ){
                                    num_1 = num_1 + (int)to_lowercase(byt->regis[op->p1].val.s[i]); 
                                    i++ ; 
                                } 
                                if ( j < byt->regis[op->p3].lenght  ){
                                    num_2 = num_2 + (int)to_lowercase(byt->regis[op->p3].val.s[i]);        
                                    j++ ;  
                                }      
                                if ( i >= byt->regis[op->p1].lenght &&  j >= byt->regis[op->p3].lenght  ) {
                                    break ; 
                                }
                            }
                            if (num_2 >= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 
                        }

                        else if (strcmp(op->p4 , "RTRIM") == 0){
                            int len_1  = byt->regis[op->p1].lenght -  1   ; 
                            while( len_1 >= 0 && byt->regis[op->p1].val.s[len_1] == ' '){
                                len_1-- ; 
                            }
                            len_1++ ; 

                            int len_2  = byt->regis[op->p3].lenght - 1   ; 
                            while( len_2 >= 0 && byt->regis[op->p3].val.s[len_2] == ' '){
                                len_2-- ; 
                            }
                            len_2++ ; 

                            int num_1 = 0 ; 
                            int num_2 = 0 ;
                            int i = 0 ; 
                            int j = 0  ; 
                            while (  num_1 == num_2  ) {
                                if (i < len_1 ){
                                    num_1 = num_1 + (int)byt->regis[op->p1].val.s[i]; 
                                    i++ ; 
                                } 
                                if ( j < len_2  ){
                                    num_2 = num_2 + (int)byt->regis[op->p3].val.s[i]; 
                                    j++ ;  
                                }      
                                if ( i >= len_1 &&  j >= len_2  ) {
                                    break ; 
                                }
                            }
                            if (num_2 >= num_1){
                                byt->pc = op->p2;
                            }
                            break ; 

                        }


                    }
                    break ; 



                case ne_op : 
                    if ( byt->regis[op->p1].type == integer_num && byt->regis[op->p3].type == integer_num  ){
                        if (byt->regis[op->p1].val.i != byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == real_num  ){
                        if (byt->regis[op->p1].val.r != byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && byt->regis[op->p1].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p1].val.i ) ; 
                        if (num !=  byt->regis[op->p3].val.r){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p1].type == real_num && byt->regis[op->p3].type == integer_num   ){
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (num !=  byt->regis[op->p1].val.r){
                            byt->pc = op->p2;
                        }
                    }
                    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(op->p4 , "BINARY") == 0 ){
                            if (strcmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) != 0 ){
                                byt->pc = op->p2;
                            }
                        }
                        else  if (strcmp(op->p4 , "NOCASE") == 0 ){
                            if (strcasecmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) != 0 ){
                                byt->pc = op->p2;
                            }
                        }
                        else if (strcmp(op->p4 , "RTRIM") == 0 ){
                            int num ; 
                            int max_num ; 
                            char * temp ; 
                            if (byt->regis[op->p3].lenght < byt->regis[op->p1].lenght){
                                num = byt->regis[op->p3].lenght  ; 
                                max = byt->regis[op->p1].lenght  ; 
                                temp = byt->regis[op->p3].val.s   ; 
                            }
                            else { 
                                num = byt->regis[op->p1].lenght   ; 
                                max = byt->regis[op->p3].lenght  ; 
                                temp = byt->regis[op->p1].val.s   ; 
                            }
                            int take = 0 ; 
                            for ( int i = 0 ; i < num ; i++ ){
                                if (byt->regis[op->p1].val.s[i] != byt->regis[op->p3].val.s[i] ){
                                    continue ; 
                                }
                                else { 
                                    take = 1 ; 
                                    break ; 
                                }
                            }
                            if ( take == 1 ){
                                break ; 
                            }
                            else { 
                                if ( num < max_num ){
                                    for ( int i = num ; i < max_num ; i++ ){
                                        if (temp[i] == ' '){
                                            take = 1  ; 
                                        }
                                        else { 
                                            take = 0 ; 
                                            break ; 
                                        }
                                    }
                                    }
                            }
                            if ( take == 0 ){
                                byt->pc = op->p2;
                            }
                        }
                    }
                    break ; 

                case open_read_op : 
                    byt->btr[op->p1].Table  = lookup_table(db , op->p2) ; 
                    byt->btr[op->p1].start_root_num  =  op->p2 ;
                    byt->btr[op->p1].page_num  =  op->p2 ;
                    byt->btr[op->p1].mode  = cursor_read ; 
                    end = false ; 
                
                    
                case open_write_op : 
                    byt->btr[op->p1].Table  = lookup_table(db , op->p2) ; 
                    byt->btr[op->p1].start_root_num  =  op->p2 ;
                    byt->btr[op->p1].page_num  =  op->p2 ;
                    byt->btr[op->p1].mode  = cursor_write ; 
                    end = false ; 
                    break ; 
                
                case close_cursor_op : 
                    if (byt->btr[op->p1] ==  NULL){
                        free(byt->btr[op->p1]) ; 
                        byt->btr[op->p1] =  NULL
                    }
                    break ; 
                
                case next_cursor : 
                    path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                    page_header *hdr = pager_get_page(byt->pager , ptr->page_num ) ; 
                    if (byt->btr[op->p1].end == true ){
                        break ; 
                    }
                    if (ptr->cell_index + 1 < hdr->num_cells ){
                        byt->btr[op->p1].row_num++ ; 
                        byt->btr[op->p1].page_num = ptr->page_num ; 
                        ptr->cell_index++ ; 
                        break ; 
                    }
                    int kept = 0 ; 
                    byt->btr[op->p1].depth = byt->btr[op->p1].depth - 1  ; 
                    while ( byt->btr[op->p1].depth > 0 ){
                        ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                        hdr = pager_get_page(byt->pager , ptr->cell_index) ; 
                        if (ptr->cell_index + 1 < hdr->num_cells ){
                                ptr->cell_index++ ; 
                                uint32_t children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                                page_header chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                    while ( chdr->page_type != LEAF ){
                                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                                        children = get_child_pointer(byt->pager , ptr->page_num  ,0 )  ; 
                                        chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                    }
                                byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children , 0 } ; 
                                byt->btr[op->p1].page_num = children ; 
                                byt->btr[op->p1].row_num++ ;  
                                kept = 1 ; 
                                break ; 
                        }
                        byt->btr[op->p1].depth-- ; 
                    }
                    if (kept == 1 ){
                       byt->pc = op->p2;   
                        continue  ; 
                    }
                    byt->btr[op->p1].end  = true ; 
                    byt->pc++ ;
                    break ; 



                case prev_cursor : 
                    path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                    page_header *hdr = pager_get_page(byt->pager , ptr->page_num ) ; 
                    if (byt->btr[op->p1].start == true ){
                        break ; 
                    }
                    if (ptr->cell_index > 0 ){
                        byt->btr[op->p1].row_num-- ; 
                        byt->btr[op->p1].page_num = ptr->page_num ; 
                        ptr->cell_index-- ; 
                        break ; 
                    }                  
                    int kept = 0 ; 
                    byt->btr[op->p1].depth = byt->btr[op->p1].depth - 1  ; 
                    while (byt->btr[op->p1].depth  > 0 ){
                        hdr = pager_get_page(byt->pager , byt->btr[op->p1].page_num) ; 
                        ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                        if (ptr->cell_index > 0 ){
                            ptr->cell_index-- ; 
                            uint32_t children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                            page_header *chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                while ( chdr->page_type != LEAF ){  
                                    byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children ,chdr->num_cells - 1 } ; 
                                    children = get_child_pointer(byt->pager , children  ,chdr->num_cells - 1 )  ; 
                                    chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                }
                                byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , chdr->num_cells - 1   } ; 
                                byt->btr[op->p1].page_num = children ; 
                                byt->btr[op->p1].row_num-- ;  
                                kept = 1 ; 
                                break ; 
                        }
                        byt->btr[op->p1].depth-- ; 
                    }
                    if (kept == 1 ){
                        break ; 
                    }
                    byt->btr[op->p1].start = true 
                    
                    break ; 


                case rewind_cursor : 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , 0 } ; 
                    while ( hdr->page_type != LEAF ){
                        children = get_child_pointer(byt->pager , children  ,0 )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                    }
                    byt->btr[op->p1].page_num = children ; 
                    byt->btr[op->p1].row_num = 0 ; 
                    break ; 

                
                case cursor_end : 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , hdr->num_cells-1 } ; 
                    while ( hdr->page_type != LEAF ){
                        children = get_child_pointer(byt->pager , children  ,hdr->num_cells-1  )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , hdr->num_cells-1  } ; 
                    }
                    byt->btr[op->p1].page_num = children ; 
                    byt->btr[op->p1].row_num = byt->btr[op->p1].total - 1   ; 
                    break ; 

                case seek_ge : 
                    int size = 0 ; 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , 0 } ; 
                    while ( hdr->page_type != LEAF ){
                        int id = hdr->num_cells ; 
                        for ( int i = 0 ; i < hdr->num_cells ; i++  ){
                            void *cell = get_cell(hdr, i);
                            reg cell_key = get_key_from_cell(cell);
                            if (campare(byt->regis[op->p3], cell_key, cur->data_type) == 0) {
                                id = i;
                                break ;
                            }
                        }
                        children = get_child_pointer(byt->pager , children  ,id )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        size = size + hdr->num_cells ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , id } ; 
                    }

                    int tot = byt->btr[op->p1].total - 1  ; 
                    int ans_id = -1 ; 
                    int found = 0 ; 
                    while ( size < tot  ){
                        if(bs(hdr ,byt->regis[op->p3]  , byt->btr[op->p1].data_type ,&ans_id ,  campare_ge ) == 1 ){
                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , ans_id } ; 
                            byt->btr[op->p1].page_num = children ; 
                            byt->btr[op->p1].row_num = ans_id ; 
                            found = 1 ; 
                            break ; 
                        }

                        int kept = 0 ; 
                        while ( byt->btr[op->p1].depth > 0 ){
                            path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                            page_header * hdr = pager_get_page(byt->pager , ptr->page_num) ; 
                            if (ptr->cell_index + 1 < hdr->num_cells ){
                                    ptr->cell_index++ ; 
                                    children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                                    page_header *chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        while ( chdr->page_type != LEAF ){
                                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                                            children = get_child_pointer(byt->pager , ptr->page_num  ,0 )  ; 
                                            chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        }
                                    byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children , 0 } ; 
                                    byt->btr[op->p1].page_num = children ; 
                                    byt->btr[op->p1].row_num++ ;  
                                    kept = 1 ; 
                                    size = size + chdr->num_cells ; 
                                    hdr = chdr ; 
                                    break ; 
                            }
                            byt->btr[op->p1].depth-- ; 
                        }
                        if (kept == 0 ){
                            byt->pc = op->p2;
                            break ; 
                        }
                    }

                    if (!found && size >= tot) {
                        byt->pc = op->p2;
                    }

                    break;
            


                case seek_le : 
                    int size = 0 ; 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , 0 } ; 
                    while ( hdr->page_type != LEAF ){
                        int id = hdr->num_cells ; 
                        for ( int i = 0 ; i < hdr->num_cells ; i++  ){
                            void *cell = get_cell(hdr, i);
                            reg cell_key = get_key_from_cell(cell);
                            if (campare(byt->regis[op->p3], cell_key, cur->data_type) == 0) {
                                id = i;
                                break;
                            }
                        }
                        children = get_child_pointer(byt->pager , children  ,id )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        size = size + hdr->num_cells ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , id } ; 
                    }

                    int tot = byt->btr[op->p1].total - 1  ; 
                    int ans_id = -1 ; 
                    int found = 0 ; 
                    while ( size < tot  ){
                        if(bs(hdr ,byt->regis[op->p3]  , byt->btr[op->p1].data_type ,&ans_id ,  campare_le   ) == 1 ){
                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , ans_id } ; 
                            byt->btr[op->p1].page_num = children ; 
                            byt->btr[op->p1].row_num = ans_id ; 
                            found = 1 ; 
                            break ; 
                        }

                        int kept = 0 ; 
                        while ( byt->btr[op->p1].depth > 0 ){
                            path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                            page_header * hdr = pager_get_page(byt->pager , ptr->page_num) ; 
                            if (ptr->cell_index + 1 < hdr->num_cells ){
                                    ptr->cell_index++ ; 
                                    children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                                    page_header *chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        while ( chdr->page_type != LEAF ){
                                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                                            children = get_child_pointer(byt->pager , ptr->page_num  ,0 )  ; 
                                            chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        }
                                    byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children , 0 } ; 
                                    byt->btr[op->p1].page_num = children ; 
                                    byt->btr[op->p1].row_num++ ;  
                                    kept = 1 ; 
                                    size = size + chdr->num_cells ; 
                                    hdr = chdr ; 
                                    break ; 
                            }
                            byt->btr[op->p1].depth-- ; 
                        }
                        if (kept == 0 ){
                            byt->pc = op->p2;
                            break ; 
                        }
                    }

                    if (!found && size >= tot) {
                        byt->pc = op->p2;
                    }

                    break;
            
    
                case seek_lt : 
                    int size = 0 ; 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , 0 } ; 
                    while ( hdr->page_type != LEAF ){
                        int id = hdr->num_cells ; 
                        for ( int i = 0 ; i < hdr->num_cells ; i++  ){
                            void *cell = get_cell(hdr, i);
                            reg cell_key = get_key_from_cell(cell);
                            if (campare(byt->regis[op->p3], cell_key, cur->data_type) == 0) {
                                id = i;
                                break;
                            }
                        }
                        children = get_child_pointer(byt->pager , children  ,id )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        size = size + hdr->num_cells ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , id } ; 
                    }

                    int tot = byt->btr[op->p1].total - 1  ; 
                    int ans_id = -1 ; 
                    int found = 0 ; 
                    while ( size < tot  ){
                        if(bs(hdr ,byt->regis[op->p3]  , byt->btr[op->p1].data_type ,&ans_id ,  campare_lt   ) == 1 ){
                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , ans_id } ; 
                            byt->btr[op->p1].page_num = children ; 
                            byt->btr[op->p1].row_num = ans_id ; 
                            found = 1 ; 
                            break ; 
                        }

                        int kept = 0 ; 
                        while ( byt->btr[op->p1].depth > 0 ){
                            path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                            page_header * hdr = pager_get_page(byt->pager , ptr->page_num) ; 
                            if (ptr->cell_index + 1 < hdr->num_cells ){
                                    ptr->cell_index++ ; 
                                    children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                                    page_header *chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        while ( chdr->page_type != LEAF ){
                                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                                            children = get_child_pointer(byt->pager , ptr->page_num  ,0 )  ; 
                                            chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        }
                                    byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children , 0 } ; 
                                    byt->btr[op->p1].page_num = children ; 
                                    byt->btr[op->p1].row_num++ ;  
                                    kept = 1 ; 
                                    size = size + chdr->num_cells ; 
                                    hdr = chdr ; 
                                    break ; 
                            }
                            byt->btr[op->p1].depth-- ; 
                        }
                        if (kept == 0 ){
                            byt->pc = op->p2;
                            break ; 
                        }
                    }

                    if (!found && size >= tot) {
                        byt->pc = op->p2;
                    }

                    break;
            

                case seek_gt : 
                    int size = 0 ; 
                    byt->btr[op->p1].depth = 0 ; 
                    uint32_t children = byt->btr[op->p1].start_root_num   ; 
                    page_header *hdr = pager_get_page(byt->pager , children) ; 
                    byt->btr[op->p1].stack[byt->btr[op->p1].depth] = (path_entry){children  , 0 } ; 
                    while ( hdr->page_type != LEAF ){
                        int id = hdr->num_cells ; 
                        for ( int i = 0 ; i < hdr->num_cells ; i++  ){
                            void *cell = get_cell(hdr, i);
                            reg cell_key = get_key_from_cell(cell);
                            if (campare(byt->regis[op->p3], cell_key, cur->data_type) == 0) {
                                id = i;
                                break;
                            }
                        }
                        children = get_child_pointer(byt->pager , children  ,id )  ; 
                        hdr = (page_header*)pager_get_page(byte->pager,children  ) ; 
                        size = size + hdr->num_cells ; 
                        byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , id } ; 
                    }

                    int tot = byt->btr[op->p1].total - 1  ; 
                    int ans_id = -1 ; 
                    int found = 0 ; 
                    while ( size < tot  ){
                        if(bs(hdr ,byt->regis[op->p3]  , byt->btr[op->p1].data_type ,&ans_id ,  campare_gt   ) == 1 ){
                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , ans_id } ; 
                            byt->btr[op->p1].page_num = children ; 
                            byt->btr[op->p1].row_num = ans_id ; 
                            found = 1 ; 
                            break ; 
                        }

                        int kept = 0 ; 
                        while ( byt->btr[op->p1].depth > 0 ){
                            path_entry * ptr = &byt->btr[op->p1].stack[byt->btr[op->p1].depth - 1 ] ; 
                            page_header * hdr = pager_get_page(byt->pager , ptr->page_num) ; 
                            if (ptr->cell_index + 1 < hdr->num_cells ){
                                    ptr->cell_index++ ; 
                                    children = get_child_pointer(byt->pager , ptr->page_num ,ptr->cell_index ) ; 
                                    page_header *chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        while ( chdr->page_type != LEAF ){
                                            byt->btr[op->p1].stack[byt->btr[op->p1].depth++] = (path_entry){children , 0 } ; 
                                            children = get_child_pointer(byt->pager , ptr->page_num  ,0 )  ; 
                                            chdr = (page_header*)pager_get_page(pager,children  ) ; 
                                        }
                                    byt->btr[op->p1].stack[byt->btr[op->p1].depth++]  = (path_entry){children , 0 } ; 
                                    byt->btr[op->p1].page_num = children ; 
                                    byt->btr[op->p1].row_num++ ;  
                                    kept = 1 ; 
                                    size = size + chdr->num_cells ; 
                                    hdr = chdr ; 
                                    break ; 
                            }
                            byt->btr[op->p1].depth-- ; 
                        }
                        if (kept == 0 ){
                            byt->pc = op->p2;
                            break ; 
                        }
                    }

                    if (!found && size >= tot) {
                        byt->pc = op->p2;
                    }

                    break;

                case if_op : 
                    reg * temp = &byt->regis[op->p1];
                    int check = 0 ; 
                        if (temp->type == integer_num ){
                            if (temp.val.i != 0 ){
                                check = 1 ; 
                            } 
                        }
                        else if (temp->type == real_num){
                            if (temp.val.r != 0.0 ){
                                check = 1 ; 
                            } 
                        }
                        else if(temp->type == string_num) {
                            if (temp->lenght > 0 ){
                                check = 1 ; 
                            }
                        }
                        if ( check == 1 ){
                            byt->pc = op->p2;
                        }
                        break ; 

                case is_null : 
                    if (byt->regis[op->p1].type == null_op ){
                        byt->pc = op->p2;
                    }
                    break ; 

                case is_null_no_jump : 
                    if (byt->regis[op->p1].type == null_op ){
                        byt->regis[op->p3].type = integer_op ; 
                        byt->regis[op->p3].val.i = 1 ;
                    }
                    else {
                        byt->regis[op->p3].type = integer_op ; 
                        byt->regis[op->p3].val.i = 0 ;
                    }
                    break ; 

                case is_not_null : 
                    if (byt->regis[op->p1].type != null_op ){
                        byt->pc = op->p2;
                    }
                    break ; 

                case is_not_null_no_jump : 
                    if (byt->regis[op->p1].type != null_op ){
                        byt->regis[op->p3].type = integer_op ; 
                        byt->regis[op->p3].val.i = 1 ;
                    }
                    else {
                        byt->regis[op->p3].type = integer_op ; 
                        byt->regis[op->p3].val.i = 0 ;
                    }
                    break ; 

                case and_op : 
                    if (byt->regis[op->p1].type != string_op && byt->regis[op->p2].type != string_op  ){
                        byt->regis[op->p3].type = integer_num ; 
                        if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  && byt->regis[op->p2].val.i  ; 
                        }
                        else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r && byt->regis[op->p2].val.r ; 
                        }
                        else if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == real_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  && byt->regis[op->p2].val.r ; 
                        }
                        else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == integer_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r  && byt->regis[op->p2].val.i  ; 
                        }
                    }
                    break ; 
                
                case or_op : 
                    if (byt->regis[op->p1].type != string_op && byt->regis[op->p2].type != string_op  ){
                        byt->regis[op->p3].type = integer_num ; 
                        if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == integer_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  || byt->regis[op->p2].val.i  ; 
                        }
                        else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == real_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r || byt->regis[op->p2].val.r ; 
                        }
                        else if (byt->regis[op->p1].type == integer_num && byt->regis[op->p2].type == real_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.i  || byt->regis[op->p2].val.r ; 
                        }
                        else if (byt->regis[op->p1].type == real_num && byt->regis[op->p2].type == integer_num   ){
                            byt->regis[op->p3].val.i =  byt->regis[op->p1].val.r  ||  byt->regis[op->p2].val.i  ; 
                        }
                    }
                    break ; 

                

                case if_not_op : 
                    reg * temp = &byt->regis[op->p1];
                    int check = 0 ; 
                        if (temp->type == integer_num ){
                            if (temp.val.i == 0 ){
                                check = 1 ; 
                            } 
                        }
                        else if (temp->type == real_num){
                            if (temp.val.r == 0.0 ){
                                check = 1 ; 
                            } 
                        }
                        else if(temp->type == string_num) {
                            if (temp->lenght == 0 ){
                                check = 1 ; 
                            }
                        }
                        if ( check == 1 ){
                            byt->pc = op->p2;
                        }
                        break ; 

                case aggregate_init : 
                    byt->agg[op->p1].total = 0 ; 
                    byt->agg[op->p1].have_value = false ; 
                    byt->agg[op->p1].count = 0 ; 
                    byt->agg[op->p1].concat_string = NULL ; 
                    break  ; 

                case aggregate_reset:
                    if (byt->agg[op->p1].concat_string != NULL) {
                        free(byt->agg[op->p1].concat_string);
                    }
                    byt->agg[op->p1].total = 0;
                    byt->agg[op->p1].have_value = false;
                    byt->agg[op->p1].count = 0;
                    byt->agg[op->p1].concat_string = NULL;
                    break;

                    
                case aggregate_step : 
                    char *  operation = *(char*)op->p4 ; 
                    agg_operation(byt->agg[op->p1] , byt->regis[op->p2] , operation )  ; 
                    break ; 
                
                case aggregate_final : 
                    char * operation = *(char*)op->p4  ; 
                    agg_final(byt->agg[op->p1] , byt->regis[op->p3] , operation) ; 
                    break ; 

                case sorter_open : 
                    byt->sort[op->p1].array = NULL ; 
                    byt->sort[op->p1].row_count = 0 ; 
                    byt->sort[op->p1].capacity = 0 ; 
                    byt->sort[op->p1].cols_to_look = op->p2  ; 
                    byt->sort[op->p1].keycols = op->p4  ; 
                    byt->sort[op->p1].cursor = 0 ;  
                    
                case sorter_insert : 
                    if (byt->sort[op->p1].row_count == byt->sort[op->p1].capacity) {
                            if (byt->sort[op->p1].capacity == 0 ){
                                byt->sort[op->p1].capacity  = 4  ; 
                            }
                            else if (byt->sort[op->p1].capacity > 0 ){
                                byt->sort[op->p1].capacity  = byt->sort[op->p1].capacity*2   ; 
                            }
                            byt->sort[op->p1].capacity = byt->sort[op->p1].capacity == 0 ? 4 : byt->sort[op->p1].capacity * 2;  
                            sort_arr *tmp = realloc( byt->sort[op->p1].array, byt->sort[op->p1].capacity * sizeof(*byt->sort[op->p1].array)   );
                            if (tmp == NULL) { 
                                break; 
                            }/
                            byt->sort[op->p1].array = tmp;
                        }
                        sort_arr * temp  = malloc(sizeof(sort_arr)) ; 
                        temp->array = malloc(byt->regis[op->p2].lenght);
                        memcpy(temp->array, byt->regis[op->p2].val.s, byt->regis[op->p2].lenght ); 
                        temp.len = byt->regis[op->p2].lenght  ;     
                        temp->key = NULL;  
                        temp->key_type = byt->regis[op->p2].key_type;    
                        byt->sort[op->p1].array[byt->sort[op->p1].row_count++ ] = *temp ; 
                        free(temp) ; 
                        break ; 
                    


                    
                case sorter_sort:
                     qsort_r(byt->sort[op->p1].array,byt->sort[op->p1].row_count, sizeof(sort_arr), compare_sort, &byt->sort[op->p1]);             
                     break;

                case orderby_sort : 
                    qsort_r(byt->sort[op->p1].array,byt->sort[op->p1].row_count, sizeof(sort_arr), orderby_campare_sort , &byt->sort[op->p1]);             
                     break; 

                case sorter_next : 
                    if (byt->sort[op->p1].cursor < byt->sort[op->p1].row_count ){
                        byt->sort[op->p1].cursor++ ; 
                    }
                    else { 
                        byt->pc = op->p2; 
                    }
                    break ; 

                case sorter_data : 
                    sort_arr *temp =  byt->sort[op->p1].array ; 
                    byt->regis[op->p2].lenght = temp->len[ byt->sort[op->p1].cursor]  ; 
                    byt->regis[op->p2].type =  blob_op ; 
                    byt->regis[op->p2].val.s = temp->array[ byt->sort[op->p1].cursor]  ; 
                    break ; 

                case gb_sorter_data:
                    unsigned char * first = malloc(byt->regis[op->p1].lenght) ; 
                    memcpy(first ,byt->regis[op->p1].val.s , byt->regis[op->p1].lenght ) ; 
                    int key = op->p2 ; 
                    int i  = 0 ; 
                    int temp = 0 ; 
                    int type = integer_num ; 
                    int copy = 0 ; 
                    unsigned char *tempo ; 
                    unsigned char *blob = byt->regis[op->p1].val.s  ; 
                    while (i < byt->regis[op->p1].lenght ){
                        type = blob[i];    
                        i  = i +  1;
                        if (temp < key ){
                            if (type == integer_num) {
                                i = i +  sizeof(int);     
                            }
                             else if (type == real_num) {
                                i = i +  sizeof(float);   
                            }
                             else if (type == string_num || type == blob_num) {
                                int len;
                                memcpy(&len, blob + i, sizeof(int));
                                i = i +  sizeof(int);     
                                i = i +  len;            
                            }
                        }
                        else { 
                            copy =  byt->regis[op->p1].lenght - i ; 
                            tempo = malloc(copy); 
                            memcpy(tempo , first+ i  , copy ) ; 
                            break ; 
                        }
                        temp++ ;    
                    }
                    byt->regis[op->p3].type = blob_num;
                    byt->regis[op->p3].val.s = tempo ;
                    byt->regis[op->p3].lenght = copy ;
                    break ; 


                case gb_specific_column_op : 
                    unsigned char * first = malloc(byt->regis[op->p1].lenght) ; 
                    memcpy(first ,byt->regis[op->p1].val.s , byt->regis[op->p1].lenght ) ; 
                    int pos = 0 ;
                    unsigned char *thing = first; 
                    unsigned char temp = thing[pos] ;  
                    int key = op->p2 ; 
                    reg * ans = &byt->regis[op->p3] ; 
                    for ( int i =  0 ; i < key ; i++ ){
                        temp = thing[pos] ; 
                        pos = pos + 1;  
                        if (temp == integer_num){
                            pos = pos + sizeof(int) ; 
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

                    int len = 0  ; 
                    pos = pos + 1 ; 
                    if (temp == integer_num){
                        len =  sizeof(int) ; 
                        ans->type = integer_num ; 
                    }
                    else if (temp == real_num){
                        len =  sizeof(float ) ; 
                        ans->type =  real_num ; 
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        len =  sizeof(uint32_t) ; 
                        len = len + size ; 
                        ans->type = string_num ; 
                    }

                    unsigned char * valli = malloc(len ) ; 
                    memcpy(valli , thing + pos   , len ) ; 
                    if (ans->type == integer_num ){
                        memcpy(&ans->val.i, valli, sizeof(int)) ;
                        free(valli) ; 
                    }
                    else if (ans->type == real_num ){
                        memcpy(&ans->val.r, valli, sizeof(float)) ; 
                        free(valli) ;   
                    }
                    else {
                        ans->val.s = valli ;     
                        ans->lenght = len ; 
                    }
                    free(first) ; 
                    byt->btr[op->p1].null == false ; 
                    break ; 
                    

                case sorter_campare : 
                    sort_arr *temp =  byt->sort[op->p1].array ; 
                    if ( temp->key_type[byt->sort[op->p1].cursor] == integer_num && byt->regis[op->p3].type == integer_num  ){
                        int comp ; 
                        memcpy(&comp, temp->key[byt->sort[op->p1].cursor] , sizeof(int));
                        int comp = (int)temp->key[byt->sort[op->p1].cursor]  ; 
                        if (comp == byt->regis[op->p3].val.i ){
                            byt->pc = op->p2;
                        }
                    }
                    else if ( temp->key_type[byt->sort[op->p1].cursor]  == real_num && byt->regis[op->p3].type == real_num  ){
                        float comp  ;
                        memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                        if (comp == byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (byt->regis[op->p3].type == real_num && temp->key_type[byt->sort[op->p1].cursor]  == integer_num   ){
                        float comp  ;
                        memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                        if (comp == byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }

                    else if (temp->key_type[byt->sort[op->p1].cursor]  == real_num && byt->regis[op->p3].type == integer_num   ){
                        float comp  ;
                        memcpy(&comp, temp->key[byt->sort[op->p1].cursor], sizeof(float));
                        float num  = (float)( byt->regis[op->p3].val.i ) ; 
                        if (comp == byt->regis[op->p3].val.r ){
                            byt->pc = op->p2;
                        }
                    }
                    else if (temp->key_type[byt->sort[op->p1].cursor] == string_num && byt->regis[op->p3].type == string_num ){
                        if (strcmp(temp->key[byt->sort[op->p1].cursor] ,byt->regis[op->p3].val.s ) == 0 ){
                            byt->pc = op->p2;
                        }
                    }
                    break ; 


                case register_get_stuff : 
                    int pos = 0 ;
                    unsigned char thing = byt->regis[op->p1]->val.s ; 
                    unsigned char temp = thing[pos] ;  
                    int key = op->p2 ; 
                    reg * ans = byt->regis[op->p3] ; 
                    for ( int i =  0 ; i < key ; i++ ){
                        temp = thing[pos] ; 
                        pos = pos + 1;  
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

                    int len = 0  ; 
                    pos = pos + 1 ; 
                    if (temp == integer_num){
                        len =  sizeof(long) ; 
                        ans->type = integer_num ; 
                    }
                    else if (temp == real_num){
                        len =  sizeof(float ) ; 
                        ans->type =  real_num ; 
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        len =  sizeof(uint32_t) ; 
                        len = len + size ; 
                        ans->type = string_num ; 
                    }

                    unsigned char * valli = malloc(len ) ; 
                    memcpy(valli , thing + pos   , len ) ; 
                    if (ans->type == integer_num ){
                        ans->val.i = (int)valli ;  
                    }
                    else if (ans->type == real_num ){
                        ans->val.r = (float)valli ;     
                    }
                    else {
                        ans->val.s = valli ; 
                        ans->lenght = len ; 
                    }
                    byt->btr[op->p1].null == false ; 
                    break ; 


                case column_op : 
                    if (byt->btr[op->p1].null == true ){
                        break ; 
                    }
                    void * temp = pager_get_page( byt->pager , byt->btr[op->p1].stack[depth].page_num  ) ; 
                    void * cell = get_cell(temp , byt->btr[op->p1].stack[depth].cell_num  ) ; 
                    reg tempo = get_key_from_cell(cell) ; 
                    int pos = 0 ;
                    unsigned char thing = tempo->val.s ; 
                    unsigned char temp = thing[pos] ;  
                    int key = op->p2 ; 
                    reg * ans = byt->regis[op->p3] ; 
                    for ( int i =  0 ; i < key ; i++ ){
                        temp = thing[pos] ; 
                        pos = pos + 1;  
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

                    int len = 0  ; 
                    pos = pos + 1 ; 
                    if (temp == integer_num){
                        len =  sizeof(long) ; 
                        ans->type = integer_num ; 
                    }
                    else if (temp == real_num){
                        len =  sizeof(float ) ; 
                        ans->type =  real_num ; 
                    }
                    else if (temp == string_num ){
                        uint32_t size ; 
                        memcpy(&size , thing + pos , sizeof(uint32_t)) ; 
                        len =  sizeof(uint32_t) ; 
                        len = len + size ; 
                        ans->type = string_num ; 
                    }

                    unsigned char * valli = malloc(len ) ; 
                    memcpy(valli , thing + pos   , len ) ; 
                    if (ans->type == integer_num ){
                        ans->val.i = (int)valli ;  
                    }
                    else if (ans->type == real_num ){
                        ans->val.r = (float)valli ;     
                    }
                    else {
                        ans->val.s = valli ; 
                        ans->lenght = len ; 
                    }
                    byt->btr[op->p1].null == false ; 
                    break ; 
                    
                    
                case make_record : 
                   byt->reg[op->p3].type = string_num ; 
                   byt->reg[op->p3].val.s = make_record_data(byt , op->p1 , op->p1 + op->p2 ) ; 
                   byt->reg[op->p3].lenght = strlen(byt->reg[op->p3].val.s) ; 
                   break ; 
                
                case result_row : 
                    byt->reg_start = op->p1 ; 
                    byt->reg_end = byt->reg_start  + op->p2 ; 
                    byt->reg_mode = end_num ;
                    for ( int i = byt->reg_start ; i < byt->reg_end  ; i++ ){

                    } 
                    

                case limit_op : 
                    if (byt->reg[op->p1].val.i > 0 ){
                        byt->reg[op->p1].val.i-- ; 
                        if (byt->reg[op->p1].val.i == 0  ){
                            byt->pc = op->p2;
                            continue ; 
                        }
                    } break ; 

                case null_join_op : 
                    byt->btr[op->p1].null = true ; 
                    break ; 

                case push_to_hash:
                    unsigned char  * blob = op->p4 ; 
                    int temp = 0;
                    int pk_1  ; 
                    int str_len ; 
                    int pk_2 ; 
                    int hash_bin_nums ; 
                    int row_num  = -1  ; 
                    char * table_name  = NULL ; 

                    memcpy(hash_bins_num, blob + temp, sizeof(int));
                    temp += sizeof(int);

                    memcpy(pk_1, blob + temp, sizeof(int));
                    temp += sizeof(int);

                    memcpy(pk_2, blob + temp, sizeof(int));
                    temp += sizeof(int);

                    if (blob + temp != NULL) {
                        memcpy(row_num, blob + temp, sizeof(int));
                        temp += sizeof(int);
                    } 

                    if (blob + temp != NULL ){
                        str_len = 0;
                        memcpy(str_len, blob + temp, sizeof(int));
                        temp += sizeof(int);
                        if (str_len > 0) {
                            table_name = (char *)malloc(str_len + 1);
                            memcpy(table_name, blob + temp, str_len);
                            table_name[str_len] = '\0';
                        }
                    }
                    byt->hash_bins[op->p1]->bn[hbw]->num_of_entires++ ; 
                    int hbw = hash_bin_which( byt->regis[op->p2] , hash_bins_num ) ; 
                    if (byt->regis[pk_1].type ==   integer_num  ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.i = byt->regis[pk_1].val.i ; 
                    }
                    else if (byt->regis[pk_1].type == real_num){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.r = byt->regis[pk_1].val.r ; 
                    }
                    else if (byt->regis[pk_1].type == string_num || byt->regis[pk_1].type == blob_num ){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.s = malloc(byt->regis[pk_1].lenght)  ; 
                        memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_first.s  , byt->regis[pk_1].val.s , byt->regis[pk_1].lenght ) ; 
                    }
                    if (byt->regis[pk_2].type ==   integer_num  ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.i = byt->regis[pk_2].val.i ; 
                    }
                    else if (byt->regis[pk_2].type == real_num){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.r = byt->regis[pk_2].val.r ; 
                    }
                    else if (byt->regis[pk_2].type == string_num || byt->regis[pk_2].type == blob_num ){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.s = malloc(byt->regis[pk_2].lenght)  ; 
                        memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].primary_key_second.s  , byt->regis[pk_2].val.s , byt->regis[pk_2].lenght ) ; 
                    }
                    if (byt->regis[op->p2].type ==   integer_num  ){
                    byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.i = byt->regis[op->p2].val.i ; 
                    }
                    else if (byt->regis[op->p2].type == real_num){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.r = byt->regis[op->p2].val.r ; 
                    }
                    else if (byt->regis[op->p2].type == string_num || byt->regis[op->p2].type == blob_num ){
                        byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.s = malloc(byt->regis[op->p2].lenght)  ; 
                        memcpy(byt->hash_bins->bn[hbw].data[ byt->hash_bins->bn[hbw].num_of_entires ].compare.s  , byt->regis[op->p2].val.s , byt->regis[op->p2].lenght ) ;  
                    }
                    if (row_num != -1 ){
                        byt->hash_bins->bn[hbw].row_number = row_num ; 
                    }
                    if (table_name != NULL ){
                        byt->hash_bins->bn[hbw].tbl = malloc(str_len) ; 
                        memcpy(byt->hash_bins->bn[hbw].tbl , table_name , str_len) ; 
                    }
                    break  ; 


                
               
                }
            }
            
    }




int like_campare(reg *a , reg *b ){
    int ans = 0 ; 
    if (a->type == string_num && b->type == string_num ){
        int first = 0 ; 
        int second = 0 ; 
        while ( first < a->len && second < b->len ){
            if (strcmp(a->val.s[first] , b->val.s[second] ) == 0 ){
                first++ ; 
                second++ ; 
            }
            else if (strcmp(a->val.s[first] , '%' ) == 0 ){
                int temp_fir = first ; 
                unsigned char temp ; 
                while (temp_fir < a->lenght &&  strcmp(a->val.s[temp_fir] , '%' ) != 0 ){
                    memcpy(temp , )
                }
            }
            else if ( strcmp(a->val.s[first] , "_" ) == 0 ){
                int first_temp = first ;  
            }
        }
    }
}



}
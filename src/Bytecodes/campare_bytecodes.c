int campare_ge(reg *target , void * cell_key , int  data_type ){
    if (target->type == int_num){
        if (data_type == 1 ){
            long long num = *(uint32_t*)cell_key ; 
            if (num >= target->val.i){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
            double num = *(float*)cell_key ; 
            if (num >= target->val.i){
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
            double num = (double)num_temp ; 
            if (num >= target->val.r){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
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
    else if ( target->type == string_num ){ 
        if (data_type == 3 ){
            char *str_temp = (char *)cell_key; 
            if (strcmp(str_temp, target->val.s) >= 0){
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
        return -1 ; 
    }
}

int campare_gt(reg *target , void * cell_key , int  data_type ){
    if (target->type == int_num){
        if (data_type == 1 ){
            long long num = *(uint32_t*)cell_key ; 
            if (num > target->val.i){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
            double num = *(float*)cell_key ; 
            if (num > target->val.i){
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
            double num = (double)num_temp ; 
            if (num > target->val.r){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
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
    else if ( target->type == string_num ){ 
        if (data_type == 3 ){
            char *str_temp = (char *)cell_key; 
            if (strcmp(str_temp, target->val.s) > 0){
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
        return -1 ; 
    }
}

int campare_lt(reg *target , void * cell_key , int  data_type ){
    if (target->type == int_num){
        if (data_type == 1 ){
            long long num = *(uint32_t*)cell_key ; 
            if (num < target->val.i){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
            double num = *(float*)cell_key ; 
            if (num < target->val.i){
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
            double num = (double)num_temp ; 
            if (num < target->val.r){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
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
    else if ( target->type == string_num ){ 
        if (data_type == 3 ){
            char *str_temp = (char *)cell_key; 
            if (strcmp(str_temp, target->val.s) < 0){
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
        return -1 ; 
    }
}

int campare_le(reg *target , void * cell_key , int  data_type ){
    if (target->type == int_num){
        if (data_type == 1 ){
            long long num = *(uint32_t*)cell_key ; 
            if (num <= target->val.i){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
            double num = *(float*)cell_key ; 
            if (num <= target->val.i){
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
            double num = (double)num_temp ; 
            if (num <= target->val.r){
                return 0 ; 
            }
            else {
                return -1 ; 
            }
        }
        else if (data_type == 2 ){
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
    else if ( target->type == string_num ){ 
        if (data_type == 3 ){
            char *str_temp = (char *)cell_key; 
            if (strcmp(str_temp, target->val.s) <= 0){
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
        return -1 ; 
    }
}



void eq_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
    return ; 
}


void ne_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
            byt->regis[op->p2].val.i = 0  ;
        }
    }
    else if (byt->regis[op->p1].type == string_num && byt->regis[op->p3].type == string_num ){
        if (strcmp(byt->regis[op->p1].val.s ,byt->regis[op->p3].val.s ) != 0 ){
            byt->regis[op->p2].val.i = 1  ;
        }
        else { 
            byt->regis[op->p2].val.i = 0  ;
        }
    }
    return ; 
}


void gt_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
        if (byt->regis[op->p1].val.r < num){
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
    return ; 
}


void lt_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
        if (byt->regis[op->p1].val.r > num){
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
    return ; 
}


void le_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
        if (byt->regis[op->p1].val.r >= num){
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
    return ; 
}


void ge_select_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
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
        if (byt->regis[op->p1].val.r <= num){
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
    return ; 
}

 
void eq_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i == byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r == byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num ==  byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (num ==  byt->regis[p1].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) == 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}


void ne_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i != byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r != byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num !=  byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (num !=  byt->regis[p1].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) != 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}


void gt_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i < byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r < byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num <  byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (byt->regis[p1].val.r < num){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) < 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}


void lt_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i > byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r > byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num >  byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (byt->regis[p1].val.r > num){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) > 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}


void le_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i >= byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r >= byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num >=  byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (byt->regis[p1].val.r >= num){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) >= 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}


void ge_op_fn( int p1 , int p2 , int p3 , int p4 , byte * byt){
    if ( byt->regis[p1].type == int_num && byt->regis[p3].type == int_num  ){
        if (byt->regis[p1].val.i <= byt->regis[p3].val.i ){
            byt->pc = p2;
        }
    }
    else if ( byt->regis[p1].type == real_num && byt->regis[p3].type == real_num  ){
        if (byt->regis[p1].val.r <= byt->regis[p3].val.r ){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p3].type == real_num && byt->regis[p1].type == int_num   ){
        double num  = (double)( byt->regis[p1].val.i ) ; 
        if (num <= byt->regis[p3].val.r){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == real_num && byt->regis[p3].type == int_num   ){
        double num  = (double)( byt->regis[p3].val.i ) ; 
        if (byt->regis[p1].val.r <= num){
            byt->pc = p2;
        }
    }
    else if (byt->regis[p1].type == string_num && byt->regis[p3].type == string_num ){
        if (strcmp(byt->regis[p1].val.s ,byt->regis[p3].val.s ) <= 0 ){
            byt->pc = p2;
        }
    }
    return ; 
}
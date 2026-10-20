void tree_func_for_insert(internal_node * node  , int pk , char * data , int page_id , int offset , int slot_num ){
    internal_node * stack[32] ; 
    if (node == NULL ){
        return  ; 
    }
    int stack_index[32] ; 
    int k = 0  ; 
    int which_pos = -1 ; 
    internal_node * use_temp = node ; 
    if (node->num_of_leaf == 0 && node->num_of_children == 0){
        node->leaf = true ;
    }

    stack[k] = use_temp ;
    stack_index[k] = 0 ;
    while (use_temp != NULL && use_temp->leaf != true ){
        int low = 0 ; 
        int high = use_temp->num_of_leaf - 1 ;
        while ( low <= high ){
            int mid = (low + high ) / 2 ; 
            if (pk > use_temp->primary_key[mid] ){
                low = mid + 1 ; 
            }
            else if (pk ==  use_temp->primary_key[mid]  ){
                return ; 
            }
            else {
                high = mid - 1 ; 
            }
        }
        k++ ; 
        stack[k] = use_temp->children[low] ; 
        stack_index[k] = low ; 
        use_temp = use_temp->children[low] ; 
    }

    if (stack[k] == NULL){
        return ;
    }
    int low = 0 ;
    int high = stack[k]->num_of_leaf - 1 ;
    while (low <= high){
        int mid = (low + high) / 2 ;
        if (pk == stack[k]->primary_key[mid]){
            return ;
        }
        else if (pk > stack[k]->primary_key[mid]){
            low = mid + 1 ;
        }
        else {
            high = mid - 1 ;
        }
    }
    which_pos = low ;


    if (stack[k]->num_of_leaf < no_of_data ){
        for (int i = stack[k]->num_of_leaf ; i > which_pos ; i--) {
            stack[k]->primary_key[i] = stack[k]->primary_key[i-1];
            stack[k]->data[i] = stack[k]->data[i-1];
        }
        stack[k]->primary_key[which_pos] = pk ;  
        stack[k]->data[which_pos].page_id = page_id ; 
        stack[k]->data[which_pos].file_offset = offset ; 
        stack[k]->data[which_pos].slot_num = slot_num ; 
        stack[k]->num_of_leaf++ ; 
        stack[k]->num_of_data_pushed = stack[k]->num_of_leaf ;
    }

    else { 
        int catch_which_to_put_as_parent = 0 ; 
        int lame = k -1  ; 
        while (lame >= 0 && stack[lame]->num_of_leaf == no_of_data ){
            lame--  ; 
        }
        internal_node * where_to_add = NULL ;
        int index_thing = 0 ;
        internal_node * the_emptier_one = stack[0] ; 
        if (lame == -1){
            internal_node * the_ansestor = (internal_node *)malloc(sizeof(internal_node)) ;
            if (the_ansestor == NULL){
                return ;
            }
            memcpy(the_ansestor , stack[0] , sizeof(internal_node)) ;
            memset(stack[0] , 0 , sizeof(internal_node)) ;
            stack[0]->leaf = false ;
            stack[0]->children[0] = the_ansestor ;
            stack[0]->num_of_children = 1 ;
            stack[0]->num_of_leaf = 0 ;
            stack[0] = the_ansestor ;
        }

        int kalos = lame ; 
        int turner = 0 ; 
        while ( kalos <= k ){ 
            if (turner == 0 ){
                internal_node *  greninja  ; 
                int greninja_index = -1 ; 
                if (lame != -1 ){
                    greninja = stack[lame] ; 
                    greninja_index = stack_index[lame + 1]  ; 
                }
                else { 
                    greninja = the_emptier_one ; 
                    greninja_index = 0  ; 
                }
                where_to_add = greninja ;
                index_thing = greninja_index + 1 ;
                turner = 1 ; 
                kalos++ ;
                continue ;
            }

            internal_node * nadda = stack[kalos] ;
            internal_node *tempo = (internal_node *)malloc(sizeof(internal_node)) ;
            if (tempo == NULL ){
                return ; 
            }
            memset(tempo , 0 , sizeof(internal_node)) ;


            int for_the_temp = 0 ; 
            int tempacola = nadda->num_of_leaf / 2 ; 
            int skip_or_not = 0  ; 
            catch_which_to_put_as_parent = nadda->primary_key[tempacola] ;
            if (nadda->leaf == false ){
                for (int o = tempacola +1   ; o < nadda->num_of_leaf  ; o++ ){
                    tempo->primary_key[for_the_temp] = nadda->primary_key[o] ; 
                    tempo->num_of_leaf++ ; 
                    for_the_temp++ ; 
                }
            }
            else { 
                for (int o = tempacola   ; o < nadda->num_of_leaf  ; o++ ){
                    tempo->primary_key[for_the_temp] = nadda->primary_key[o] ; 
                    tempo->num_of_leaf++ ; 
                    for_the_temp++ ; 
                }
            }

            nadda->num_of_leaf = tempacola ;
            tempo->leaf = false ; 

            for (int i = where_to_add->num_of_leaf ; i > index_thing - 1 ; i--) {
                where_to_add->primary_key[i] = where_to_add->primary_key[i-1] ;
            }
            where_to_add->primary_key[index_thing - 1] = catch_which_to_put_as_parent ;
            where_to_add->num_of_leaf++ ;
            for (int i = where_to_add->num_of_children ; i > index_thing ; i--) {
                where_to_add->children[i] = where_to_add->children[i-1] ;
            }
            where_to_add->children[index_thing] = tempo ;
            where_to_add->num_of_children++ ;

            if ( nadda->leaf == false ){
                tempacola = nadda->num_of_children / 2 ; 
                for_the_temp = 0 ; 
                for (int o = tempacola + 1 ; o < nadda->num_of_children ; o++ ){
                    tempo->children[for_the_temp] = nadda->children[o] ; 
                    nadda->children[o] = NULL ;
                    tempo->num_of_children++ ; 
                    for_the_temp++ ; 
                }
                nadda->num_of_children = tempacola + 1 ; 

                int the_next_node_index_to_put = stack_index[kalos + 1] ;
                if (the_next_node_index_to_put <= tempacola){
                    where_to_add = nadda ;
                    index_thing = the_next_node_index_to_put + 1 ;
                }
                else {
                    where_to_add = tempo ;
                    index_thing = the_next_node_index_to_put - tempacola ;
                }

                kalos++  ; 
            }


            else {
                tempacola = nadda->num_of_data_pushed / 2 ; 
                for_the_temp = 0 ;
                for (int o = tempacola  ; o < nadda->num_of_data_pushed ; o++ ){
                    tempo->data[for_the_temp] = nadda->data[o] ; 
                    tempo->num_of_data_pushed++ ; 
                    for_the_temp++ ;   
                }
                nadda->num_of_data_pushed = tempacola ; 
                nadda->num_of_leaf = tempacola ;          
                tempo->num_of_leaf = tempo->num_of_data_pushed ;
                tempo->leaf = true ;

                tempo->next = nadda->next ;
                nadda->next = tempo ;

                int where = which_pos ;                              
                if ( where <= tempacola ){
                    for (int i = nadda->num_of_data_pushed ; i > where ; i--) {
                        nadda->data[i] = nadda->data[i-1] ;
                        nadda->primary_key[i] = nadda->primary_key[i-1] ;
                    }
                    nadda->primary_key[where] = pk ;
                    nadda->data[where].page_id = page_id ; 
                    nadda->data[where].file_offset = offset ; 
                    nadda->data[where].slot_num = slot_num ;
                    nadda->num_of_data_pushed++ ; 
                    nadda->num_of_leaf++ ;
                }

                else { 
                    where = where - tempacola ; 
                    for (int i = tempo->num_of_data_pushed ; i > where ; i--) {
                        tempo->data[i] = tempo->data[i-1] ;
                        tempo->primary_key[i] = tempo->primary_key[i-1] ;
                    }
                    tempo->primary_key[where] = pk ;
                    tempo->data[where].page_id = page_id ;
                    tempo->data[where].file_offset = offset ;
                    tempo->data[where].slot_num = slot_num ;
                    tempo->num_of_data_pushed++ ;
                    tempo->num_of_leaf++ ;
                }
                return ; 
            }
        }
    }
}

     
int validate_the_tree_data(internal_node *  root){
    while ( root->leaf != true ){
        root = root->children[0] ;
    }
    int check = -1 ; 
    while ( root->next != NULL ){
        int i = 0 ; 
        while(i < root->num_of_leaf ){
            if (root->primary_key[i]  <=  check  ){
                return 1 ; 
            }
            check = root->primary_key[i] ; 
            i++ ; 
        }
        root = root->next ; 
    }
    return 0 ; 
}

int validate_the_tree_node ( internal_node * root  , int high , int low ){
    for ( int i = 0 ; i < root->num_of_leaf ; i++ ){
        if (root->primary_key[i] < high && root->primary_key[i] > low  ){
            if (root->num_of_children > 0 ){
                for ( int j = 0 ; j < root->num_of_children ; j++ ){
                    if ( j == 0 ){
                        validate_the_tree_node(root->children[i] ,  root->primary_key[i]  , low ) ; 
                    }
                    if (j == root->num_of_children  - 1 ){
                        validate_the_tree_node(root->children[i] ,  high , root->primary_key[i]  ) ; 
                    }
                    else { 
                        validate_the_tree_node(root->children[i]  , root->primary_key[i+ 1 ] , root->primary_key[i]  ) ; 
                    }
                }
            }
        }
        else { 
            return 1 ; 
        }
        
    }
    return 0 ; 
}

int validate_the_tree(internal_node * root ){
    if (validate_the_tree_data(root) == 1 ){
        return 1  ; 
    }
    else if (validate_the_tree_node(root) == 1){
        return 1 ; 
    }
    return 0 ; 
}   


int min_key_of(internal_node * node){
    while (node != NULL && node->leaf != true ){
        node = node->children[0] ;
    }
    if (node == NULL || node->num_of_leaf == 0 ){
        return -1 ;
    }
    return node->primary_key[0] ;
} 

void the_correct_representative(internal_node ** stack , int * pos , int depth ){
    for (int i = depth - 1 ; i >= 0 ; i-- ){
        int minimum_key ;
        if (pos[i] > 0 && pos[i] < stack[i]->num_of_children ){
            minimum_key = min_key_of(stack[i]->children[pos[i] ]) ;
            if (minimum_key != -1 ){
                stack[i]->primary_key[pos[i]- 1] = minimum_key ;
            }
        }
        if (pos[i] + 1 < stack[i]->num_of_children ){
            minimum_key = min_key_of(stack[i]->children[pos[i]  + 1]) ;
            if (minimum_key != -1 ){
                stack[i]->primary_key[pos[i]] = minimum_key ;
            }
        }
    }
}

void termination_child(internal_node * node , int term ){
    int remove_key  ; 
    if (term > 0 ){
        remove_key = term -1 ; 
    } 
    else  {
        term = 0  ; 
    }
    if (node->num_of_leaf > 0 ){
        for (int m = remove_key ; m < node->num_of_leaf - 1 ; m++ ){
            node->primary_key[m] = node->primary_key[m+1] ;
        }
        node->primary_key[node->num_of_leaf - 1] = -1 ;
        node->num_of_leaf-- ;
    }
    for (int m = term ; m < par->num_of_children - 1 ; m++ ){
        par->children[m] = par->children[m+1] ;
    }
    par->children[par->num_of_children - 1] = NULL ;
    par->num_of_children-- ;
}



internal_node * delete_the_thing(internal_node * node , int key ){
    int first_pos[32] ; 
    internal_node * temp = node ; 
    internal_node * temp_stack[32] ; 
    int second_pos[32] ; 
    int k = 0 ; 
    temp_stack[k++] = node ; 
    int first_counter ; 
    int second_counter ; 
    internal_node * next_temp_stack[32] ; 
    int temp_counter = 0 ; 
    int first_alrm = -1 ;
    int scnd_alrm = -1  ;

    while (temp->leaf != true ){
        int low = 0 ; 
        int high = temp->num_of_leaf -1  ; 
        while (low <= high ){
            int mid = (low + high ) / 2 ; 
            if (key <=  temp->primary_key[mid] ){
                high  = mid - 1 ; 
            }
            else { 
                low = mid + 1 ; 
            }
        }
        if (low < temp->num_of_leaf && temp->primary_key[low] == key){
            if (first_alrm == -1 ){
                first_alrm = temp_counter ; 
            } 
        }
        first_pos[temp_counter] = low ; 
        temp_counter++ ; 
        temp = temp->children[low] ; 
        temp_stack[k++] = temp ; 
    }
    k-- ; 
    first_counter = k ; 

    int num = temp->num_of_data_pushed -1  ; 
    int happen_or_no = 0 ; 
    int low = 0 ; 
    int high = num ; 
    while ( low <= high ){
        int mid = ( low + high ) / 2 ; 
        if (key == temp->data[mid].page_id ){
            temp->num_of_data_pushed-- ; 
            happen_or_no = 1 ; 
            temp->num_of_leaf-- ;
            for (int i = mid ; i < num  ; i++ ) {
                temp->data[i] = temp->data[i+1];
                temp->primary_key[i] = temp->primary_key[i+1];
            }
            memset(&temp->data[num], 0, sizeof(temp->data[num])) ;
            temp->primary_key[num] = -1 ; 
            break ;
        }
        else if (key < temp->data[mid].page_id  ){
            high = mid -1 ; 
        }
        else {
            low = mid + 1 ; 
        }
    }
    if ( happen_or_no == 0 ){
        return node ;
    }

    if (temp->num_of_data_pushed >= no_of_data / 2 ){
        if (first_alrm >= 0 ){
            the_correct_representative(temp_stack , first_pos , first_counter) ;
        }
        return node ;
    }
    else { 
        internal_node * the_next_one = temp->next ; 
        if (the_next_one == NULL ){
            if (first_counter == 0){
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                return node ; 
            }
            if (first_pos[first_counter - 1] == 0 ){
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                return node ;
            }
            internal_node * prev = temp_stack[first_counter - 1]->children[first_pos[first_counter - 1]  - 1] ; 
            int the_updated_mid ; 
            int first_steps ; 
            if (prev->num_of_data_pushed - 1 >= no_of_data / 2){
                for (int x = temp->num_of_data_pushed ; x > 0 ; x--){
                    temp->data[x] = temp->data[x-1] ; 
                }
                for (int x = temp->num_of_leaf ; x > 0 ; x--){
                    temp->primary_key[x] = temp->primary_key[x-1] ; 
                }

                int last_one = prev->num_of_data_pushed - 1 ; 
                temp->data[0] = prev->data[last_one] ; 
                temp->primary_key[0] = prev->primary_key[last_one] ; 
                temp->num_of_data_pushed++ ; 
                temp->num_of_leaf++ ; 

                memset(&prev->data[last_one] , 0 , sizeof(prev->data[last_one])) ; 
                prev->primary_key[last_one] = -1 ; 
                prev->num_of_data_pushed-- ; 
                prev->num_of_leaf-- ; 

                the_correct_representative(temp_stack , first_pos , first_counter) ;
                return node ; 
            }
        
            for (int s = 0 ; s < temp->num_of_leaf ; s++){
                prev->primary_key[prev->num_of_leaf] = temp->primary_key[s] ; 
                prev->num_of_leaf++ ; 
            }
            for (int s = 0 ; s < temp->num_of_data_pushed ; s++){
                prev->data[prev->num_of_data_pushed] = temp->data[s] ; 
                prev->num_of_data_pushed++ ; 
            }
            prev->next = temp->next ; 
            termination_child(temp_stack[first_counter - 1] , first_pos[first_counter - 1]) ;

            release_page(temp) ; 
            free(temp) ; 
            the_correct_representative(temp_stack , first_pos , first_counter - 1) ;
            for (int x = 0 ; x < 32 ; x++){
                next_temp_stack[x] = temp_stack[x] ; 
                second_pos[x] = first_pos[x] ; 
            }
            second_counter = first_counter - 1 ; 
        }
        else { 
            int next_one_pk = the_next_one->data[0].page_id ; 
            temp_counter = 0 ; 
            internal_node * the_next_temp = node ;
            k = 0 ;
            next_temp_stack[k++] = node ;
            while (the_next_temp->leaf != true ){
                int low_second = 0 ; 
                int high_second = the_next_temp->num_of_leaf -1 ; 
                while (low_second <= high_second ){
                    int mid_second = (low_second + high_second ) / 2 ;
                    if (next_one_pk <  the_next_temp->primary_key[mid_second] ){
                        high_second  = mid_second - 1 ;
                    }
                    else {
                        low_second = mid_second + 1 ;
                    }
                }
                second_pos[temp_counter] = low_second ; 
                temp_counter++ ; 
                the_next_temp = the_next_temp->children[low_second] ; 
                next_temp_stack[k++] = the_next_temp ; 
            }
            k-- ; 
            second_counter = k ; 
            assert(next_temp_stack[second_counter] == the_next_one) ;

            if (the_next_one != NULL && the_next_one->num_of_data_pushed - 1 >= no_of_data / 2 ){
                temp->primary_key[temp->num_of_leaf] = the_next_one->data[0].page_id ;  
                temp->data[temp->num_of_data_pushed] = the_next_one->data[0] ; 
                temp->num_of_data_pushed++ ; 
                temp->num_of_leaf++ ;
                num = the_next_one->num_of_data_pushed - 1   ; 
                the_next_one->num_of_data_pushed-- ; 
                the_next_one->num_of_leaf-- ;
                memset(&the_next_one->data[0] , 0 , sizeof(the_next_one->data[0])) ;
                for (int i = 0 ; i < num  ; i++ ) {
                    the_next_one->data[i] = the_next_one->data[i+1];
                    the_next_one->primary_key[i] = the_next_one->primary_key[i+1];
                }
                memset(&the_next_one->data[num] , 0 , sizeof(the_next_one->data[num])) ;
                the_next_one->primary_key[num] = -1 ; 
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                the_correct_representative(next_temp_stack , second_pos , second_counter) ;
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                return node ;
            }

            else {
                for ( int s = 0 ; s < the_next_one->num_of_leaf ; s++  ){
                    temp->primary_key[temp->num_of_leaf] = the_next_one->primary_key[s] ; 
                    temp->num_of_leaf++ ; 
                }
                for ( int s = 0 ; s < the_next_one->num_of_data_pushed ; s++  ){
                    temp->data[temp->num_of_data_pushed ] = the_next_one->data[s] ; 
                    temp->num_of_data_pushed++ ; 
                }
                temp->next = the_next_one->next ;
                termination_child(next_temp_stack[second_counter - 1] , second_pos[second_counter - 1]) ;
                release_page(the_next_one) ;
                free(the_next_one) ;
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                the_correct_representative(next_temp_stack , second_pos , second_counter) ;
                the_correct_representative(temp_stack , first_pos , first_counter) ;
                second_counter-- ;
            }

            while (second_counter > 0 && next_temp_stack[second_counter]->num_of_children <  no_of_data / 2 ){
                for ( int i = 0 ; i < 32 ; i++ ){
                    temp_stack[i] = next_temp_stack[i] ;
                    first_pos[i] = second_pos[i] ;
                }
                first_counter = second_counter ;
                temp = temp_stack[first_counter] ;
                int current_counter = first_counter - 1 ;
                while (current_counter >= 0 && first_pos[current_counter] + 1 >= temp_stack[current_counter]->num_of_children ){
                    current_counter-- ;
                }

                if (current_counter < 0 ){
                    if (first_pos[first_counter - 1] == 0 ){
                        break ;
                    }
                    internal_node * prev = temp_stack[first_counter - 1]->children[first_pos[first_counter - 1] - 1] ;
                    if (prev->num_of_children - 1 >= no_of_data / 2){
                        int sep = min_key_of(temp->children[0]) ;
                        for (int x = temp->num_of_children ; x > 0 ; x--){
                            temp->children[x] = temp->children[x-1] ;
                        }
                        for (int x = temp->num_of_leaf ; x > 0 ; x--){
                            temp->primary_key[x] = temp->primary_key[x-1] ;
                        }
                        temp->children[0] = prev->children[prev->num_of_children - 1] ;
                        temp->primary_key[0] = sep ;
                        temp->num_of_children++ ;
                        temp->num_of_leaf++ ;
                        prev->children[prev->num_of_children - 1] = NULL ;
                        prev->primary_key[prev->num_of_leaf - 1] = -1 ;
                        prev->num_of_children-- ;
                        prev->num_of_leaf-- ;
                        the_correct_representative(temp_stack , first_pos , first_counter) ;
                        break ;
                    }
                    prev->primary_key[prev->num_of_leaf] = min_key_of(temp->children[0]) ;
                    prev->num_of_leaf++ ;
                    for (int s = 0 ; s < temp->num_of_leaf ; s++){
                        prev->primary_key[prev->num_of_leaf]  = temp->primary_key[s] ;
                        prev->num_of_leaf++ ;
                    }
                    for (int s = 0 ; s < temp->num_of_children ; s++){
                        prev->children[prev->num_of_children] =  temp->children[s] ;
                        prev->num_of_children++ ;
                    }
                    termination_child(temp_stack[first_counter - 1] , first_pos[first_counter - 1]) ;
                    release_page(temp) ;
                    free(temp) ;
                    the_correct_representative(temp_stack , first_pos , first_counter - 1) ;
                    second_counter = first_counter - 1 ;
                    continue ;
                }

                internal_node * merger_node  = temp_stack[current_counter] ;
                next_temp_stack[current_counter] = merger_node ;
                second_pos[current_counter] = first_pos[current_counter] + 1 ;
                internal_node * the_next_one  = merger_node->children[second_pos[current_counter]]  ;
                int kj = 1 ;
                while ( current_counter + kj <= first_counter ){
                    next_temp_stack[current_counter + kj] = the_next_one ;
                    if (current_counter + kj < first_counter ){
                        second_pos[current_counter + kj] = 0 ;
                        the_next_one = the_next_one->children[0] ;
                    }
                    kj++  ;
                }

                if (the_next_one->num_of_children - 1 >= no_of_data / 2 ){
                    temp->primary_key[temp->num_of_leaf] = min_key_of(the_next_one->children[0]) ;
                    temp->num_of_leaf++ ;
                    temp->children[temp->num_of_children] = the_next_one->children[0] ;
                    temp->num_of_children++ ;
                    num = the_next_one->num_of_children - 1   ;
                    the_next_one->num_of_children-- ;
                    for (int i = 0 ; i < num  ; i++ ) {
                        the_next_one->children[i] = the_next_one->children[i+1];
                    }
                    the_next_one->children[num] = NULL ;
                    for (int i = 0 ; i < the_next_one->num_of_leaf - 1 ; i++ ){
                        the_next_one->primary_key[i] = the_next_one->primary_key[i+1] ;
                    }
                    the_next_one->num_of_leaf-- ;
                    the_next_one->primary_key[the_next_one->num_of_leaf] = -1 ;
                    the_correct_representative(temp_stack , first_pos , first_counter) ;
                    the_correct_representative(next_temp_stack , second_pos , first_counter) ;
                    the_correct_representative(temp_stack , first_pos , first_counter) ;
                    break ;
                }
                else {
                    temp->primary_key[temp->num_of_leaf] = min_key_of(the_next_one->children[0]) ;
                    temp->num_of_leaf++ ;
                    for ( int s = 0 ; s < the_next_one->num_of_leaf ; s++ ){
                        temp->primary_key[temp->num_of_leaf] = the_next_one->primary_key[s] ;
                        temp->num_of_leaf++ ;
                    }
                    for ( int s = 0 ; s < the_next_one->num_of_children ; s++  ){
                        temp->children[temp->num_of_children ] = the_next_one->children[s] ;
                        temp->num_of_children++ ;
                    }
                    termination_child(next_temp_stack[first_counter - 1] , second_pos[first_counter - 1]) ;
                    release_page(the_next_one) ;
                    free(the_next_one) ;
                    the_correct_representative(temp_stack , first_pos , first_counter) ;
                    the_correct_representative(next_temp_stack , second_pos , first_counter) ;
                    the_correct_representative(temp_stack , first_pos , first_counter) ;
                    second_counter = first_counter - 1 ;
                }
            }
   

        }

    }
    while (node->leaf != true && node->num_of_children == 1 ){
        internal_node * the_old_root = node ;
        node = node->children[0] ;
        release_page(the_old_root) ;
        free(the_old_root) ;
    }

    return node ;
}



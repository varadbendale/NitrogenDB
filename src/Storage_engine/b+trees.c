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
                return // error ; 
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
        stack[k]->num_of_data_pushed = stack[k]->num_of_leaf ;
        stack[k]->num_of_leaf++ ; 
    }

    else { 
        internal_node * temp_node ; 
        int catch_which_to_put_as_parent = stack[k]->primary_key[stack[k]->num_of_leaf / 2] ;  ; 
        int lame = k -1  ; 
        while (lame >= 0 && stack[lame]->num_of_leaf == no_of_data ){
            int low = stack_index[lame + 1] ;
            for (int i = stack[lame]->num_of_leaf ; i > low ; i--) {
                stack[lame]->primary_key[i] = stack[lame]->primary_key[i-1] ;
            }
            stack[lame]->primary_key[low] = catch_which_to_put_as_parent ; 
            stack[lame]->num_of_leaf++ ;

            for (int i = stack[lame]->num_of_children ; i > low + 1 ; i--) {
                stack[lame]->children[i] = stack[lame]->children[i-1] ;
            }

            stack[lame]->children[low + 1] = NULL ;
            stack[lame]->num_of_children++ ;
            catch_which_to_put_as_parent = temp_catch_which_to_put_as_parent ; 

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
                for (int i = greninja->num_of_leaf ; i > greninja_index ; i--) {
                    greninja->primary_key[i] = greninja->primary_key[i-1] ;
                }
                greninja->primary_key[greninja_index] = catch_which_to_put_as_parent ;    
                greninja->num_of_leaf++ ; 
                for (int i = greninja->num_of_children ; i > greninja_index + 1  ; i--) {
                    greninja->children[i] = greninja->children[i-1] ;
                }
                greninja->children[greninja_index + 1] = NULL ;
                greninja->num_of_children++ ;

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

            
            for (int o = tempacola  ; o < nadda->num_of_leaf  ; o++ ){
                tempo->primary_key[for_the_temp] = nadda->primary_key[o] ; 
                tempo->num_of_leaf++ ; 
                for_the_temp++ ; 
            }
            nadda->num_of_leaf = tempacola ;
            tempo->leaf = false ; 

            if ( nadda->leaf == false ){
                tempacola = nadda->num_of_children / 2 ; 
                for_the_temp = 0 ; 
                for (int o = tempacola  ; o < nadda->num_of_children ; o++ ){
                    tempo->children[for_the_temp] = nadda->children[o] ; 
                    nadda->children[o] = NULL ;
                    tempo->num_of_children++ ; 
                    for_the_temp++ ; 
                }
                nadda->num_of_children = tempacola + 1 ; 

                where_to_add->children[index_thing] = tempo ;

                int the_next_node_index_to_put = stack_index[kalos + 1] ;                              // FIX: position taken inside nadda is one level down
                if (the_next_node_index_to_put + 1 <= tempacola){
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


                where_to_add->children[index_thing] = tempo ;
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
        return node ; 
    }
    else { 
        internal_node * the_next_one = temp->next ; 
        if (the_next_one == NULL ){
            if (first_counter == 0){
                return node ; 
            }
            assert( first_pos[first_counter - 1]  > 0 ) ; 
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

                temp_stack[first_counter - 1]->primary_key[first_pos[first_counter - 1]  - 1] = prev->primary_key[prev->num_of_leaf / 2] ; 
                the_updated_mid = temp_stack[first_counter - 1]->primary_key[temp_stack[first_counter - 1]->num_of_leaf / 2] ; 
                first_steps = first_counter - 1 ; 
                k = first_counter - 1 ; 
                while (first_steps > 0){
                    if (first_pos[k-1] < temp_stack[k-1]->num_of_leaf){
                        temp_stack[k-1]->primary_key[first_pos[k-1]] = the_updated_mid ; 
                    }
                    the_updated_mid = temp_stack[k-1]->primary_key[temp_stack[k-1]->num_of_leaf / 2] ; 
                    k-- ; 
                    first_steps-- ; 
                }
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

            temp_stack[first_counter - 1]->children[first_pos[first_counter - 1] ] = NULL ; 
            temp_stack[first_counter - 1]->primary_key[first_pos[first_counter - 1]  - 1] = -1 ; 
            temp_stack[first_counter - 1]->num_of_leaf-- ; 
            temp_stack[first_counter - 1]->num_of_children-- ; 

            release_page(temp) ; 
            free(temp) ; 

            the_updated_mid = temp_stack[first_counter - 1]->primary_key[temp_stack[first_counter - 1]->num_of_leaf / 2] ; 
            first_steps = first_counter - 1 ; 
            k = first_counter - 1 ; 
            while (first_steps > 0){
                if (first_pos[k-1] < temp_stack[k-1]->num_of_leaf){
                    temp_stack[k-1]->primary_key[first_pos[k-1]] = the_updated_mid ; 
                }
                the_updated_mid = temp_stack[k-1]->primary_key[temp_stack[k-1]->num_of_leaf / 2] ; 
                k-- ; 
                first_steps-- ; 
            }


            for (int x = 0 ; x < 32 ; x++){
                next_temp_stack[x] = temp_stack[x] ; 
                second_pos[x] = first_pos[x] ; 
            }
            second_counter = first_counter - 1 ; 
        }
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
                if (next_one_pk <=  the_next_temp->primary_key[mid_second] ){
                    high_second  = mid_second - 1 ; 
                    if (next_one_pk ==  the_next_temp->primary_key[mid_second] ){
                        scnd_alrm = temp_counter ; 
                    }
                }
                else { 
                    low_second = mid_second + 1 ; 
                }
            }
            if (low_second < the_next_temp->num_of_leaf) {
                for_second_change[temp_counter] = the_next_temp->primary_key[low_second] ; 
            }
            else {
                for_second_change[temp_counter] = -1;
            }
            second_pos[temp_counter] = low_second ; 
            temp_counter++ ; 
            the_next_temp = the_next_temp->children[low_second] ; 
            next_temp_stack[k++] = the_next_temp ; 
        }
        k-- ; 
        second_counter = k ; 


        if (the_next_one != NULL && the_next_one->num_of_data_pushed - 1 >= no_of_data / 2 ){
            temp->primary_key[temp->num_of_leaf] = the_next_one->data[0].page_id ;  
            temp->data[temp->num_of_data_pushed] = the_next_one->data[0] ; 
            temp->num_of_data_pushed++ ; 
            temp->num_of_leaf++ ;

            num = the_next_one->num_of_data_pushed - 1   ; 
            the_next_one->num_of_data_pushed-- ; 
            memset(&the_next_one->data[0] , 0 , sizeof(the_next_one->data[0])) ;
            for (int i = 0 ; i < num  ; i++ ) {
                the_next_one->data[i] = the_next_one->data[i+1];
                the_next_one->primary_key[i] = the_next_one->primary_key[i+1];
            }
            memset(&the_next_one->data[num] , 0 , sizeof(the_next_one->data[num])) ;
            the_next_one->primary_key[num] = -1 ; 



            if (first_alrm >= 0 ){ 
                int the_updated_mid = temp->primary_key[temp->num_of_leaf / 2] ;
                int first_stuff = first_counter - first_alrm ; 
                k = first_counter ;  
                while (first_stuff > 0  ){
                    int m = ( temp_stack[ k-1 ]->num_of_leaf ) / 2 ; 
                    if (first_pos[ k-1 ] < temp_stack[ k-1 ]->num_of_leaf ){
                        temp_stack[ k-1 ]->primary_key[ first_pos[ k-1 ] ] = the_updated_mid ; 
                    }
                    the_updated_mid = temp_stack[ k -1  ]->primary_key[m]  ; 
                    k-- ;
                    first_stuff-- ; 
                }
            }

            if (scnd_alrm >= 0 ){                                         
                int the_updated_mid = the_next_one->primary_key[the_next_one->num_of_leaf / 2] ;
                int scnd_steps = second_counter - scnd_alrm ;
                k = second_counter ;
                while (scnd_steps > 0 ){
                    internal_node * par = next_temp_stack[ k-1 ] ;
                    if (second_pos[ k-1 ] < par->num_of_leaf ){
                        par->primary_key[ second_pos[ k-1 ] ] = the_updated_mid ;
                    }
                    the_updated_mid = par->primary_key[ par->num_of_leaf / 2 ] ;
                    k-- ;
                    scnd_steps-- ;
                }
            }

            if (first_alrm >= 0 ){ 
                int the_updated_mid = temp->primary_key[temp->num_of_leaf / 2] ;
                int first_stuff = first_counter - first_alrm ; 
                k = first_counter ;  
                while (first_stuff > 0  ){
                    int m = ( temp_stack[ k-1 ]->num_of_leaf ) / 2 ; 
                    if (first_pos[ k-1 ] < temp_stack[ k-1 ]->num_of_leaf ){
                        temp_stack[ k-1 ]->primary_key[ first_pos[ k-1 ] ] = the_updated_mid ; 
                    }
                    the_updated_mid = temp_stack[ k -1  ]->primary_key[m]  ; 
                    k-- ;
                    first_stuff-- ; 
                }
            }
        }

        
        else {
            int i = 0 ; 
            int j = 0 ; 
            internal_node * the_merger_one = NULL ;
            int the_merger_num = -1 ; 
            while (1){
                if (i <= first_counter && j <= second_counter && temp_stack[i] != NULL && next_temp_stack[j] != NULL && temp_stack[i] == next_temp_stack[j]  ){
                    the_merger_one = temp_stack[i] ;
                    the_merger_num = i ; 
                    i++ ; 
                    j++ ; 
                }
                else {
                    break ; 
                }
            }

            int lapoliza = first_counter -1 ; 
            int for_change_counter = first_counter ; 
            for ( int s = 0 ; s < the_next_one->num_of_leaf ; s++  ){
                temp->primary_key[temp->num_of_leaf] = the_next_one->primary_key[s] ; 
                temp->num_of_leaf++ ; 
            }
            for ( int s = 0 ; s < the_next_one->num_of_data_pushed ; s++  ){
                temp->children[temp->num_of_data_pushed ] = the_next_one->children[s] ; 
                temp->num_of_data_pushed++ ; 
            }

            internal_node * the_merged_node = temp ; 
            int bit = 0 ; 
            int catch_which_to_put_as_parent = -1 ; 
            while (  temp_stack[lapoliza] != NULL  && lapoliza >= the_merger_num  ){
                if (bit == 0 ){
                    temp_stack[lapoliza]->primary_key[for_change[lapoliza]] =  temp->primary_key[temp->num_of_leaf / 2 ]  ; 
                }
                else{
                    temp_stack[lapoliza]->primary_key[for_change[lapoliza]] =  temp_stack[lapoliza + 1]->primary_key[temp_stack[lapoliza + 1]->num_of_leaf / 2]  ; 
                }
                bit = 1 ; 
                lapoliza-- ; 
            }

            int j = the_merger_num ; 
            internal_node * just_there ; 
            while ( j <  second_counter ){
                internal_node * for_the_func_temp ; 
                if (the_merger_one->children[second_pos[j+1]] == NULL ){
                    return ; 
                }
                for_the_func_temp = the_merger_one->children[second_pos[j+1]]  ; 

                the_merger_one->primary_key[second_pos[j]] = for_the_func_temp->primary_key[second_pos[j + 1]]  ;
                the_merger_one = for_the_func_temp ; 
                j++ ;
            }

            just_there = the_merger_one ; 
            int final_to_reach_index = second_pos[second_counter - 1] ; 
            if ( just_there->num_of_leaf  > 0 ){
                int index ; 
                if (second_pos[second_counter - 1] < just_there->num_of_leaf ){
                    index = second_pos[second_counter - 1] ; 
                }
                else { 
                    index = second_pos[second_counter - 1] - 1 ; 
                }

                for (int m = index ; m < just_there->num_of_leaf - 1 ; m++ ) {
                    just_there->primary_key[m] = just_there->primary_key[m+1] ;
                }
                just_there->primary_key[just_there->num_of_leaf - 1 ] = -1 ; 

                for (int m = second_pos[second_counter - 1] ; m < just_there->num_of_data_pushed  ; m++ ) {
                    just_there->children[m] = just_there->children[m+1] ;
                }
                just_there->children[n] = NULL ;   
                just_there->num_of_leaf-- ;
            }
            if (second_pos[second_counter] < just_there->num_of_leaf && just_there->children[second_pos[second_counter]] != NULL  ){
                the_merger_one->primary_key[second_pos[second_counter]] = the_merger_one->children[second_pos[second_counter]]->primary_key[he_merger_one->children[second_pos[second_counter]]->num_of_leaf /2 ] ;
            }

            temp->next = the_next_one->next ;  
            free(the_next_one) ;


            // int remove_counter = 0 ; 
            // while ( temp_stack[remove_counter]->num_of_children == 1 ){
            //     remove_counter++ ; 
            // }
            // int kappa = 0 ; 
            // int jojo = remove_counter  ; 
            // int first = 0 ; 
            // while ( jojo < second_counter ){
            //     if (first  == 0 ){
            //         node = temp_stack[kappa ] ; 
            //     }
            //     temp_stack[kappa ] = temp_stack[jojo] ; 
            //     for_change[kappa] = for_change[jojo] ; 
            //     first_pos[kappa] = first_pos[jojo] ; 
            //     first_counter-- ; 
            //     next_temp_stack[kappa ] = next_temp_stack[jojo] ; 
            //     for_second_change[kappa] = for_second_change[jojo] ; 
            //     second_pos[kappa] = second_pos[jojo] ; 
            //     first_counter-- ; 
            //     kappa++ ; 
            //     jojo++ ; 
            // }
            // while ( kappa < remove_counter ){
            //     temp_stack[kappa ] = NULL ; 
            //     for_change[kappa] = -1 ; 
            //     first_pos[kappa] = -1  ; 
            //     next_temp_stack[kappa ] = NULL ; 
            //     for_second_change[kappa] = -1 ; 
            //     second_pos[kappa] = -1 ; 
            //     kappa++ ; 
            // }
            // if (validate_the_tree(node )  == 1 ){
            //      return ; //error ; 
            // }



            second_counter-- ; 
            while (second_counter >= 0 && next_temp_stack[second_counter]->num_of_children <  no_of_data / 2 ){
                for ( int i = 0 ; i < 32 ; i++ ){
                    temp_stack[i] = next_temp_stack[i] ; 
                    for_change[i] = for_second_change[i] ; 
                    first_pos[i] = second_pos[i] ; 
                }
                first_counter = second_counter ; 

                temp = temp_stack[first_counter] ; 
                int current_counter = first_counter - 1 ; 
                while (first_pos[current_counter] + 1 >  temp_stack[current_counter]->num_of_children ){
                    if (current_counter >= 0 ) {
                        current_counter-- ; 
                    }
                    else { 
                        assert( first_pos[first_counter - 1] > 0 ) ; 
                        internal_node * prev = temp_stack[first_counter - 1]->children[first_pos[first_counter - 1] - 1] ; 
                        int the_updated_mid ; 
                        int first_steps ; 
                        if (prev->num_of_children - 1 >= no_of_data / 2){
                            for (int x = temp->num_of_children ; x > 0 ; x--){
                                temp->children[x] = temp->children[x-1] ; 
                            }
                            for (int x = temp->num_of_leaf ; x > 0 ; x--){
                                temp->primary_key[x] = temp->primary_key[x-1] ; 
                            }
                            temp->children[0] = prev->children[prev->num_of_children - 1] ; 
                            temp->primary_key[0] = prev->children[prev->num_of_children - 1]->primary_key[prev->children[prev->num_of_children - 1]->num_of_leaf / 2] ; 
                            temp->num_of_children++ ; 
                            temp->num_of_leaf++ ; 

                            prev->children[prev->num_of_children - 1] = NULL ; 
                            prev->primary_key[prev->num_of_leaf - 1] = -1 ; 
                            prev->num_of_children-- ; 
                            prev->num_of_leaf-- ; 

                            temp_stack[first_counter - 1]->primary_key[first_pos[first_counter - 1] - 1] = prev->primary_key[prev->num_of_leaf / 2] ; 
                            the_updated_mid = temp_stack[first_counter - 1]->primary_key[temp_stack[first_counter - 1]->num_of_leaf / 2] ; 
                            first_steps = first_counter - 1 ; 
                            k = first_counter - 1 ; 
                            while (first_steps > 0){
                                if (first_pos[k-1] < temp_stack[k-1]->num_of_leaf){
                                    temp_stack[k-1]->primary_key[first_pos[k-1]] = the_updated_mid ; 
                                }
                                the_updated_mid = temp_stack[k-1]->primary_key[temp_stack[k-1]->num_of_leaf / 2] ; 
                                k-- ; 
                                first_steps-- ; 
                            }
                            break ; 
                        }

                        prev->primary_key[prev->num_of_leaf] = prev->children[prev->num_of_children - 1]->primary_key[prev->children[prev->num_of_children - 1]->num_of_leaf / 2] ; 
                        prev->num_of_leaf++ ; 

                        for (int s = 0 ; s < temp->num_of_leaf ; s++){
                            prev->primary_key[prev->num_of_leaf]  = temp->primary_key[s] ; 
                            prev->num_of_leaf++ ; 
                        }

                        for (int s = 0 ; s < temp->num_of_children ; s++){
                            prev->children[prev->num_of_children] =  temp->children[s] ; 
                            prev->num_of_children++ ; 
                        }

                        temp_stack[first_counter- 1]->children[first_pos[first_counter - 1]] = NULL ; 
                        temp_stack[first_counter- 1]->primary_key[first_pos[first_counter - 1] - 1] = -1 ; 
                        temp_stack[first_counter- 1]->num_of_leaf-- ; 
                        temp_stack[first_counter -1]->num_of_children-- ; 

                        release_page(temp) ; 
                        free(temp) ; 

                        the_updated_mid = temp_stack[first_counter - 1]->primary_key[temp_stack[first_counter - 1]->num_of_leaf / 2] ; 
                        first_steps = first_counter - 1 ; 
                        k = first_counter - 1 ; 
                        while (first_steps > 0){
                            if (first_pos[k-1] < temp_stack[k-1]->num_of_leaf){
                                temp_stack[k-1]->primary_key[first_pos[k-1]] = the_updated_mid ; 
                            }
                            the_updated_mid = temp_stack[k-1]->primary_key[temp_stack[k-1]->num_of_leaf / 2] ; 
                            k-- ; 
                            first_steps-- ; 
                        }

                        for (int x = 0 ; x < 32 ; x++){
                            next_temp_stack[x] = temp_stack[x] ; 
                            second_pos[x] = first_pos[x] ; 
                        }
                        second_counter = first_counter - 1 ; 
                        continue ; 
                    }
                }
                
                internal_node * merger_node  = temp_stack[current_counter] ; 
                int kj = 1 ; 
                internal_node * the_next_one  = merger_node->children[first_pos[current_counter] + 1]  ; 
                next_temp_stack[current_counter] = the_next_one ; 
                second_pos[current_counter]++ ; 
                while ( kj <= first_counter - current_counter ){
                    second_pos[current_counter + kj ] = 0 ; 
                    the_next_one = the_next_one->children[0] ; 
                    next_temp_stack[current_counter + kj] = the_next_one ; 
                    for_second_change[current_counter + kj ] = the_next_one->primary_key[0] ; 
                    kj++  ; 
                }
                internal_node * second_alrm_node = next_temp_stack[second_pos[0]] ; 
                scnd_alrm = current_counter ; 

                if (the_next_one != NULL && the_next_one->num_of_children - 1 >= no_of_data / 2 ){ 
                    temp->primary_key[temp->num_of_leaf] = temp->children[temp->num_of_children - 1]->primary_key[temp->children[temp->num_of_children - 1]->num_of_leaf / 2] ; 
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

                    if (scnd_alrm >= 0 ){                                         
                        int the_updated_mid = the_next_one->primary_key[the_next_one->num_of_leaf / 2] ;
                        int scnd_steps = second_counter - scnd_alrm ;
                        k = second_counter ;
                        while (scnd_steps > 0 ){
                            if (second_pos[ k-1 ] < next_temp_stack[ k-1 ]->num_of_leaf ){
                                next_temp_stack[ k-1 ]->primary_key[ second_pos[ k-1 ] ] = the_updated_mid ;
                            }
                            the_updated_mid = next_temp_stack[ k-1 ]->primary_key[ next_temp_stack[ k-1 ]->num_of_leaf / 2 ] ;
                            k-- ;
                            scnd_steps-- ;
                        }
                    }
                    break ; 
                }

                else {
                    int i = 0 ; 
                    int j = 0 ; 
                    internal_node * the_merger_one = NULL ;
                    int the_merger_num = -1 ; 
                    while (1){
                        if (i <= first_counter && j <= second_counter && temp_stack[i] != NULL && next_temp_stack[j] != NULL && temp_stack[i] == next_temp_stack[j]  ){
                            the_merger_one = temp_stack[i] ;
                            the_merger_num = i ; 
                            i++ ; 
                            j++ ; 
                        }
                        else {
                            break ; 
                        }
                    }

                    temp->primary_key[temp->num_of_leaf] = temp->children[temp->num_of_children - 1]->primary_key[temp->children[temp->num_of_children - 1]->num_of_leaf / 2] ; 
                    temp->num_of_leaf++ ; 
                    int for_change_counter = first_counter ; 
                    for ( int s = 0 ; s < the_next_one->num_of_leaf ; s++ ){
                        temp->primary_key[temp->num_of_leaf] = the_next_one->primary_key[s] ; 
                        temp->num_of_leaf++ ; 
                    }
                    for ( int s = 0 ; s < the_next_one->num_of_data_pushed ; s++  ){
                        temp->children[temp->num_of_data_pushed ] = the_next_one->children[s] ; 
                        temp->num_of_children++ ; 
                    }

                    int lapoliza = first_counter -1 ; 
                    int bit = 0 ; 
                    int catch_which_to_put_as_parent = -1 ; 

                    while (  temp_stack[lapoliza] != NULL  && lapoliza >= the_merger_num  ){
                        if (first_pos[lapoliza] < temp_stack[lapoliza]->num_of_leaf ){  
                            if (bit == 0 ){
                                temp_stack[lapoliza]->primary_key[for_change[lapoliza]] =  temp->primary_key[temp->num_of_leaf / 2 ]  ; 
                            }
                            else{
                                temp_stack[lapoliza]->primary_key[for_change[lapoliza]] =  temp_stack[lapoliza + 1]->primary_key[temp_stack[lapoliza + 1]->num_of_leaf / 2]  ; 
                            }
                        }
                        bit = 1 ; 
                        lapoliza-- ; 
                    }

                    int j = the_merger_num ; 
                    internal_node * just_there ; 
                    while ( j <  second_counter - 1){
                        internal_node * for_the_func_temp ; 
                        if (the_merger_one->children[second_pos[j+1]] == NULL ){
                            return ; 
                        }
                        for_the_func_temp = the_merger_one->children[second_pos[j+1]]  ; 
                        if (second_pos[j] < the_merger_one->num_of_leaf && second_pos[j + 1] < for_the_func_temp->num_of_leaf ){ 
                            the_merger_one->primary_key[second_pos[j]] = for_the_func_temp->primary_key[second_pos[j + 1]] ;
                        }   
                        the_merger_one = for_the_func_temp ; 
                        j++ ;
                    }

                    just_there = the_merger_one ; 
                    int final_to_reach_index = second_pos[second_counter - 1] ; 
                    if ( just_there->num_of_leaf  > 0 ){
                        int index ; 
                        if (final_to_reach_index < just_there->num_of_leaf ){
                            index = final_to_reach_index; 
                        }
                        else { 
                            index = final_to_reach_index - 1 ; 
                        }

                        for (int m = index ; m < just_there->num_of_leaf - 1 ; m++ ) {
                            just_there->primary_key[m] = just_there->primary_key[m+1] ;
                        }
                        just_there->primary_key[just_there->num_of_leaf - 1 ] = -1 ; 

                        for (int m = final_to_reach_index ; m < just_there->num_of_children  ; m++ ) {
                            just_there->children[m] = just_there->children[m+1] ;
                        }
                        just_there->children[just_there->num_of_children] = NULL ;   
                        just_there->num_of_leaf-- ;
                        just_there->num_of_children-- ;   
                    }
                    if (final_to_reach_index < just_there->num_of_leaf && just_there->children[final_to_reach_index] != NULL  ){
                        just_there->primary_key[p] = just_there->children[p]->primary_key[just_there->children[p]->num_of_leaf / 2] ; 
                    }

                    free(the_next_one) ;

                    second_counter-- ; 
                    if (next_temp_stack[second_counter]->num_of_children >=  no_of_data / 2 ){
                        break ; 
                    }
                    // remove_counter = 0 ; 
                    // while ( temp_stack[remove_counter]->num_of_children == 1 ){
                    //     remove_counter++ ; 
                    // }
                    // kappa = 0 ; 
                    // jojo = remove_counter  ; 
                    // first = 0 ; 
                    // while ( jojo < second_counter ){
                    //     if (first  == 0 ){
                    //         node = temp_stack[kappa ] ; 
                    //     }
                    //     temp_stack[kappa ] = temp_stack[jojo] ; 
                    //     for_change[kappa] = for_change[jojo] ; 
                    //     first_pos[kappa] = first_pos[jojo] ; 
                    //     first_counter-- ; 
                    //     next_temp_stack[kappa ] = next_temp_stack[jojo] ; 
                    //     for_second_change[kappa] = for_second_change[jojo] ; 
                    //     second_pos[kappa] = second_pos[jojo] ; 
                    //     first_counter-- ; 
                    //     kappa++ ; 
                    //     jojo++ ; 
                    // }
                    // while ( kappa < remove_counter ){
                    //     temp_stack[kappa ] = NULL ; 
                    //     for_change[kappa] = -1 ; 
                    //     first_pos[kappa] = -1  ; 
                    //     next_temp_stack[kappa ] = NULL ; 
                    //     for_second_change[kappa] = -1 ; 
                    //     second_pos[kappa] = -1 ; 
                    //     kappa++ ; 
                    // }
                    if (validate_the_tree(node )  == 1 ){
                        return ; //error ; 
                    }
                }
            }
        }

    }

}


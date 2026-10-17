#include "b+trees.h"

#define OFF 1000000
#define KEYSPACE 5000000
#define MAX_DEPTH 32
#define TIMEOUT_SECONDS 120

static unsigned char *present ;
static int *inserted ;
static int inserted_count = 0 ;
static internal_node *root = NULL ;
static unsigned long long rng_state = 88172645463325252ULL ;
static char errbuf[256] ;
static int leaf_depth_seen ;

static unsigned long long rnd(void){
    rng_state ^= rng_state << 13 ;
    rng_state ^= rng_state >> 7 ;
    rng_state ^= rng_state << 17 ;
    return rng_state ;
}

static void shuffle(int *a , int n){
    for (int i = n - 1 ; i > 0 ; i--){
        int j = (int)(rnd() % (unsigned long long)(i + 1)) ;
        int t = a[i] ;
        a[i] = a[j] ;
        a[j] = t ;
    }
}

static int fail(const char *fmt , ...){
    va_list args ;
    va_start(args , fmt) ;
    vsnprintf(errbuf , sizeof(errbuf) , fmt , args) ;
    va_end(args) ;
    return 1 ;
}

static void reset_tree(void){
    root = (internal_node *)calloc(1 , sizeof(internal_node)) ;
    memset(present , 0 , KEYSPACE) ;
    inserted_count = 0 ;
}

static void ins(int key){
    if (key + OFF < 0 || key + OFF >= KEYSPACE){
        fprintf(stderr , "test bug: key %d outside tracked key space\n" , key) ;
        _exit(2) ;
    }
    tree_func_for_insert(root , key , NULL , key , key * 2 , key % 7) ;
    if (!present[key + OFF]){
        present[key + OFF] = 1 ;
        inserted[inserted_count++] = key ;
    }
}

static int validate_the_tree_data(internal_node *tree_root){
    internal_node *n = tree_root ;
    int depth = 0 ;

    while (!n->leaf){
        if (n->num_of_children < 1 || n->children[0] == NULL){
            return fail("cannot walk down to first leaf") ;
        }
        if (++depth > MAX_DEPTH){
            return fail("tree deeper than %d levels" , MAX_DEPTH) ;
        }
        n = n->children[0] ;
    }

    long long prev = LLONG_MIN ;
    long long count = 0 ;
    long long hops = 0 ;

    while (n != NULL){
        if (++hops > (long long)inserted_count + 2){
            return fail("leaf chain too long or cyclic") ;
        }
        for (int i = 0 ; i < n->num_of_leaf ; i++){
            long long key = n->primary_key[i] ;
            if (key <= prev){
                return fail("leaf chain not ascending at key %lld" , key) ;
            }
            prev = key ;
            if (key + OFF < 0 || key + OFF >= KEYSPACE || !present[key + OFF]){
                return fail("leaf chain has unknown key %lld" , key) ;
            }
            if (n->data[i].page_id != (int)key || n->data[i].file_offset != (int)key * 2 || n->data[i].slot_num != (int)key % 7){
                return fail("data mismatch for key %lld" , key) ;
            }
            count++ ;
        }
        n = n->next ;
    }

    if (count != inserted_count){
        return fail("leaf chain has %lld keys, expected %d" , count , inserted_count) ;
    }
    return 0 ;
}

static int validate_the_tree_node(internal_node *n , long long low , long long high , int depth){
    if (n == NULL){
        return fail("NULL child at depth %d" , depth) ;
    }
    if (depth >= MAX_DEPTH){
        return fail("tree deeper than %d levels" , MAX_DEPTH) ;
    }
    if (n->num_of_leaf < 0 || n->num_of_leaf > no_of_data){
        return fail("bad num_of_leaf %d at depth %d" , n->num_of_leaf , depth) ;
    }

    for (int i = 0 ; i < n->num_of_leaf ; i++){
        long long key = n->primary_key[i] ;
        if (key < low || key >= high){
            return fail("key %lld outside range [%lld,%lld) at depth %d" , key , low , high , depth) ;
        }
        if (i > 0 && n->primary_key[i - 1] >= key){
            return fail("keys not strictly ascending at depth %d idx %d" , depth , i) ;
        }
    }

    if (n->leaf){
        if (leaf_depth_seen == -1){
            leaf_depth_seen = depth ;
        }
        else if (leaf_depth_seen != depth){
            return fail("leaves at different depths %d and %d" , leaf_depth_seen , depth) ;
        }
        return 0 ;
    }

    if (n->num_of_children != n->num_of_leaf + 1){
        return fail("children %d != keys %d + 1 at depth %d" , n->num_of_children , n->num_of_leaf , depth) ;
    }

    for (int i = 0 ; i <= n->num_of_leaf ; i++){
        long long child_low = (i == 0) ? low : n->primary_key[i - 1] ;
        long long child_high = (i == n->num_of_leaf) ? high : n->primary_key[i] ;
        if (validate_the_tree_node(n->children[i] , child_low , child_high , depth + 1) == 1){
            return 1 ;
        }
    }
    return 0 ;
}

static int validate_the_tree(internal_node *tree_root){
    errbuf[0] = 0 ;
    leaf_depth_seen = -1 ;

    if (tree_root == NULL){
        return fail("root is NULL") ;
    }
    if (tree_root->num_of_leaf == 0 && tree_root->num_of_children == 0){
        if (inserted_count != 0){
            return fail("tree is empty but %d keys were inserted" , inserted_count) ;
        }
        return 0 ;
    }
    if (validate_the_tree_node(tree_root , LLONG_MIN , LLONG_MAX , 0) == 1){
        return 1 ;
    }
    if (validate_the_tree_data(tree_root) == 1){
        return 1 ;
    }
    return 0 ;
}

static bool find_key(internal_node *n , int key , leaf_node_data *out){
    int depth = 0 ;

    while (n != NULL && !n->leaf){
        if (++depth > MAX_DEPTH){
            return false ;
        }
        int low = 0 ;
        int high = n->num_of_leaf ;
        while (low < high){
            int mid = (low + high) / 2 ;
            if (n->primary_key[mid] <= key){
                low = mid + 1 ;
            }
            else {
                high = mid ;
            }
        }
        n = n->children[low] ;
    }

    if (n == NULL){
        return false ;
    }

    for (int i = 0 ; i < n->num_of_leaf ; i++){
        if (n->primary_key[i] == key){
            *out = n->data[i] ;
            return true ;
        }
    }
    return false ;
}

static bool validate_lookup(void){
    for (int i = 0 ; i < inserted_count ; i++){
        int key = inserted[i] ;
        leaf_node_data d ;
        if (!find_key(root , key , &d)){
            fail("lookup failed for key %d" , key) ;
            return false ;
        }
        if (d.page_id != key || d.file_offset != key * 2 || d.slot_num != key % 7){
            fail("lookup data mismatch for key %d" , key) ;
            return false ;
        }
    }
    return true ;
}

static bool check_all(void){
    if (validate_the_tree(root) == 1){
        return false ;
    }
    return validate_lookup() ;
}

static bool test_null_root(int n , int p , unsigned long long seed){
    tree_func_for_insert(NULL , 5 , NULL , 1 , 1 , 1) ;
    return true ;
}

static bool test_empty(int n , int p , unsigned long long seed){
    reset_tree() ;
    return check_all() ;
}

static bool test_single(int n , int p , unsigned long long seed){
    reset_tree() ;
    ins(42) ;
    return check_all() ;
}

static bool test_fill(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(i) ;
    }
    return check_all() ;
}

static bool test_descending(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = n - 1 ; i >= 0 ; i--){
        ins(i) ;
    }
    return check_all() ;
}

static bool test_descending_negative(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 1 ; i <= n ; i++){
        ins(-i) ;
    }
    return check_all() ;
}

static bool test_zigzag(int n , int p , unsigned long long seed){
    reset_tree() ;
    int lo = 0 ;
    int hi = n - 1 ;
    while (lo <= hi){
        ins(lo++) ;
        if (lo <= hi){
            ins(hi--) ;
        }
    }
    return check_all() ;
}

static bool test_middle_out(int n , int p , unsigned long long seed){
    reset_tree() ;
    int mid = n / 2 ;
    for (int d = 0 ; d <= n / 2 ; d++){
        if (mid + d < n){
            ins(mid + d) ;
        }
        if (d > 0 && mid - d >= 0){
            ins(mid - d) ;
        }
    }
    return check_all() ;
}

static bool test_random_unique(int n , int p , unsigned long long seed){
    rng_state = seed ;
    reset_tree() ;
    int *arr = (int *)malloc(sizeof(int) * n) ;
    for (int i = 0 ; i < n ; i++){
        arr[i] = i ;
    }
    shuffle(arr , n) ;
    for (int i = 0 ; i < n ; i++){
        ins(arr[i]) ;
    }
    free(arr) ;
    return check_all() ;
}

static bool test_random_with_duplicates(int n , int p , unsigned long long seed){
    rng_state = seed ;
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins((int)(rnd() % (unsigned long long)p)) ;
    }
    return check_all() ;
}

static bool test_negative_random(int n , int p , unsigned long long seed){
    rng_state = seed ;
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins((int)(rnd() % 1500000ULL) - 500000) ;
    }
    return check_all() ;
}

static bool test_same_key_repeat(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(7) ;
    }
    return check_all() ;
}

static bool test_reinsert_all(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(i) ;
    }
    for (int i = 0 ; i < n ; i++){
        ins(i) ;
    }
    return check_all() ;
}

static bool test_negative_to_positive(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = -n ; i < n ; i++){
        ins(i) ;
    }
    return check_all() ;
}

static bool test_stride(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(i * p) ;
    }
    return check_all() ;
}

static bool test_fill_gaps(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i += 2){
        ins(i) ;
    }
    for (int i = 1 ; i < n ; i += 2){
        ins(i) ;
    }
    return check_all() ;
}

static bool test_blocks_reverse(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int b = n - 1 ; b >= 0 ; b--){
        for (int i = 0 ; i < p ; i++){
            ins(b * p + i) ;
        }
    }
    return check_all() ;
}

static bool test_sawtooth(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins((i % p) * (n / p + 1) + i / p) ;
    }
    return check_all() ;
}

static bool test_two_streams(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(i) ;
        ins(3000000 - i) ;
    }
    return check_all() ;
}

static bool test_bit_reversal(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < (1 << p) ; i++){
        int r = 0 ;
        for (int b = 0 ; b < p ; b++){
            if ((i >> b) & 1){
                r |= 1 << (p - 1 - b) ;
            }
        }
        ins(r) ;
    }
    return check_all() ;
}

static bool test_leaf_boundary(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < no_of_data ; i++){
        ins(i * 10) ;
    }
    int mid = no_of_data / 2 ;
    if (p == 0){
        ins(-1) ;
    }
    else if (p == 1){
        ins(no_of_data * 10) ;
    }
    else if (p == 2){
        ins(mid * 10 + 5) ;
    }
    else if (p == 3){
        ins(mid * 10 - 5) ;
    }
    else if (p == 4){
        ins(mid * 10 + 1) ;
    }
    else if (p == 5){
        ins(mid * 10 - 1) ;
    }
    else if (p == 6){
        ins(10 - 5) ;
    }
    else if (p == 7){
        ins((no_of_data - 1) * 10 - 5) ;
    }
    return check_all() ;
}

static bool test_stepwise_ascending(int n , int p , unsigned long long seed){
    reset_tree() ;
    for (int i = 0 ; i < n ; i++){
        ins(i) ;
        if (validate_the_tree(root) == 1){
            char saved[256] ;
            snprintf(saved , sizeof(saved) , "%s" , errbuf) ;
            fail("after insert #%d (key %d): %s" , i , i , saved) ;
            return false ;
        }
    }
    return true ;
}

static bool test_stepwise_random(int n , int p , unsigned long long seed){
    rng_state = seed ;
    reset_tree() ;
    int *arr = (int *)malloc(sizeof(int) * n) ;
    for (int i = 0 ; i < n ; i++){
        arr[i] = i ;
    }
    shuffle(arr , n) ;
    for (int i = 0 ; i < n ; i++){
        ins(arr[i]) ;
        if (validate_the_tree(root) == 1){
            char saved[256] ;
            snprintf(saved , sizeof(saved) , "%s" , errbuf) ;
            fail("after insert #%d (key %d): %s" , i , arr[i] , saved) ;
            free(arr) ;
            return false ;
        }
    }
    free(arr) ;
    return true ;
}

typedef bool (*test_fn)(int n , int p , unsigned long long seed) ;

typedef struct test_case{
    const char *name ;
    test_fn fn ;
    int n ;
    int p ;
    unsigned long long seed ;
}test_case ;

static int run_case(const test_case *t){
    fflush(stdout) ;
    pid_t pid = fork() ;

    if (pid == 0){
        alarm(TIMEOUT_SECONDS) ;
        bool ok = t->fn(t->n , t->p , t->seed) ;
        if (ok){
            printf("[PASS]    %-45s keys=%d\n" , t->name , inserted_count) ;
        }
        else {
            printf("[FAIL]    %-45s keys=%d  reason: %s\n" , t->name , inserted_count , errbuf) ;
        }
        fflush(stdout) ;
        _exit(ok ? 0 : 1) ;
    }

    int status = 0 ;
    waitpid(pid , &status , 0) ;

    if (WIFSIGNALED(status)){
        int sig = WTERMSIG(status) ;
        if (sig == SIGALRM){
            printf("[TIMEOUT] %-45s no result after %d seconds\n" , t->name , TIMEOUT_SECONDS) ;
        }
        else {
            printf("[CRASH]   %-45s signal %d (%s)\n" , t->name , sig , strsignal(sig)) ;
        }
        return 2 ;
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0){
        return 0 ;
    }
    return 1 ;
}

int main(void){
    present = (unsigned char *)calloc(KEYSPACE , 1) ;
    inserted = (int *)malloc(sizeof(int) * 3000000) ;

    test_case tests[] = {
        {"insert into NULL root does not crash" , test_null_root , 0 , 0 , 0} ,
        {"empty tree" , test_empty , 0 , 0 , 0} ,
        {"single insert" , test_single , 0 , 0 , 0} ,
        {"leaf holds no_of_data-1 keys" , test_fill , no_of_data - 1 , 0 , 0} ,
        {"leaf holds exactly no_of_data keys" , test_fill , no_of_data , 0 , 0} ,
        {"first leaf split (no_of_data+1 keys)" , test_fill , no_of_data + 1 , 0 , 0} ,
        {"second leaf split (1.5*no_of_data keys)" , test_fill , no_of_data * 3 / 2 , 0 , 0} ,
        {"2*no_of_data keys ascending" , test_fill , no_of_data * 2 , 0 , 0} ,
        {"leaf full then insert at front" , test_leaf_boundary , 0 , 0 , 0} ,
        {"leaf full then insert at end" , test_leaf_boundary , 0 , 1 , 0} ,
        {"leaf full then insert at middle" , test_leaf_boundary , 0 , 2 , 0} ,
        {"leaf full then insert just before split" , test_leaf_boundary , 0 , 3 , 0} ,
        {"leaf full then insert just after split" , test_leaf_boundary , 0 , 4 , 0} ,
        {"leaf full then insert right at split key-1" , test_leaf_boundary , 0 , 5 , 0} ,
        {"leaf full then insert second slot" , test_leaf_boundary , 0 , 6 , 0} ,
        {"leaf full then insert second last slot" , test_leaf_boundary , 0 , 7 , 0} ,
        {"same key repeated 1000 times" , test_same_key_repeat , 1000 , 0 , 0} ,
        {"reinsert all keys after splits" , test_reinsert_all , 5000 , 0 , 0} ,
        {"negative and positive ascending" , test_negative_to_positive , 20000 , 0 , 0} ,
        {"descending negative keys" , test_descending_negative , 20000 , 0 , 0} ,
        {"negative and positive random" , test_negative_random , 60000 , 0 , 123456789ULL} ,
        {"stride 1000 ascending" , test_stride , 3000 , 1000 , 0} ,
        {"stride 7 ascending" , test_stride , 20000 , 7 , 0} ,
        {"evens first then odds fill gaps" , test_fill_gaps , 30000 , 0 , 0} ,
        {"descending blocks of ascending keys" , test_blocks_reverse , 100 , 1000 , 0} ,
        {"sawtooth pattern period 10" , test_sawtooth , 50000 , 10 , 0} ,
        {"sawtooth pattern period 1000" , test_sawtooth , 50000 , 1000 , 0} ,
        {"two streams low ascending high descending" , test_two_streams , 30000 , 0 , 0} ,
        {"bit reversal order 2^12" , test_bit_reversal , 0 , 12 , 0} ,
        {"bit reversal order 2^16" , test_bit_reversal , 0 , 16 , 0} ,
        {"ascending 2000" , test_fill , 2000 , 0 , 0} ,
        {"ascending 50000" , test_fill , 50000 , 0 , 0} ,
        {"descending 2000" , test_descending , 2000 , 0 , 0} ,
        {"descending 50000" , test_descending , 50000 , 0 , 0} ,
        {"zigzag low/high 50000" , test_zigzag , 50000 , 0 , 0} ,
        {"middle-out 50000" , test_middle_out , 50000 , 0 , 0} ,
        {"stepwise validate ascending 3000" , test_stepwise_ascending , 3000 , 0 , 0} ,
        {"stepwise validate descending-ish random seed 1" , test_stepwise_random , 3000 , 0 , 1ULL} ,
        {"stepwise validate random seed 2" , test_stepwise_random , 3000 , 0 , 2ULL} ,
        {"stepwise validate random seed 3" , test_stepwise_random , 3000 , 0 , 3ULL} ,
        {"random unique 10000 seed 11" , test_random_unique , 10000 , 0 , 11ULL} ,
        {"random unique 10000 seed 22" , test_random_unique , 10000 , 0 , 22ULL} ,
        {"random unique 100000 seed 33" , test_random_unique , 100000 , 0 , 33ULL} ,
        {"random 20000 inserts in range 500" , test_random_with_duplicates , 20000 , 500 , 44ULL} ,
        {"random 100000 inserts in range 50000" , test_random_with_duplicates , 100000 , 50000 , 55ULL} ,
        {"random 400000 inserts in range 100000" , test_random_with_duplicates , 400000 , 100000 , 56ULL} ,
        {"ascending 58000 (root nearly full)" , test_fill , 58000 , 0 , 0} ,
        {"ascending 60000 (first internal split)" , test_fill , 60000 , 0 , 0} ,
        {"ascending 120000 (internal splits)" , test_fill , 120000 , 0 , 0} ,
        {"ascending 200000 (internal splits)" , test_fill , 200000 , 0 , 0} ,
        {"descending 200000 (internal splits)" , test_descending , 200000 , 0 , 0} ,
        {"random unique 300000 (internal splits)" , test_random_unique , 300000 , 0 , 66ULL} ,
        {"ascending 1500000 (deep tree)" , test_fill , 1500000 , 0 , 0} ,
        {"random unique 1500000 (deep tree)" , test_random_unique , 1500000 , 0 , 77ULL}
    } ;

    int count = (int)(sizeof(tests) / sizeof(tests[0])) ;
    int passed = 0 ;
    int failed = 0 ;
    int crashed = 0 ;

    for (int i = 0 ; i < count ; i++){
        int result = run_case(&tests[i]) ;
        if (result == 0){
            passed++ ;
        }
        else if (result == 1){
            failed++ ;
        }
        else {
            crashed++ ;
        }
    }

    printf("\nPASSED: %d   FAILED: %d   CRASHED/TIMEOUT: %d   TOTAL: %d\n" , passed , failed , crashed , count) ;

    free(present) ;
    free(inserted) ;
    return (failed == 0 && crashed == 0) ? 0 : 1 ;
}

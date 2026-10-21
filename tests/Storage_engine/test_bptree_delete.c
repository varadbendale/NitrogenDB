#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <wal.h>

static long g_alloc = 0;

static void * t_malloc(size_t n)
{
    g_alloc++;
    return malloc(n);
}

static void * t_calloc(size_t a, size_t b)
{
    g_alloc++;
    return calloc(a, b);
}

static void t_free(void * p)
{
    if (p) {
        g_alloc--;
    }
    free(p);
}

#define malloc(n)    t_malloc(n)
#define calloc(a, b) t_calloc(a, b)
#define free(p)      t_free(p)

#ifdef USE_MY_HEADER
#include "your_header.h"
#else

#ifndef NO_OF_DATA
#define NO_OF_DATA 4
#endif
#define no_of_data NO_OF_DATA

typedef struct record_slot {
    int page_id;
    int file_offset;
    int slot_num;
} record_slot;

typedef struct internal_node internal_node;

struct internal_node {
    bool leaf;
    int num_of_leaf;
    int num_of_children;
    int num_of_data_pushed;
    int primary_key[no_of_data + 2];
    record_slot data[no_of_data + 2];
    internal_node * children[no_of_data + 3];
    internal_node * next;
};

static void release_page(internal_node * n)
{
    (void)n;
}

#endif

#ifndef KEYMAX
#define KEYMAX 400
#endif
#define LOGMAX 100000

static int present[KEYMAX + 1];
static int log_op[LOGMAX];
static int log_key[LOGMAX];
static int log_n = 0;
static int cur_seed = 0;
static int cur_order = 0;
static int key_limit = KEYMAX;
static internal_node * g_root = NULL;
static int leaf_depth;

#define BAD(...)                    \
    do {                            \
        printf("  INVARIANT: ");    \
        printf(__VA_ARGS__);        \
        printf("\n");               \
        return 1;                   \
    } while (0)

#define XEXPECT(c, msg)                                                         \
    do {                                                                        \
        if (!(c)) {                                                             \
            printf("  EXPECT FAILED (line %d): %s\n", __LINE__, msg);           \
            return 1;                                                           \
        }                                                                       \
    } while (0)

#define T(call)                                                                 \
    do {                                                                        \
        if (call) {                                                             \
            printf("FAILED: %s  (seed %d, order %d)\n",                         \
                   #call, cur_seed, no_of_data);                                \
            return 1;                                                           \
        }                                                                       \
    } while (0)

#define RUN(name, call)                 \
    do {                                \
        printf("[%s] ", name);          \
        fflush(stdout);                 \
        if (call) {                     \
            printf("FAILED\n");         \
            total_fail++;               \
        } else {                        \
            printf("ok\n");             \
        }                               \
    } while (0)

#define NPAT  5
#define NMODE 7

static internal_node * new_root(void)
{
    internal_node * r = (internal_node *)calloc(1, sizeof(internal_node));
    r->leaf = true;
    return r;
}

static void free_tree(internal_node * n)
{
    if (n == NULL) {
        return;
    }
    if (!n->leaf) {
        for (int i = 0; i < n->num_of_children; i++) {
            free_tree(n->children[i]);
        }
    }
    free(n);
}

static int check_node(internal_node * n, int depth, int lo, int hi, int is_root)
{
    if (n == NULL) {
        BAD("NULL node at depth %d", depth);
    }

    if (n->leaf) {
        if (leaf_depth == -1) {
            leaf_depth = depth;
        } else if (leaf_depth != depth) {
            BAD("leaves at different depths (%d vs %d)", leaf_depth, depth);
        }
        if (n->num_of_leaf != n->num_of_data_pushed) {
            BAD("leaf num_of_leaf %d != num_of_data_pushed %d",
                n->num_of_leaf, n->num_of_data_pushed);
        }
        if (n->num_of_leaf > no_of_data) {
            BAD("leaf overflow %d", n->num_of_leaf);
        }
        if (!is_root && n->num_of_data_pushed < no_of_data / 2) {
            BAD("leaf underflow %d (min %d)", n->num_of_data_pushed, no_of_data / 2);
        }
        for (int i = 0; i < n->num_of_leaf; i++) {
            if (i > 0 && n->primary_key[i] <= n->primary_key[i - 1]) {
                BAD("leaf keys not sorted");
            }
            if (n->primary_key[i] < lo || n->primary_key[i] >= hi) {
                BAD("leaf key %d outside [%d,%d)", n->primary_key[i], lo, hi);
            }
        }
        return 0;
    }

    if (n->num_of_children != n->num_of_leaf + 1) {
        BAD("internal children %d != keys+1 (%d)", n->num_of_children, n->num_of_leaf + 1);
    }
    if (is_root && n->num_of_children < 2) {
        BAD("root has %d child", n->num_of_children);
    }
    if (!is_root && n->num_of_children < no_of_data / 2) {
        BAD("internal underflow %d children", n->num_of_children);
    }
    if (n->num_of_children > no_of_data + 1) {
        BAD("internal overflow %d children", n->num_of_children);
    }

    for (int i = 0; i < n->num_of_leaf; i++) {
        if (i > 0 && n->primary_key[i] <= n->primary_key[i - 1]) {
            BAD("internal keys not sorted");
        }
        if (n->primary_key[i] < lo || n->primary_key[i] >= hi) {
            BAD("separator %d outside [%d,%d)", n->primary_key[i], lo, hi);
        }
    }

    for (int i = 0; i < n->num_of_children; i++) {
        int child_lo = (i == 0) ? lo : n->primary_key[i - 1];
        int child_hi = (i == n->num_of_leaf) ? hi : n->primary_key[i];
        if (check_node(n->children[i], depth + 1, child_lo, child_hi, 0)) {
            return 1;
        }
    }
    return 0;
}

static int find_key(internal_node * root, int key)
{
    internal_node * t = root;

    while (!t->leaf) {
        int i = 0;
        while (i < t->num_of_leaf && key >= t->primary_key[i]) {
            i++;
        }
        t = t->children[i];
        if (!t) {
            return 0;
        }
    }

    for (int i = 0; i < t->num_of_leaf; i++) {
        if (t->primary_key[i] == key) {
            return 1;
        }
    }
    return 0;
}

static int check_tree(internal_node * root)
{
    leaf_depth = -1;
    if (check_node(root, 0, INT_MIN, INT_MAX, 1)) {
        return 1;
    }

    internal_node * t = root;
    while (!t->leaf) {
        t = t->children[0];
    }

    int expect = 1;
    int count = 0;
    int ref_count = 0;
    long guard = 0;

    for (int k = 1; k <= key_limit; k++) {
        ref_count += present[k];
    }

    while (t) {
        if (++guard > 1000000) {
            BAD("leaf chain does not end (cycle?)");
        }
        for (int i = 0; i < t->num_of_leaf; i++) {
            while (expect <= key_limit && !present[expect]) {
                expect++;
            }
            if (expect > key_limit || t->primary_key[i] != expect) {
                BAD("leaf chain: got %d, expected %d", t->primary_key[i], expect);
            }
            expect++;
            count++;
        }
        t = t->next;
    }

    if (count != ref_count) {
        BAD("chain has %d keys, reference has %d", count, ref_count);
    }

    for (int k = 1; k <= key_limit; k++) {
        if (present[k] != find_key(root, k)) {
            BAD("find(%d)=%d but reference=%d", k, find_key(root, k), present[k]);
        }
    }
    return 0;
}

static void dump_fail(void)
{
    printf("  order=%d seed=%d key_limit=%d, ops so far (%d), last 60:\n  ",
           cur_order, cur_seed, key_limit, log_n);

    int start = (log_n > 60) ? log_n - 60 : 0;
    for (int i = start; i < log_n; i++) {
        printf("%c%d ", log_op[i] ? 'D' : 'I', log_key[i]);
    }
    printf("\n  failing op: %c%d\n",
           log_op[log_n - 1] ? 'D' : 'I', log_key[log_n - 1]);
}

static int apply(internal_node ** root, int op, int key)
{
    if (log_n < LOGMAX) {
        log_op[log_n] = op;
        log_key[log_n] = key;
        log_n++;
    }

    if (op == 0) {
        tree_func_for_insert(*root, key, NULL, key, 0, 0);
        present[key] = 1;
    } else {
        *root = delete_the_thing(*root, key);
        present[key] = 0;
    }

    g_root = *root;
    if (check_tree(*root)) {
        g_root = NULL;
        dump_fail();
        return 1;
    }
    return 0;
}

static void reset(internal_node ** root)
{
    memset(present, 0, sizeof(present));
    log_n = 0;
    if (g_root) {
        free_tree(g_root);
    }
    *root = new_root();
    g_root = *root;
    srand(cur_seed);
}

static void shuffle(int * a, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}

static int phase_insert_only(int n, int mode)
{
    internal_node * r;
    int a[KEYMAX + 1];

    reset(&r);
    key_limit = n;

    for (int i = 0; i < n; i++) {
        a[i] = (mode == 1) ? n - i : i + 1;
    }
    if (mode == 2) {
        shuffle(a, n);
    }
    for (int i = 0; i < n; i++) {
        if (apply(&r, 0, a[i])) {
            return 1;
        }
    }
    return 0;
}

static int phase_asc_asc(int n)
{
    internal_node * r;

    reset(&r);
    key_limit = n;

    for (int k = 1; k <= n; k++) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int k = 1; k <= n; k++) {
        if (apply(&r, 1, k)) {
            return 1;
        }
    }
    return 0;
}

static int phase_asc_desc(int n)
{
    internal_node * r;

    reset(&r);
    key_limit = n;

    for (int k = 1; k <= n; k++) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int k = n; k >= 1; k--) {
        if (apply(&r, 1, k)) {
            return 1;
        }
    }
    return 0;
}

static int phase_asc_random(int n)
{
    internal_node * r;
    int a[KEYMAX + 1];

    reset(&r);
    key_limit = n;

    for (int k = 1; k <= n; k++) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int i = 0; i < n; i++) {
        a[i] = i + 1;
    }
    shuffle(a, n);
    for (int i = 0; i < n; i++) {
        if (apply(&r, 1, a[i])) {
            return 1;
        }
    }
    return 0;
}

static int phase_rand_rand(int n)
{
    internal_node * r;
    int a[KEYMAX + 1];

    reset(&r);
    key_limit = n;

    for (int i = 0; i < n; i++) {
        a[i] = i + 1;
    }
    shuffle(a, n);
    for (int i = 0; i < n; i++) {
        if (apply(&r, 0, a[i])) {
            return 1;
        }
    }
    shuffle(a, n);
    for (int i = 0; i < n; i++) {
        if (apply(&r, 1, a[i])) {
            return 1;
        }
    }
    return 0;
}

static int phase_nonexistent(int n)
{
    internal_node * r;

    reset(&r);
    key_limit = n;

    for (int k = 2; k <= n; k += 2) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int k = 1; k <= n; k += 2) {
        if (apply(&r, 1, k)) {
            return 1;
        }
    }
    for (int k = 2; k <= n; k += 2) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    return 0;
}

static int phase_reinsert_each(int n)
{
    internal_node * r;

    reset(&r);
    key_limit = n;

    for (int k = 1; k <= n; k++) {
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int k = 1; k <= n; k++) {
        if (apply(&r, 1, k)) {
            return 1;
        }
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    for (int k = n; k >= 1; k--) {
        if (apply(&r, 1, k)) {
            return 1;
        }
        if (apply(&r, 0, k)) {
            return 1;
        }
    }
    return 0;
}

static int phase_cycles(int n, int cycles)
{
    internal_node * r;
    int a[KEYMAX + 1];

    reset(&r);
    key_limit = n;

    for (int c = 0; c < cycles; c++) {
        for (int i = 0; i < n; i++) {
            a[i] = (c % 3 == 1) ? n - i : i + 1;
        }
        if (c % 3 == 2) {
            shuffle(a, n);
        }
        for (int i = 0; i < n; i++) {
            if (apply(&r, 0, a[i])) {
                return 1;
            }
        }

        for (int i = 0; i < n; i++) {
            a[i] = (c % 3 == 2) ? n - i : i + 1;
        }
        if (c % 3 == 0) {
            shuffle(a, n);
        }
        for (int i = 0; i < n; i++) {
            if (apply(&r, 1, a[i])) {
                return 1;
            }
        }
    }
    return 0;
}

static void build_order(int * a, int n, int pat)
{
    int c = 0;

    switch (pat) {
    case 0:
        for (int k = 2; k <= n; k += 2) {
            a[c++] = k;
        }
        for (int k = 1; k <= n; k += 2) {
            a[c++] = k;
        }
        break;

    case 1:
        for (int r = 0; r < 3; r++) {
            for (int k = 1; k <= n; k++) {
                if (k % 3 == r) {
                    a[c++] = k;
                }
            }
        }
        break;

    case 2: {
        int lo = n / 4 + 1;
        int hi = (3 * n) / 4;
        for (int k = lo; k <= hi; k++) {
            a[c++] = k;
        }
        for (int k = 1; k < lo; k++) {
            a[c++] = k;
        }
        for (int k = hi + 1; k <= n; k++) {
            a[c++] = k;
        }
        break;
    }

    case 3: {
        int l = 1;
        int r = n;
        while (l <= r) {
            a[c++] = l++;
            if (l <= r) {
                a[c++] = r--;
            }
        }
        break;
    }

    case 4: {
        int m = (n + 1) / 2;
        a[c++] = m;
        for (int d = 1; c < n; d++) {
            if (m + d <= n) {
                a[c++] = m + d;
            }
            if (m - d >= 1) {
                a[c++] = m - d;
            }
        }
        break;
    }
    }

    if (c != n) {
        printf("harness bug: pattern %d built %d of %d keys\n", pat, c, n);
        exit(2);
    }
}

static int phase_pattern(int n, int pat, int insert_desc)
{
    internal_node * r;
    int a[KEYMAX + 1];

    reset(&r);
    key_limit = n;

    for (int k = 1; k <= n; k++) {
        if (apply(&r, 0, insert_desc ? n - k + 1 : k)) {
            return 1;
        }
    }
    build_order(a, n, pat);
    for (int i = 0; i < n; i++) {
        if (apply(&r, 1, a[i])) {
            return 1;
        }
    }
    return 0;
}

static int phase_random(int n_ops, int ins_pct, int range)
{
    internal_node * r;

    reset(&r);
    key_limit = range;

    for (int i = 0; i < n_ops; i++) {
        int key = 1 + rand() % range;
        int op = (rand() % 100 < ins_pct) ? 0 : 1;
        if (apply(&r, op, key)) {
            return 1;
        }
    }
    return 0;
}

static int phase_bursts(int n_ops, int range)
{
    internal_node * r;

    reset(&r);
    key_limit = range;

    for (int i = 0; i < n_ops; i++) {
        int pct = ((i / 150) % 2) ? 8 : 92;
        int key = 1 + rand() % range;
        int op = (rand() % 100 < pct) ? 0 : 1;
        if (apply(&r, op, key)) {
            return 1;
        }
    }
    return 0;
}

static int ex_n;
static int ex_ins[12];
static int ex_del[12];
static int ex_used_i[12];
static int ex_used_d[12];

static int ex_run(void)
{
    internal_node * r;

    reset(&r);
    key_limit = ex_n;

    for (int i = 0; i < ex_n; i++) {
        if (apply(&r, 0, ex_ins[i])) {
            return 1;
        }
    }
    for (int i = 0; i < ex_n; i++) {
        if (apply(&r, 1, ex_del[i])) {
            return 1;
        }
    }
    return 0;
}

static int ex_gen_del(int pos)
{
    if (pos == ex_n) {
        return ex_run();
    }
    for (int v = 1; v <= ex_n; v++) {
        if (ex_used_d[v]) {
            continue;
        }
        ex_used_d[v] = 1;
        ex_del[pos] = v;
        if (ex_gen_del(pos + 1)) {
            return 1;
        }
        ex_used_d[v] = 0;
    }
    return 0;
}

static int ex_gen_ins(int pos)
{
    if (pos == ex_n) {
        memset(ex_used_d, 0, sizeof(ex_used_d));
        return ex_gen_del(0);
    }
    for (int v = 1; v <= ex_n; v++) {
        if (ex_used_i[v]) {
            continue;
        }
        ex_used_i[v] = 1;
        ex_ins[pos] = v;
        if (ex_gen_ins(pos + 1)) {
            return 1;
        }
        ex_used_i[v] = 0;
    }
    return 0;
}

static int phase_exhaustive(int n, int permute_inserts)
{
    if (n > 10) {
        n = 10;
    }
    ex_n = n;
    memset(ex_used_i, 0, sizeof(ex_used_i));
    memset(ex_used_d, 0, sizeof(ex_used_d));

    if (permute_inserts) {
        return ex_gen_ins(0);
    }
    for (int i = 0; i < n; i++) {
        ex_ins[i] = i + 1;
    }
    return ex_gen_del(0);
}

static int bplus_main(int argc, char ** argv)
{
    int seeds  = (argc > 1) ? atoi(argv[1]) : 40;
    int n_ops  = (argc > 2) ? atoi(argv[2]) : 2000;
    int do_exh = (argc > 3) ? atoi(argv[3]) : 1;

    cur_order = no_of_data;
    printf("no_of_data=%d KEYMAX=%d seeds=%d ops/seed=%d exhaustive=%d\n",
           no_of_data, KEYMAX, seeds, n_ops, do_exh);

    if (no_of_data % 2) {
        printf("WARNING: odd no_of_data - insert's internal split only keeps children==keys+1 for even values\n");
    }
    if (no_of_data >= 64) {
        printf("NOTE: with KEYMAX=%d the tree has only 2 levels, internal-node delete code is NOT exercised. Use no_of_data 4/6/8/16.\n",
               KEYMAX);
    }

    int total_fail = 0;
    char nm[96];
    cur_seed = 0;

    RUN("insert only: ascending",  phase_insert_only(KEYMAX, 0));
    RUN("insert only: descending", phase_insert_only(KEYMAX, 1));
    RUN("insert only: random",     phase_insert_only(KEYMAX, 2));

    RUN("asc insert / asc delete",        phase_asc_asc(KEYMAX));
    RUN("asc insert / desc delete",       phase_asc_desc(KEYMAX));
    RUN("nonexistent + duplicates",       phase_nonexistent(KEYMAX));
    RUN("delete then re-insert each key", phase_reinsert_each(KEYMAX));
    RUN("fill / empty cycles x6",         phase_cycles(KEYMAX, 6));

    for (int p = 0; p < NPAT; p++) {
        snprintf(nm, sizeof(nm), "delete pattern %d, ascending insert", p);
        RUN(nm, phase_pattern(KEYMAX, p, 0));

        snprintf(nm, sizeof(nm), "delete pattern %d, descending insert", p);
        RUN(nm, phase_pattern(KEYMAX, p, 1));
    }

    if (do_exh) {
        RUN("exhaustive: every delete order, n=8",          phase_exhaustive(8, 0));
        RUN("exhaustive: every insert x delete order, n=6", phase_exhaustive(6, 1));
    }

    for (int s = 1; s <= seeds; s++) {
        cur_seed = s;

        snprintf(nm, sizeof(nm), "asc insert / random delete seed %d", s);
        RUN(nm, phase_asc_random(KEYMAX));

        snprintf(nm, sizeof(nm), "random insert / random delete seed %d", s);
        RUN(nm, phase_rand_rand(KEYMAX));

        snprintf(nm, sizeof(nm), "random 50/50 seed %d", s);
        RUN(nm, phase_random(n_ops, 50, KEYMAX));

        snprintf(nm, sizeof(nm), "random 30/70 seed %d", s);
        RUN(nm, phase_random(n_ops, 30, KEYMAX));

        snprintf(nm, sizeof(nm), "random 70/30 seed %d", s);
        RUN(nm, phase_random(n_ops, 70, KEYMAX));

        snprintf(nm, sizeof(nm), "random 50/50, tiny range 12, seed %d", s);
        RUN(nm, phase_random(n_ops, 50, 12));

        snprintf(nm, sizeof(nm), "random 50/50, range 60, seed %d", s);
        RUN(nm, phase_random(n_ops, 50, 60));

        snprintf(nm, sizeof(nm), "bursts seed %d", s);
        RUN(nm, phase_bursts(n_ops, KEYMAX));

        if (total_fail) {
            break;
        }
    }

    printf("\n%s\n", total_fail ? "SOME TESTS FAILED" : "ALL PASSED");
    return total_fail != 0;
}

static int snapA[8 * KEYMAX + 64];
static int snapB[8 * KEYMAX + 64];

static int x_count_nodes(internal_node * n)
{
    if (!n) {
        return 0;
    }

    int c = 1;
    if (!n->leaf) {
        for (int i = 0; i < n->num_of_children; i++) {
            c += x_count_nodes(n->children[i]);
        }
    }
    return c;
}

static int x_check_records(internal_node * n)
{
    if (n->leaf) {
        for (int i = 0; i < n->num_of_leaf; i++) {
            int k = n->primary_key[i];
            if (n->data[i].page_id != k ||
                n->data[i].file_offset != k * 7 ||
                n->data[i].slot_num != k * 3) {
                BAD("record of key %d corrupted (page %d off %d slot %d)",
                    k, n->data[i].page_id, n->data[i].file_offset, n->data[i].slot_num);
            }
        }
        return 0;
    }

    for (int i = 0; i < n->num_of_children; i++) {
        if (x_check_records(n->children[i])) {
            return 1;
        }
    }
    return 0;
}

static int x_check_all(internal_node * root)
{
    if (check_tree(root)) {
        return 1;
    }
    if (x_check_records(root)) {
        return 1;
    }
    if (validate_the_tree(root) != 0) {
        BAD("validate_the_tree()=1 but harness says tree is fine");
    }

    int c = x_count_nodes(root);
    if (c != g_alloc) {
        BAD("tree has %d nodes but %ld are allocated (leak or double count)", c, g_alloc);
    }
    return 0;
}

static int x_prep(internal_node ** r, int n)
{
    reset(r);
    key_limit = n;

    if (g_alloc != 1) {
        printf("  LEAK: %ld live nodes after reset (expected 1)\n", g_alloc);
        return 1;
    }
    return 0;
}

static int x_empty(internal_node * r)
{
    if (!r->leaf || r->num_of_leaf || r->num_of_data_pushed || r->next) {
        BAD("not an empty leaf root after deleting everything");
    }
    if (g_alloc != 1) {
        BAD("%ld live nodes after emptying tree, expected 1", g_alloc);
    }
    return 0;
}

static int x_apply(internal_node ** root, int op, int key)
{
    if (log_n < LOGMAX) {
        log_op[log_n] = op;
        log_key[log_n] = key;
        log_n++;
    }

    if (op == 0) {
        tree_func_for_insert(*root, key, NULL, key, key * 7, key * 3);
        present[key] = 1;
    } else {
        *root = delete_the_thing(*root, key);
        present[key] = 0;
    }

    g_root = *root;
    if (x_check_all(*root)) {
        dump_fail();
        return 1;
    }
    return 0;
}

static void x_order(int * a, int n, int kind)
{
    for (int i = 0; i < n; i++) {
        a[i] = (kind == 1) ? n - i : i + 1;
    }
    if (kind == 2) {
        shuffle(a, n);
    }
}

static internal_node * x_leaf(const int * k, int n)
{
    internal_node * l = (internal_node *)calloc(1, sizeof(internal_node));

    l->leaf = true;
    l->num_of_leaf = n;
    l->num_of_data_pushed = n;

    for (int i = 0; i < n; i++) {
        l->primary_key[i] = k[i];
        l->data[i].page_id = k[i];
        l->data[i].file_offset = k[i] * 7;
        l->data[i].slot_num = k[i] * 3;
    }
    return l;
}

static internal_node * x_inner(internal_node ** ch, int nch, const int * keys)
{
    internal_node * p = (internal_node *)calloc(1, sizeof(internal_node));

    p->leaf = false;
    p->num_of_children = nch;
    p->num_of_leaf = nch - 1;

    for (int i = 0; i < nch; i++) {
        p->children[i] = ch[i];
    }
    for (int i = 0; i < nch - 1; i++) {
        p->primary_key[i] = keys[i];
    }
    return p;
}

static int x_unit_min_key(void)
{
    internal_node * r0;
    if (x_prep(&r0, 1)) {
        return 1;
    }

    XEXPECT(min_key_of(NULL) == -1, "min_key_of(NULL)");
    XEXPECT(min_key_of(r0) == -1, "min_key_of(empty leaf)");

    int ka[] = {3, 4};
    int kb[] = {7, 8};
    int kc[] = {20, 21};
    int s1[] = {7};
    int s2[] = {20};

    internal_node * A = x_leaf(ka, 2);
    internal_node * B = x_leaf(kb, 2);
    internal_node * C = x_leaf(kc, 2);

    XEXPECT(min_key_of(A) == 3, "min_key_of(leaf)");

    internal_node * ch1[] = {A, B};
    internal_node * P = x_inner(ch1, 2, s1);
    XEXPECT(min_key_of(P) == 3, "min_key_of(2 levels)");

    internal_node * ch2[] = {P, C};
    internal_node * R = x_inner(ch2, 2, s2);
    XEXPECT(min_key_of(R) == 3, "min_key_of(3 levels)");

    A->num_of_leaf = 0;
    A->num_of_data_pushed = 0;
    XEXPECT(min_key_of(R) == -1, "min_key_of(empty leftmost leaf)");

    free_tree(R);
    XEXPECT(g_alloc == 1, "leak in unit test");
    return 0;
}

static int x_unit_remove_child(void)
{
    static internal_node d[4];
    internal_node p;
    int keys[3] = {10, 20, 30};

    for (int c = 0; c < 4; c++) {
        memset(&p, 0, sizeof p);
        p.leaf = false;
        p.num_of_leaf = 3;
        p.num_of_children = 4;
        for (int i = 0; i < 3; i++) {
            p.primary_key[i] = keys[i];
        }
        for (int i = 0; i < 4; i++) {
            p.children[i] = &d[i];
        }

        remove_child_at(&p, c);

        int rk = (c > 0) ? c - 1 : 0;
        int ek[2];
        int j = 0;
        for (int i = 0; i < 3; i++) {
            if (i != rk) {
                ek[j++] = keys[i];
            }
        }

        internal_node * ec[3];
        j = 0;
        for (int i = 0; i < 4; i++) {
            if (i != c) {
                ec[j++] = &d[i];
            }
        }

        XEXPECT(p.num_of_leaf == 2 && p.num_of_children == 3, "counts after remove_child_at");
        XEXPECT(p.primary_key[0] == ek[0] && p.primary_key[1] == ek[1], "keys after remove_child_at");
        for (int i = 0; i < 3; i++) {
            XEXPECT(p.children[i] == ec[i], "children after remove_child_at");
        }
        XEXPECT(p.children[3] == NULL && p.primary_key[2] == -1, "tail slots not cleared");
    }

    for (int c = 0; c < 2; c++) {
        memset(&p, 0, sizeof p);
        p.leaf = false;
        p.num_of_leaf = 1;
        p.num_of_children = 2;
        p.primary_key[0] = 50;
        p.children[0] = &d[0];
        p.children[1] = &d[1];

        remove_child_at(&p, c);

        XEXPECT(p.num_of_leaf == 0 && p.num_of_children == 1, "counts (2 children case)");
        XEXPECT(p.children[0] == &d[1 - c] && p.children[1] == NULL, "children (2 children case)");
    }
    return 0;
}

static int x_unit_update_reps(void)
{
    internal_node * r0;
    if (x_prep(&r0, 1)) {
        return 1;
    }

    int k1[] = {1, 2};
    int k2[] = {3, 4};
    int k3[] = {10, 11};
    int k4[] = {12, 13};
    int st[] = {999};

    internal_node * L1 = x_leaf(k1, 2);
    internal_node * L2 = x_leaf(k2, 2);
    internal_node * L3 = x_leaf(k3, 2);
    internal_node * L4 = x_leaf(k4, 2);

    internal_node * c1[] = {L1, L2};
    internal_node * c2[] = {L3, L4};
    internal_node * P1 = x_inner(c1, 2, st);
    internal_node * P2 = x_inner(c2, 2, st);

    internal_node * c3[] = {P1, P2};
    internal_node * R = x_inner(c3, 2, st);

    internal_node * stk[3];
    int pos[2];

    stk[0] = R;
    stk[1] = P2;
    stk[2] = L3;
    pos[0] = 1;
    pos[1] = 0;
    update_reps(stk, pos, 2);
    XEXPECT(R->primary_key[0] == 10, "root separator should become min of right subtree (10)");
    XEXPECT(P2->primary_key[0] == 12, "P2 separator should become min of L4 (12)");
    XEXPECT(P1->primary_key[0] == 999, "update_reps touched a node off the path");

    R->primary_key[0] = 999;
    P1->primary_key[0] = 999;
    stk[1] = P1;
    stk[2] = L2;
    pos[0] = 0;
    pos[1] = 1;
    update_reps(stk, pos, 2);
    XEXPECT(P1->primary_key[0] == 3, "P1 separator should be 3");
    XEXPECT(R->primary_key[0] == 10, "root separator should be 10");

    R->primary_key[0] = 777;
    update_reps(stk, pos, 0);
    XEXPECT(R->primary_key[0] == 777, "depth 0 must be a no-op");

    stk[1] = P2;
    stk[2] = L3;
    pos[0] = 1;
    pos[1] = 0;
    L4->num_of_leaf = 0;
    L4->num_of_data_pushed = 0;
    P2->primary_key[0] = 555;
    update_reps(stk, pos, 2);
    XEXPECT(P2->primary_key[0] == 555, "empty leaf (-1) must not overwrite a separator");

    free_tree(R);
    XEXPECT(g_alloc == 1, "leak in unit test");
    return 0;
}

static int x_unit_validators(void)
{
    internal_node * r;
    if (x_prep(&r, 1)) {
        return 1;
    }

    int n = no_of_data * 4;
    int t;

    for (int k = 1; k <= n; k++) {
        tree_func_for_insert(r, k, NULL, k, k * 7, k * 3);
    }

    XEXPECT(!r->leaf, "root should be internal");
    XEXPECT(validate_the_tree(r) == 0, "valid tree rejected");
    XEXPECT(validate_the_tree_data(r) == 0, "valid tree rejected by data validator");
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 0, "valid tree rejected by node validator");

    internal_node * L0 = r;
    while (!L0->leaf) {
        L0 = L0->children[0];
    }
    internal_node * L1 = L0->next;
    XEXPECT(L1 != NULL && L0->num_of_leaf >= 2, "setup");

    t = L0->primary_key[0];
    L0->primary_key[0] = L0->primary_key[1];
    L0->primary_key[1] = t;
    XEXPECT(validate_the_tree(r) == 1, "unsorted leaf not detected");
    XEXPECT(validate_the_tree_data(r) == 1, "unsorted leaf not detected (data)");
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "unsorted leaf not detected (node)");
    t = L0->primary_key[0];
    L0->primary_key[0] = L0->primary_key[1];
    L0->primary_key[1] = t;

    t = L1->primary_key[0];
    L1->primary_key[0] = L0->primary_key[L0->num_of_leaf - 1];
    XEXPECT(validate_the_tree_data(r) == 1, "duplicate across leaves not detected");
    XEXPECT(validate_the_tree(r) == 1, "duplicate across leaves not detected (full)");
    L1->primary_key[0] = t;

    t = r->primary_key[0];
    r->primary_key[0] = t + 1000;
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "bad separator not detected");
    r->primary_key[0] = t;

    r->num_of_children--;
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "children != keys+1 not detected");
    r->num_of_children++;

    internal_node * saved = r->children[r->num_of_children - 1];
    r->children[r->num_of_children - 1] = NULL;
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "NULL child not detected");
    r->children[r->num_of_children - 1] = saved;

    t = L0->primary_key[L0->num_of_leaf - 1];
    L0->primary_key[L0->num_of_leaf - 1] = t + 100000;
    XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "leaf key beyond separator not detected");
    XEXPECT(validate_the_tree_data(r) == 1, "leaf key beyond next leaf not detected");
    L0->primary_key[L0->num_of_leaf - 1] = t;

    if (r->num_of_leaf >= 2) {
        t = r->primary_key[0];
        r->primary_key[0] = r->primary_key[1];
        r->primary_key[1] = t;
        XEXPECT(validate_the_tree_node(r, INT_MAX, INT_MIN) == 1, "unsorted separators not detected");
        t = r->primary_key[0];
        r->primary_key[0] = r->primary_key[1];
        r->primary_key[1] = t;
    }

    XEXPECT(validate_the_tree(r) == 0, "tree not restored properly");
    return 0;
}

static int x_unit_edge(void)
{
    internal_node * r;
    if (x_prep(&r, 10)) {
        return 1;
    }

    XEXPECT(delete_the_thing(NULL, 5) == NULL, "delete(NULL) must return NULL");

    tree_func_for_insert(NULL, 5, NULL, 5, 35, 15);
    XEXPECT(validate_the_tree(r) == 0, "empty tree should validate");

    internal_node * r2 = delete_the_thing(r, 5);
    XEXPECT(r2 == r && r->leaf && r->num_of_leaf == 0, "delete on empty tree");

    tree_func_for_insert(r, 5, NULL, 5, 35, 15);
    XEXPECT(r->num_of_leaf == 1 &&
            r->primary_key[0] == 5 &&
            r->data[0].page_id == 5 &&
            r->data[0].file_offset == 35 &&
            r->data[0].slot_num == 15,
            "single insert");

    tree_func_for_insert(r, 5, NULL, 9, 9, 9);
    XEXPECT(r->num_of_leaf == 1 && r->data[0].page_id == 5, "duplicate insert changed the tree");

    r2 = delete_the_thing(r, 6);
    XEXPECT(r2 == r && r->num_of_leaf == 1, "delete of absent key changed the tree");

    r = delete_the_thing(r, 5);
    g_root = r;
    XEXPECT(r->leaf && r->num_of_leaf == 0 && r->num_of_data_pushed == 0 && g_alloc == 1,
            "delete last key");

    r = delete_the_thing(r, 5);
    g_root = r;
    XEXPECT(r->leaf && r->num_of_leaf == 0 && g_alloc == 1, "delete from empty again");

    internal_node * u = (internal_node *)calloc(1, sizeof(internal_node));
    int n = 10 * no_of_data;

    for (int k = 1; k <= n; k++) {
        tree_func_for_insert(u, k, NULL, k, k * 7, k * 3);
    }
    XEXPECT(validate_the_tree(u) == 0 && x_count_nodes(u) == g_alloc - 1,
            "root created with leaf=false");

    for (int k = n; k >= 1; k--) {
        u = delete_the_thing(u, k);
    }
    XEXPECT(u->leaf && u->num_of_leaf == 0 && g_alloc == 2,
            "drain tree built on uninitialised root");

    free(u);
    XEXPECT(g_alloc == 1, "leak");
    return 0;
}

static int x_boundary_run(int m, int ik, int dk)
{
    internal_node * r;
    int a[KEYMAX + 1];
    int b[KEYMAX + 1];

    if (x_prep(&r, m)) {
        return 1;
    }

    x_order(a, m, ik);
    for (int i = 0; i < m; i++) {
        if (x_apply(&r, 0, a[i])) {
            return 1;
        }
    }

    if (dk == 0) {
        x_order(b, m, 0);
    } else if (dk == 1) {
        x_order(b, m, 1);
    } else {
        build_order(b, m, (dk == 2) ? 4 : 3);
    }

    for (int i = 0; i < m; i++) {
        if (x_apply(&r, 1, b[i])) {
            return 1;
        }
    }
    return x_empty(r);
}

static int x_ph_boundary(void)
{
    int lim = 3 * no_of_data + 3;
    if (lim > 120) {
        lim = 120;
    }
    if (lim > KEYMAX) {
        lim = KEYMAX;
    }

    for (int m = 1; m <= lim; m++) {
        for (int ik = 0; ik < 2; ik++) {
            for (int dk = 0; dk < 4; dk++) {
                if (x_boundary_run(m, ik, dk)) {
                    printf("  (boundary m=%d ins=%d del=%d)\n", m, ik, dk);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int x_ph_insert(int n, int kind)
{
    internal_node * r;
    int a[KEYMAX + 1];

    if (x_prep(&r, n)) {
        return 1;
    }

    x_order(a, n, kind);
    for (int i = 0; i < n; i++) {
        if (x_apply(&r, 0, a[i])) {
            return 1;
        }
    }
    return 0;
}

static int x_ph_drain(int n, int ik, int dk)
{
    internal_node * r;
    int a[KEYMAX + 1];

    if (x_prep(&r, n)) {
        return 1;
    }

    x_order(a, n, ik);
    for (int i = 0; i < n; i++) {
        if (x_apply(&r, 0, a[i])) {
            return 1;
        }
    }

    x_order(a, n, dk);
    for (int i = 0; i < n; i++) {
        if (x_apply(&r, 1, a[i])) {
            return 1;
        }
    }
    return x_empty(r);
}

static int x_ph_cycles(int n, int cycles)
{
    internal_node * r;
    int a[KEYMAX + 1];

    if (x_prep(&r, n)) {
        return 1;
    }

    for (int c = 0; c < cycles; c++) {
        x_order(a, n, c % 3);
        for (int i = 0; i < n; i++) {
            if (x_apply(&r, 0, a[i])) {
                return 1;
            }
        }

        x_order(a, n, (c + 1) % 3);
        for (int i = 0; i < n; i++) {
            if (x_apply(&r, 1, a[i])) {
                return 1;
            }
        }

        if (x_empty(r)) {
            return 1;
        }
    }
    return 0;
}

static int x_leftmost_min(internal_node * r)
{
    while (!r->leaf) {
        r = r->children[0];
    }
    return r->primary_key[0];
}

static int x_rightmost_max(internal_node * r)
{
    while (!r->leaf) {
        r = r->children[r->num_of_children - 1];
    }
    return r->primary_key[r->num_of_leaf - 1];
}

static int x_pick(internal_node * r, int mode, int step)
{
    int key = -1;

    switch (mode) {
    case 0:
        key = x_leftmost_min(r);
        break;

    case 1:
        key = x_rightmost_max(r);
        break;

    case 2:
        key = r->primary_key[0];
        break;

    case 3:
        key = r->primary_key[r->num_of_leaf - 1];
        break;

    case 4:
    case 5: {
        internal_node * t = r;
        while (!t->leaf) {
            t = t->children[0];
        }

        int nl = 0;
        for (internal_node * u = t; u; u = u->next) {
            nl++;
        }

        int which = (mode == 4) ? (step % nl) : ((step * 7 + 3) % nl);
        for (int i = 0; i < which; i++) {
            t = t->next;
        }
        key = (mode == 4) ? t->primary_key[0] : t->primary_key[t->num_of_leaf - 1];
        break;
    }

    case 6: {
        internal_node * t = r;
        while (!t->leaf && rand() % 3) {
            t = t->children[rand() % t->num_of_children];
        }
        key = t->primary_key[rand() % t->num_of_leaf];
        break;
    }
    }

    if (!present[key]) {
        key = x_leftmost_min(r);
    }
    return key;
}

static int x_ph_targeted(int n, int ik, int mode)
{
    internal_node * r;
    int a[KEYMAX + 1];

    if (x_prep(&r, n)) {
        return 1;
    }

    x_order(a, n, ik);
    for (int i = 0; i < n; i++) {
        if (x_apply(&r, 0, a[i])) {
            return 1;
        }
    }

    for (int step = 0; step < n; step++) {
        if (x_apply(&r, 1, x_pick(r, mode, step))) {
            return 1;
        }
    }
    return x_empty(r);
}

static int x_ph_random(int n_ops, int ins_pct, int range)
{
    internal_node * r;

    if (x_prep(&r, range)) {
        return 1;
    }

    for (int i = 0; i < n_ops; i++) {
        int key = 1 + rand() % range;
        int op = (rand() % 100 < ins_pct) ? 0 : 1;
        if (x_apply(&r, op, key)) {
            return 1;
        }
    }

    for (int k = 1; k <= range; k++) {
        if (present[k]) {
            if (x_apply(&r, 1, k)) {
                return 1;
            }
        }
    }
    return x_empty(r);
}

static int x_ph_bursts(int n_ops, int range)
{
    internal_node * r;

    if (x_prep(&r, range)) {
        return 1;
    }

    for (int i = 0; i < n_ops; i++) {
        int pct = ((i / 150) % 2) ? 8 : 92;
        int key = 1 + rand() % range;
        if (x_apply(&r, (rand() % 100 < pct) ? 0 : 1, key)) {
            return 1;
        }
    }

    for (int k = 1; k <= range; k++) {
        if (present[k]) {
            if (x_apply(&r, 1, k)) {
                return 1;
            }
        }
    }
    return x_empty(r);
}

static int x_snap(internal_node * n, int * buf, int pos)
{
    buf[pos++] = n->leaf;
    buf[pos++] = n->num_of_leaf;
    buf[pos++] = n->num_of_children;

    for (int i = 0; i < n->num_of_leaf; i++) {
        buf[pos++] = n->primary_key[i];
    }

    if (!n->leaf) {
        for (int i = 0; i < n->num_of_children; i++) {
            pos = x_snap(n->children[i], buf, pos);
        }
    }
    return pos;
}

static int x_same(internal_node * r, int la, const char * what, int k)
{
    int lb = x_snap(r, snapB, 0);

    if (lb != la || memcmp(snapA, snapB, (size_t)la * sizeof(int))) {
        printf("  %s %d changed the tree\n", what, k);
        return 1;
    }
    return 0;
}

static int x_ph_noop(int n)
{
    internal_node * r;
    int a[KEYMAX + 1];
    int m = 0;

    if (x_prep(&r, n)) {
        return 1;
    }

    for (int k = 2; k <= n; k += 2) {
        a[m++] = k;
    }
    shuffle(a, m);
    for (int i = 0; i < m; i++) {
        if (x_apply(&r, 0, a[i])) {
            return 1;
        }
    }

    int la = x_snap(r, snapA, 0);

    for (int k = 1; k <= n + 1; k += 2) {
        r = delete_the_thing(r, k);
        g_root = r;
        if (x_same(r, la, "deleting absent key", k)) {
            return 1;
        }
    }

    int extra[] = {0, n + 1000, INT_MAX};
    for (int i = 0; i < 3; i++) {
        r = delete_the_thing(r, extra[i]);
        g_root = r;
        if (x_same(r, la, "deleting absent key", extra[i])) {
            return 1;
        }
    }

    for (int k = 2; k <= n; k += 2) {
        tree_func_for_insert(r, k, NULL, -5, -5, -5);
        if (x_same(r, la, "duplicate insert of", k)) {
            return 1;
        }
    }

    if (x_check_all(r)) {
        printf("  after no-op phase\n");
        return 1;
    }
    return 0;
}

static int sp_ref[20000];
static int sp_n;

static int sp_lb(int key)
{
    int lo = 0;
    int hi = sp_n;

    while (lo < hi) {
        int m = (lo + hi) / 2;
        if (sp_ref[m] < key) {
            lo = m + 1;
        } else {
            hi = m;
        }
    }
    return lo;
}

static void sp_ins(int key)
{
    int i = sp_lb(key);

    if (i < sp_n && sp_ref[i] == key) {
        return;
    }
    memmove(&sp_ref[i + 1], &sp_ref[i], (size_t)(sp_n - i) * sizeof(int));
    sp_ref[i] = key;
    sp_n++;
}

static void sp_del(int key)
{
    int i = sp_lb(key);

    if (i < sp_n && sp_ref[i] == key) {
        memmove(&sp_ref[i], &sp_ref[i + 1], (size_t)(sp_n - i - 1) * sizeof(int));
        sp_n--;
    }
}

static int sp_rnd(void)
{
    return 1 + (int)((((unsigned)rand() << 15) ^ (unsigned)rand()) % 1000000u);
}

static int sp_check(internal_node * root)
{
    leaf_depth = -1;
    if (check_node(root, 0, INT_MIN, INT_MAX, 1)) {
        return 1;
    }
    if (x_check_records(root)) {
        return 1;
    }

    internal_node * t = root;
    while (!t->leaf) {
        t = t->children[0];
    }

    int idx = 0;
    long guard = 0;

    for (; t; t = t->next) {
        if (++guard > 1000000) {
            BAD("leaf chain cycle");
        }
        for (int i = 0; i < t->num_of_leaf; i++, idx++) {
            if (idx >= sp_n || t->primary_key[i] != sp_ref[idx]) {
                BAD("leaf chain mismatch at position %d", idx);
            }
        }
    }

    if (idx != sp_n) {
        BAD("chain has %d keys, reference %d", idx, sp_n);
    }
    if (validate_the_tree(root)) {
        BAD("validate_the_tree() disagrees with harness");
    }

    int c = x_count_nodes(root);
    if (c != g_alloc) {
        BAD("tree has %d nodes but %ld allocated", c, g_alloc);
    }

    for (int i = 0; i < 20 && sp_n; i++) {
        int k = sp_ref[rand() % sp_n];
        if (!find_key(root, k)) {
            BAD("find(%d) failed through routing", k);
        }
    }
    return 0;
}

static int sp_apply(internal_node ** root, int op, int key)
{
    if (log_n < LOGMAX) {
        log_op[log_n] = op;
        log_key[log_n] = key;
        log_n++;
    }

    if (op == 0) {
        tree_func_for_insert(*root, key, NULL, key, key * 7, key * 3);
        sp_ins(key);
    } else {
        *root = delete_the_thing(*root, key);
        sp_del(key);
    }

    g_root = *root;
    if (sp_check(*root)) {
        dump_fail();
        return 1;
    }
    return 0;
}

static int sp_ph_random(int n_ops, int ins_pct)
{
    internal_node * r;

    if (x_prep(&r, 1)) {
        return 1;
    }
    sp_n = 0;

    for (int i = 0; i < n_ops; i++) {
        int ins = (rand() % 100 < ins_pct) && sp_n < 19000;
        int rc;

        if (ins) {
            rc = sp_apply(&r, 0, sp_rnd());
        } else if (sp_n > 0 && rand() % 10 < 8) {
            rc = sp_apply(&r, 1, sp_ref[rand() % sp_n]);
        } else {
            rc = sp_apply(&r, 1, sp_rnd());
        }

        if (rc) {
            return 1;
        }
    }

    while (sp_n > 0) {
        if (sp_apply(&r, 1, sp_ref[rand() % sp_n])) {
            return 1;
        }
    }
    return x_empty(r);
}

static int sp_ph_drain(int n, int mode)
{
    internal_node * r;

    if (x_prep(&r, 1)) {
        return 1;
    }
    sp_n = 0;

    for (int i = 0; i < n; i++) {
        if (sp_apply(&r, 0, sp_rnd())) {
            return 1;
        }
    }

    for (int step = 0; sp_n > 0; step++) {
        int idx;

        switch (mode) {
        case 0:
            idx = rand() % sp_n;
            break;
        case 1:
            idx = 0;
            break;
        case 2:
            idx = sp_n - 1;
            break;
        case 3:
            idx = (step % 2) ? sp_n - 1 : 0;
            break;
        default:
            idx = sp_n / 2;
            break;
        }

        if (sp_apply(&r, 1, sp_ref[idx])) {
            return 1;
        }
    }
    return x_empty(r);
}

static int sp_ph_window(int n_ops, int w)
{
    internal_node * r;

    if (x_prep(&r, 1)) {
        return 1;
    }
    sp_n = 0;

    int next = 1;

    for (int i = 0; i < n_ops; i++) {
        next += 1 + rand() % 5;
        if (sp_apply(&r, 0, next)) {
            return 1;
        }
        if (sp_n > w) {
            if (sp_apply(&r, 1, sp_ref[0])) {
                return 1;
            }
        }
    }

    while (sp_n > 0) {
        if (sp_apply(&r, 1, sp_ref[0])) {
            return 1;
        }
    }
    return x_empty(r);
}

int main(int argc, char ** argv)
{
    int seeds = (argc > 1) ? atoi(argv[1]) : 10;

    cur_order = no_of_data;
    cur_seed = 0;
    printf("no_of_data=%d KEYMAX=%d seeds=%d\n", no_of_data, KEYMAX, seeds);

    T(x_unit_min_key());
    T(x_unit_remove_child());
    T(x_unit_update_reps());
    T(x_unit_validators());
    T(x_unit_edge());
    printf("unit tests ok\n");

    T(x_ph_boundary());
    printf("boundary sizes ok\n");

    for (int k = 0; k < 3; k++) {
        T(x_ph_insert(KEYMAX, k));
    }
    for (int ik = 0; ik < 2; ik++) {
        for (int dk = 0; dk < 2; dk++) {
            T(x_ph_drain(KEYMAX, ik, dk));
        }
    }
    for (int ik = 0; ik < 2; ik++) {
        for (int m = 0; m < NMODE; m++) {
            T(x_ph_targeted(KEYMAX, ik, m));
        }
    }
    printf("deterministic phases ok\n");

    for (int s = 1; s <= seeds; s++) {
        cur_seed = s;

        T(x_ph_drain(KEYMAX, 2, 2));
        T(x_ph_drain(KEYMAX, 0, 2));
        T(x_ph_drain(KEYMAX, 2, 0));

        for (int m = 0; m < NMODE; m++) {
            T(x_ph_targeted(KEYMAX, 2, m));
        }

        T(x_ph_cycles(KEYMAX, 9));
        T(x_ph_noop(KEYMAX));

        T(x_ph_random(2000, 50, KEYMAX));
        T(x_ph_random(2000, 30, KEYMAX));
        T(x_ph_random(2000, 70, KEYMAX));
        T(x_ph_random(2000, 50, 12));
        T(x_ph_random(2000, 50, 60));
        T(x_ph_bursts(2000, KEYMAX));

        T(sp_ph_random(4000, 55));
        T(sp_ph_random(4000, 40));
        for (int m = 0; m < 5; m++) {
            T(sp_ph_drain(1500, m));
        }
        T(sp_ph_window(2500, 3 * no_of_data));
        T(sp_ph_window(2500, 40 * no_of_data));

        printf("seed %d ok\n", s);
    }

    printf("\n--- original harness ---\n");

    char sbuf[16];
    snprintf(sbuf, sizeof sbuf, "%d", seeds);
    char * a2[] = { argv[0], sbuf, "2000", "1", NULL };
    return bplus_main(4, a2);
}

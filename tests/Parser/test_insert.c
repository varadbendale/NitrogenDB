#include<insert_parser.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct { char ***query; } proper_data_t;
static proper_data_t proper_data;
int priority(char *s) {
    if (s == NULL) return 0;
    if (strcmp(s, "=") == 0) return 1;
    if (strcmp(s, "+") == 0 || strcmp(s, "-") == 0) return 2;
    if (strcmp(s, "*") == 0 || strcmp(s, "/") == 0) return 3;
    return 0;
}

bool if_function(char *w) {
    static const char *names[] = { "VALUES", "NOW", "CONCAT", "COALESCE",
                                   "MAX", "MIN", "LEAST", "GREATEST", NULL };
    if (w == NULL) return false;
    for (int k = 0; names[k]; k++)
        if (strcmp(w, names[k]) == 0) return true;
    return false;
}

insert_parser_tree *function_insert(char ***buf, insert_parser_tree *node, int check,
                                    int i, int j, int end_row, int end_col) {
    (void)node; (void)check; (void)end_row; (void)end_col;
    return createNodeInsert(buf[i][j]);
}

static int g_quiet = 0;

#define TEST(n) static int n(void)

#define CHECK(c) do { if (!(c)) { \
    if (!g_quiet) printf("      line %d: check failed: %s\n", __LINE__, #c); \
    return 1; } } while (0)

#define CHECK_STR(got, want) do { const char *g_ = (got), *w_ = (want); \
    if (g_ == NULL || strcmp(g_, w_) != 0) { \
        if (!g_quiet) printf("      line %d: expected \"%s\", got \"%s\"\n", \
                             __LINE__, w_, g_ ? g_ : "(null)"); \
        return 1; } } while (0)

#define EXPECT_NULL(n, spec) TEST(n) { CHECK(parse((spec), 0) == NULL); return 0; }

static char ***build(const char *spec, int one_per_row) {
    size_t cap = strlen(spec) + 2;
    char ***rows = calloc(cap, sizeof(char **));
    char *copy = strdup(spec);
    size_t nrows = 0;
    char *save_line = NULL;
    for (char *line = strtok_r(copy, "\n", &save_line); line;
         line = strtok_r(NULL, "\n", &save_line)) {
        char *tmp = strdup(line);
        char **toks = calloc(strlen(line) + 2, sizeof(char *));
        size_t n = 0;
        char *save_tok = NULL;
        for (char *t = strtok_r(tmp, " ", &save_tok); t; t = strtok_r(NULL, " ", &save_tok))
            toks[n++] = strdup(t);
        free(tmp);
        if (n == 0) { free(toks); continue; }
        if (one_per_row) {
            for (size_t k = 0; k < n; k++) {
                char **r = calloc(2, sizeof(char *));
                r[0] = toks[k];
                rows[nrows++] = r;
            }
            free(toks);
        } else {
            rows[nrows++] = toks;
        }
    }
    rows[nrows] = calloc(1, sizeof(char *));
    free(copy);
    return rows;
}

static itree *parse(const char *spec, int one_per_row) {
    proper_data.query = build(spec, one_per_row);
    return insert_parser();
}

static int cols_eq(itree *r, int count, ...) {
    if (r->table_col_num != count) return 0;
    va_list ap;
    va_start(ap, count);
    int ok = 1;
    for (int k = 0; k < count; k++) {
        const char *want = va_arg(ap, const char *);
        if (!r->table_cols[k] || strcmp(r->table_cols[k], want) != 0) ok = 0;
    }
    va_end(ap);
    return ok;
}

static int row_eq(itree *root, int idx, int count, ...) {
    if (idx >= root->num) return 0;
    itree *r = root->children[idx];
    if (!r || !r->comp || strcmp(r->comp, "VALUE_ROW") != 0) return 0;
    if (r->insert_val_num != count) return 0;
    va_list ap;
    va_start(ap, count);
    int ok = 1;
    for (int k = 0; k < count; k++) {
        const char *want = va_arg(ap, const char *);
        if (!r->insert_values[k] || strcmp(r->insert_values[k], want) != 0) ok = 0;
    }
    va_end(ap);
    return ok;
}

static itree *child_named(itree *root, const char *name) {
    for (int k = 0; root && k < root->num; k++)
        if (root->children[k] && root->children[k]->comp &&
            strcmp(root->children[k]->comp, name) == 0)
            return root->children[k];
    return NULL;
}

static char g_big[1 << 20];
static size_t g_big_len;

static void big_reset(void) { g_big_len = 0; g_big[0] = '\0'; }

static void big_add(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(g_big + g_big_len, sizeof(g_big) - g_big_len, fmt, ap);
    va_end(ap);
    if (n > 0 && g_big_len + (size_t)n < sizeof(g_big)) g_big_len += (size_t)n;
}

static void big_add_repeat(char c, size_t n) {
    for (size_t k = 0; k < n && g_big_len + 1 < sizeof(g_big); k++) g_big[g_big_len++] = c;
    g_big[g_big_len] = '\0';
}

static void build_rows(int nrows) {
    big_reset();
    big_add("INSERT INTO t ( a , b ) VALUES ");
    for (int k = 0; k < nrows; k++) {
        if (k) big_add(", ");
        big_add("( %d , %d ) ", k, k + 1);
    }
    big_add(";");
}

TEST(basic_insert) {
    itree *r = parse("INSERT INTO users ( id , name ) VALUES ( 1 , 'bob' ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->comp, "INSERT INTO");
    CHECK_STR(r->table, "users");
    CHECK(r->ignore == 0);
    CHECK(r->replace == 0);
    CHECK(cols_eq(r, 2, "id", "name"));
    CHECK(r->num == 1);
    CHECK(row_eq(r, 0, 2, "1", "'bob'"));
    return 0;
}

TEST(no_column_list) {
    itree *r = parse("INSERT INTO t VALUES ( 1 , 2 , 3 ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->table, "t");
    CHECK(r->table_col_num == 0);
    CHECK(r->num == 1);
    CHECK(row_eq(r, 0, 3, "1", "2", "3"));
    return 0;
}

TEST(multi_row_values) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) , ( 3 , 4 ) , ( 5 , 6 ) ;", 0);
    CHECK(r != NULL);
    CHECK(r->num == 3);
    CHECK(row_eq(r, 0, 2, "1", "2"));
    CHECK(row_eq(r, 1, 2, "3", "4"));
    CHECK(row_eq(r, 2, 2, "5", "6"));
    return 0;
}

TEST(insert_ignore) {
    itree *r = parse("INSERT IGNORE INTO t ( a ) VALUES ( 1 ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->comp, "INSERT INTO");
    CHECK(r->ignore == 1);
    CHECK(r->replace == 0);
    CHECK(row_eq(r, 0, 1, "1"));
    return 0;
}

TEST(replace_into) {
    itree *r = parse("REPLACE INTO t ( a , b ) VALUES ( 1 , 2 ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->comp, "REPLACE INTO");
    CHECK(r->replace == 1);
    CHECK(r->ignore == 0);
    CHECK(row_eq(r, 0, 2, "1", "2"));
    return 0;
}

TEST(schema_qualified_table) {
    itree *r = parse("INSERT INTO db.t VALUES ( 1 ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->table, "db.t");
    return 0;
}

TEST(literal_kinds) {
    itree *r = parse("INSERT INTO t ( a , b , c , d ) VALUES ( 'xy' , NULL , -5 , 3.14 ) ;", 0);
    CHECK(r != NULL);
    CHECK(row_eq(r, 0, 4, "'xy'", "NULL", "-5", "3.14"));
    return 0;
}

TEST(tokens_spread_over_rows) {
    itree *r = parse("INSERT INTO\nt ( a ,\nb )\nVALUES\n( 1 ,\n2 ) ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->table, "t");
    CHECK(cols_eq(r, 2, "a", "b"));
    CHECK(row_eq(r, 0, 2, "1", "2"));
    return 0;
}

TEST(one_token_per_row) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) , ( 3 , 4 ) ;", 1);
    CHECK(r != NULL);
    CHECK_STR(r->table, "t");
    CHECK(cols_eq(r, 2, "a", "b"));
    CHECK(r->num == 2);
    CHECK(row_eq(r, 1, 2, "3", "4"));
    return 0;
}

TEST(odku_simple) {
    itree *r = parse("INSERT INTO t ( id , n ) VALUES ( 1 , 2 ) ON DUPLICATE KEY UPDATE n = n + 1 ;", 0);
    CHECK(r != NULL);
    CHECK(r->num == 2);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL);
    CHECK(u->num == 1);
    itree *e = u->children[0];
    CHECK(e != NULL);
    CHECK_STR(e->comp, "=");
    CHECK(e->num == 2);
    CHECK_STR(e->children[0]->comp, "n");
    CHECK_STR(e->children[1]->comp, "+");
    CHECK_STR(e->children[1]->children[0]->comp, "n");
    CHECK_STR(e->children[1]->children[1]->comp, "1");
    return 0;
}

TEST(odku_multiple_assignments) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) ON DUPLICATE KEY UPDATE a = 1 , b = 2 ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL);
    CHECK(u->num == 2);
    CHECK_STR(u->children[0]->children[0]->comp, "a");
    CHECK_STR(u->children[1]->children[0]->comp, "b");
    CHECK_STR(u->children[1]->children[1]->comp, "2");
    return 0;
}

TEST(odku_values_function) {
    itree *r = parse("INSERT INTO t ( n ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE n = VALUES ( n ) ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    CHECK_STR(u->children[0]->comp, "=");
    CHECK_STR(u->children[0]->children[0]->comp, "n");
    CHECK_STR(u->children[0]->children[1]->comp, "VALUES");
    return 0;
}

TEST(odku_function_with_commas) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) ON DUPLICATE KEY UPDATE a = CONCAT ( a , 'x' ) , b = 2 ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL);
    CHECK(u->num == 2);
    CHECK_STR(u->children[0]->children[1]->comp, "CONCAT");
    CHECK_STR(u->children[1]->children[0]->comp, "b");
    return 0;
}

TEST(odku_precedence) {
    itree *r = parse("INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE a = 1 + 2 * 3 ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    itree *e = u->children[0];
    CHECK_STR(e->comp, "=");
    CHECK_STR(e->children[1]->comp, "+");
    CHECK_STR(e->children[1]->children[0]->comp, "1");
    CHECK_STR(e->children[1]->children[1]->comp, "*");
    CHECK_STR(e->children[1]->children[1]->children[0]->comp, "2");
    CHECK_STR(e->children[1]->children[1]->children[1]->comp, "3");
    return 0;
}

TEST(odku_left_associative) {
    itree *r = parse("INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE a = 1 - 2 - 3 ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    itree *top = u->children[0]->children[1];
    CHECK_STR(top->comp, "-");
    CHECK_STR(top->children[1]->comp, "3");
    CHECK_STR(top->children[0]->comp, "-");
    CHECK_STR(top->children[0]->children[0]->comp, "1");
    CHECK_STR(top->children[0]->children[1]->comp, "2");
    return 0;
}

TEST(odku_spread_over_rows) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 )\nON DUPLICATE KEY\nUPDATE a =\na + 1 ,\nb = 5 ;", 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL);
    CHECK(u->num == 2);
    CHECK_STR(u->children[0]->children[1]->comp, "+");
    CHECK_STR(u->children[1]->children[1]->comp, "5");
    return 0;
}

TEST(returning_multi) {
    itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) RETURNING a , b ;", 0);
    CHECK(r != NULL);
    CHECK(r->num == 2);
    itree *ret = child_named(r, "RETURNING");
    CHECK(ret != NULL);
    CHECK(ret->num == 2);
    CHECK_STR(ret->children[0]->comp, "a");
    CHECK_STR(ret->children[1]->comp, "b");
    return 0;
}

TEST(returning_star) {
    itree *r = parse("INSERT INTO t ( a ) VALUES ( 1 ) RETURNING * ;", 0);
    CHECK(r != NULL);
    itree *ret = child_named(r, "RETURNING");
    CHECK(ret != NULL && ret->num == 1);
    CHECK_STR(ret->children[0]->comp, "*");
    return 0;
}

TEST(odku_then_returning) {
    itree *r = parse("INSERT INTO t ( id ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE id = id + 1 RETURNING id ;", 0);
    CHECK(r != NULL);
    CHECK(r->num == 3);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    itree *ret = child_named(r, "RETURNING");
    CHECK(ret != NULL && ret->num == 1);
    CHECK_STR(ret->children[0]->comp, "id");
    return 0;
}

EXPECT_NULL(not_an_insert,          "UPDATE t SET a = 1 ;")
EXPECT_NULL(missing_into,           "INSERT t VALUES ( 1 ) ;")
EXPECT_NULL(ignore_missing_into,    "INSERT IGNORE t VALUES ( 1 ) ;")
EXPECT_NULL(replace_missing_into,   "REPLACE t ( a ) VALUES ( 1 ) ;")
EXPECT_NULL(replace_ignore,         "REPLACE IGNORE INTO t ( a ) VALUES ( 1 ) ;")
EXPECT_NULL(empty_query,            "")
EXPECT_NULL(only_semicolon,         ";")
EXPECT_NULL(too_many_values,        "INSERT INTO t ( a , b ) VALUES ( 1 , 2 , 3 ) ;")
EXPECT_NULL(too_few_values,         "INSERT INTO t ( a , b ) VALUES ( 1 ) ;")
EXPECT_NULL(second_row_wrong_size,  "INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) , ( 3 ) ;")
EXPECT_NULL(unclosed_row_with_cols, "INSERT INTO t ( a ) VALUES ( 1 ;")
EXPECT_NULL(unclosed_row_no_cols,   "INSERT INTO t VALUES ( 1 , 2 ;")
EXPECT_NULL(odku_missing_key,       "INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE UPDATE a = 1 ;")
EXPECT_NULL(odku_missing_duplicate, "INSERT INTO t ( a ) VALUES ( 1 ) ON KEY UPDATE a = 1 ;")
EXPECT_NULL(odku_missing_update,    "INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY a = 1 ;")

TEST(select_stub_insert_select) {
    itree *r = parse("INSERT INTO t ( a ) SELECT 1 ;", 0);
    CHECK(r != NULL);
    CHECK_STR(r->table, "t");
    CHECK(r->num == 0);
    return 0;
}

EXPECT_NULL(select_stub_in_values, "INSERT INTO t ( a ) VALUES ( ( SELECT 1 ) ) ;")

TEST(cols_128_values_128) {
    big_reset();
    big_add("INSERT INTO t ( ");
    for (int k = 0; k < 128; k++) { if (k) big_add(", "); big_add("c%d ", k); }
    big_add(") VALUES ( ");
    for (int k = 0; k < 128; k++) { if (k) big_add(", "); big_add("v%d ", k); }
    big_add(") ;");
    itree *r = parse(g_big, 0);
    CHECK(r != NULL);
    CHECK(r->table_col_num == 128);
    CHECK_STR(r->table_cols[127], "c127");
    CHECK(r->num == 1);
    CHECK(r->children[0]->insert_val_num == 128);
    CHECK_STR(r->children[0]->insert_values[127], "v127");
    return 0;
}

TEST(cols_129_rejected) {
    big_reset();
    big_add("INSERT INTO t ( ");
    for (int k = 0; k < 129; k++) { if (k) big_add(", "); big_add("c%d ", k); }
    big_add(") VALUES ( ");
    for (int k = 0; k < 129; k++) { if (k) big_add(", "); big_add("v%d ", k); }
    big_add(") ;");
    CHECK(parse(g_big, 0) == NULL);
    return 0;
}

TEST(rows_300) {
    build_rows(300);
    itree *r = parse(g_big, 0);
    CHECK(r != NULL);
    CHECK(r->num == 300);
    CHECK(row_eq(r, 299, 2, "299", "300"));
    return 0;
}

TEST(rows_50_one_token_per_row) {
    build_rows(50);
    itree *r = parse(g_big, 1);
    CHECK(r != NULL);
    CHECK(r->num == 50);
    CHECK(row_eq(r, 49, 2, "49", "50"));
    return 0;
}

TEST(odku_100_assignments) {
    big_reset();
    big_add("INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE ");
    for (int k = 0; k < 100; k++) { if (k) big_add(", "); big_add("c%d = %d ", k, k); }
    big_add(";");
    itree *r = parse(g_big, 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL);
    CHECK(u->num == 100);
    CHECK_STR(u->children[0]->children[0]->comp, "c0");
    CHECK_STR(u->children[99]->children[0]->comp, "c99");
    CHECK_STR(u->children[99]->children[1]->comp, "99");
    return 0;
}

TEST(odku_long_expression) {
    big_reset();
    big_add("INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE a = 1 ");
    for (int k = 1; k < 300; k++) big_add("+ 1 ");
    big_add(";");
    itree *r = parse(g_big, 0);
    CHECK(r != NULL);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    itree *n = u->children[0]->children[1];
    int depth = 0;
    while (n && n->comp && strcmp(n->comp, "+") == 0) { depth++; n = n->children[0]; }
    CHECK(depth == 299);
    return 0;
}

TEST(long_tokens) {
    big_reset();
    big_add("INSERT INTO t ( a ) VALUES ( ");
    big_add_repeat('x', 4096);
    big_add(" ) ON DUPLICATE KEY UPDATE a = ");
    big_add_repeat('y', 4096);
    big_add(" ;");
    itree *r = parse(g_big, 0);
    CHECK(r != NULL);
    CHECK(strlen(r->children[0]->insert_values[0]) == 4096);
    itree *u = child_named(r, "ON DUPLICATE KEY UPDATE");
    CHECK(u != NULL && u->num == 1);
    CHECK(strlen(u->children[0]->children[1]->comp) == 4096);
    return 0;
}

TEST(repeat_parse_500) {
    for (int k = 0; k < 500; k++) {
        itree *r = parse("INSERT INTO t ( a , b ) VALUES ( 1 , 2 ) , ( 3 , 4 ) ON DUPLICATE KEY UPDATE a = a + 1 RETURNING a ;", 0);
        CHECK(r != NULL);
        CHECK(r->num == 4);
        CHECK(row_eq(r, 1, 2, "3", "4"));
    }
    return 0;
}

EXPECT_NULL(gap_missing_table,           "INSERT INTO ;")
EXPECT_NULL(gap_unclosed_column_list,    "INSERT INTO t ( a , b VALUES ( 1 , 2 ) ;")
EXPECT_NULL(gap_no_values_or_select,     "INSERT INTO t ( a ) ;")
EXPECT_NULL(gap_odku_no_semicolon,       "INSERT INTO t ( a ) VALUES ( 1 ) ON DUPLICATE KEY UPDATE a = 1")
EXPECT_NULL(gap_returning_no_semicolon,  "INSERT INTO t ( a ) VALUES ( 1 ) RETURNING a")

TEST(gap_rows_301) {
    build_rows(301);
    CHECK(parse(g_big, 0) == NULL);
    return 0;
}

typedef struct { const char *name; int (*fn)(void); int gap; } TestCase;
#define T(n) { #n, n, 0 }
#define G(n) { #n, n, 1 }

static const TestCase TESTS[] = {

    T(basic_insert), T(no_column_list), T(multi_row_values), T(insert_ignore),
    T(replace_into), T(schema_qualified_table), T(literal_kinds),
    T(tokens_spread_over_rows), T(one_token_per_row),

    T(odku_simple), T(odku_multiple_assignments), T(odku_values_function),
    T(odku_function_with_commas), T(odku_precedence), T(odku_left_associative),
    T(odku_spread_over_rows),

    T(returning_multi), T(returning_star), T(odku_then_returning),

    T(not_an_insert), T(missing_into), T(ignore_missing_into), T(replace_missing_into),
    T(replace_ignore), T(empty_query), T(only_semicolon), T(too_many_values),
    T(too_few_values), T(second_row_wrong_size), T(unclosed_row_with_cols),
    T(unclosed_row_no_cols), T(odku_missing_key), T(odku_missing_duplicate),
    T(odku_missing_update),

    T(select_stub_insert_select), T(select_stub_in_values),

    T(cols_128_values_128), T(cols_129_rejected), T(rows_300), T(rows_50_one_token_per_row),
    T(odku_100_assignments), T(odku_long_expression), T(long_tokens), T(repeat_parse_500),

    G(gap_missing_table), G(gap_unclosed_column_list), G(gap_no_values_or_select),
    G(gap_odku_no_semicolon), G(gap_returning_no_semicolon), G(gap_rows_301),
};

enum { ST_OK, ST_FAIL, ST_CRASH, ST_HANG };

static int run_forked(int (*fn)(void), unsigned secs, int *info) {
    fflush(stdout);
    fflush(stderr);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(2); }
    if (pid == 0) {
        alarm(secs);
        int rc = fn();
        fflush(stdout);
        _exit(rc ? 7 : 0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    *info = 0;
    if (WIFSIGNALED(status)) {
        *info = WTERMSIG(status);
        return (*info == SIGALRM) ? ST_HANG : ST_CRASH;
    }
    if (WIFEXITED(status)) {
        *info = WEXITSTATUS(status);
        if (*info == 0) return ST_OK;
        if (*info == 7) return ST_FAIL;
    }
    return ST_CRASH;
}

static unsigned long g_rng = 20261023UL;
static unsigned rnd(unsigned n) {
    g_rng = g_rng * 6364136223846793005UL + 1442695040888963407UL;
    return (unsigned)((g_rng >> 33) % n);
}

static const char *FRAG[] = {
    "( a , b )", "VALUES ( 1 , 2 )", "VALUES ( 1 , 2 ) , ( 3 , 4 )",
    "ON DUPLICATE KEY UPDATE a = a + 1",
    "ON DUPLICATE KEY UPDATE a = VALUES ( a ) , b = 2",
    "RETURNING a , b", "SELECT 1"
};
static const char *VOC[] = {
    "INSERT", "IGNORE", "REPLACE", "INTO", "t", "(", ")", ",", "VALUES", "SELECT", "ON",
    "DUPLICATE", "KEY", "UPDATE", "RETURNING", "=", "+", "-", "*", "a", "b", "1", "'x'",
    "NOW", ";"
};
#define N_FRAG (sizeof(FRAG) / sizeof(FRAG[0]))
#define N_VOC  (sizeof(VOC) / sizeof(VOC[0]))

static char g_spec[4096];
static int g_one_per_row;

static void gen_fuzz(char *out, size_t cap) {
    static const char *PRE[] = { "INSERT INTO t", "INSERT INTO t", "INSERT INTO t",
                                 "REPLACE INTO t", "INSERT IGNORE INTO t", "" };
    size_t len = 0;
    out[0] = '\0';
    len += (size_t)snprintf(out, cap, "%s", PRE[rnd(6)]);
    int pieces = (int)rnd(14);
    for (int k = 0; k < pieces; k++) {
        const char *p = (rnd(2) == 0) ? FRAG[rnd((unsigned)N_FRAG)] : VOC[rnd((unsigned)N_VOC)];
        if (len + strlen(p) + 4 >= cap) break;
        out[len++] = (rnd(5) == 0) ? '\n' : ' ';
        len += (size_t)snprintf(out + len, cap - len, "%s", p);
    }
    if (len + 4 < cap) { out[len++] = ' '; out[len++] = ';'; out[len] = '\0'; }
}

static int fuzz_case(void) {
    itree *r = parse(g_spec, g_one_per_row);
    (void)r;
    return 0;
}

static void print_spec(const char *s) {
    for (; *s; s++) { if (*s == '\n') fputs(" | ", stdout); else putchar(*s); }
}

#define FUZZ_N 1000

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    int n = (int)(sizeof(TESTS) / sizeof(TESTS[0]));
    int pass = 0, fail = 0, crash = 0, hang = 0, gap_open = 0, gap_fixed = 0;

    printf("insert_parser tests: %d cases, each in its own process\n\n", n);

    for (int k = 0; k < n; k++) {
        const char *name = TESTS[k].name;
        int info = 0;
        g_quiet = TESTS[k].gap;
        int st = run_forked(TESTS[k].fn, 2, &info);
        g_quiet = 0;

        if (TESTS[k].gap) {
            if (st == ST_OK) {
                gap_fixed++;
                printf("[FIXED] %s  (known gap, now passes)\n", name);
            } else {
                gap_open++;
                printf("[GAP  ] %s  (%s)\n", name,
                       st == ST_HANG ? "hangs" : st == ST_CRASH ? "crashes" : "wrong result");
            }
        } else if (st == ST_OK) {
            pass++;
            printf("[PASS ] %s\n", name);
        } else if (st == ST_FAIL) {
            fail++;
            printf("[FAIL ] %s\n", name);
        } else if (st == ST_HANG) {
            hang++;
            printf("[HANG ] %s  (no answer after 2s)\n", name);
        } else {
            crash++;
            printf("[CRASH] %s  (code/signal %d, see sanitizer output above)\n", name, info);
        }
    }

    printf("\nfuzzing: %d random token sequences (fixed seed, so reproducible)\n", FUZZ_N);
    int fz_bad = 0;
    for (int k = 0; k < FUZZ_N; k++) {
        int info = 0;
        gen_fuzz(g_spec, sizeof(g_spec));
        g_one_per_row = (rnd(4) == 0);
        g_quiet = 1;
        int st = run_forked(fuzz_case, 2, &info);
        g_quiet = 0;
        if (st == ST_CRASH || st == ST_HANG) {
            fz_bad++;
            if (fz_bad <= 5) {
                printf("  case #%d %s (one_per_row=%d): ", k, st == ST_HANG ? "HANG" : "CRASH", g_one_per_row);
                print_spec(g_spec);
                printf("\n");
            }
        }
    }
    printf("fuzz problems: %d\n", fz_bad);

    printf("\n---- summary ----\n");
    printf("passed %d | failed %d | crashed %d | hung %d\n", pass, fail, crash, hang);
    printf("known gaps still open %d | known gaps now fixed %d | fuzz problems %d\n",
           gap_open, gap_fixed, fz_bad);

    return (fail || crash || hang || fz_bad) ? 1 : 0;
}

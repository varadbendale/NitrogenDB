#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "Create_parser.c"

static struct { char ***query; } proper_data ;

#define TFAIL 42
static int g_warn;

#define CHECK(cond) do { if (!(cond)) { \
    fprintf(stderr, "    CHECK failed: %s  (line %d)\n", #cond, __LINE__); return TFAIL; } } while (0)
#define SOFT(cond)  do { if (!(cond)) { g_warn++; \
    fprintf(stderr, "    WARN (tokenizer-dependent): %s  (line %d)\n", #cond, __LINE__); } } while (0)

enum { ST_PASS, ST_WARN, ST_FAIL, ST_HANG, ST_CRASH, ST_N };
static const char *ST_NAME[ST_N] = { "PASS", "WARN", "FAIL", "HANG", "CRASH" };

typedef struct { char *s; size_t len, cap; } SB;
static void sb_init(SB *b) { b->cap = 256; b->len = 0; b->s = malloc(b->cap); b->s[0] = 0; }
static void sb_add(SB *b, const char *fmt, ...)
{
    for (;;) {
        va_list ap;
        va_start(ap, fmt);
        int w = vsnprintf(b->s + b->len, b->cap - b->len, fmt, ap);
        va_end(ap);
        if (w >= 0 && (size_t)w < b->cap - b->len) { b->len += (size_t)w; return; }
        b->cap = b->cap * 2 + (size_t)(w > 0 ? w : 0);
        b->s = realloc(b->s, b->cap);
    }
}
static void sb_rep(SB *b, const char *unit, size_t times)
{
    size_t ul = strlen(unit);
    if (b->len + ul * times + 1 > b->cap) {
        b->cap = b->len + ul * times + 1;
        b->s = realloc(b->s, b->cap);
    }
    for (size_t k = 0; k < times; k++) { memcpy(b->s + b->len, unit, ul); b->len += ul; }
    b->s[b->len] = 0;
}

static char **tokenize(const char *s, int *n_out)
{
    int cap = 64, n = 0;
    char **t = malloc((size_t)cap * sizeof *t);
    const char *p = s;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;
        const char *st = p;
        if (strchr("(),;", *p)) p++;
        else while (*p && !isspace((unsigned char)*p) && !strchr("(),;", *p)) p++;
        if (n == cap) { cap *= 2; t = realloc(t, (size_t)cap * sizeof *t); }
        t[n++] = strndup(st, (size_t)(p - st));
    }
    *n_out = n;
    return t;
}

static char ***layout(char **tok, int n, int per_row)
{
    if (per_row <= 0) per_row = n > 0 ? n : 1;
    int nrows = (n + per_row - 1) / per_row;
    char ***buf = calloc((size_t)nrows + 1, sizeof *buf);
    for (int r = 0; r < nrows; r++) {
        int from = r * per_row;
        int cnt = (n - from < per_row) ? (n - from) : per_row;
        buf[r] = calloc((size_t)cnt + 1, sizeof **buf);
        for (int k = 0; k < cnt; k++) buf[r][k] = tok[from + k];
    }
    buf[nrows] = calloc(1, sizeof **buf);
    return buf;
}

static ctree *parse(const char *sql, int per_row)
{
    int n;
    char **tok = tokenize(sql, &n);
    proper_data.query = layout(tok, n, per_row);
    return create_parser();
}

static ctree *table_of(ctree *root) { return (root && root->num > 0) ? root->children[0] : NULL; }

static ctree *find_col(ctree *tb, const char *name)
{
    if (!tb) return NULL;
    for (int k = 0; k < tb->num; k++)
        if (tb->children[k] && tb->children[k]->comp && strcmp(tb->children[k]->comp, name) == 0)
            return tb->children[k];
    return NULL;
}

static char g_dump[1 << 22];
static size_t g_len;
static void dump_node(const ctree *n, int depth)
{
    if (!n) return;
    g_len += (size_t)snprintf(g_dump + g_len, sizeof g_dump - g_len,
        "%*s%.60s type=%d bits=%d dec=%d/%d nn=%d uq=%d def=%.40s pk=%d fk=%d ref=%d kids=%d\n",
        depth * 2, "", n->comp ? n->comp : "(null)", n->type, n->type_bits_count,
        n->first_decimal, n->second_decimal, (int)n->not_null, (int)n->unique,
        n->default_val ? n->default_val : "(null)", n->primary_key_counter,
        n->foreign_key_counter, n->references, n->num);
    if (g_len >= sizeof g_dump) g_len = sizeof g_dump - 1;
    for (int k = 0; k < n->num; k++) dump_node(n->children[k], depth + 1);
}
static const char *dump(const ctree *root)
{
    g_len = 0;
    g_dump[0] = 0;
    if (!root) return "(null)\n";
    dump_node(root, 0);
    return g_dump;
}

static int t_empty(int u)        { CHECK(parse("", 0) == NULL); return 0; }
static int t_only_semi(int u)    { CHECK(parse(";", 0) == NULL); return 0; }
static int t_not_create(int u)   { CHECK(parse("SELECT * FROM t ;", 0) == NULL); return 0; }
static int t_create_alone(int u) { CHECK(parse("CREATE", 0) == NULL); return 0; }
static int t_create_index(int u) { CHECK(parse("CREATE INDEX i ON t ( a ) ;", 0) == NULL); return 0; }

static int t_simple_table(int u)
{
    ctree *r = parse("CREATE TABLE t ( a INT ) ;", 0);
    CHECK(r != NULL);
    CHECK(r->comp && strcmp(r->comp, "CREATE TABLE") == 0);
    ctree *tb = table_of(r);
    CHECK(tb && tb->comp && strcmp(tb->comp, "t") == 0);
    ctree *a = find_col(tb, "a");
    CHECK(a != NULL);
    CHECK(a->type == INT);
    return 0;
}

static const struct { const char *kw; int type; int bits; } KT[] = {
    {"INT", INT, 4}, {"BIGINT", BIGINT, 8}, {"SMALLINT", SMALLINT, 2}, {"TINYINT", TINYINT, 1},
    {"FLOAT", FLOAT, 4}, {"DOUBLE", DOUBLE, 8}, {"BOOL", BOOL, 1}, {"DATE", DATE, 4},
    {"TIMESTAMP", TIMESTAMP, 8}
};
#define NKT (sizeof KT / sizeof *KT)

static int t_all_types(int u)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t (");
    for (size_t k = 0; k < NKT; k++) sb_add(&sb, " c_%s %s ,", KT[k].kw, KT[k].kw);
    sb_add(&sb, " z INT ) ;");
    ctree *tb = table_of(parse(sb.s, 0));
    CHECK(tb != NULL);
    for (size_t k = 0; k < NKT; k++) {
        char nm[32];
        snprintf(nm, sizeof nm, "c_%s", KT[k].kw);
        ctree *c = find_col(tb, nm);
        if (!c) { fprintf(stderr, "    column %s not found\n", nm); return TFAIL; }
        if (c->type != KT[k].type || c->type_bits_count != KT[k].bits) {
            fprintf(stderr, "    column %s: type=%d bits=%d, expected type=%d bits=%d\n",
                    nm, c->type, c->type_bits_count, KT[k].type, KT[k].bits);
            return TFAIL;
        }
    }
    return 0;
}

#define COL(var, name) ctree *var = find_col(tb, name); CHECK(var != NULL)

static int t_constraints(int u)
{
    ctree *r = parse("CREATE TABLE t ( a INT NOT NULL , b INT UNIQUE , c INT DEFAULT 5 , "
                     "d INT NOT NULL DEFAULT 7 UNIQUE , z INT ) ;", 0);
    ctree *tb = table_of(r);
    CHECK(tb != NULL);
    COL(a, "a"); COL(b, "b"); COL(c, "c"); COL(d, "d");
    CHECK(a->not_null && !a->unique && !a->default_val);
    CHECK(b->unique && !b->not_null);
    CHECK(c->default_val && strcmp(c->default_val, "5") == 0);
    CHECK(d->not_null && d->unique && d->default_val && strcmp(d->default_val, "7") == 0);
    return 0;
}

static int t_defaults(int u)
{
    ctree *r = parse("CREATE TABLE t ( a INT DEFAULT abc , b BOOL DEFAULT true , c FLOAT DEFAULT -1.5 , "
                     "d INT DEFAULT -3 , e INT DEFAULT 1.5 , z INT ) ;", 0);
    ctree *tb = table_of(r);
    CHECK(tb != NULL);
    COL(a, "a"); COL(b, "b"); COL(c, "c"); COL(d, "d"); COL(e, "e");
    CHECK(a->default_val == NULL);
    CHECK(b->default_val && strcmp(b->default_val, "true") == 0);
    CHECK(c->default_val && strcmp(c->default_val, "-1.5") == 0);
    CHECK(d->default_val && strcmp(d->default_val, "-3") == 0);
    CHECK(e->default_val == NULL);
    return 0;
}

static int t_pk_inline(int u)
{
    ctree *tb = table_of(parse("CREATE TABLE t ( id INT PRIMARY KEY , z INT ) ;", 0));
    CHECK(tb != NULL);
    COL(id, "id");
    CHECK(id->primary_key_counter == 1);
    return 0;
}

static int t_pk_table(int u)
{
    ctree *tb = table_of(parse("CREATE TABLE t ( a INT , b INT , PRIMARY KEY ( a , b ) ) ;", 0));
    CHECK(tb != NULL);
    COL(pk, "PRIMARY KEY");
    CHECK(pk->primary_key_counter >= 2);
    return 0;
}

static int t_foreign(int u)
{
    ctree *tb = table_of(parse("CREATE TABLE t ( a INT , FOREIGN KEY ( a ) REFERENCES r ( x ) ) ;", 0));
    CHECK(tb != NULL);
    COL(f, "FOREIGN KEY");
    CHECK(f->foreign_key_counter == 1);
    CHECK(f->references == 1);
    CHECK(f->num == 1 && f->children[0] && f->children[0]->comp && strcmp(f->children[0]->comp, "r") == 0);
    CHECK(f->children[0]->num == 1 && strcmp(f->children[0]->children[0]->comp, "x") == 0);
    return 0;
}

static int t_sizes_soft(int u)
{
    ctree *tb = table_of(parse("CREATE TABLE t ( c CHAR ( 5 ) , v VARCHAR ( 20 ) , p DECIMAL ( 10 , 2 ) , z INT ) ;", 0));
    CHECK(tb != NULL);
    ctree *c = find_col(tb, "c"), *v = find_col(tb, "v"), *p = find_col(tb, "p");
    SOFT(c && c->type == CHAR && c->type_bits_count == 5);
    SOFT(v && v->type == VARCHAR && v->type_bits_count == 20);
    SOFT(p && p->first_decimal == 10 && p->second_decimal == 2);
    return 0;
}

static int t_layout(int u)
{
    const char *S = "CREATE TABLE t ( a INT NOT NULL , b INT DEFAULT 5 , c INT UNIQUE , z INT ) ;";
    char *base = strdup(dump(parse(S, 0)));
    static const int modes[] = { 1, 2, 3, 5, 7 };
    for (size_t m = 0; m < sizeof modes / sizeof *modes; m++) {
        const char *d = dump(parse(S, modes[m]));
        if (strcmp(base, d) != 0) {
            fprintf(stderr, "    %d token(s)/row differs from single-row result\n"
                            "--- single row ---\n%s--- this layout ---\n%s", modes[m], base, d);
            return TFAIL;
        }
    }
    return 0;
}

static int t_repeat(int n)
{
    const char *S = "CREATE TABLE t ( a INT NOT NULL , b INT DEFAULT 5 , z INT ) ;";
    char *first = strdup(dump(parse(S, 0)));
    for (int k = 0; k < n; k++) {
        parse("SELECT 1 ;", 0);
        if (strcmp(first, dump(parse(S, k % 3))) != 0) {
            fprintf(stderr, "    iteration %d produced a different tree\n", k);
            return TFAIL;
        }
    }
    return 0;
}

static const struct { const char *name; const char *sql; } NC[] = {
    {"missing_table_name",     "CREATE TABLE ( a INT ) ;"},
    {"empty_column_list",      "CREATE TABLE t ( ) ;"},
    {"no_paren",               "CREATE TABLE t ;"},
    {"column_without_type",    "CREATE TABLE t ( a ) ;"},
    {"double_comma",           "CREATE TABLE t ( a INT , , b INT ) ;"},
    {"leading_comma",          "CREATE TABLE t ( , a INT ) ;"},
    {"trailing_comma",         "CREATE TABLE t ( a INT , ) ;"},
    {"extra_close_parens",     "CREATE TABLE t ( a INT ) ) ) ;"},
    {"nested_parens",          "CREATE TABLE t ( ( a INT ) ) ;"},
    {"semicolon_inside",       "CREATE TABLE t ( a INT ; b INT ) ;"},
    {"two_statements",         "CREATE TABLE t ( a INT ) ; CREATE TABLE u ( b INT ) ;"},
    {"keywords_as_names",      "CREATE TABLE CREATE ( PRIMARY INT , TABLE INT , z INT ) ;"},
    {"lowercase",              "create table t ( a int ) ;"},
    {"no_semicolon",           "CREATE TABLE t ( a INT , b INT )"},
    {"default_no_value",       "CREATE TABLE t ( a INT DEFAULT ) ;"},
    {"not_without_null",       "CREATE TABLE t ( a INT NOT ) ;"},
    {"dangling_references",    "CREATE TABLE t ( a INT REFERENCES ) ;"},
    {"foreign_key_truncated",  "CREATE TABLE t ( a INT , FOREIGN KEY ) ;"},
    {"pk_empty_list",          "CREATE TABLE t ( a INT , PRIMARY KEY ( ) ) ;"},
    {"fk_no_ref_columns",      "CREATE TABLE t ( a INT , FOREIGN KEY ( a ) REFERENCES r ) ;"},
    {"fk_no_ref_table",        "CREATE TABLE t ( a INT , FOREIGN KEY ( a ) REFERENCES ) ;"},
    {"pk_in_the_middle",       "CREATE TABLE t ( a INT , PRIMARY KEY ( a ) , b INT ) ;"},
    {"decimal_bare",           "CREATE TABLE t ( a DECIMAL ) ;"},
    {"decimal_empty_args",     "CREATE TABLE t ( a DECIMAL ( ) ) ;"},
    {"decimal_one_arg",        "CREATE TABLE t ( a DECIMAL ( 10 ) ) ;"},
    {"varchar_no_size",        "CREATE TABLE t ( a VARCHAR ) ;"},
    {"char_non_numeric_size",  "CREATE TABLE t ( a CHAR ( x ) ) ;"},
    {"huge_number",            "CREATE TABLE t ( a VARCHAR ( 99999999999999999999 ) ) ;"},
    {"create_table_as_select", "CREATE TABLE t ( a INT ) AS SELECT * FROM u ;"},
    {"only_create_table",      "CREATE TABLE"},
    {"create_table_name_only", "CREATE TABLE t"},
    {"just_open_paren",        "CREATE TABLE t ("},
};
#define NNC (int)(sizeof NC / sizeof *NC)

static int t_nocrash(int arg)
{
    int idx = arg / 2, mode = arg % 2;
    ctree *r = parse(NC[idx].sql, mode ? 1 : 0);
    if (getenv("VERBOSE")) fprintf(stderr, "%s", dump(r));
    return 0;
}

static char *gen_wide(int n)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t (");
    for (int k = 0; k < n; k++) sb_add(&sb, " c%d INT ,", k);
    sb_add(&sb, " z INT ) ;");
    return sb.s;
}

static int t_wide(int n)
{
    ctree *tb = table_of(parse(gen_wide(n), 0));
    if (n <= 250) {
        char last[32];
        snprintf(last, sizeof last, "c%d", n - 1);
        CHECK(tb != NULL && tb->num >= n);
        CHECK(find_col(tb, "c0") != NULL);
        CHECK(find_col(tb, last) != NULL);
    }
    return 0;
}

static int t_wide_rows(int n)
{
    ctree *tb = table_of(parse(gen_wide(n), 1));
    if (n <= 250) CHECK(tb != NULL && tb->num >= n);
    return 0;
}

static int t_pk_wide(int n)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( a INT , PRIMARY KEY (");
    for (int k = 0; k < n; k++) sb_add(&sb, " c%d%s", k, k + 1 < n ? " ," : "");
    sb_add(&sb, " ) ) ;");
    parse(sb.s, 0);
    return 0;
}
static int t_fk_wide(int n)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( a INT , FOREIGN KEY (");
    for (int k = 0; k < n; k++) sb_add(&sb, " c%d%s", k, k + 1 < n ? " ," : "");
    sb_add(&sb, " ) REFERENCES r ( x ) ) ;");
    parse(sb.s, 0);
    return 0;
}

static int t_deep_balanced(int n)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( a INT , b");
    sb_rep(&sb, " (", (size_t)n);
    sb_rep(&sb, " )", (size_t)n);
    sb_add(&sb, " , z INT ) ;");
    parse(sb.s, 0);
    return 0;
}
static int t_deep_open(int n)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( a INT , b");
    sb_rep(&sb, " (", (size_t)n);
    sb_add(&sb, " ) ;");
    parse(sb.s, 0);
    return 0;
}

static int t_long_ident(int len)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( ");
    sb_rep(&sb, "x", (size_t)len);
    sb_add(&sb, " INT , z INT ) ;");
    parse(sb.s, 0);
    return 0;
}

static int t_long_default(int len)
{
    SB sb; sb_init(&sb);
    sb_add(&sb, "CREATE TABLE t ( a INT DEFAULT ");
    sb_rep(&sb, "7", (size_t)len);
    sb_add(&sb, " , z INT ) ;");
    ctree *tb = table_of(parse(sb.s, 0));
    CHECK(tb != NULL);
    COL(a, "a");
    CHECK(a->default_val && strlen(a->default_val) == (size_t)len);
    return 0;
}

static const char *TRUNC_SRC =
    "CREATE TABLE t ( a INT NOT NULL DEFAULT 5 , b VARCHAR ( 20 ) UNIQUE , "
    "PRIMARY KEY ( a ) , FOREIGN KEY ( b ) REFERENCES r ( x ) ) ;";

static char *trunc_sql(int k)
{
    int n;
    char **tok = tokenize(TRUNC_SRC, &n);
    SB sb; sb_init(&sb);
    for (int i = 0; i < k && i < n; i++) sb_add(&sb, "%s ", tok[i]);
    return sb.s;
}
static int t_trunc(int arg)
{
    char *s = trunc_sql(arg / 2);
    parse(s, (arg % 2) ? 1 : 0);
    return 0;
}
static void desc_trunc(int arg, char *out, size_t n)
{
    char *s = trunc_sql(arg / 2);
    snprintf(out, n, "%d tokens, %s: %s", arg / 2, (arg % 2) ? "1 token/row" : "single row", s);
    free(s);
}

static unsigned g_fuzz_base = 1;
static unsigned long long rs;
static void seed_rng(unsigned s)
{
    rs = 0x9E3779B97F4A7C15ULL ^ ((unsigned long long)s * 0xD1B54A32D192ED03ULL);
    if (!rs) rs = 1;
    for (int k = 0; k < 4; k++) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; }
}
static unsigned rnd(void) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs >> 11); }

static const char *VOCAB[] = {
    "CREATE", "TABLE", "t", "a", "b", "(", ")", ",", ";", "INT", "BIGINT", "SMALLINT", "TINYINT",
    "FLOAT", "DOUBLE", "CHAR", "VARCHAR", "DECIMAL", "BOOL", "DATE", "TIMESTAMP", "NOT", "NULL",
    "DEFAULT", "UNIQUE", "PRIMARY", "KEY", "FOREIGN", "REFERENCES", "AS", "10", "2", "-5", "1.5",
    "abc", "true", "*", "SELECT"
};
#define NV (sizeof VOCAB / sizeof *VOCAB)

static char *gen_fuzz(unsigned seed, int terminated, int *per_row)
{
    seed_rng(seed);
    SB sb; sb_init(&sb);
    if (rnd() % 10 < 7) sb_add(&sb, "CREATE TABLE t (");
    int len = 1 + (int)(rnd() % 40);
    for (int k = 0; k < len; k++) sb_add(&sb, " %s", VOCAB[rnd() % NV]);
    if (terminated) sb_add(&sb, " ;");
    *per_row = (int)(rnd() % 4);
    return sb.s;
}
static int t_fuzz_term(int arg)
{
    int pr; char *s = gen_fuzz(g_fuzz_base + (unsigned)arg, 1, &pr);
    parse(s, pr);
    return 0;
}
static int t_fuzz_unterm(int arg)
{
    int pr; char *s = gen_fuzz(g_fuzz_base + (unsigned)arg, 0, &pr);
    parse(s, pr);
    return 0;
}
static void desc_fuzz_term(int arg, char *out, size_t n)
{
    int pr; char *s = gen_fuzz(g_fuzz_base + (unsigned)arg, 1, &pr);
    snprintf(out, n, "seed=%u rows=%d: %s", g_fuzz_base + (unsigned)arg, pr, s);
    free(s);
}
static void desc_fuzz_unterm(int arg, char *out, size_t n)
{
    int pr; char *s = gen_fuzz(g_fuzz_base + (unsigned)arg, 0, &pr);
    snprintf(out, n, "seed=%u rows=%d: %s", g_fuzz_base + (unsigned)arg, pr, s);
    free(s);
}

static int run_isolated(int (*fn)(int), int arg, int timeout_ms, int quiet, int *detail)
{
    fflush(NULL);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(2); }
    if (pid == 0) {
        if (quiet) {
            int fd = open("/dev/null", O_WRONLY);
            if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); }
        }
        struct itimerval it;
        memset(&it, 0, sizeof it);
        it.it_value.tv_sec  = timeout_ms / 1000;
        it.it_value.tv_usec = (timeout_ms % 1000) * 1000;
        setitimer(ITIMER_REAL, &it, NULL);
        g_warn = 0;
        int rc = fn(arg);
        if (rc == 0 && g_warn) rc = 43;
        _exit(rc);
    }
    int st = 0;
    waitpid(pid, &st, 0);
    if (WIFSIGNALED(st)) {
        *detail = -WTERMSIG(st);
        return WTERMSIG(st) == SIGALRM ? ST_HANG : ST_CRASH;
    }
    int code = WEXITSTATUS(st);
    *detail = code;
    if (code == 0)  return ST_PASS;
    if (code == 43) return ST_WARN;
    if (code == TFAIL) return ST_FAIL;
    return ST_CRASH;
}

static int g_cnt[ST_N];
static const char *g_filter;

static void run_case(const char *name, int (*fn)(int), int arg, int timeout_ms)
{
    if (g_filter && !strstr(name, g_filter)) return;
    int detail = 0;
    int st = run_isolated(fn, arg, timeout_ms, 0, &detail);
    g_cnt[st]++;
    printf("[%-5s] %s", ST_NAME[st], name);
    if (st == ST_HANG)  printf("   (no result within %d ms -- probably an infinite loop)", timeout_ms);
    if (st == ST_CRASH) {
        if (detail < 0) printf("   (signal %d: %s)", -detail, strsignal(-detail));
        else            printf("   (exit code %d -- sanitizer abort?)", detail);
    }
    printf("\n");
}

static void run_suite(const char *name, int (*fn)(int), int from, int to, int timeout_ms,
                      void (*describe)(int, char *, size_t))
{
    if (g_filter && !strstr(name, g_filter)) return;
    int cnt[ST_N] = {0}, shown = 0, total = 0;
    for (int arg = from; arg <= to; arg++) {
        int detail = 0;
        int st = run_isolated(fn, arg, timeout_ms, 1, &detail);
        cnt[st]++; total++; g_cnt[st]++;
        if (st != ST_PASS && st != ST_WARN && shown < 5) {
            char d[2048];
            describe(arg, d, sizeof d);
            printf("   first failures: [%s] %s\n", ST_NAME[st], d);
            shown++;
        }
    }
    printf("[SUITE] %s: %d cases | pass %d | warn %d | fail %d | hang %d | crash %d\n",
           name, total, cnt[ST_PASS], cnt[ST_WARN], cnt[ST_FAIL], cnt[ST_HANG], cnt[ST_CRASH]);
}

typedef struct { const char *name; int (*fn)(int); int arg; int timeout_ms; } Case;

static const Case CASES[] = {

    {"empty_input",               t_empty,        0, 1000},
    {"only_semicolon",            t_only_semi,    0, 1000},
    {"not_a_create",              t_not_create,   0, 1000},
    {"create_alone",              t_create_alone, 0, 1000},
    {"create_index_not_table",    t_create_index, 0, 1000},
    {"simple_table",              t_simple_table, 0, 2000},
    {"all_types_fixed_widths",    t_all_types,    0, 2000},
    {"constraints_nn_unique_def", t_constraints,  0, 2000},
    {"default_validation",        t_defaults,     0, 2000},
    {"primary_key_inline",        t_pk_inline,    0, 2000},
    {"primary_key_table_level",   t_pk_table,     0, 2000},
    {"foreign_key_references",    t_foreign,      0, 2000},
    {"sizes_char_varchar_decimal",t_sizes_soft,   0, 2000},
    {"layout_invariance",         t_layout,       0, 2000},
    {"repeat_2000_determinism",   t_repeat,    2000, 20000},

    {"stress_wide_50",            t_wide,        50, 5000},
    {"stress_wide_250",           t_wide,       250, 5000},
    {"stress_wide_299",           t_wide,       299, 5000},
    {"stress_wide_300",           t_wide,       300, 5000},
    {"stress_wide_301",           t_wide,       301, 5000},
    {"stress_wide_1000",          t_wide,      1000, 5000},
    {"stress_wide_rows_250",      t_wide_rows,  250, 5000},
    {"stress_wide_rows_300",      t_wide_rows,  300, 5000},
    {"stress_pk_list_63",         t_pk_wide,     32, 5000},
    {"stress_pk_list_70",         t_pk_wide,     70, 5000},
    {"stress_fk_list_70",         t_fk_wide,     70, 5000},
    {"stress_deep_parens_5000",   t_deep_balanced, 5000, 5000},
    {"stress_open_parens_5000",   t_deep_open,     5000, 5000},
    {"stress_long_identifier_1MB",t_long_ident, 1000000, 5000},
    {"stress_long_default_100k",  t_long_default, 100000, 15000},
};
#define NCASES (int)(sizeof CASES / sizeof *CASES)

int main(int argc, char **argv)
{
    g_filter = argc > 1 ? argv[1] : NULL;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    for (int k = 0; k < NCASES; k++)
        run_case(CASES[k].name, CASES[k].fn, CASES[k].arg, CASES[k].timeout_ms);

    for (int k = 0; k < NNC; k++) {
        for (int mode = 0; mode < 2; mode++) {
            char name[160];
            snprintf(name, sizeof name, "nc_%s%s", NC[k].name, mode ? "/1-token-per-row" : "");
            run_case(name, t_nocrash, k * 2 + mode, 1000);
        }
    }

    int ntok;
    tokenize(TRUNC_SRC, &ntok);
    run_suite("suite_truncation", t_trunc, 2, 2 * (ntok - 1) + 1, 300, desc_trunc);

    int fuzz_n = getenv("FUZZ_N") ? atoi(getenv("FUZZ_N")) : 300;
    if (getenv("FUZZ_SEED")) g_fuzz_base = (unsigned)atoi(getenv("FUZZ_SEED"));
    run_suite("suite_fuzz_terminated",   t_fuzz_term,   0, fuzz_n - 1, 200, desc_fuzz_term);
    run_suite("suite_fuzz_unterminated", t_fuzz_unterm, 0, fuzz_n - 1, 200, desc_fuzz_unterm);

    int bad = g_cnt[ST_FAIL] + g_cnt[ST_HANG] + g_cnt[ST_CRASH];
    printf("\nTOTAL: pass %d | warn %d | fail %d | hang %d | crash %d\n",
           g_cnt[ST_PASS], g_cnt[ST_WARN], g_cnt[ST_FAIL], g_cnt[ST_HANG], g_cnt[ST_CRASH]);
    return bad ? 1 : 0;
}

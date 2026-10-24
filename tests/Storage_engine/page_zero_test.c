#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define _POSIX_C_SOURCE 200809L
#include "Header_page.c"

void write_pagezero(pagezero *pg, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        return;
    }
    fwrite(pg, sizeof(pagezero), 1, fp);
    fclose(fp);
}

void read_pagezero(pagezero *pg, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        memset(pg, 0, sizeof(pagezero));
        return;
    }
    size_t got = fread(pg, sizeof(pagezero), 1, fp);
    fclose(fp);
    if (got != 1) {
        memset(pg, 0, sizeof(pagezero));
    }
}

static ctree *g_root;
ctree *create_parser() { return g_root; }

static ctree *mk(const char *comp, int type) {
    ctree *n = calloc(1, sizeof(ctree));
    n->comp = strdup(comp);
    n->type = type;
    n->children = calloc(32, sizeof(ctree *));
    return n;
}
static void add(ctree *p, ctree *c) { p->children[p->num++] = c; }

static ctree *begin_table(const char *name, int type) {
    ctree *r = mk("ROOT", -1);
    ctree *t = mk(name, type);
    add(r, t);
    g_root = r;
    return t;
}
static ctree *col(ctree *t, const char *name, int type) {
    ctree *c = mk(name, type);
    add(t, c);
    return c;
}

static pagezero PG;
static pagezero PG2;

static int fails;
#define CHECK(c) do { if (!(c)) { fails++; if (fails <= 5) printf("      FAIL line %d: %s\n", __LINE__, #c); } } while (0)
#define BEGIN() do { fails = 0; } while (0)

static void rname(char *b, int len) {
    b[0] = 'c';
    for (int i = 1; i < len; i++) b[i] = 'a' + rand() % 26;
    b[len] = 0;
}

static void find_colliding_pair(char *a, char *b) {
    char seen[307][16]; memset(seen, 0, sizeof(seen));
    a[0] = b[0] = 0;
    for (int i = 0; i < 5000; i++) {
        char nm[16]; snprintf(nm, 16, "col%d", i);
        int h = hash_num(nm);
        if (seen[h][0]) { strcpy(a, seen[h]); strcpy(b, nm); return; }
        strcpy(seen[h], nm);
    }
}

static char gnames[16][16];
static int  gnn[16], gun[16], gpk;
static int  gtype;
static void build_random_valid(int n) {
    for (int i = 0; i < n; i++) {
        int ok;
        do {
            rname(gnames[i], 3 + rand() % 8);
            ok = 1;
            for (int j = 0; j < i; j++)
                if (strcmp(gnames[j], gnames[i]) == 0) ok = 0;
        } while (!ok);
    }
    gtype = INT + rand() % (TIMESTAMP - INT + 1);
    ctree *t = begin_table("tbl", gtype);
    for (int i = 0; i < n; i++) {
        gnn[i] = rand() % 2;
        gun[i] = rand() % 2;
        ctree *c = col(t, gnames[i], gtype);
        c->not_null = gnn[i];
        c->unique   = gun[i];
    }
    gpk = -1;
    if (rand() % 2) {
        gpk = rand() % n;
        ctree *p = mk("PRIMARY KEY", -1);
        add(p, mk(gnames[gpk], -1));
        add(t, p);
    }
}

static int t_defaults(void) {
    BEGIN();
    column_info c; default_column_info(&c);
    CHECK(strcmp(c.col_name, "none") == 0);
    CHECK(!c.primary_key && !c.foreign_key && !c.not_null && !c.unique && !c.default_stuff);
    default_pagezero(&PG);
    CHECK(PG.db_magic == 0xF1ACDB);
    CHECK(PG.version == 1);
    CHECK(PG.pg_size == 4096);
    CHECK(PG.num_tables == 0);
    CHECK(strcmp(PG.tables[0].table_name, "none") == 0);
    CHECK(strcmp(PG.tables[MAX_TABLES - 1].columns[MAX_COLUMNS - 1].col_name, "none") == 0);
    return fails;
}

static int t_hash(void) {
    BEGIN();
    CHECK(hash_num("") == 162);
    CHECK(hash_num("id") == hash_num("id"));
    char b[16];
    for (int i = 0; i < 20000; i++) {
        rname(b, 1 + rand() % 12);
        int h = hash_num(b);
        CHECK(h >= 0 && h < 307);
    }
    return fails;
}

static int t_simple(void) {
    BEGIN();
    ctree *t = begin_table("users", INT);
    col(t, "id", INT)->not_null = true;
    col(t, "email", INT)->unique = true;
    col(t, "age", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(strcmp(PG.tables[0].table_name, "users") == 0);
    CHECK(PG.tables[0].num_columns == 3);
    CHECK(strcmp(PG.tables[0].columns[0].col_name, "id") == 0);
    CHECK(strcmp(PG.tables[0].columns[1].col_name, "email") == 0);
    CHECK(strcmp(PG.tables[0].columns[2].col_name, "age") == 0);
    CHECK(PG.tables[0].columns[0].not_null && !PG.tables[0].columns[1].not_null);
    CHECK(PG.tables[0].columns[1].unique && !PG.tables[0].columns[0].unique);
    CHECK(PG.tables[0].columns[0].col_type == INT);
    return fails;
}

static int t_pk_node(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    col(t, "id", INT);
    col(t, "name", INT);
    ctree *p = mk("PRIMARY KEY", -1);
    add(p, mk("id", -1));
    add(t, p);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 2);
    CHECK(PG.tables[0].columns[0].primary_key);
    CHECK(!PG.tables[0].columns[1].primary_key);
    return fails;
}

static int t_pk_inline(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    col(t, "id", INT)->primary_key_counter = 1;
    col(t, "name", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].columns[0].primary_key);
    CHECK(!PG.tables[0].columns[1].primary_key);
    return fails;
}

static int t_pk_hash_bucket(void) {
    BEGIN();
    char a[16], b[16];
    find_colliding_pair(a, b);
    CHECK(a[0] != 0);
    ctree *t = begin_table("t", INT);
    col(t, a, INT);
    col(t, b, INT);
    ctree *p = mk("PRIMARY KEY", -1);
    add(p, mk(a, -1));
    add(t, p);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 2);
    CHECK(PG.tables[0].columns[0].primary_key);
    CHECK(!PG.tables[0].columns[1].primary_key);
    return fails;
}

static ctree *make_fk(const char *colname, const char *reftable, const char *refcol) {
    ctree *f = mk("FOREIGN KEY", -1);
    f->foreign_key_counter = 1;
    f->foreign_key = calloc(4, sizeof(char *));
    f->foreign_key[0] = (char *)colname;
    ctree *ref = mk(reftable, -1);
    add(ref, mk(refcol, -1));
    add(f, ref);
    return f;
}

static int t_fk(void) {
    BEGIN();
    ctree *t = begin_table("orders", INT);
    col(t, "id", INT);
    col(t, "user_id", INT);
    add(t, make_fk("user_id", "users", "id"));
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 2);
    column_info *c = &PG.tables[0].columns[1];
    CHECK(strcmp(c->col_name, "user_id") == 0);
    CHECK(c->foreign_key);
    CHECK(strcmp(c->foreign_key_table, "users") == 0);
    CHECK(strcmp(c->foreign_key_column, "id") == 0);
    CHECK(!PG.tables[0].columns[0].foreign_key);
    return fails;
}

static int t_fk_two(void) {
    BEGIN();
    ctree *t = begin_table("orders", INT);
    col(t, "id", INT);
    col(t, "user_id", INT);
    col(t, "item_id", INT);
    add(t, make_fk("user_id", "users", "id"));
    add(t, make_fk("item_id", "items", "id"));
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 3);
    CHECK(PG.tables[0].columns[1].foreign_key && PG.tables[0].columns[2].foreign_key);
    CHECK(strcmp(PG.tables[0].columns[1].foreign_key_table, "users") == 0);
    CHECK(strcmp(PG.tables[0].columns[2].foreign_key_table, "items") == 0);
    return fails;
}

static int t_fk_creates_column(void) {
    BEGIN();
    ctree *t = begin_table("orders", INT);
    col(t, "id", INT);
    add(t, make_fk("ghost", "users", "id"));
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 2);
    CHECK(strcmp(PG.tables[0].columns[1].col_name, "ghost") == 0);
    CHECK(PG.tables[0].columns[1].foreign_key);
    return fails;
}

static int t_dup_column(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    col(t, "id", INT);
    col(t, "id", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    return fails;
}

static int t_type_missing_rejected(void) {
    BEGIN();
    ctree *t = begin_table("t", -1);
    col(t, "id", -1);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    return fails;
}

static int t_null_tree(void) {
    BEGIN();
    g_root = NULL;
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    CHECK(PG.db_magic == 0xF1ACDB);
    g_root = mk("ROOT", -1);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    return fails;
}

static int t_per_column_type(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    col(t, "a", VARCHAR);
    col(t, "b", BIGINT);
    ctree *d = col(t, "c", DECIMAL);
    d->first_decimal = 10; d->second_decimal = 2;
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].columns[0].col_type == VARCHAR);
    CHECK(PG.tables[0].columns[1].col_type == BIGINT);
    CHECK(PG.tables[0].columns[2].col_type == DECIMAL);
    CHECK(PG.tables[0].columns[2].first_decimal == 10);
    CHECK(PG.tables[0].columns[2].second_decimal == 2);
    return fails;
}

static int t_type_only_on_columns(void) {
    BEGIN();
    ctree *t = begin_table("t", -1);
    col(t, "a", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].columns[0].col_type == INT);
    return fails;
}

static int t_default_on_column(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    ctree *c = col(t, "a", INT);
    c->default_val = "42";
    col(t, "b", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].columns[0].default_stuff);
    CHECK(strcmp(PG.tables[0].columns[0].default_val, "42") == 0);
    CHECK(!PG.tables[0].columns[1].default_stuff);
    return fails;
}

static int t_default_plus_unique(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    ctree *c = col(t, "a", INT);
    c->default_val = "42";
    c->unique = true;
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    return fails;
}

static int t_exactly_16(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    for (int i = 0; i < 16; i++) { char nm[16]; snprintf(nm, sizeof(nm), "c%02d", i); col(t, nm, INT); }
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 16);
    CHECK(strcmp(PG.tables[0].columns[15].col_name, "c15") == 0);
    return fails;
}

static int t_too_many_columns(void) {
    BEGIN();
    ctree *t = begin_table("t", INT);
    for (int i = 0; i < 17; i++) { char nm[16]; snprintf(nm, sizeof(nm), "c%02d", i); col(t, nm, INT); }
    main_create_parser(&PG);
    CHECK(PG.num_tables == 0);
    return fails;
}

static int t_second_call_resets(void) {
    BEGIN();
    ctree *t = begin_table("first", INT);
    col(t, "a", INT);
    main_create_parser(&PG);
    t = begin_table("second", INT);
    col(t, "b", INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(strcmp(PG.tables[0].table_name, "second") == 0);
    return fails;
}

static int t_roundtrip(void) {
    BEGIN();
    ctree *t = begin_table("users", INT);
    col(t, "id", INT)->not_null = true;
    col(t, "name", INT)->unique = true;
    main_create_parser(&PG);
    write_pagezero(&PG, "/tmp/pz_test.db");
    FILE *fp = fopen("/tmp/pz_test.db", "rb");
    CHECK(fp != NULL);
    if (fp) { fseek(fp, 0, SEEK_END); CHECK((size_t)ftell(fp) == sizeof(pagezero)); fclose(fp); }
    read_pagezero(&PG2, "/tmp/pz_test.db");
    CHECK(memcmp(&PG, &PG2, sizeof(pagezero)) == 0);
    CHECK(PG2.db_magic == 0xF1ACDB);
    CHECK(strcmp(PG2.tables[0].table_name, "users") == 0);
    CHECK(PG2.tables[0].num_columns == 2);
    return fails;
}

static int t_read_bad_files(void) {
    BEGIN();
    read_pagezero(&PG, "/tmp/definitely_not_here.db");
    CHECK(PG.db_magic != 0xF1ACDB);
    FILE *fp = fopen("/tmp/pz_short.db", "wb");
    fwrite("abc", 1, 3, fp);
    fclose(fp);
    read_pagezero(&PG, "/tmp/pz_short.db");
    CHECK(PG.db_magic != 0xF1ACDB);
    return fails;
}

static int t_stress_valid(void) {
    BEGIN();
    srand(12345);
    for (int it = 0; it < 3000; it++) {
        int n = 1 + rand() % 16;
        build_random_valid(n);
        main_create_parser(&PG);
        int ok = (PG.num_tables == 1 && PG.tables[0].num_columns == (uint32_t)n);
        CHECK(ok);
        if (!ok) continue;
        for (int i = 0; i < n; i++) {
            column_info *c = &PG.tables[0].columns[i];
            int good = strcmp(c->col_name, gnames[i]) == 0
                    && c->not_null == (bool)gnn[i]
                    && c->unique == (bool)gun[i]
                    && c->primary_key == (i == gpk)
                    && c->col_type == gtype;
            CHECK(good);
        }
    }
    return fails;
}

static int t_stress_roundtrip(void) {
    BEGIN();
    srand(777);
    for (int it = 0; it < 300; it++) {
        build_random_valid(1 + rand() % 16);
        main_create_parser(&PG);
        write_pagezero(&PG, "/tmp/pz_stress.db");
        read_pagezero(&PG2, "/tmp/pz_stress.db");
        CHECK(memcmp(&PG, &PG2, sizeof(pagezero)) == 0);
    }
    return fails;
}

static int t_stress_collision_logic(void) {
    BEGIN();
    srand(4242);
    int with_collision = 0, total = 5000;
    for (int it = 0; it < total; it++) {
        int n = 2 + rand() % 15;
        char names[16][16];
        ctree *t = begin_table("t", INT);
        for (int i = 0; i < n; i++) {
            int dup;
            do {
                rname(names[i], 3 + rand() % 8);
                dup = 0;
                for (int j = 0; j < i; j++) if (strcmp(names[i], names[j]) == 0) dup = 1;
            } while (dup);
            col(t, names[i], INT);
        }
        int collide = 0;
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (hash_num(names[i]) == hash_num(names[j])) collide = 1;
        main_create_parser(&PG);
        CHECK(PG.num_tables == 1 && PG.tables[0].num_columns == (uint32_t)n);
        if (collide) with_collision++;
    }
    printf("      info: %d / %d tables had two names in the same hash bucket, all must be accepted\n", with_collision, total);
    return fails;
}

static int t_collision_valid_schema(void) {
    BEGIN();
    char a[16], b[16];
    find_colliding_pair(a, b);
    CHECK(a[0] != 0);
    ctree *t = begin_table("t", INT);
    col(t, a, INT);
    col(t, b, INT);
    main_create_parser(&PG);
    CHECK(PG.num_tables == 1);
    CHECK(PG.tables[0].num_columns == 2);
    return fails;
}

typedef struct { const char *name; int (*fn)(void); int known; } T;
static T tests[] = {
    {"defaults",                          t_defaults, 0},
    {"hash_num range/determinism",        t_hash, 0},
    {"simple table",                      t_simple, 0},
    {"PRIMARY KEY node",                  t_pk_node, 0},
    {"inline primary key",                t_pk_inline, 0},
    {"pk column sharing a hash bucket",   t_pk_hash_bucket, 0},
    {"single FOREIGN KEY",                t_fk, 0},
    {"two FOREIGN KEYs in one table",     t_fk_two, 0},
    {"FOREIGN KEY creates missing column", t_fk_creates_column, 0},
    {"duplicate column rejected",         t_dup_column, 0},
    {"type -1 rejected",                  t_type_missing_rejected, 0},
    {"null / empty parser result",        t_null_tree, 0},
    {"per-column type + decimals",        t_per_column_type, 0},
    {"type only on column nodes",         t_type_only_on_columns, 0},
    {"default_val on column",             t_default_on_column, 0},
    {"default + unique rejected",         t_default_plus_unique, 0},
    {"exactly 16 columns accepted",       t_exactly_16, 0},
    {"17 columns rejected cleanly",       t_too_many_columns, 0},
    {"second call starts fresh pagezero", t_second_call_resets, 0},
    {"write/read roundtrip",              t_roundtrip, 0},
    {"read missing/short file",           t_read_bad_files, 0},
    {"collision on valid schema accepted", t_collision_valid_schema, 0},
    {"STRESS 3000 random valid tables",   t_stress_valid, 0},
    {"STRESS 300 write/read roundtrips",  t_stress_roundtrip, 0},
    {"STRESS 5000 tables with bucket collisions", t_stress_collision_logic, 0},
};

int main(void) {
    int n = sizeof(tests) / sizeof(tests[0]);
    int pass = 0, failed = 0, known = 0, fixed = 0;
    printf("sizeof(pagezero) = %zu bytes, sizeof(table_info) = %zu, sizeof(column_info) = %zu\n\n",
           sizeof(pagezero), sizeof(table_info), sizeof(column_info));
    for (int i = 0; i < n; i++) {
        fflush(stdout);
        char log[64]; snprintf(log, sizeof(log), "/tmp/pz_err_%d.log", i);
        pid_t pid = fork();
        if (pid == 0) {
            alarm(60);
            int fd = open(log, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, 2);
            srand(1);
            int r = tests[i].fn();
            fflush(stdout);
            _exit(r ? 1 : 0);
        }
        int st; waitpid(pid, &st, 0);
        int ok = WIFEXITED(st) && WEXITSTATUS(st) == 0;
        const char *how = ok ? "" : (WIFSIGNALED(st) ? " (crashed)" : "");
        const char *label;
        if (ok && !tests[i].known)      { label = "PASS ";  pass++; }
        else if (ok && tests[i].known)  { label = "FIXED";  fixed++; }
        else if (tests[i].known)        { label = "KNOWN";  known++; }
        else                            { label = "FAIL ";  failed++; }
        printf("[%s] %s%s\n", label, tests[i].name, how);
        if (!ok) {
            FILE *lf = fopen(log, "r");
            char line[256];
            if (lf) {
                while (fgets(line, sizeof(line), lf))
                    if (strstr(line, "ERROR") || strstr(line, "runtime error")) { printf("      sanitizer: %s", line); break; }
                fclose(lf);
            }
        }
    }
    printf("\n%d passed, %d failed, %d known issues still open, %d known issues now fixed\n", pass, failed, known, fixed);
    return failed ? 1 : 0;
}

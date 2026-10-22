#include<wal.h>
#include<wal.c>

static int g_pass = 0, g_fail = 0;
#define CK(c) do{ if(!(c)){ g_fail++; if (g_fail <= 30) printf("  FAIL line %d: %s\n", __LINE__, #c); } else g_pass++; }while(0)

typedef struct {
    int type, is_clr, lsn, prev, undo_next, txn, len, crc_ok, plen, ulen;
    long long off;
    uint32_t dc_a, dc_b;
} rinfo;

#define MAXREC 100000
static rinfo L[MAXREC];

static void reset(void){
    if (dat) free(dat);
    dat = NULL;
    if (system("rm -rf wal") != 0) { }
    start_the_wal();
    g_cp_calls = 0;
    g_cp_frees = 1;
}

static int scan_all(rinfo *out, int max){
    int n = 0;
    records_struct *r = malloc(sizeof *r);
    for (int seg = 0; seg <= dat->current_segment; seg++){
        int fd = create_segment_file(seg);
        segment_struct s;
        if (pread(fd, &s, sizeof s, 0) != (ssize_t)sizeof s){ close(fd); continue; }
        long long pos = sizeof(segment_struct);
        while (pos < s.size && n < max){
            if (pread(fd, r, records_sturct_size, pos) != (ssize_t)records_sturct_size) break;
            int len = r->header.lenght;
            if (len < 0 || len > 2 * max_size_for_data + 4) break;
            size_t total = records_sturct_size + len;
            if (pread(fd, r, total, pos) != (ssize_t)total) break;
            rinfo *x = &out[n++];
            memset(x, 0, sizeof *x);
            x->type = r->header.type;
            x->is_clr = r->header.is_clr;
            x->lsn = r->header.lsn;
            x->prev = r->header.prev;
            x->undo_next = r->header.undo_next_lsn;
            x->txn = r->header.transaction;
            x->len = len;
            x->off = pos + (long long)seg * default_segment_size;
            x->crc_ok = (r->crc == crc32c((char *)r, 4, total - 4));
            if (x->type == 1 || x->type == 2 || x->type == 7 || x->type == 8){
                x->dc_a = crc32c(r->record_value.insert.inserted_data, 0, len);
            } else if (x->type == 3 || x->type == 9){
                int pl = r->record_value.update.prev_lenght;
                int ul = r->record_value.update.updated_lenght;
                if (pl >= 0 && ul >= 0 && pl + ul <= 2 * max_size_for_data){
                    x->plen = pl; x->ulen = ul;
                    x->dc_a = crc32c(r->record_value.update.data, 0, pl);
                    x->dc_b = crc32c(r->record_value.update.data, pl, ul);
                }
            }
            pos += total;
        }
        close(fd);
    }
    free(r);
    return n;
}

static int all_crc_ok(int n){
    for (int i = 0; i < n; i++) if (!L[i].crc_ok) return 0;
    return 1;
}

static void fill_segment(int seg){
    int fd = create_segment_file(seg);
    segment_struct s;
    if (pread(fd, &s, sizeof s, 0) != (ssize_t)sizeof s) default_segment(&s, seg);
    s.size = default_segment_size - 10;
    s.status = 1;
    write_header(fd, s);
    close(fd);
}

#define H(s) crc32c(s, 0, strlen(s))

static void t_unknown(void){
    puts("T1 unknown txn");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "x", 1);
    int n0 = scan_all(L, MAXREC);
    int before = dat->num_of_run_transactions;
    abort_transaction(999);
    CK(scan_all(L, MAXREC) == n0);
    CK(dat->num_of_run_transactions == before);
}

static void t_empty(void){
    puts("T2 abort empty txn");
    reset();
    int t = begin_transaction();
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 1);
    CK(L[0].type == 10 && L[0].prev == -1);
    CK(find_slot(t) == -1);
    CK(dat->num_of_run_transactions == 0);
}

static void t_single(void){
    puts("T3 single insert");
    reset();
    int t = begin_transaction();
    wal_insert(t, 7, 100, "hello", 5);
    int ins = dat->running_last_lsn[find_slot(t)];
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 3);
    CK(L[0].type == 1 && L[1].type == 8 && L[2].type == 10);
    CK(L[1].is_clr == 1 && L[1].undo_next == -1 && L[1].prev == ins && L[1].txn == t);
    CK(L[1].len == 5 && L[1].dc_a == H("hello"));
    CK(L[2].prev == L[1].lsn);
    CK(all_crc_ok(n));
    CK(find_slot(t) == -1 && dat->num_of_run_transactions == 0);
}

static void t_mixed(void){
    puts("T4 insert + update + delete");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "AAAA", 4);
    int sl = find_slot(t);
    int l_ins = dat->running_last_lsn[sl];
    wal_update(t, 1, 0, "AAAA", 4, "BBBBBB", 6);
    int l_upd = dat->running_last_lsn[sl];
    wal_delete(t, 1, 10, "CC", 2);
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 7);
    int want[7] = {1, 3, 2, 7, 9, 8, 10};
    for (int i = 0; i < 7 && i < n; i++) CK(L[i].type == want[i]);
    CK(L[3].dc_a == H("CC") && L[3].undo_next == l_upd);
    CK(L[4].plen == 6 && L[4].ulen == 4 && L[4].dc_a == H("BBBBBB") && L[4].dc_b == H("AAAA"));
    CK(L[4].len == 14 && L[4].undo_next == l_ins);
    CK(L[5].dc_a == H("AAAA") && L[5].undo_next == -1);
    CK(L[4].prev == L[3].lsn && L[5].prev == L[4].lsn && L[6].prev == L[5].lsn);
    CK(all_crc_ok(n));
    CK(find_slot(t) == -1);
}

static void t_resume(void){
    puts("T5 resume after partial abort (CLR skip)");
    reset();
    int t = begin_transaction();
    int sl = find_slot(t);
    wal_insert(t, 1, 0, "aa", 2);
    int lsnA = dat->running_last_lsn[sl];
    wal_insert(t, 1, 5, "bb", 2);
    int lsnB = dat->running_last_lsn[sl];
    long long offB = dat->previous_offset[sl];
    wal_insert(t, 1, 9, "cc", 2);
    int lsnC = dat->running_last_lsn[sl];
    long long offC = dat->previous_offset[sl];
    records_struct r;
    default_record(&r, 8);
    r.header.transaction = t;
    r.header.lenght = 2;
    r.header.is_clr = 1;
    r.header.prev = lsnC;
    r.header.prev_offset = offC;
    r.header.undo_next_lsn = lsnB;
    r.header.undo_next_offset = offB;
    r.record_value.delete.page_id = 1;
    r.record_value.delete.offset = 9;
    r.record_value.delete.lenght = 2;
    memcpy(r.record_value.delete.deleted_data, "cc", 2);
    append_record(sl, &r, records_sturct_size + 2);
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 7);
    int want[7] = {1, 1, 1, 8, 8, 8, 10};
    for (int i = 0; i < 7 && i < n; i++) CK(L[i].type == want[i]);
    CK(L[4].dc_a == H("bb") && L[4].undo_next == lsnA && L[4].prev == L[3].lsn);
    CK(L[5].dc_a == H("aa") && L[5].undo_next == -1);
    CK(all_crc_ok(n));
    CK(find_slot(t) == -1);
}

static void t_seg_switch(void){
    puts("T6 abort when segment is full");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "zz", 2);
    fill_segment(0);
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 3);
    CK(dat->current_segment == 1);
    CK(L[0].off / default_segment_size == 0);
    CK(L[1].type == 8 && L[1].off / default_segment_size == 1);
    CK(L[2].type == 10 && L[2].off / default_segment_size == 1);
    CK(all_crc_ok(n));
}

static void t_cross_seg_decode(void){
    puts("T7 undo records living in two segments");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "AAAA", 4);
    fill_segment(0);
    wal_insert(t, 1, 4, "BBBB", 4);
    CK(dat->current_segment == 1);
    abort_transaction(t);
    int n = scan_all(L, MAXREC);
    CK(n == 5);
    CK(L[0].type == 1 && L[1].type == 1 && L[2].type == 8 && L[3].type == 8 && L[4].type == 10);
    CK(L[1].off / default_segment_size == 1);
    CK(L[2].dc_a == H("BBBB") && L[3].dc_a == H("AAAA"));
    CK(L[3].undo_next == -1);
    CK(all_crc_ok(n));
}

static void t_commit(void){
    puts("T8 commit + abort-after-commit is a no-op");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "hi", 2);
    int ins = dat->running_last_lsn[find_slot(t)];
    commit(t);
    int n = scan_all(L, MAXREC);
    CK(n == 3);
    CK(L[0].type == 1 && L[1].type == 5 && L[2].type == 10);
    CK(L[1].prev == ins);
    CK(L[2].prev == L[1].lsn);
    CK(all_crc_ok(n));
    CK(find_slot(t) == -1 && dat->num_of_run_transactions == 0);
    abort_transaction(t);
    CK(scan_all(L, MAXREC) == 3);
}

static void t_commit_retry(void){
    puts("T9 commit: log blocked, checkpoint frees it");
    reset();
    int t = begin_transaction();
    wal_insert(t, 1, 0, "q", 1);
    fill_segment(0);
    dat->last_lsn_in_segment[1] = 1000;
    dat->the_farthest_lsn_required = 500;
    commit(t);
    int n = scan_all(L, MAXREC);
    CK(g_cp_calls == 1);
    CK(n == 3);
    CK(L[1].type == 5 && L[1].off / default_segment_size == 1);
    CK(find_slot(t) == -1);
    CK(dat->retrying == 0);
}

static void t_commit_blocked(void){
    puts("T10 commit: log blocked, checkpoint cannot free (no infinite loop)");
    reset();
    g_cp_frees = 0;
    int t = begin_transaction();
    wal_insert(t, 1, 0, "q", 1);
    fill_segment(0);
    dat->last_lsn_in_segment[1] = 1000;
    dat->the_farthest_lsn_required = 500;
    commit(t);
    CK(g_cp_calls == 1);
    CK(find_slot(t) != -1);
    CK(dat->retrying == 0);
}

#define MAXTX 20000
#define MAXOPS 16
static struct { int state, nops, target, ty[MAXOPS], la[MAXOPS], lb[MAXOPS]; uint32_t da[MAXOPS], db[MAXOPS]; } M[MAXTX + 2];
static struct { int nop, ncl, commit, end, op[MAXOPS], cl[MAXOPS]; } T[MAXTX + 2];
static char b1[8192], b2[8192];

static void fillbuf(char *b, int n){
    for (int i = 0; i < n; i++) b[i] = (char)('a' + rand() % 26);
}

static void verify(int ntxn){
    int n = scan_all(L, MAXREC);
    printf("  log records: %d, segments used: %d\n", n, dat->current_segment + 1);
    CK(n < MAXREC);
    CK(all_crc_ok(n));
    for (int i = 1; i <= ntxn; i++){ T[i].nop = T[i].ncl = 0; T[i].commit = T[i].end = -1; }
    int stray = 0;
    for (int i = 0; i < n; i++){
        int id = L[i].txn;
        if (id < 1 || id > ntxn){ stray++; continue; }
        int ty = L[i].type;
        if (ty == 1 || ty == 2 || ty == 3){ if (T[id].nop < MAXOPS) T[id].op[T[id].nop++] = i; }
        else if (ty == 7 || ty == 8 || ty == 9){ if (T[id].ncl < MAXOPS) T[id].cl[T[id].ncl++] = i; }
        else if (ty == 5) T[id].commit = i;
        else if (ty == 10) T[id].end = i;
    }
    CK(stray == 0);
    for (int id = 1; id <= ntxn; id++){
        int no = T[id].nop, nc = T[id].ncl;
        CK(no == M[id].nops);
        if (no != M[id].nops) continue;
        for (int j = 0; j < no; j++){
            if (j == 0) CK(L[T[id].op[j]].prev == -1);
            else CK(L[T[id].op[j]].prev == L[T[id].op[j - 1]].lsn);
        }
        int last_op_lsn = no ? L[T[id].op[no - 1]].lsn : -1;
        CK(T[id].end >= 0);
        if (T[id].end < 0) continue;
        if (M[id].state == 1){
            CK(nc == 0 && T[id].commit >= 0);
            if (T[id].commit < 0) continue;
            CK(L[T[id].commit].prev == last_op_lsn);
            CK(L[T[id].end].prev == L[T[id].commit].lsn);
        } else {
            CK(nc == no && T[id].commit < 0);
            if (nc != no) continue;
            for (int i = 0; i < nc; i++){
                int k = no - 1 - i;
                rinfo *c = &L[T[id].cl[i]];
                int want_ty = (M[id].ty[k] == 1) ? 8 : (M[id].ty[k] == 2) ? 7 : 9;
                CK(c->type == want_ty && c->is_clr == 1);
                if (M[id].ty[k] == 3){
                    CK(c->plen == M[id].lb[k] && c->ulen == M[id].la[k]);
                    CK(c->dc_a == M[id].db[k] && c->dc_b == M[id].da[k]);
                    CK(c->len == M[id].la[k] + M[id].lb[k] + 4);
                } else {
                    CK(c->len == M[id].la[k] && c->dc_a == M[id].da[k]);
                }
                CK(c->undo_next == (k > 0 ? L[T[id].op[k - 1]].lsn : -1));
                CK(c->prev == (i == 0 ? last_op_lsn : L[T[id].cl[i - 1]].lsn));
            }
            int tail = nc ? L[T[id].cl[nc - 1]].lsn : last_op_lsn;
            CK(L[T[id].end].prev == tail);
        }
    }
    CK(dat->num_of_run_transactions == 0);
    int busy = 0;
    for (int i = 0; i < 256; i++) if (dat->running_transaction[i] != -1) busy++;
    CK(busy == 0);
}

static void stress(unsigned seed, int ntxn, int maxactive){
    printf("STRESS seed=%u txns=%d concurrent<=%d\n", seed, ntxn, maxactive);
    if (ntxn > MAXTX) ntxn = MAXTX;
    if (maxactive > 200) maxactive = 200;
    reset();
    g_verbose = 0;
    srand(seed);
    memset(M, 0, sizeof M);
    int active[256], na = 0, started = 0, finished = 0;
    while (finished < ntxn){
        if (na < maxactive && started < ntxn && (na == 0 || rand() % 3 == 0)){
            int id = begin_transaction();
            CK(id != -1);
            if (id == -1) break;
            M[id].target = rand() % 7;
            active[na++] = id;
            started++;
            continue;
        }
        int k = rand() % na;
        int id = active[k];
        if (M[id].nops < M[id].target){
            int ty = 1 + rand() % 3, page = rand() % 100, off = rand() % 4000, j = M[id].nops;
            M[id].ty[j] = ty;
            if (ty == 3){
                int pl = 1 + rand() % 2000, nl = 1 + rand() % 2000;
                fillbuf(b1, pl); fillbuf(b2, nl);
                wal_update(id, page, off, b1, pl, b2, nl);
                M[id].la[j] = pl; M[id].lb[j] = nl;
                M[id].da[j] = crc32c(b1, 0, pl); M[id].db[j] = crc32c(b2, 0, nl);
            } else {
                int l = 1 + rand() % 4000;
                fillbuf(b1, l);
                if (ty == 1) wal_insert(id, page, off, b1, l);
                else wal_delete(id, page, off, b1, l);
                M[id].la[j] = l;
                M[id].da[j] = crc32c(b1, 0, l);
            }
            M[id].nops++;
        } else {
            if (rand() % 4 == 0){ abort_transaction(id); M[id].state = 2; }
            else { commit(id); M[id].state = 1; }
            active[k] = active[--na];
            finished++;
        }
    }
    verify(ntxn);
    g_verbose = 1;
}

int main(int argc, char **argv){
    int ntxn = 3000;
    unsigned seed = 1;
    if (argc > 1) ntxn = atoi(argv[1]);
    if (argc > 2) seed = (unsigned)atoi(argv[2]);
    dat = NULL;

    t_unknown();
    t_empty();
    t_single();
    t_mixed();
    t_resume();
    t_seg_switch();
    t_cross_seg_decode();
    t_commit();
    t_commit_retry();
    t_commit_blocked();
    stress(seed, ntxn, 64);

    printf("\nPASSED %d   FAILED %d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

// test_wal.c  (single file: your wal code + update_wal + tests)
// Build: gcc -g -O1 -fsanitize=address,undefined update_testing.c -o test_wal && ./test_wal [seed]

#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

#define default_segment_size 16777216
#define max_size_for_data 4096

typedef struct frames { int dummy; } frames;

static uint32_t crc_table[256];

typedef struct {
int page_id;
int offset;
int lenght;
char inserted_data[max_size_for_data];
} insert_info;

typedef struct {
int lsn;
int lenght;
int type;
int prev;
int transaction;
int is_clr;
long long prev_offset;
} wal_page_header;

typedef struct {
int page_id;
int offset;
int lenght;
char deleted_data[max_size_for_data];
} delete_info;

typedef struct {
int page_id;
int offset;
int updated_lenght;
int prev_lenght;
char data[2 * max_size_for_data];
} update_info;

typedef struct {
int transactions_number;
int running_transaction[max_size_for_data];
int dirty_page_number;
int dirty_page_id[max_size_for_data];
} checkpoint_struct;

typedef struct {
uint32_t crc;
uint8_t type;
    wal_page_header header;
union {
        insert_info insert;
        update_info update;
        delete_info delete;
        checkpoint_struct checkpoint;
    } record_value;
} records_struct;

#define records_sturct_size offsetof(records_struct, record_value.insert.inserted_data)

typedef struct {
int checkpoint_lsn;
int checkpoint_segment;
int checkpoint_offset;
int redo_lsn;
uint32_t crc;
} control_struct;

typedef struct {
int segment_id;
int status;
int size;
int records_offset;
int first_lsn;
int seq;
} segment_struct;

typedef struct {
int segment_num;
int transaction_num;
int lsn_num;
int num_of_run_transactions;
int running_transaction[256];
int num_of_offset;
int offset[256];
int current_segment;
int redo_checkpoint;
int seq;
int checkpoint_running;
int last_lsn_in_segment[256];
int running_first_lsn[256];
int running_last_lsn[256];
int the_farthest_lsn_required;
long long previous_offset[256];
int dead_segments[max_size_for_data];
    frames *checkpoint_frames;
int data_fd;
} req_data;

void checkpoint(void);
req_data *dat;

int wal_folder(const char *path) {
if (mkdir(path, 0755) == 0) {
return 0;
    }
if (errno == EEXIST) {
return 0;
    }
return -1;
}

void crc_init(void) {
for (uint32_t i = 0; i < 256; i++) {
uint32_t c = i;
for (int k = 0; k < 8; k++) {
            c = (c >> 1) ^ (0x82F63B78 & -(c & 1));
        }
        crc_table[i] = c;
    }
}

int write_record(int fd, off_t offset, records_struct *record, size_t size) {
ssize_t n = pwrite(fd, record, size, offset);
if (n == (ssize_t)size) {
return 0;
    }
return -1;
}

int write_header(int fd, segment_struct header) {
ssize_t n = pwrite(fd, &header, sizeof header, 0);
if (n == (ssize_t)sizeof header) {
return 0;
    }
return -1;
}

int create_segment_file(int seg) {
char name[64];
    snprintf(name, sizeof(name), "wal/wal_%08llu", (unsigned long long)seg);
return open(name, O_CREAT | O_RDWR, 0644);
}

uint32_t crc32c(const char *data, size_t offset, size_t size) {
const uint8_t *p = (const uint8_t *)data + offset;
uint32_t crc = 0xFFFFFFFF;

while (size > 0) {
        crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
        size--;
    }

return ~crc;
}

void start_the_wal() {
    dat = malloc(sizeof(req_data));
    memset(dat, 0, sizeof(req_data));

    dat->num_of_run_transactions = 0;
    dat->lsn_num = 0;
    dat->segment_num = 0;
    dat->transaction_num = 0;

for (int i = 0; i < max_size_for_data; i++) {
        dat->dead_segments[i] = -1;
    }

for (int i = 0; i < 256; i++) {
        dat->running_transaction[i] = -1;
        dat->running_first_lsn[i] = -1;
        dat->running_last_lsn[i] = -1;
        dat->previous_offset[i] = -1;
        dat->last_lsn_in_segment[i] = -1;
    }

    wal_folder("wal");
    crc_init();
}

int begin_transaction(void) {
for (int i = 0; i < 256; i++) {
if (dat->running_transaction[i] == -1) {
            dat->transaction_num++ ;
int id = dat->transaction_num;
            dat->running_transaction[i] = id;
            dat->running_first_lsn[i] = -1;
            dat->running_last_lsn[i] = -1;
            dat->previous_offset[i] = -1;
            dat->num_of_run_transactions++;

return id;
        }
    }
return -1;
}

void default_segment(segment_struct *seg_str, int seg_num) {
    seg_str->segment_id = seg_num;
    seg_str->status = 0;
    seg_str->size = sizeof(segment_struct);
    seg_str->records_offset = 0;
    seg_str->first_lsn = 0;
    seg_str->seq = 0;
}

void default_record(records_struct *record, int type) {
    memset(record, 0, sizeof(records_struct));
    record->crc = 0;
    record->type = type;
    record->header.lsn = dat->lsn_num++;
    record->header.lenght = 0;
    record->header.type = type;
    record->header.prev = -1;
    record->header.prev_offset = -1;
    record->header.is_clr = 0;
    record->header.transaction = dat->transaction_num;
if (type == 1) {
        record->record_value.insert.page_id = -1;
        record->record_value.insert.offset = 0;
        record->record_value.insert.lenght = 0;
    }
else if (type == 3) {
        record->record_value.update.page_id = -1;
        record->record_value.update.offset = 0;
        record->record_value.update.updated_lenght = 0;
        record->record_value.update.prev_lenght = 0;
    }
else if (type == 2) {
        record->record_value.delete.page_id = -1;
        record->record_value.delete.offset = 0;
        record->record_value.delete.lenght = 0;
    }
else if (type == 6) {
        record->record_value.checkpoint.transactions_number = 0;
        record->record_value.checkpoint.dirty_page_number = 0;
    }
}

/* ===================== update_wal (your new code) ===================== */

void update_wal(int transaction_id , int page_id , int offset , char *prev_data , int prev_len , char *updated_data , int updated_len ){
    if (transaction_id < 0) {
        return ;
    }
    int len = prev_len ;
    if (updated_len > prev_len ){
        len = updated_len ;
    }
    int prev_putted = 0 ;
    int updated_putted = 0 ;
    records_struct record ;
    int declration = 2 ;
    int slot = -1 ;
    int retries = 0 ;
    for (int i = 0 ; i < 256 ; i++){
        if (dat->running_transaction[i] == transaction_id){
            slot = i ;
            break ;
        }
    }
    if (slot == -1){
        return ;
    }
    int prev_lsn = dat->running_last_lsn[slot] ;
    long long prev_file_offset = dat->previous_offset[slot] ;
    size_t upd_hdr = offsetof(records_struct, record_value.update.data) ;   // CHANGED
    int record_size = upd_hdr + 2 * max_size_for_data ;

    while ( len > 0 ){
        int seg_num = dat->current_segment ;
        int file_num ;

        file_num = create_segment_file(seg_num) ;
        if (file_num == -1  ){
            return  ;
        }
        segment_struct seg_str ;
        int size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);
        if (size_of_the_thing != sizeof(seg_str) ){
            default_segment(&seg_str , seg_num ) ;
            seg_str.first_lsn = dat->lsn_num ;
        }

        if (default_segment_size - seg_str.size < record_size){
            close(file_num) ;
            int next_seg_num = (seg_num + 1) % 256 ;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required){
                retries++ ;
                if (retries > 3) {
                    return ;
                }
                checkpoint() ;
                return ;
            }
            dat->current_segment = next_seg_num ;
            seg_num = next_seg_num  ;
            file_num = create_segment_file(seg_num) ;
            if (file_num == -1){
                return ;
            }
            ftruncate(file_num , 0) ;
            default_segment(&seg_str , seg_num) ;
            seg_str.seq = ++dat->seq ;
            seg_str.first_lsn = dat->lsn_num ;
            seg_str.records_offset = 0 ;
            dat->offset[seg_num] = seg_str.size ;
        }
        seg_str.status = 1 ;

        int step = max_size_for_data ;
        if (len < max_size_for_data){
            step = len ;
        }
        int prev_chunk = prev_len - prev_putted ;
        if (prev_chunk > step){
            prev_chunk = step ;
        }
        int updated_chunk = updated_len - updated_putted ;
        if (updated_chunk > step){
            updated_chunk = step ;
        }
        int data_we_put = upd_hdr + prev_chunk + updated_chunk ;
        default_record( &record , 3 ) ;
        record.header.transaction = transaction_id ;
        memcpy(record.record_value.update.data , prev_data + prev_putted , prev_chunk );
        memcpy(record.record_value.update.data + prev_chunk , updated_data + updated_putted , updated_chunk );
        prev_putted = prev_putted + prev_chunk ;
        updated_putted = updated_putted + updated_chunk ;
        record.header.lenght = prev_chunk + updated_chunk ;
        record.record_value.update.offset = offset ;
        record.record_value.update.page_id = page_id ;
        record.record_value.update.prev_lenght = prev_chunk ;
        record.record_value.update.updated_lenght = updated_chunk ;
        record.header.prev = prev_lsn ;
        record.header.prev_offset = prev_file_offset ;
        record.crc = crc32c((char *)&record , 4 , data_we_put - 4);
        prev_lsn = record.header.lsn ;
        int record_off = seg_str.size ;
        declration = write_record(file_num , record_off , &record , data_we_put ) ;
        if (declration == -1  ){
            close(file_num) ;
            return  ;
        }
        fsync(file_num) ;
        seg_str.size = seg_str.size + data_we_put ;
        dat->offset[seg_num] = seg_str.size ;
        dat->last_lsn_in_segment[seg_num] = record.header.lsn ;
        prev_file_offset = (long long)seg_num * default_segment_size + record_off ;
        dat->previous_offset[slot] = prev_file_offset ;
        if (dat->running_first_lsn[slot] == -1) dat->running_first_lsn[slot] = record.header.lsn ;
        dat->running_last_lsn[slot] = record.header.lsn ;
        declration = write_header(file_num , seg_str ) ;
        if ( declration == -1 ){
            close(file_num) ;
            return  ;
        }
        len = len - step ;
        fsync(file_num) ;
        close(file_num) ;
    }
    return ;
}

/* ============================== tests ============================== */

static int ckpt_calls = 0;
void checkpoint(void) { ckpt_calls++; }

static int fails = 0, checks = 0;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)
#define UPD_HDR offsetof(records_struct, record_value.update.data)
#define SEGHDR  ((int)sizeof(segment_struct))

static char base_dir[256];
static uint64_t rng_state = 88172645463325252ULL;

static uint32_t rnd(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return (uint32_t)(rng_state >> 11);
}
static void fill(char *b, int n) { for (int i = 0; i < n; i++) b[i] = (char)rnd(); }

static void fresh(void) {
    if (dat) { free(dat); dat = NULL; }
    if (chdir(base_dir) != 0) { perror("chdir"); exit(2); }
    if (system("rm -rf wal") != 0) { }
    start_the_wal();
    ckpt_calls = 0;
}

static int open_seg(int seg) {
    char n[64];
    snprintf(n, sizeof n, "wal/wal_%08d", seg);
    return open(n, O_RDWR);
}
static int seg_exists(int seg) {
    char n[64];
    snprintf(n, sizeof n, "wal/wal_%08d", seg);
    return access(n, F_OK) == 0;
}
static int read_seg_hdr(int seg, segment_struct *h) {
    int fd = open_seg(seg);
    if (fd < 0) return -1;
    ssize_t n = pread(fd, h, sizeof *h, 0);
    close(fd);
    return n == (ssize_t)sizeof *h ? 0 : -1;
}
static void force_full(int seg) {
    segment_struct h;
    if (read_seg_hdr(seg, &h) != 0) { CHECK(0); return; }
    h.size = default_segment_size - 100;
    int fd = open_seg(seg);
    write_header(fd, h);
    close(fd);
}
static long read_rec_at(long long goff, records_struct *r) {
    int seg = (int)(goff / default_segment_size);
    off_t off = (off_t)(goff % default_segment_size);
    int fd = open_seg(seg);
    if (fd < 0) return -1;
    memset(r, 0, sizeof *r);
    ssize_t n = pread(fd, r, sizeof *r, off);
    close(fd);
    return (long)n;
}
static int crc_ok(records_struct *r) {
    size_t put = UPD_HDR + r->record_value.update.prev_lenght + r->record_value.update.updated_lenght;
    return r->crc == crc32c((char *)r, 4, put - 4);
}
static int find_slot(int tid) {
    for (int i = 0; i < 256; i++) if (dat->running_transaction[i] == tid) return i;
    return -1;
}
static int count_fds(void) {
    int n = 0;
    DIR *d = opendir("/proc/self/fd");
    if (!d) return -1;
    while (readdir(d)) n++;
    closedir(d);
    return n;
}

static void verify_update(int slot, int tid, int lsn_before, int plsn_before, long long poff_before,
                          int page, int off, const char *p, int pl, const char *u, int ul) {
    int len = pl > ul ? pl : ul;
    int nrec = (len + max_size_for_data - 1) / max_size_for_data;
    CHECK(dat->lsn_num - lsn_before == nrec);
    if (len == 0) return;

    records_struct *recs = malloc(sizeof(records_struct) * nrec);
    long long go = dat->previous_offset[slot];
    for (int k = nrec - 1; k >= 0; k--) {
        CHECK(read_rec_at(go, &recs[k]) > 0);
        CHECK(crc_ok(&recs[k]));
        CHECK(recs[k].header.lsn == lsn_before + k);
        CHECK(recs[k].header.transaction == tid);
        CHECK(recs[k].type == 3 && recs[k].header.type == 3);
        CHECK(recs[k].record_value.update.page_id == page);
        CHECK(recs[k].record_value.update.offset == off);
        CHECK(recs[k].record_value.update.prev_lenght <= max_size_for_data);
        CHECK(recs[k].record_value.update.updated_lenght <= max_size_for_data);
        CHECK(recs[k].header.lenght ==
              recs[k].record_value.update.prev_lenght + recs[k].record_value.update.updated_lenght);
        if (k == 0) {
            CHECK(recs[k].header.prev == plsn_before);
            CHECK(recs[k].header.prev_offset == poff_before);
        } else {
            CHECK(recs[k].header.prev == lsn_before + k - 1);
        }
        go = recs[k].header.prev_offset;
    }
    char *rp = malloc(pl + 1), *ru = malloc(ul + 1);
    int po = 0, uo = 0;
    for (int k = 0; k < nrec; k++) {
        int pc = recs[k].record_value.update.prev_lenght;
        int uc = recs[k].record_value.update.updated_lenght;
        if (po + pc > pl || uo + uc > ul) { CHECK(0); break; }
        memcpy(rp + po, recs[k].record_value.update.data, pc);
        memcpy(ru + uo, recs[k].record_value.update.data + pc, uc);
        po += pc;
        uo += uc;
    }
    CHECK(po == pl);
    CHECK(uo == ul);
    CHECK(memcmp(rp, p, pl) == 0);
    CHECK(memcmp(ru, u, ul) == 0);
    CHECK(recs[nrec - 1].header.lsn == dat->lsn_num - 1);
    free(rp); free(ru); free(recs);
}

/* runs one update_wal call and fully verifies it */
static void do_update(int tid, int page, int off, int pl, int ul) {
    int slot = find_slot(tid);
    CHECK(slot >= 0);
    if (slot < 0) return;
    char *p = malloc(pl + 1), *u = malloc(ul + 1);
    fill(p, pl); fill(u, ul);
    int lsn_before = dat->lsn_num;
    int plsn = dat->running_last_lsn[slot];
    long long poff = dat->previous_offset[slot];
    int first_before = dat->running_first_lsn[slot];
    update_wal(tid, page, off, p, pl, u, ul);
    verify_update(slot, tid, lsn_before, plsn, poff, page, off, p, pl, u, ul);
    int len = pl > ul ? pl : ul;
    if (len > 0) {
        CHECK(dat->running_last_lsn[slot] == dat->lsn_num - 1);
        if (first_before == -1) CHECK(dat->running_first_lsn[slot] == lsn_before);
        else CHECK(dat->running_first_lsn[slot] == first_before);
    }
    free(p); free(u);
}

static void test_basic_small(void) {
    printf("basic small\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 7, 100, 10, 20);
    segment_struct h;
    CHECK(read_seg_hdr(0, &h) == 0);
    CHECK(h.status == 1);
    CHECK(h.segment_id == 0);
    CHECK(h.size == SEGHDR + (int)UPD_HDR + 30);
    CHECK(dat->offset[0] == h.size);
    CHECK(dat->last_lsn_in_segment[0] == 0);
    CHECK(dat->running_first_lsn[find_slot(t)] == 0);
}

static void test_boundaries(void) {
    printf("chunk boundaries\n");
    int sizes[] = {1, 2, 4095, 4096, 4097, 8191, 8192, 8193, 12288, 12289, 20000};
    int n = sizeof sizes / sizeof sizes[0];
    for (int i = 0; i < n; i++) {
        fresh();
        int t = begin_transaction();
        do_update(t, 1, 0, sizes[i], sizes[i]);
    }
}

static void test_unequal_lengths(void) {
    printf("unequal lengths\n");
    int cases[][2] = {{0, 5000}, {5000, 0}, {100, 9000}, {9000, 100}, {4096, 1}, {1, 4096},
                      {4097, 4096}, {4096, 4097}, {0, 1}, {1, 0}, {10000, 3}};
    int n = sizeof cases / sizeof cases[0];
    for (int i = 0; i < n; i++) {
        fresh();
        int t = begin_transaction();
        do_update(t, 3, 55, cases[i][0], cases[i][1]);
    }
}

static void test_zero_len(void) {
    printf("zero length\n");
    fresh();
    int t = begin_transaction();
    char b[4] = {1, 2, 3, 4};
    update_wal(t, 1, 0, b, 0, b, 0);
    CHECK(dat->lsn_num == 0);
    CHECK(!seg_exists(0));
    CHECK(dat->running_first_lsn[find_slot(t)] == -1);
}

static void test_invalid_tx(void) {
    printf("invalid transactions\n");
    fresh();
    char b[8] = {0};
    update_wal(-1, 1, 0, b, 8, b, 8);
    update_wal(999, 1, 0, b, 8, b, 8);
    update_wal(0, 1, 0, b, 8, b, 8);
    CHECK(dat->lsn_num == 0);
    CHECK(!seg_exists(0));
}

static void test_chain_same_tx(void) {
    printf("chain same tx\n");
    fresh();
    int t = begin_transaction();
    for (int i = 0; i < 20; i++) do_update(t, i, i * 3, 50 + i, 70 + i);
    int slot = find_slot(t);
    long long go = dat->previous_offset[slot];
    int cnt = 0;
    records_struct *r = malloc(sizeof *r);
    while (go != -1 && cnt < 1000) {
        CHECK(read_rec_at(go, r) > 0);
        CHECK(crc_ok(r));
        CHECK(r->header.transaction == t);
        go = r->header.prev_offset;
        cnt++;
    }
    CHECK(cnt == 20);
    CHECK(dat->running_first_lsn[slot] == 0);
    free(r);
}

static void test_interleaved(void) {
    printf("interleaved transactions\n");
    fresh();
    int a = begin_transaction(), b = begin_transaction(), c = begin_transaction();
    int ids[3] = {a, b, c};
    for (int i = 0; i < 60; i++) {
        int t = ids[rnd() % 3];
        do_update(t, (int)(rnd() % 100), (int)(rnd() % 4000), (int)(rnd() % 6000), (int)(rnd() % 6000));
    }
    for (int k = 0; k < 3; k++) {
        int slot = find_slot(ids[k]);
        long long go = dat->previous_offset[slot];
        records_struct *r = malloc(sizeof *r);
        int guard = 0;
        while (go != -1 && guard++ < 10000) {
            CHECK(read_rec_at(go, r) > 0);
            CHECK(r->header.transaction == ids[k]);
            CHECK(crc_ok(r));
            go = r->header.prev_offset;
        }
        free(r);
    }
}

static void test_rollover(void) {
    printf("segment rollover\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 1, 0, 10, 10);
    force_full(0);
    int seq_before = dat->seq;
    do_update(t, 2, 0, 10, 10);
    CHECK(dat->current_segment == 1);
    CHECK(seg_exists(1));
    CHECK(dat->seq == seq_before + 1);
    segment_struct h;
    CHECK(read_seg_hdr(1, &h) == 0);
    CHECK(h.segment_id == 1);
    CHECK(h.status == 1);
    CHECK(h.seq == dat->seq);
    CHECK(h.size == SEGHDR + (int)UPD_HDR + 20);
    CHECK(h.first_lsn == 1);
    CHECK(dat->last_lsn_in_segment[1] == 1);
    CHECK(dat->last_lsn_in_segment[0] == 0);
    CHECK(ckpt_calls == 0);
    /* chain must still link across segments */
    int slot = find_slot(t);
    records_struct *r = malloc(sizeof *r);
    CHECK(read_rec_at(dat->previous_offset[slot], r) > 0);
    CHECK(r->header.prev == 0);
    CHECK(r->header.prev_offset >= 0 && r->header.prev_offset < default_segment_size);
    free(r);
}

static void test_rollover_mid_update(void) {
    printf("rollover in the middle of a multi chunk update\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 1, 0, 10, 10);
    /* leave room for exactly one full record then force the next one to roll */
    segment_struct h;
    CHECK(read_seg_hdr(0, &h) == 0);
    h.size = default_segment_size - ((int)UPD_HDR + 2 * max_size_for_data);
    int fd = open_seg(0);
    write_header(fd, h);
    close(fd);
    do_update(t, 2, 0, 12000, 12000);
    CHECK(dat->current_segment == 1);
}

static void test_rollover_blocked(void) {
    printf("rollover blocked -> checkpoint\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 1, 0, 10, 10);
    force_full(0);
    dat->last_lsn_in_segment[1] = 5;
    dat->the_farthest_lsn_required = 3;
    int lsn_before = dat->lsn_num;
    char b[16] = {0};
    update_wal(t, 2, 0, b, 16, b, 16);
    CHECK(ckpt_calls == 1);
    CHECK(dat->current_segment == 0);
    CHECK(dat->lsn_num == lsn_before);
    CHECK(!seg_exists(1));
}

static void test_wraparound(void) {
    printf("segment wrap around\n");
    fresh();
    int t = begin_transaction();
    dat->current_segment = 255;
    do_update(t, 1, 0, 10, 10);
    force_full(255);
    do_update(t, 2, 0, 10, 10);
    CHECK(dat->current_segment == 0);
    CHECK(seg_exists(0));
    segment_struct h;
    CHECK(read_seg_hdr(0, &h) == 0);
    CHECK(h.segment_id == 0);
    CHECK(h.status == 1);
}

static void test_crc_detects_corruption(void) {
    printf("crc corruption\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 1, 0, 100, 100);
    int slot = find_slot(t);
    long long go = dat->previous_offset[slot];
    int fd = open_seg(0);
    char byte;
    off_t pos = (off_t)(go % default_segment_size) + (off_t)UPD_HDR + 10;
    CHECK(pread(fd, &byte, 1, pos) == 1);
    byte ^= 0x5A;
    CHECK(pwrite(fd, &byte, 1, pos) == 1);
    close(fd);
    records_struct *r = malloc(sizeof *r);
    CHECK(read_rec_at(go, r) > 0);
    CHECK(!crc_ok(r));
    free(r);
}

static void test_fd_leak(void) {
    printf("fd leak\n");
    fresh();
    int t = begin_transaction();
    do_update(t, 1, 0, 10, 10);
    int before = count_fds();
    for (int i = 0; i < 200; i++) {
        char b[64] = {0};
        update_wal(t, 1, 0, b, 64, b, 64);
    }
    int after = count_fds();
    CHECK(before == after);
    /* failure paths should not leak either */
    force_full(0);
    dat->last_lsn_in_segment[1] = 5;
    dat->the_farthest_lsn_required = 3;
    before = count_fds();
    for (int i = 0; i < 50; i++) {
        char b[64] = {0};
        update_wal(t, 1, 0, b, 64, b, 64);
    }
    after = count_fds();
    CHECK(before == after);
}

static void test_random_stress(int iters) {
    printf("random stress (%d updates)\n", iters);
    fresh();
    int ids[8];
    int nt = 8;
    for (int i = 0; i < nt; i++) ids[i] = begin_transaction();
    for (int i = 0; i < iters; i++) {
        int t = ids[rnd() % nt];
        int pl = (int)(rnd() % 9000);
        int ul = (int)(rnd() % 9000);
        if (rnd() % 10 == 0) pl = 0;
        if (rnd() % 10 == 0) ul = 0;
        do_update(t, (int)(rnd() % 10000), (int)(rnd() % 4096), pl, ul);
        if (fails > 50) { printf("too many failures, stopping\n"); return; }
    }
    CHECK(ckpt_calls == 0);
    for (int k = 0; k < nt; k++) {
        int slot = find_slot(ids[k]);
        long long go = dat->previous_offset[slot];
        records_struct *r = malloc(sizeof *r);
        int guard = 0, last_lsn = INT_MAX;
        while (go != -1 && guard++ < 1000000) {
            CHECK(read_rec_at(go, r) > 0);
            CHECK(r->header.transaction == ids[k]);
            CHECK(crc_ok(r));
            CHECK(r->header.lsn < last_lsn);
            last_lsn = r->header.lsn;
            go = r->header.prev_offset;
        }
        free(r);
    }
}

static void test_many_segments(void) {
    printf("many segment rollovers\n");
    fresh();
    int t = begin_transaction();
    for (int s = 0; s < 6; s++) {
        do_update(t, s, 0, 100, 100);
        force_full(dat->current_segment);
    }
    do_update(t, 99, 0, 100, 100);
    CHECK(dat->current_segment == 6);
    CHECK(dat->seq == 6);
    for (int s = 0; s <= 6; s++) CHECK(seg_exists(s));
    long long go = dat->previous_offset[find_slot(t)];
    records_struct *r = malloc(sizeof *r);
    int cnt = 0;
    while (go != -1 && cnt < 100) {
        CHECK(read_rec_at(go, r) > 0);
        CHECK(crc_ok(r));
        go = r->header.prev_offset;
        cnt++;
    }
    CHECK(cnt == 7);
    free(r);
}

static void test_transaction_slots(void) {
    printf("transaction slots\n");
    fresh();
    int ids[256];
    for (int i = 0; i < 256; i++) { ids[i] = begin_transaction(); CHECK(ids[i] > 0); }
    CHECK(begin_transaction() == -1);
    do_update(ids[0], 1, 0, 10, 10);
    do_update(ids[255], 1, 0, 10, 10);
    do_update(ids[128], 1, 0, 5000, 5000);
}

int main(int argc, char **argv) {
    if (argc > 1) rng_state ^= (uint64_t)strtoull(argv[1], NULL, 10) * 2654435761ULL;
    if (!getcwd(base_dir, sizeof base_dir)) { perror("getcwd"); return 2; }
    printf("record struct size = %zu, update header = %zu, segment header = %zu\n",
           sizeof(records_struct), (size_t)UPD_HDR, sizeof(segment_struct));

    test_basic_small();
    test_boundaries();
    test_unequal_lengths();
    test_zero_len();
    test_invalid_tx();
    test_chain_same_tx();
    test_interleaved();
    test_rollover();
    test_rollover_mid_update();
    test_rollover_blocked();
    test_wraparound();
    test_many_segments();
    test_crc_detects_corruption();
    test_fd_leak();
    test_transaction_slots();
    test_random_stress(4000);

    printf("\n%d checks, %d failed\n", checks, fails);
    if (system("rm -rf wal") != 0) { }
    return fails ? 1 : 0;
}
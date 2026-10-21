#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#ifndef default_segment_size
#define default_segment_size 16777216
#endif
#define max_size_for_data 4096

static uint32_t crc_table[256];

typedef struct frames frames;   /* placeholder, delete if you declare it elsewhere */

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

/* ===================== YOUR CODE ===================== */

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

void insert_wal(int transaction_id, int page_id, int offset, char *data) {
    int len = strlen(data);
    int data_putted = 0;
    records_struct record;
    int declration = 2;
    int slot = -1;
    int retries = 0;                                   // CHANGED

    for (int i = 0; i < 256; i++) {
        if (dat->running_transaction[i] == transaction_id) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return;
    }

    long long prev_file_offset = dat->previous_offset[slot];
    int prev_lsn = dat->running_last_lsn[slot];
    int record_size = records_sturct_size + max_size_for_data;

    while (len > 0) {
        int seg_num = dat->current_segment;
        int file_num = create_segment_file(seg_num);

        if (file_num == -1) {
            return;
        }

        segment_struct seg_str;
        ssize_t size_of_the_thing = pread(file_num, &seg_str, sizeof seg_str, 0);

        if (size_of_the_thing != (ssize_t)sizeof(seg_str)) {
            default_segment(&seg_str, seg_num);
        }

        if (default_segment_size - seg_str.size < record_size) {
            close(file_num);
            int next_seg_num = (seg_num + 1) % 256;
            if (dat->last_lsn_in_segment[next_seg_num] >= dat->the_farthest_lsn_required) {
                if (++retries > 3) {                   // CHANGED
                    return;                            // CHANGED
                }                                      // CHANGED
                checkpoint();
                continue;                              // CHANGED (was return)
            }
            dat->current_segment = next_seg_num;
            seg_num = next_seg_num;
            file_num = create_segment_file(seg_num);
            if (file_num == -1) {
                return;
            }
            ftruncate(file_num, 0);                    // CHANGED
            default_segment(&seg_str, seg_num);
            seg_str.seq = ++dat->seq;
            seg_str.first_lsn = dat->lsn_num;
            seg_str.records_offset = 0;
            dat->offset[seg_num] = seg_str.size;
        }

        seg_str.status = 1;
        int step = max_size_for_data;
        if (len < max_size_for_data) {
            step = len;
        }
        size_t data_we_put = records_sturct_size + step;
        default_record(&record, 1);
        record.header.transaction = transaction_id;
        memcpy(record.record_value.insert.inserted_data, data + data_putted, step);
        data_putted = data_putted + step;
        record.header.lenght = step;
        record.record_value.insert.offset = offset + data_putted - step;   // CHANGED
        record.record_value.insert.page_id = page_id;
        record.record_value.insert.lenght = step;
        record.header.prev = prev_lsn;
        record.header.prev_offset = prev_file_offset;
        record.crc = crc32c((char *)&record, 4, data_we_put - 4);

        prev_lsn = record.header.lsn;
        int record_off = seg_str.size;
        declration = write_record(file_num, record_off, &record, data_we_put);

        if (declration == -1) {
            close(file_num);
            return;
        }
        fsync(file_num);                               // CHANGED
        seg_str.size = seg_str.size + data_we_put;
        dat->offset[seg_num] = seg_str.size;
        prev_file_offset = (long long)seg_num * default_segment_size + record_off;
        dat->previous_offset[slot] = prev_file_offset;
        dat->last_lsn_in_segment[seg_num] = record.header.lsn;
        if (dat->running_first_lsn[slot] == -1) {
            dat->running_first_lsn[slot] = record.header.lsn;
        }
        dat->running_last_lsn[slot] = record.header.lsn;
        declration = write_header(file_num, seg_str);
        if (declration == -1) {
            close(file_num);
            return;
        }

        len = len - step;
        fsync(file_num);
        close(file_num);
    }
}

/* ===================== DUMMY CHECKPOINT ===================== */

static long checkpoint_calls = 0;
static int checkpoint_mode = 0;   /* 0 = "yes, all old data is free", 1 = never frees anything */

void checkpoint(void) {
    checkpoint_calls++;
    if (checkpoint_mode == 1) {
        return;
    }
    dat->the_farthest_lsn_required = dat->lsn_num + 1;
    dat->checkpoint_running = 0;
}

/* ===================== TEST FRAMEWORK ===================== */

#define TXN_MAX 200000
#define TEST_TIMEOUT_SEC 900
#define SMALL_SEG (default_segment_size <= 262144)

static uint64_t g_seed = 1;
static int g_scale = 1;
static int t_fail = 0;

#define CHECK(cond, ...) do { if (!(cond)) { t_fail++; if (t_fail <= 12) { printf("      check failed: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* ---- small deterministic RNG ---- */
static uint64_t rng_s = 88172645463325252ULL;
static void rng_seed(uint64_t s) {
    rng_s = s * 2685821657736338717ULL + 1442695040888963407ULL;
    if (rng_s == 0) rng_s = 1;
}
static uint32_t rnd(void) {
    rng_s ^= rng_s << 13;
    rng_s ^= rng_s >> 7;
    rng_s ^= rng_s << 17;
    return (uint32_t)(rng_s >> 11);
}
static int rnd_range(int lo, int hi) {
    return lo + (int)(rnd() % (uint32_t)(hi - lo + 1));
}

/* ---- expectation table, indexed by LSN ---- */
typedef struct {
    int valid, txn, page, off, len, prev;
    uint32_t dcrc;
} expected_t;

static expected_t *exp_arr = NULL;
static int exp_cap = 0;
static int exp_max = -1;
static int txn_last[TXN_MAX];

static void exp_set(int lsn, int txn, int page, int off, int len, uint32_t dcrc, int prev) {
    if (lsn < 0) return;
    if (lsn >= exp_cap) {
        int ncap = exp_cap ? exp_cap : 4096;
        while (ncap <= lsn) ncap *= 2;
        exp_arr = realloc(exp_arr, (size_t)ncap * sizeof(expected_t));
        if (!exp_arr) { printf("      out of memory\n"); exit(2); }
        memset(exp_arr + exp_cap, 0, (size_t)(ncap - exp_cap) * sizeof(expected_t));
        exp_cap = ncap;
    }
    exp_arr[lsn].valid = 1;
    exp_arr[lsn].txn = txn;
    exp_arr[lsn].page = page;
    exp_arr[lsn].off = off;
    exp_arr[lsn].len = len;
    exp_arr[lsn].dcrc = dcrc;
    exp_arr[lsn].prev = prev;
    if (lsn > exp_max) exp_max = lsn;
}

static void reset_state(void) {
    t_fail = 0;
    free(exp_arr);
    exp_arr = NULL;
    exp_cap = 0;
    exp_max = -1;
    for (int i = 0; i < TXN_MAX; i++) txn_last[i] = -1;
    checkpoint_calls = 0;
    checkpoint_mode = 0;
    if (system("rm -rf wal") != 0) { /* ignore */ }
    start_the_wal();
}

/* ---- data generators ---- */
static char dbuf[70000];

static char *fill_text(int len) {
    for (int i = 0; i < len; i++) dbuf[i] = (char)(33 + rnd() % 94);
    dbuf[len] = '\0';
    return dbuf;
}

static char *fill_bytes(int len) {
    for (int i = 0; i < len; i++) dbuf[i] = (char)(1 + rnd() % 255);
    dbuf[len] = '\0';
    return dbuf;
}

/* ---- insert helpers ---- */
static int chunks_of(int len) {
    return (len + max_size_for_data - 1) / max_size_for_data;
}

/* calls insert_wal and records what SHOULD be on disk, using the LSN delta
   to learn how many chunks were really logged. Returns chunks logged. */
static int do_insert(int txn, int page, int off, const char *data) {
    int len = (int)strlen(data);
    int L = dat->lsn_num;
    insert_wal(txn, page, off, (char *)data);
    int logged = dat->lsn_num - L;
    int chunks = chunks_of(len);
    int n = logged < chunks ? logged : chunks;
    for (int c = 0; c < n; c++) {
        int step = len - c * max_size_for_data;
        if (step > max_size_for_data) step = max_size_for_data;
        int ok = (txn >= 0 && txn < TXN_MAX);
        int prev = ok ? txn_last[txn] : -1;
        exp_set(L + c, txn, page, off + c * max_size_for_data, step,
                crc32c(data + c * max_size_for_data, 0, (size_t)step), prev);
        if (ok) txn_last[txn] = L + c;
    }
    return logged;
}

static void ins_ok(int txn, int page, int off, const char *data) {
    int len = (int)strlen(data);
    int logged = do_insert(txn, page, off, data);
    CHECK(logged == chunks_of(len), "insert of %d bytes logged %d chunk(s), expected %d",
          len, logged, chunks_of(len));
}

static void rand_insert(int txn, int maxlen) {
    static const int edge[] = { 1, 4095, 4096, 4097, 8192, 8193 };
    int len = rnd_range(1, maxlen);
    if (rnd() % 10 == 0) {
        len = edge[rnd() % 6];
        if (len > maxlen) len = maxlen;
    }
    int page = rnd_range(0, 999);
    int off = rnd_range(0, 4095);
    ins_ok(txn, page, off, fill_text(len));
}

static void end_txn(int id) {
    for (int i = 0; i < 256; i++) {
        if (dat->running_transaction[i] == id) {
            dat->running_transaction[i] = -1;
            dat->num_of_run_transactions--;
            return;
        }
    }
}

static void churn(long ops, int maxlen, int max_active) {
    int active[256];
    int n = 0;
    if (max_active > 256) max_active = 256;
    for (long op = 0; op < ops; op++) {
        int r = (int)(rnd() % 100);
        if (n == 0 || (r < 4 && n < max_active)) {
            int id = begin_transaction();
            CHECK(id != -1, "begin_transaction failed with %d active", n);
            if (id != -1) active[n++] = id;
            continue;
        }
        if (r < 8 && n > 1) {
            int k = (int)(rnd() % (uint32_t)n);
            end_txn(active[k]);
            active[k] = active[--n];
            continue;
        }
        rand_insert(active[rnd() % (uint32_t)n], maxlen);
    }
}

/* ===================== VERIFIER =====================
   Reads every wal/wal_XXXXXXXX file back and checks:
   headers, sizes, seq order, CRC of every record, LSN contiguity,
   per-transaction prev/prev_offset chain, and (unless VF_NOEXP)
   txn/page/offset/length/data of every record against what was inserted.
   Returns number of records found on disk. */
#define VF_WRAP  1   /* ring may have wrapped: oldest records may be gone */
#define VF_CRASH 2   /* writer was killed: tolerate a torn tail */
#define VF_NOEXP 4   /* no expectation table, structure checks only */

static long long *loc_off = NULL;
static int *loc_txn = NULL;
static int loc_cap = 0;

static int loc_ensure(int lsn) {
    if (lsn < 0 || lsn > 50000000) return -1;
    if (lsn >= loc_cap) {
        int ncap = loc_cap ? loc_cap : 4096;
        while (ncap <= lsn) ncap *= 2;
        loc_off = realloc(loc_off, (size_t)ncap * sizeof(long long));
        loc_txn = realloc(loc_txn, (size_t)ncap * sizeof(int));
        if (!loc_off || !loc_txn) { printf("      out of memory\n"); exit(2); }
        for (int i = loc_cap; i < ncap; i++) { loc_off[i] = -1; loc_txn[i] = -1; }
        loc_cap = ncap;
    }
    return 0;
}

static int vflags(void) {
    return dat->seq >= 256 ? VF_WRAP : 0;
}

static int verify(int flags) {
    static records_struct rec;
    int scanned = 0;
    int refs_seq[256], refs_seg[256], nref = 0;
    int prev_lsn = -1, first_disk_lsn = -1;

    free(loc_off); free(loc_txn);
    loc_off = NULL; loc_txn = NULL; loc_cap = 0;

    for (int s = 0; s < 256; s++) {
        char name[64];
        snprintf(name, sizeof name, "wal/wal_%08d", s);
        int fd = open(name, O_RDONLY);
        if (fd == -1) continue;
        segment_struct sh;
        ssize_t n = pread(fd, &sh, sizeof sh, 0);
        close(fd);
        if (n != (ssize_t)sizeof sh || sh.size < (int)sizeof(segment_struct)) {
            if (!(flags & VF_CRASH)) {
                CHECK(0, "segment %d: header missing or zero", s);
            }
            continue;
        }
        refs_seq[nref] = sh.seq;
        refs_seg[nref] = s;
        nref++;
    }

    for (int i = 1; i < nref; i++) {
        int ks = refs_seq[i], kg = refs_seg[i], j = i - 1;
        while (j >= 0 && refs_seq[j] > ks) {
            refs_seq[j + 1] = refs_seq[j];
            refs_seg[j + 1] = refs_seg[j];
            j--;
        }
        refs_seq[j + 1] = ks;
        refs_seg[j + 1] = kg;
    }

    if (!(flags & (VF_WRAP | VF_CRASH))) {
        for (int i = 0; i < nref; i++) {
            CHECK(refs_seg[i] == i, "segment files not contiguous: position %d holds segment %d", i, refs_seg[i]);
            CHECK(refs_seq[i] == i, "segment %d has seq %d, expected %d", refs_seg[i], refs_seq[i], i);
        }
    } else if (!(flags & VF_CRASH)) {
        for (int i = 1; i < nref; i++) {
            CHECK(refs_seq[i] == refs_seq[i - 1] + 1, "seq jumps from %d to %d", refs_seq[i - 1], refs_seq[i]);
        }
    }

    for (int i = 0; i < nref; i++) {
        int seg = refs_seg[i];
        char name[64];
        snprintf(name, sizeof name, "wal/wal_%08d", seg);
        int fd = open(name, O_RDONLY);
        if (fd == -1) { CHECK(0, "segment %d vanished", seg); continue; }
        segment_struct sh;
        struct stat st;
        if (pread(fd, &sh, sizeof sh, 0) != (ssize_t)sizeof sh || fstat(fd, &st) != 0) {
            CHECK(0, "segment %d: cannot re-read header", seg);
            close(fd);
            continue;
        }

        CHECK(sh.segment_id == seg, "segment %d: header says segment_id %d", seg, sh.segment_id);
        CHECK(sh.status == 1, "segment %d: status %d, expected 1", seg, sh.status);
        CHECK(sh.size <= default_segment_size, "segment %d: size %d exceeds segment size", seg, sh.size);
        if (flags & VF_CRASH) {
            CHECK((long long)sh.size <= (long long)st.st_size, "segment %d: header size %d beyond file size %lld",
                  seg, sh.size, (long long)st.st_size);
        } else {
            CHECK((long long)sh.size == (long long)st.st_size, "segment %d: header size %d != file size %lld",
                  seg, sh.size, (long long)st.st_size);
        }
        if (!(flags & VF_CRASH) && i + 1 < nref) {
            CHECK(default_segment_size - sh.size < (int)(records_sturct_size + max_size_for_data),
                  "segment %d rotated too early (size %d)", seg, sh.size);
        }

        int off = (int)sizeof(segment_struct);
        int first_in_seg = 1;
        int broke = 0;

        while (off < sh.size) {
            memset(&rec, 0, sizeof rec);
            if (pread(fd, &rec, records_sturct_size, off) != (ssize_t)records_sturct_size) {
                CHECK(0, "seg %d off %d: short read of record head", seg, off);
                broke = 1; break;
            }
            int dl = rec.header.lenght;
            if (dl < 1 || dl > max_size_for_data) {
                CHECK(0, "seg %d off %d: bad data length %d", seg, off, dl);
                broke = 1; break;
            }
            if (pread(fd, ((char *)&rec) + records_sturct_size, (size_t)dl,
                      (off_t)off + (off_t)records_sturct_size) != (ssize_t)dl) {
                CHECK(0, "seg %d off %d: short read of record data", seg, off);
                broke = 1; break;
            }
            size_t total = records_sturct_size + (size_t)dl;
            if (crc32c((char *)&rec, 4, total - 4) != rec.crc) {
                CHECK(0, "seg %d off %d: CRC mismatch", seg, off);
                broke = 1; break;
            }

            int lsn = rec.header.lsn;
            int t = rec.header.transaction;
            if (loc_ensure(lsn) != 0) {
                CHECK(0, "seg %d off %d: absurd lsn %d", seg, off, lsn);
                broke = 1; break;
            }
            scanned++;

            CHECK(rec.type == 1 && rec.header.type == 1, "lsn %d: bad record type", lsn);
            CHECK(loc_off[lsn] == -1, "lsn %d appears more than once on disk", lsn);

            if (prev_lsn == -1) {
                first_disk_lsn = lsn;
                if (!(flags & VF_WRAP)) {
                    CHECK(lsn == 0, "first record on disk has lsn %d, expected 0", lsn);
                }
            } else {
                CHECK(lsn == prev_lsn + 1, "lsn %d follows lsn %d (gap or reorder)", lsn, prev_lsn);
            }
            prev_lsn = lsn;

            if (first_in_seg) {
                CHECK(sh.first_lsn == lsn, "segment %d: header first_lsn %d but first record has lsn %d",
                      seg, sh.first_lsn, lsn);
            }
            first_in_seg = 0;

            long long here = (long long)seg * default_segment_size + off;
            int pv = rec.header.prev;
            CHECK(pv < lsn, "lsn %d: prev %d is not older", lsn, pv);
            if (pv == -1) {
                CHECK(rec.header.prev_offset == -1, "lsn %d: prev is -1 but prev_offset is %lld",
                      lsn, rec.header.prev_offset);
            } else if (pv >= 0 && pv < lsn && pv < loc_cap && loc_off[pv] != -1) {
                CHECK(rec.header.prev_offset == loc_off[pv], "lsn %d: prev_offset %lld, record %d is at %lld",
                      lsn, rec.header.prev_offset, pv, loc_off[pv]);
                CHECK(loc_txn[pv] == t, "lsn %d: prev lsn %d belongs to txn %d, not txn %d",
                      lsn, pv, loc_txn[pv], t);
            } else if (!(flags & VF_WRAP)) {
                CHECK(0, "lsn %d: prev lsn %d is not on disk", lsn, pv);
            }

            if (!(flags & VF_NOEXP)) {
                if (lsn >= exp_cap || !exp_arr[lsn].valid) {
                    CHECK(0, "lsn %d is on disk but was never expected (extra record)", lsn);
                } else {
                    expected_t *e = &exp_arr[lsn];
                    CHECK(t == e->txn, "lsn %d: txn %d, expected %d", lsn, t, e->txn);
                    CHECK(rec.record_value.insert.page_id == e->page, "lsn %d: page %d, expected %d",
                          lsn, rec.record_value.insert.page_id, e->page);
                    CHECK(rec.record_value.insert.offset == e->off, "lsn %d: offset %d, expected %d",
                          lsn, rec.record_value.insert.offset, e->off);
                    CHECK(rec.record_value.insert.lenght == e->len && dl == e->len,
                          "lsn %d: length %d, expected %d", lsn, dl, e->len);
                    CHECK(crc32c(rec.record_value.insert.inserted_data, 0, (size_t)dl) == e->dcrc,
                          "lsn %d: data differs from what was inserted", lsn);
                    CHECK(rec.header.prev == e->prev, "lsn %d: prev %d, expected %d", lsn, rec.header.prev, e->prev);
                }
            }

            loc_off[lsn] = here;
            loc_txn[lsn] = t;
            off += (int)total;
        }
        if (!broke) {
            CHECK(off == sh.size, "segment %d: record walk ended at %d, header size %d", seg, off, sh.size);
        }
        close(fd);
    }

    if (!(flags & VF_NOEXP)) {
        int from = first_disk_lsn < 0 ? 0 : first_disk_lsn;
        for (int l = from; l <= exp_max; l++) {
            if (exp_arr[l].valid && (l >= loc_cap || loc_off[l] == -1)) {
                CHECK(0, "lsn %d was inserted but is missing from disk", l);
            }
        }
    }
    return scanned;
}

/* ===================== THE TESTS ===================== */

/* 1-8: single insert of various sizes */
static void single_len(int len) {
    int t = begin_transaction();
    ins_ok(t, 3, 17, fill_text(len));
    int n = verify(0);
    CHECK(n == chunks_of(len), "disk has %d record(s), expected %d", n, chunks_of(len));
}
static void t_len_1(void)     { single_len(1); }
static void t_len_5(void)     { single_len(5); }
static void t_len_4095(void)  { single_len(4095); }
static void t_len_4096(void)  { single_len(4096); }
static void t_len_4097(void)  { single_len(4097); }
static void t_len_8192(void)  { single_len(8192); }
static void t_len_8193(void)  { single_len(8193); }
static void t_len_60000(void) { single_len(60000); }

/* 9 */
static void t_sizes_sweep(void) {
    int t = begin_transaction();
    int total = 0;
    int bases[4] = { 4096, 8192, 12288, 16384 };
    for (int len = 1; len <= 40; len++) {
        ins_ok(t, len, len, fill_text(len));
        total += chunks_of(len);
    }
    for (int b = 0; b < 4; b++) {
        for (int d = -6; d <= 6; d++) {
            int len = bases[b] + d;
            ins_ok(t, 1, 0, fill_text(len));
            total += chunks_of(len);
        }
    }
    int n = verify(0);
    CHECK(n == total, "disk has %d record(s), expected %d", n, total);
}

/* 10 */
static void t_empty(void) {
    int t = begin_transaction();
    int logged = do_insert(t, 1, 0, "");
    CHECK(logged == 0, "empty insert logged %d record(s)", logged);
    int n = verify(0);
    CHECK(n == 0, "disk has %d record(s), expected 0", n);
}

/* 11 */
static void t_bad_txn(void) {
    int t = begin_transaction();
    ins_ok(t, 1, 0, "real");
    int a = do_insert(999999, 1, 0, "ghost");
    int b = do_insert(0, 1, 0, "ghost");
    int c = do_insert(-5, 1, 0, "ghost");
    CHECK(a == 0, "unknown txn 999999 logged %d record(s)", a);
    CHECK(b == 0, "txn 0 logged %d record(s)", b);
    CHECK(c == 0, "txn -5 logged %d record(s)", c);
    int n = verify(0);
    CHECK(n == 1, "disk has %d record(s), expected 1", n);
}

/* 12: begin_transaction returns -1 when full; a caller passing that on must not log */
static void t_txn_minus_one(void) {
    int t = begin_transaction();
    ins_ok(t, 1, 0, "real");
    int logged = do_insert(-1, 1, 0, "ghost");
    CHECK(logged == 0, "txn id -1 (result of a failed begin) was accepted and logged %d record(s)", logged);
}

/* 13 */
static void t_ended_txn(void) {
    int t = begin_transaction();
    ins_ok(t, 1, 0, "before end");
    end_txn(t);
    int logged = do_insert(t, 1, 0, "after end");
    CHECK(logged == 0, "insert on an ended txn logged %d record(s)", logged);
    int n = verify(0);
    CHECK(n == 1, "disk has %d record(s), expected 1", n);
}

/* 14 */
static void t_chain_single(void) {
    int t = begin_transaction();
    for (int i = 0; i < 300; i++) rand_insert(t, 6000);
    verify(vflags());
}

/* 15 */
static void t_two_txn(void) {
    int a = begin_transaction();
    int b = begin_transaction();
    for (int i = 0; i < 3000; i++) rand_insert((rnd() & 1) ? a : b, 3000);
    verify(vflags());
}

/* 16 */
static void t_churn16(void) {
    churn(8000L * g_scale, 6000, 16);
    verify(vflags());
}

/* 17 */
static void t_256_slots(void) {
    int ids[256];
    for (int i = 0; i < 256; i++) {
        ids[i] = begin_transaction();
        CHECK(ids[i] != -1, "begin #%d returned -1", i + 1);
    }
    int extra = begin_transaction();
    CHECK(extra == -1, "257th begin returned %d, expected -1", extra);
    CHECK(dat->num_of_run_transactions == 256, "num_of_run_transactions is %d, expected 256",
          dat->num_of_run_transactions);
    for (int i = 0; i < 256; i++)
        for (int j = 0; j < i; j++)
            CHECK(ids[i] != ids[j], "duplicate txn id %d", ids[i]);
    int again = begin_transaction();
    CHECK(again == -1, "begin after full returned %d, expected -1", again);
}

/* 18 */
static void t_256_inserts(void) {
    int ids[256];
    for (int i = 0; i < 256; i++) ids[i] = begin_transaction();
    for (int i = 0; i < 256; i++) {
        if (ids[i] == -1) continue;
        rand_insert(ids[i], 5000);
    }
    verify(vflags());
}

/* 19 */
static void t_slot_reuse(void) {
    int a = begin_transaction();
    for (int i = 0; i < 3; i++) rand_insert(a, 3000);
    end_txn(a);
    int b = begin_transaction();
    CHECK(b > a, "reused slot got id %d, not greater than %d", b, a);
    for (int i = 0; i < 3; i++) rand_insert(b, 3000);
    verify(vflags());
}

/* 20 */
static void t_txn_ids_monotonic(void) {
    int last = 0;
    for (int i = 0; i < 2000; i++) {
        int id = begin_transaction();
        CHECK(id != -1, "begin failed on cycle %d", i);
        CHECK(id > last, "txn id %d not greater than previous %d", id, last);
        last = id;
        end_txn(id);
    }
}

/* 21 */
static void t_page_offsets(void) {
    int t = begin_transaction();
    int pages[6] = { 0, 1, 255, 65535, 100000, 2147483000 };
    int offs[6] = { 0, 1, 4095, 4096, 1048576, 100000000 };
    for (int p = 0; p < 6; p++)
        for (int o = 0; o < 6; o++)
            ins_ok(t, pages[p], offs[o], fill_text(9000));
    verify(vflags());
}

/* 22 */
static void t_all_bytes(void) {
    int t = begin_transaction();
    for (int i = 0; i < 60; i++) {
        int len = rnd_range(1, 9000);
        ins_ok(t, rnd_range(0, 99), rnd_range(0, 4095), fill_bytes(len));
    }
    verify(vflags());
}

/* 23 */
static void t_bookkeeping(void) {
    int a = begin_transaction();
    int b = begin_transaction();
    ins_ok(a, 1, 0, "aaaa");
    ins_ok(b, 2, 0, "bbbb");
    ins_ok(a, 3, 0, "AAAA");
    int sa = -1, sb = -1;
    for (int i = 0; i < 256; i++) {
        if (dat->running_transaction[i] == a) sa = i;
        if (dat->running_transaction[i] == b) sb = i;
    }
    CHECK(sa != -1 && sb != -1, "could not find txn slots");
    if (sa == -1 || sb == -1) return;
    int n = verify(0);
    CHECK(n == 3, "disk has %d record(s), expected 3", n);
    CHECK(dat->running_first_lsn[sa] == 0, "txn A first lsn %d, expected 0", dat->running_first_lsn[sa]);
    CHECK(dat->running_last_lsn[sa] == 2, "txn A last lsn %d, expected 2", dat->running_last_lsn[sa]);
    CHECK(dat->running_first_lsn[sb] == 1, "txn B first lsn %d, expected 1", dat->running_first_lsn[sb]);
    CHECK(dat->running_last_lsn[sb] == 1, "txn B last lsn %d, expected 1", dat->running_last_lsn[sb]);
    if (loc_cap > 2) {
        CHECK(dat->previous_offset[sa] == loc_off[2], "txn A previous_offset %lld, record 2 is at %lld",
              dat->previous_offset[sa], loc_off[2]);
        CHECK(dat->previous_offset[sb] == loc_off[1], "txn B previous_offset %lld, record 1 is at %lld",
              dat->previous_offset[sb], loc_off[1]);
    }
    CHECK(dat->last_lsn_in_segment[0] == 2, "last_lsn_in_segment[0] is %d, expected 2", dat->last_lsn_in_segment[0]);
    struct stat st;
    if (stat("wal/wal_00000000", &st) == 0) {
        CHECK((long long)dat->offset[0] == (long long)st.st_size, "dat->offset[0] %d != file size %lld",
              dat->offset[0], (long long)st.st_size);
    }
}

/* 24 */
static void t_tiny_flood(void) {
    int ids[4];
    for (int i = 0; i < 4; i++) ids[i] = begin_transaction();
    long n = 20000L * g_scale;
    for (long i = 0; i < n; i++) {
        ins_ok(ids[i % 4], rnd_range(0, 99), rnd_range(0, 4095), fill_text(1));
    }
    int found = verify(vflags());
    if (!(vflags() & VF_WRAP)) CHECK(found == n, "disk has %d record(s), expected %ld", found, n);
}

/* 25 */
static void t_rotation(void) {
    int t = begin_transaction();
    long target = (long)default_segment_size * 5 / 2;
    long written = 0;
    while (written < target) {
        int len = rnd_range(3000, 9000);
        ins_ok(t, rnd_range(0, 999), rnd_range(0, 4095), fill_text(len));
        written += len;
    }
    CHECK(dat->current_segment >= 2, "only reached segment %d after 2.5 segments of data", dat->current_segment);
    CHECK(checkpoint_calls == 0, "checkpoint() was called %ld time(s) before the ring wrapped", checkpoint_calls);
    verify(vflags());
}

/* 26 */
static void t_stress_mixed(void) {
    churn(20000L * g_scale, 10000, 16);
    CHECK(exp_max >= 0, "nothing was logged");
    verify(vflags());
}

/* 27 */
static void t_stress_big(void) {
    int ids[4];
    for (int i = 0; i < 4; i++) ids[i] = begin_transaction();
    long n = 1500L * g_scale;
    for (long i = 0; i < n; i++) {
        int len = rnd_range(20000, 60000);
        ins_ok(ids[rnd() % 4], rnd_range(0, 999), rnd_range(0, 4095), fill_text(len));
    }
    verify(vflags());
}

/* 28 */
static void t_stress_64(void) {
    churn(40000L * g_scale, 3000, 64);
    verify(vflags());
}

/* 29, 30: ring wrap with a working checkpoint */
static void wrap_laps(int laps, int ntxn) {
    int ids[16];
    for (int i = 0; i < ntxn; i++) ids[i] = begin_transaction();
    long cap = 400000;
    while (dat->seq < 256 * laps && cap-- > 0) {
        ins_ok(ids[rnd() % (uint32_t)ntxn], rnd_range(0, 999), rnd_range(0, 4095), fill_text(rnd_range(500, 9000)));
    }
    CHECK(dat->seq >= 256 * laps, "never completed %d laps (seq=%d)", laps, dat->seq);
    CHECK(checkpoint_calls >= 1, "ring wrapped but checkpoint() was never called");
    verify(VF_WRAP);
}
static void t_wrap_2laps(void)  { wrap_laps(2, 2); }
static void t_wrap_3laps(void)  { wrap_laps(3, 16); }

/* 31, 32: ring full */
static int fill_ring_until_refused(int t) {
    for (long i = 0; i < 400000; i++) {
        int logged = do_insert(t, 1, 0, fill_text(rnd_range(1000, 4000)));
        if (logged == 0) return 1;
    }
    return 0;
}

static void t_ring_full_stuck(void) {
    checkpoint_mode = 1;
    int t = begin_transaction();
    int refused = fill_ring_until_refused(t);
    CHECK(refused, "ring never filled: insert was never refused");
    CHECK(checkpoint_calls >= 1, "checkpoint() was not asked to free space");
    CHECK(checkpoint_calls <= 10, "checkpoint() called %ld times for one refused insert (spinning?)", checkpoint_calls);
    int again = do_insert(t, 1, 0, fill_text(2000));
    CHECK(again == 0, "second insert on a full ring logged %d record(s)", again);
    verify(0);
}

static void t_ring_full_recover(void) {
    checkpoint_mode = 1;
    int t = begin_transaction();
    int refused = fill_ring_until_refused(t);
    CHECK(refused, "ring never filled: insert was never refused");
    checkpoint_mode = 0;
    for (int i = 0; i < 100; i++) ins_ok(t, rnd_range(0, 999), rnd_range(0, 4095), fill_text(rnd_range(1000, 4000)));
    verify(VF_WRAP);
}

/* 33, 34: kill the writer, then check the disk */
static void crash_rounds(int rounds, int maxsleep_us) {
    for (int r = 0; r < rounds; r++) {
        if (system("rm -rf wal") != 0) { /* ignore */ }
        fflush(stdout);
        pid_t p = fork();
        if (p < 0) { CHECK(0, "fork failed"); return; }
        if (p == 0) {
            close(STDOUT_FILENO);
            alarm(120);
            rng_seed(g_seed + (uint64_t)r * 7919 + 13);
            start_the_wal();
            int ids[8];
            for (int i = 0; i < 8; i++) ids[i] = begin_transaction();
            for (;;) {
                int len = rnd_range(1, 9000);
                int page = rnd_range(0, 999);
                int off = rnd_range(0, 4095);
                insert_wal(ids[rnd() % 8], page, off, fill_text(len));
            }
        }
        usleep(100000 + rnd() % (uint32_t)maxsleep_us);
        kill(p, SIGKILL);
        int st = 0;
        waitpid(p, &st, 0);
        int n = verify(VF_CRASH | VF_WRAP | VF_NOEXP);
        CHECK(n > 0, "round %d: nothing was logged before the kill", r);
    }
}
static void t_crash_default(void) { crash_rounds(8, 500000); }
static void t_crash_small(void)   { crash_rounds(12, 800000); }

/* ===================== RUNNER ===================== */

typedef struct {
    const char *name;
    void (*fn)(void);
    int small_only;
} test_def;

static const test_def tests[] = {
    { "insert 1 byte",                                         t_len_1,          0 },
    { "insert 5 bytes",                                        t_len_5,          0 },
    { "insert 4095 bytes",                                     t_len_4095,       0 },
    { "insert exactly 4096 bytes (1 full chunk)",              t_len_4096,       0 },
    { "insert 4097 bytes (2 chunks, offsets continue)",        t_len_4097,       0 },
    { "insert exactly 8192 bytes (2 chunks)",                  t_len_8192,       0 },
    { "insert 8193 bytes (3 chunks)",                          t_len_8193,       0 },
    { "insert 60000 bytes (15 chunks)",                        t_len_60000,      0 },
    { "size sweep: 1..40 and around chunk edges",              t_sizes_sweep,    0 },
    { "empty string writes nothing",                           t_empty,          0 },
    { "unknown txn id writes nothing",                         t_bad_txn,        0 },
    { "txn id -1 (failed begin) writes nothing",               t_txn_minus_one,  0 },
    { "insert after txn ended writes nothing",                 t_ended_txn,      0 },
    { "1 txn, 300 inserts, prev/prev_offset chain",            t_chain_single,   0 },
    { "2 txns interleaved, 3000 inserts",                      t_two_txn,        0 },
    { "16 txns, random begin/end churn",                       t_churn16,        0 },
    { "begin_transaction: 256 slots, then -1",                 t_256_slots,      0 },
    { "256 open txns, one insert each",                        t_256_inserts,    0 },
    { "slot reuse starts a fresh chain",                       t_slot_reuse,     0 },
    { "txn ids strictly increase over 2000 begin/end",         t_txn_ids_monotonic, 0 },
    { "extreme page ids and offsets preserved",                t_page_offsets,   0 },
    { "all byte values 1..255 preserved",                      t_all_bytes,      0 },
    { "bookkeeping: first/last lsn, previous_offset, offset[]",t_bookkeeping,    0 },
    { "flood of one-byte inserts",                             t_tiny_flood,     0 },
    { "segment rotation over 2.5 segments",                    t_rotation,       0 },
    { "stress: mixed sizes, 16 txns",                          t_stress_mixed,   0 },
    { "stress: huge inserts 20000-60000 bytes",                t_stress_big,     0 },
    { "stress: 64 txns heavy churn",                           t_stress_64,      0 },
    { "ring wrap: 2 laps, checkpoint works",                   t_wrap_2laps,     1 },
    { "ring wrap: 3 laps, 16 txns",                            t_wrap_3laps,     1 },
    { "ring full, checkpoint never frees: refuses, no hang",   t_ring_full_stuck, 1 },
    { "ring full, then checkpoint recovers",                   t_ring_full_recover, 1 },
    { "SIGKILL during inserts x8, disk stays consistent",      t_crash_default,  0 },
    { "SIGKILL during inserts x12, tiny segments",             t_crash_small,    1 },
};

enum { R_PASS, R_FAIL, R_CRASH, R_HANG, R_SKIP };

static char out_buf[65536];

static int run_test(int idx, double *secs, char *label, size_t label_sz) {
    const test_def *td = &tests[idx];
    int pfd[2];
    if (pipe(pfd) != 0) { perror("pipe"); exit(2); }
    fflush(stdout);
    double t0 = now_sec();
    pid_t p = fork();
    if (p < 0) { perror("fork"); exit(2); }
    if (p == 0) {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        setvbuf(stdout, NULL, _IONBF, 0);
        alarm(TEST_TIMEOUT_SEC);
        rng_seed(g_seed + (uint64_t)(idx + 1) * 1000003ULL);
        if (td->small_only && !SMALL_SEG) _exit(77);
        reset_state();
        td->fn();
        if (t_fail > 12) printf("      ... %d more failed check(s) not shown\n", t_fail - 12);
        _exit(t_fail ? 1 : 0);
    }
    close(pfd[1]);
    size_t len = 0;
    ssize_t r;
    char tmp[4096];
    while ((r = read(pfd[0], tmp, sizeof tmp)) > 0) {
        if (len + (size_t)r < sizeof out_buf - 1) {
            memcpy(out_buf + len, tmp, (size_t)r);
            len += (size_t)r;
        }
    }
    out_buf[len] = '\0';
    close(pfd[0]);

    int st = 0;
    waitpid(p, &st, 0);
    *secs = now_sec() - t0;

    if (WIFEXITED(st)) {
        int ec = WEXITSTATUS(st);
        if (ec == 0) { snprintf(label, label_sz, "PASS"); return R_PASS; }
        if (ec == 1) { snprintf(label, label_sz, "FAIL"); return R_FAIL; }
        if (ec == 77) { snprintf(label, label_sz, "SKIP (needs -Ddefault_segment_size=131072)"); return R_SKIP; }
        snprintf(label, label_sz, "CRASH (exit code %d)", ec);
        return R_CRASH;
    }
    if (WIFSIGNALED(st)) {
        int sg = WTERMSIG(st);
        if (sg == SIGALRM) {
            snprintf(label, label_sz, "HANG (killed after %d s)", TEST_TIMEOUT_SEC);
            return R_HANG;
        }
        snprintf(label, label_sz, "CRASH (signal %d: %s)", sg, strsignal(sg));
        return R_CRASH;
    }
    snprintf(label, label_sz, "CRASH (unknown status)");
    return R_CRASH;
}

/* usage: ./wal_suite [seed] [only_test_number] [scale]
   seed  : default = time
   only  : run just that test number (0 = all)
   scale : multiplies op counts of the stress tests (default 1) */
int main(int argc, char **argv) {
    g_seed = argc > 1 ? (uint64_t)strtoull(argv[1], NULL, 10) : (uint64_t)time(NULL);
    int only = argc > 2 ? atoi(argv[2]) : 0;
    g_scale = argc > 3 ? atoi(argv[3]) : 1;
    if (g_scale < 1) g_scale = 1;

    int ntests = (int)(sizeof tests / sizeof tests[0]);
    int counts[5] = { 0, 0, 0, 0, 0 };
    int bad[64];
    int nbad = 0;

    printf("WAL test suite: seed=%llu scale=%d segment_size=%d tests=%d\n",
           (unsigned long long)g_seed, g_scale, (int)default_segment_size, ntests);
    printf("--------------------------------------------------------------------------\n");

    double total0 = now_sec();
    for (int i = 0; i < ntests; i++) {
        if (only > 0 && only != i + 1) continue;
        double secs = 0;
        char label[128];
        int code = run_test(i, &secs, label, sizeof label);
        counts[code]++;
        printf("[%2d] %-54s %s (%.1fs)\n", i + 1, tests[i].name, label, secs);
        if (out_buf[0]) printf("%s", out_buf);
        if ((code == R_FAIL || code == R_CRASH || code == R_HANG) && nbad < 64) bad[nbad++] = i + 1;
        fflush(stdout);
    }
    double total = now_sec() - total0;

    int ran = counts[R_PASS] + counts[R_FAIL] + counts[R_CRASH] + counts[R_HANG] + counts[R_SKIP];
    printf("--------------------------------------------------------------------------\n");
    printf("SUMMARY\n");
    printf("  total   : %d\n", ran);
    printf("  passed  : %d\n", counts[R_PASS]);
    printf("  failed  : %d\n", counts[R_FAIL]);
    printf("  crashed : %d\n", counts[R_CRASH]);
    printf("  hung    : %d\n", counts[R_HANG]);
    printf("  skipped : %d\n", counts[R_SKIP]);
    printf("  time    : %.1fs\n", total);
    if (nbad > 0) {
        printf("  not passing:");
        for (int i = 0; i < nbad; i++) printf(" #%d", bad[i]);
        printf("\n");
    }
    return (counts[R_FAIL] + counts[R_CRASH] + counts[R_HANG]) ? 1 : 0;
}
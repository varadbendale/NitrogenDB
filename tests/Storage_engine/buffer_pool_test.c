#define _GNU_SOURCE
#include <buffer.c>
#include <buffer.h>

#if defined(__APPLE__)
#define POSIX_FADV_NORMAL     0
#define POSIX_FADV_SEQUENTIAL 1
#define POSIX_FADV_RANDOM     2
#define POSIX_FADV_NOREUSE    3
#define POSIX_FADV_WILLNEED   4
#define POSIX_FADV_DONTNEED   5

static inline int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
    return 0;
}
#endif

static int tests = 0;
static int passes = 0;
static int failures = 0;

#define LOG(...) fprintf(stderr, __VA_ARGS__)

#define BEGIN(name) do { tests++; LOG("\n[TEST %02d] %s\n", tests, name); } while (0)

#define CHECK(cond, msg) do { \
    if (cond) { passes++; LOG("    [PASS] %s\n", msg); } \
    else { failures++; LOG("    [FAIL] %s (line %d)\n", msg, __LINE__); } \
} while (0)

#define NEED(f) do { \
    if ((f) < 0) { failures++; LOG("    [FAIL] got frame -1, skipping rest of this test (line %d)\n", __LINE__); return; } \
} while (0)

static frames *make_pool(int keep_file) {
    if (!keep_file) {
        unlink(PAGE_FILE);
    }
    frames *p = calloc(1, sizeof(frames));
    p->mp = calloc(1, sizeof(map_out_frames));
    p->stat = calloc(1, sizeof(pool_stats));
    for (int i = 0; i < NUM_PAGES; i++) {
        p->mp->frame_page_hash[i] = -1;
    }
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    for (int i = 0; i < frames_in_buf; i++) {
        p->frames[i] = calloc(1, PAGE_SIZE);
        p->dt[i] = calloc(1, sizeof(frame_desc));
        p->dt[i]->page_id = INVALID_PAGE_ID;
        pthread_mutex_init(&p->dt[i]->latch, &attr);
    }
    pthread_mutexattr_destroy(&attr);
    LOG("    pool created: %d frames, %d pages, page file %s\n", frames_in_buf, NUM_PAGES, PAGE_FILE);
    return p;
}

static void free_pool(frames *p) {
    for (int i = 0; i < frames_in_buf; i++) {
        pthread_mutex_destroy(&p->dt[i]->latch);
        free(p->frames[i]);
        free(p->dt[i]);
    }
    free(p->mp);
    free(p->stat);
    free(p);
}

static int bit_is_set(frames *p, int f) {
    return (p->mp->bitmaps[f / 8] >> (f % 8)) & 1;
}

static int count_used(frames *p) {
    int c = 0;
    for (int i = 0; i < frames_in_buf; i++) {
        if (bit_is_set(p, i)) {
            c++;
        }
    }
    return c;
}

static long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void raw_write_page(uint32_t id, unsigned char byte) {
    unsigned char buf[PAGE_SIZE];
    memset(buf, byte, PAGE_SIZE);
    int fd = open(PAGE_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        LOG("    raw_write_page: open failed\n");
        return;
    }
    pwrite(fd, buf, PAGE_SIZE, (off_t)id * PAGE_SIZE);
    close(fd);
}

static int raw_page_bytes_are(uint32_t id, size_t from, unsigned char byte) {
    unsigned char buf[PAGE_SIZE];
    int fd = open(PAGE_FILE, O_RDONLY);
    if (fd == -1) {
        return 0;
    }
    ssize_t n = pread(fd, buf, PAGE_SIZE, (off_t)id * PAGE_SIZE);
    close(fd);
    if (n != PAGE_SIZE) {
        LOG("    raw read of page %u returned %zd bytes\n", id, n);
        return 0;
    }
    for (size_t i = from; i < PAGE_SIZE; i++) {
        if (buf[i] != byte) {
            LOG("    page %u byte %zu is 0x%02x, wanted 0x%02x\n", id, i, buf[i], byte);
            return 0;
        }
    }
    return 1;
}

static int raw_header_id(uint32_t id) {
    uint32_t v = 0;
    int fd = open(PAGE_FILE, O_RDONLY);
    if (fd == -1) {
        return -1;
    }
    ssize_t n = pread(fd, &v, sizeof(v), (off_t)id * PAGE_SIZE);
    close(fd);
    if (n != (ssize_t)sizeof(v)) {
        return -1;
    }
    return (int)v;
}

static int frame_bytes_are(frames *p, int f, size_t from, unsigned char byte) {
    unsigned char *b = (unsigned char *)p->frames[f];
    for (size_t i = from; i < PAGE_SIZE; i++) {
        if (b[i] != byte) {
            LOG("    frame %d byte %zu is 0x%02x, wanted 0x%02x\n", f, i, b[i], byte);
            return 0;
        }
    }
    return 1;
}

static void stamp(frames *p, int f, uint32_t id, unsigned char byte) {
    memset(p->frames[f], byte, PAGE_SIZE);
    page *pg = (page *)p->frames[f];
    pg->header.page_id = id;
    pg->header.reserved = 0;
}

static int fill_pool_pinned(frames *p) {
    int ok = 0;
    for (int i = 0; i < frames_in_buf; i++) {
        int f = insert_page_in_frame(p, i);
        LOG("    fetch page %d -> frame %d\n", i, f);
        if (f >= 0) {
            ok++;
        }
    }
    return ok;
}

static void unpin_all_manual(frames *p) {
    for (int i = 0; i < frames_in_buf; i++) {
        p->dt[i]->use_count = 0;
    }
}

static void t_bitmap_initially_free(void) {
    BEGIN("bitmap starts empty");
    frames *p = make_pool(0);
    LOG("    used bits = %d\n", count_used(p));
    CHECK(count_used(p) == 0, "no frame is marked used at start");
    free_pool(p);
}

static void t_get_free_marks_used(void) {
    BEGIN("get_which_is_free hands out a frame and marks it used");
    frames *p = make_pool(0);
    int f = get_which_is_free(p, normal);
    LOG("    got frame %d\n", f);
    NEED(f);
    CHECK(f < frames_in_buf, "frame index is inside the pool");
    CHECK(bit_is_set(p, f), "bit for that frame is set");
    CHECK(count_used(p) == 1, "exactly one bit set");
    CHECK(p->stat->evictions == 0, "no eviction counted when a free frame existed");
    free_pool(p);
}

static void t_get_free_all_distinct(void) {
    BEGIN("get_which_is_free returns every frame once");
    frames *p = make_pool(0);
    int seen[frames_in_buf] = {0};
    int dup = 0, bad = 0;
    for (int i = 0; i < frames_in_buf; i++) {
        int f = get_which_is_free(p, normal);
        LOG("    call %d -> frame %d\n", i, f);
        if (f < 0 || f >= frames_in_buf) {
            bad++;
        } else {
            if (seen[f]) {
                dup++;
            }
            seen[f] = 1;
        }
    }
    CHECK(bad == 0, "all returned indexes are valid");
    CHECK(dup == 0, "no frame handed out twice");
    CHECK(count_used(p) == frames_in_buf, "every bit is set afterwards");
    free_pool(p);
}

static void t_in_use_sets_bit(void) {
    BEGIN("in_use sets a bit and is idempotent");
    frames *p = make_pool(0);
    in_use(p->mp, 9);
    CHECK(bit_is_set(p, 9), "bit 9 set");
    CHECK(count_used(p) == 1, "only bit 9 set");
    in_use(p->mp, 9);
    CHECK(count_used(p) == 1, "setting twice changes nothing");
    free_pool(p);
}

static void t_bit_mapping(void) {
    BEGIN("bit layout at byte boundaries");
    frames *p = make_pool(0);
    in_use(p->mp, 7);
    in_use(p->mp, 8);
    in_use(p->mp, 15);
    LOG("    byte0=0x%02x byte1=0x%02x\n", p->mp->bitmaps[0], p->mp->bitmaps[1]);
    CHECK(p->mp->bitmaps[0] == 0x80, "frame 7 is bit 7 of byte 0");
    CHECK(p->mp->bitmaps[1] == 0x81, "frames 8 and 15 are bits 0 and 7 of byte 1");
    free_pool(p);
}

static void t_make_it_free(void) {
    BEGIN("make_it_free clears the bit and resets hash slot");
    frames *p = make_pool(0);
    in_use(p->mp, 5);
    p->mp->frame_page_hash[5] = 7;
    make_it_free(p->mp, 5);
    CHECK(!bit_is_set(p, 5), "bit 5 cleared");
    CHECK(p->mp->frame_page_hash[5] == -1, "hash slot reset to -1");
    free_pool(p);
}

static void t_make_it_free_neighbors(void) {
    BEGIN("make_it_free leaves neighbouring bits alone");
    frames *p = make_pool(0);
    in_use(p->mp, 3);
    in_use(p->mp, 4);
    in_use(p->mp, 5);
    make_it_free(p->mp, 4);
    CHECK(bit_is_set(p, 3), "bit 3 still set");
    CHECK(!bit_is_set(p, 4), "bit 4 cleared");
    CHECK(bit_is_set(p, 5), "bit 5 still set");
    free_pool(p);
}

static void t_wait_for_next(void) {
    BEGIN("wait_for_the_next sleeps about the requested time");
    long a = now_ms();
    wait_for_the_next(40);
    long b = now_ms();
    LOG("    slept %ld ms\n", b - a);
    CHECK(b - a >= 35, "slept at least ~40ms");
    CHECK(b - a < 500, "did not oversleep badly");
}

static void t_miss_loads_page(void) {
    BEGIN("first fetch is a miss and loads data from disk");
    frames *p = make_pool(0);
    raw_write_page(3, 'X');
    int f = insert_page_in_frame(p, 3);
    LOG("    page 3 -> frame %d\n", f);
    NEED(f);
    CHECK(frame_bytes_are(p, f, 0, 'X'), "frame holds the bytes from disk");
    CHECK(p->stat->misses == 1, "one miss counted");
    CHECK(p->stat->hits == 0, "no hit counted");
    CHECK(p->dt[f]->valid == 1, "valid = 1");
    CHECK(p->dt[f]->dirty == 0, "dirty = 0");
    CHECK(p->dt[f]->use_count == 1, "use_count = 1");
    CHECK(p->dt[f]->page_id == 3, "descriptor page_id = 3");
    CHECK(p->dt[f]->clock_treated_bit == 1, "clock bit = 1");
    CHECK(p->mp->frame_page_hash[3] == f, "hash maps page 3 to the frame");
    CHECK(bit_is_set(p, f), "bitmap marks the frame used");
    free_pool(p);
}

static void t_hit_same_frame(void) {
    BEGIN("second fetch is a hit on the same frame");
    frames *p = make_pool(0);
    raw_write_page(3, 'X');
    int f1 = insert_page_in_frame(p, 3);
    int f2 = insert_page_in_frame(p, 3);
    LOG("    frames: %d then %d\n", f1, f2);
    NEED(f1);
    CHECK(f1 == f2, "same frame returned");
    CHECK(p->stat->hits == 1, "one hit");
    CHECK(p->stat->misses == 1, "still one miss");
    CHECK(p->dt[f1]->use_count == 2, "use_count = 2");
    free_pool(p);
}

static void t_hit_clock_bit(void) {
    BEGIN("each hit bumps the clock bit");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 2);
    insert_page_in_frame(p, 2);
    insert_page_in_frame(p, 2);
    NEED(f);
    LOG("    clock bit = %d use_count = %d\n", p->dt[f]->clock_treated_bit, p->dt[f]->use_count);
    CHECK(p->dt[f]->clock_treated_bit == 3, "clock bit = 3 after 3 fetches");
    CHECK(p->dt[f]->use_count == 3, "use_count = 3");
    free_pool(p);
}

static void t_distinct_pages(void) {
    BEGIN("different pages land in different frames with their own data");
    frames *p = make_pool(0);
    int fr[frames_in_buf];
    for (int i = 0; i < frames_in_buf; i++) {
        raw_write_page(i, 'a' + i);
    }
    int dup = 0, wrong = 0;
    int seen[frames_in_buf] = {0};
    for (int i = 0; i < frames_in_buf; i++) {
        fr[i] = insert_page_in_frame(p, i);
        LOG("    page %d -> frame %d\n", i, fr[i]);
        NEED(fr[i]);
        if (seen[fr[i]]) {
            dup++;
        }
        seen[fr[i]] = 1;
        if (!frame_bytes_are(p, fr[i], 0, 'a' + i)) {
            wrong++;
        }
    }
    CHECK(dup == 0, "no two pages share a frame");
    CHECK(wrong == 0, "every frame has the right data");
    CHECK(p->stat->misses == frames_in_buf, "one miss per page");
    CHECK(count_used(p) == frames_in_buf, "pool is full");
    free_pool(p);
}

static void t_unwritten_page_is_zero(void) {
    BEGIN("page beyond end of file reads as zeros");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 20);
    LOG("    page 20 -> frame %d\n", f);
    NEED(f);
    CHECK(frame_bytes_are(p, f, 0, 0), "frame is all zeros");
    CHECK(p->dt[f]->valid == 1, "frame still valid");
    free_pool(p);
}

static void t_read_from_disk_target(void) {
    BEGIN("read_from_disk fills only the requested frame");
    frames *p = make_pool(0);
    raw_write_page(7, 'Q');
    read_from_disk(7, 4, p);
    CHECK(frame_bytes_are(p, 4, 0, 'Q'), "frame 4 has page 7 data");
    CHECK(frame_bytes_are(p, 3, 0, 0), "frame 3 untouched");
    CHECK(frame_bytes_are(p, 5, 0, 0), "frame 5 untouched");
    free_pool(p);
}

static void t_write_read_roundtrip(void) {
    BEGIN("write_to_disk then read_from_disk roundtrip");
    frames *p = make_pool(0);
    memset(p->frames[0], 'A', PAGE_SIZE);
    write_to_disk(5, 0, p);
    CHECK(raw_page_bytes_are(5, 0, 'A'), "file has the bytes at page 5");
    memset(p->frames[0], 0, PAGE_SIZE);
    read_from_disk(5, 0, p);
    CHECK(frame_bytes_are(p, 0, 0, 'A'), "bytes read back match");
    free_pool(p);
}

static void t_write_no_clobber(void) {
    BEGIN("writing one page does not damage its neighbours");
    frames *p = make_pool(0);
    raw_write_page(0, '0');
    raw_write_page(1, '1');
    raw_write_page(2, '2');
    memset(p->frames[0], 'N', PAGE_SIZE);
    write_to_disk(1, 0, p);
    CHECK(raw_page_bytes_are(0, 0, '0'), "page 0 unchanged");
    CHECK(raw_page_bytes_are(1, 0, 'N'), "page 1 updated");
    CHECK(raw_page_bytes_are(2, 0, '2'), "page 2 unchanged");
    free_pool(p);
}

static void t_roundtrip_many(void) {
    BEGIN("many pages written from one frame land at the right offsets");
    frames *p = make_pool(0);
    for (int i = 0; i < 10; i++) {
        memset(p->frames[1], 'a' + i, PAGE_SIZE);
        write_to_disk(i, 1, p);
    }
    int bad = 0;
    for (int i = 0; i < 10; i++) {
        if (!raw_page_bytes_are(i, 0, 'a' + i)) {
            bad++;
        }
    }
    CHECK(bad == 0, "all 10 pages hold their own pattern");
    free_pool(p);
}

static void t_persistence_across_pools(void) {
    BEGIN("data survives destroying and recreating the pool");
    frames *a = make_pool(0);
    int f = insert_page_in_frame(a, 9);
    NEED(f);
    memset(a->frames[f], 'Z', PAGE_SIZE);
    write_to_disk(9, f, a);
    free_pool(a);
    frames *b = make_pool(1);
    int g = insert_page_in_frame(b, 9);
    LOG("    new pool: page 9 -> frame %d\n", g);
    NEED(g);
    CHECK(frame_bytes_are(b, g, 0, 'Z'), "new pool reads the old data");
    free_pool(b);
}

static void t_unpin_decrements(void) {
    BEGIN("unpin decrements use_count");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 1);
    insert_page_in_frame(p, 1);
    NEED(f);
    CHECK(p->dt[f]->use_count == 2, "use_count 2 after two fetches");
    not_in_use_get_out(0, f, p);
    CHECK(p->dt[f]->use_count == 1, "use_count 1 after one unpin");
    not_in_use_get_out(0, f, p);
    CHECK(p->dt[f]->use_count == 0, "use_count 0 after two unpins");
    free_pool(p);
}

static void t_unpin_sets_dirty(void) {
    BEGIN("unpin with changed=1 marks the frame dirty");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 1);
    NEED(f);
    not_in_use_get_out(1, f, p);
    CHECK(p->dt[f]->dirty == 1, "dirty = 1");
    free_pool(p);
}

static void t_unpin_clean_keeps_dirty(void) {
    BEGIN("EXPECTED: a later clean unpin must not erase an earlier dirty flag");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 1);
    insert_page_in_frame(p, 1);
    NEED(f);
    not_in_use_get_out(1, f, p);
    not_in_use_get_out(0, f, p);
    LOG("    dirty after dirty-unpin then clean-unpin = %d\n", p->dt[f]->dirty);
    CHECK(p->dt[f]->dirty == 1, "dirty flag still set");
    free_pool(p);
}

static void t_unpin_zero_keeps_valid(void) {
    BEGIN("EXPECTED: an unpinned frame still holds a valid page");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 1);
    NEED(f);
    not_in_use_get_out(0, f, p);
    LOG("    valid after last unpin = %d\n", p->dt[f]->valid);
    CHECK(p->dt[f]->valid == 1, "valid still 1 so the clock can evict it later");
    free_pool(p);
}

static void t_clock_all_pinned(void) {
    BEGIN("clock finds nothing when every frame is pinned");
    frames *p = make_pool(0);
    int ok = fill_pool_pinned(p);
    CHECK(ok == frames_in_buf, "pool filled");
    int n = give_the_frame_through_clock(p, normal);
    int f = give_the_frame_through_clock(p, force);
    LOG("    normal -> %d, force -> %d\n", n, f);
    CHECK(n == -1, "normal returns -1");
    CHECK(f == -1, "force returns -1");
    int untouched = 1;
    for (int i = 0; i < frames_in_buf; i++) {
        if (p->dt[i]->clock_treated_bit != 1) {
            untouched = 0;
        }
    }
    CHECK(untouched, "clock bits of pinned frames untouched");
    free_pool(p);
}

static void t_clock_zero_bit_evicts(void) {
    BEGIN("clock (normal) evicts an unpinned frame whose bit reaches 0");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 2);
    NEED(f);
    p->dt[f]->use_count = 0;
    int r = give_the_frame_through_clock(p, normal);
    LOG("    victim = %d (expected %d)\n", r, f);
    CHECK(r == f, "that frame is chosen");
    CHECK(p->dt[f]->valid == 0, "frame marked invalid");
    CHECK(!bit_is_set(p, f), "frame freed in bitmap");
    free_pool(p);
}

static void t_clock_high_bit_survives(void) {
    BEGIN("clock (normal) spares a frame whose bit is still above 0");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 2);
    NEED(f);
    p->dt[f]->use_count = 0;
    p->dt[f]->clock_treated_bit = 3;
    int r = give_the_frame_through_clock(p, normal);
    LOG("    result = %d, bit now = %d\n", r, p->dt[f]->clock_treated_bit);
    CHECK(r == -1, "no victim yet");
    CHECK(p->dt[f]->clock_treated_bit == 2, "bit decremented to 2");
    CHECK(p->dt[f]->valid == 1, "frame still valid");
    free_pool(p);
}

static void t_clock_sweeps(void) {
    BEGIN("repeated clock sweeps eventually evict a frame with bit 3");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 2);
    NEED(f);
    p->dt[f]->use_count = 0;
    p->dt[f]->clock_treated_bit = 3;
    int r1 = give_the_frame_through_clock(p, normal);
    int r2 = give_the_frame_through_clock(p, normal);
    int r3 = give_the_frame_through_clock(p, normal);
    LOG("    sweeps: %d %d %d\n", r1, r2, r3);
    CHECK(r1 == -1 && r2 == -1, "first two sweeps find nothing");
    CHECK(r3 == f, "third sweep evicts it");
    free_pool(p);
}

static void t_clock_force_lowest(void) {
    BEGIN("clock (force) picks the lowest clock bit");
    frames *p = make_pool(0);
    int fa = insert_page_in_frame(p, 1);
    int fb = insert_page_in_frame(p, 2);
    NEED(fa);
    NEED(fb);
    p->dt[fa]->use_count = 0;
    p->dt[fa]->clock_treated_bit = 5;
    p->dt[fb]->use_count = 0;
    p->dt[fb]->clock_treated_bit = 2;
    int r = give_the_frame_through_clock(p, force);
    LOG("    victim = %d (fa=%d fb=%d)\n", r, fa, fb);
    CHECK(r == fb, "lowest bit frame chosen");
    CHECK(p->dt[fa]->valid == 1, "other frame survives");
    free_pool(p);
}

static void t_clock_skips_invalid(void) {
    BEGIN("clock ignores frames that are not valid");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 1);
    NEED(f);
    p->dt[f]->valid = 0;
    p->dt[f]->use_count = 0;
    int r = give_the_frame_through_clock(p, normal);
    LOG("    result = %d\n", r);
    CHECK(r == -1, "invalid frame is never a victim");
    free_pool(p);
}

static void t_clock_flush_dirty(void) {
    BEGIN("evicting a dirty frame writes it to disk first");
    frames *p = make_pool(0);
    raw_write_page(6, 'A');
    int f = insert_page_in_frame(p, 6);
    NEED(f);
    stamp(p, f, 6, 'D');
    p->dt[f]->dirty = 1;
    p->dt[f]->use_count = 0;
    int r = give_the_frame_through_clock(p, normal);
    LOG("    victim = %d flushes = %d\n", r, p->stat->flushes);
    CHECK(r == f, "dirty frame chosen");
    CHECK(p->stat->flushes == 1, "flush counted");
    CHECK(raw_header_id(6) == 6, "page 6 header intact on disk");
    CHECK(raw_page_bytes_are(6, HDR, 'D'), "page 6 body on disk is the new data");
    free_pool(p);
}

static void t_clock_clean_no_flush(void) {
    BEGIN("evicting a clean frame does not write");
    frames *p = make_pool(0);
    raw_write_page(6, 'A');
    int f = insert_page_in_frame(p, 6);
    NEED(f);
    p->dt[f]->use_count = 0;
    give_the_frame_through_clock(p, normal);
    CHECK(p->stat->flushes == 0, "no flush counted");
    CHECK(raw_page_bytes_are(6, 0, 'A'), "disk copy unchanged");
    free_pool(p);
}

static void t_clock_resets_desc(void) {
    BEGIN("evicted frame descriptor is reset");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 6);
    NEED(f);
    p->dt[f]->use_count = 0;
    give_the_frame_through_clock(p, normal);
    CHECK(p->dt[f]->valid == 0, "valid = 0");
    CHECK(p->dt[f]->dirty == 0, "dirty = 0");
    CHECK(p->dt[f]->use_count == 0, "use_count = 0");
    CHECK(p->dt[f]->clock_treated_bit == 0, "clock bit = 0");
    CHECK(p->dt[f]->page_id == INVALID_PAGE_ID, "page_id = INVALID_PAGE_ID");
    free_pool(p);
}

static void t_clock_clears_page_mapping(void) {
    BEGIN("EXPECTED: evicting a page removes its page->frame mapping");
    frames *p = make_pool(0);
    int f = insert_page_in_frame(p, 6);
    NEED(f);
    p->dt[f]->use_count = 0;
    give_the_frame_through_clock(p, normal);
    LOG("    hash[6] after eviction = %d\n", p->mp->frame_page_hash[6]);
    CHECK(p->mp->frame_page_hash[6] == -1, "hash[6] is -1 (otherwise a later fetch gets a stale frame)");
    free_pool(p);
}

static void t_get_free_normal_full(void) {
    BEGIN("get_which_is_free (normal) evicts when the pool is full");
    frames *p = make_pool(0);
    fill_pool_pinned(p);
    unpin_all_manual(p);
    int f = get_which_is_free(p, normal);
    LOG("    got frame %d, evictions = %d\n", f, p->stat->evictions);
    NEED(f);
    CHECK(p->stat->evictions == 1, "eviction counted");
    CHECK(bit_is_set(p, f), "returned frame marked used");
    free_pool(p);
}

static void t_get_free_force_full(void) {
    BEGIN("get_which_is_free (force) evicts when the pool is full");
    frames *p = make_pool(0);
    fill_pool_pinned(p);
    unpin_all_manual(p);
    int f = get_which_is_free(p, force);
    LOG("    got frame %d, evictions = %d\n", f, p->stat->evictions);
    NEED(f);
    CHECK(p->stat->evictions == 1, "eviction counted");
    CHECK(bit_is_set(p, f), "returned frame marked used");
    free_pool(p);
}

static void t_full_pool_all_pinned(void) {
    BEGIN("full pool with everything pinned: new page cannot be loaded");
    frames *p = make_pool(0);
    fill_pool_pinned(p);
    int f = insert_page_in_frame(p, frames_in_buf);
    LOG("    extra page -> %d\n", f);
    CHECK(f == -1, "returns -1");
    CHECK(p->stat->misses == frames_in_buf + 1, "miss counted for the extra page");
    CHECK(p->stat->evictions == 0, "nothing evicted");
    free_pool(p);
}

static void t_full_pool_hit_works(void) {
    BEGIN("full pool: re-fetching a resident page still hits");
    frames *p = make_pool(0);
    fill_pool_pinned(p);
    int f = insert_page_in_frame(p, 0);
    LOG("    page 0 -> frame %d\n", f);
    NEED(f);
    CHECK(p->stat->hits == 1, "hit counted");
    CHECK(p->dt[f]->use_count == 2, "pin count 2");
    free_pool(p);
}

static void t_eviction_via_api(void) {
    BEGIN("EXPECTED: full pool, all unpinned through the API, new page loads");
    frames *p = make_pool(0);
    for (int i = 0; i <= frames_in_buf; i++) {
        raw_write_page(i, 'a' + i);
    }
    int fr[frames_in_buf];
    for (int i = 0; i < frames_in_buf; i++) {
        fr[i] = insert_page_in_frame(p, i);
        NEED(fr[i]);
    }
    for (int i = 0; i < frames_in_buf; i++) {
        not_in_use_get_out(0, fr[i], p);
    }
    int f = insert_page_in_frame(p, frames_in_buf);
    LOG("    extra page -> frame %d, evictions = %d\n", f, p->stat->evictions);
    CHECK(f != -1, "extra page got a frame");
    if (f >= 0) {
        CHECK(frame_bytes_are(p, f, 0, 'a' + frames_in_buf), "extra page has the right data");
    }
    CHECK(p->stat->evictions >= 1, "eviction counted");
    free_pool(p);
}

static void t_dirty_flush_via_api(void) {
    BEGIN("EXPECTED: dirty page evicted through the API reaches disk");
    frames *p = make_pool(0);
    int fr[frames_in_buf];
    for (int i = 0; i < frames_in_buf; i++) {
        fr[i] = insert_page_in_frame(p, i);
        NEED(fr[i]);
    }
    int victim_page = -1;
    for (int i = 0; i < frames_in_buf; i++) {
        if (fr[i] == 0) {
            victim_page = i;
        }
    }
    LOG("    frame 0 holds page %d\n", victim_page);
    NEED(victim_page);
    stamp(p, 0, victim_page, 'M');
    for (int i = 0; i < frames_in_buf; i++) {
        not_in_use_get_out(i == victim_page ? 1 : 0, fr[i], p);
    }
    int f = insert_page_in_frame(p, frames_in_buf);
    LOG("    extra page -> frame %d, flushes = %d\n", f, p->stat->flushes);
    CHECK(f != -1, "extra page got a frame");
    CHECK(raw_page_bytes_are(victim_page, HDR, 'M'), "dirty page data is on disk");
    free_pool(p);
}

static void t_delete_unpinned(void) {
    BEGIN("delete_the_page on an unpinned page");
    frames *p = make_pool(0);
    raw_write_page(4, 'M');
    int f = insert_page_in_frame(p, 4);
    NEED(f);
    not_in_use_get_out(0, f, p);
    int fd = open(PAGE_FILE, O_RDWR);
    delete_the_page(fd, 4, p);
    close(fd);
    CHECK(raw_page_bytes_are(4, 0, 0), "page 4 zeroed on disk");
    CHECK(p->mp->frame_page_hash[4] == -1, "hash cleared");
    CHECK(!bit_is_set(p, f), "frame freed");
    CHECK(p->dt[f]->valid == 0, "frame invalid");
    CHECK(p->dt[f]->page_id == INVALID_PAGE_ID, "page_id invalid");
    free_pool(p);
}

static void t_delete_pinned(void) {
    BEGIN("delete_the_page refuses a pinned page");
    frames *p = make_pool(0);
    raw_write_page(4, 'M');
    int f = insert_page_in_frame(p, 4);
    NEED(f);
    int fd = open(PAGE_FILE, O_RDWR);
    delete_the_page(fd, 4, p);
    close(fd);
    CHECK(raw_page_bytes_are(4, 0, 'M'), "disk page untouched");
    CHECK(p->mp->frame_page_hash[4] == f, "mapping kept");
    CHECK(p->dt[f]->use_count == 1, "still pinned");
    free_pool(p);
}

static void t_delete_then_refetch(void) {
    BEGIN("page fetched after delete is zeros and counts as a miss");
    frames *p = make_pool(0);
    raw_write_page(4, 'M');
    int f = insert_page_in_frame(p, 4);
    NEED(f);
    not_in_use_get_out(0, f, p);
    int fd = open(PAGE_FILE, O_RDWR);
    delete_the_page(fd, 4, p);
    close(fd);
    int g = insert_page_in_frame(p, 4);
    LOG("    refetch -> frame %d, misses = %d\n", g, p->stat->misses);
    NEED(g);
    CHECK(frame_bytes_are(p, g, 0, 0), "frame is zeros");
    CHECK(p->stat->misses == 2, "second miss counted");
    free_pool(p);
}

static void t_locks_init(void) {
    BEGIN("for_the_locks_init leaves every latch usable");
    frames *p = make_pool(0);
    for (int i = 0; i < frames_in_buf; i++) {
        pthread_mutex_destroy(&p->dt[i]->latch);
    }
    fra = p;
    for_the_locks_init();
    int bad = 0;
    for (int i = 0; i < frames_in_buf; i++) {
        if (pthread_mutex_trylock(&p->dt[i]->latch) != 0) {
            bad++;
        } else {
            pthread_mutex_unlock(&p->dt[i]->latch);
        }
    }
    fra = NULL;
    CHECK(bad == 0, "all latches lock and unlock");
    free_pool(p);
}

static void t_fadvise_helpers(void) {
    BEGIN("prefetch and fadvise helpers run without breaking the fd");
    raw_write_page(0, 'x');
    int fd = open(PAGE_FILE, O_RDWR);
    CHECK(fd != -1, "file opened");
    prefetch(fd, 0);
    sequential_hint(fd);
    random_hint(fd);
    done_with_file(fd);
    off_t pos = lseek(fd, 0, SEEK_CUR);
    LOG("    fd position after hints = %ld\n", (long)pos);
    CHECK(pos != (off_t)-1, "fd still usable");
    close(fd);
}

static void t_insert_the_thing(void) {
    BEGIN("insert_the_thing copies a page to a new frame and writes it");
    frames *p = make_pool(0);
    int f = get_which_is_free(p, normal);
    NEED(f);
    stamp(p, f, 12, 'T');
    p->dt[f]->valid = 1;
    int before = count_used(p);
    insert_the_thing(f, p);
    int g = p->mp->frame_page_hash[12];
    LOG("    source frame %d, copy frame %d, used %d -> %d\n", f, g, before, count_used(p));
    CHECK(count_used(p) == before + 1, "one extra frame used");
    NEED(g);
    CHECK(g != f, "copy is in a different frame");
    CHECK(frame_bytes_are(p, g, HDR, 'T'), "copy has the page body");
    CHECK(p->dt[g]->dirty == 0, "copy marked clean after write");
    CHECK(raw_header_id(12) == 12, "page 12 header on disk");
    CHECK(raw_page_bytes_are(12, HDR, 'T'), "page 12 body on disk");
    free_pool(p);
}

static void t_background_writer(void) {
    BEGIN("background_dirty_writer flushes a dirty unpinned frame (takes ~7s)");
    frames *p = make_pool(0);
    int f = get_which_is_free(p, normal);
    NEED(f);
    stamp(p, f, 11, 'W');
    p->dt[f]->valid = 1;
    p->dt[f]->dirty = 1;
    p->dt[f]->use_count = 0;
    pthread_t th;
    pthread_create(&th, NULL, background_dirty_writer, p);
    LOG("    writer started, sleeping 7s\n");
    sleep(7);
    pthread_cancel(th);
    pthread_join(th, NULL);
    CHECK(raw_header_id(11) == 11, "page 11 header on disk");
    CHECK(raw_page_bytes_are(11, HDR, 'W'), "page 11 body on disk");
    free_pool(p);
}

static frames *g_pool;
static int g_fail;

static void *stress_worker(void *arg) {
    long tid = (long)arg;
    for (int it = 0; it < 300; it++) {
        int pg = (int)((it + tid) % 4);
        int f = insert_page_in_frame(g_pool, pg);
        if (f < 0) {
            __sync_fetch_and_add(&g_fail, 1);
            continue;
        }
        not_in_use_get_out(0, f, g_pool);
    }
    return NULL;
}

static void t_stress_threads(void) {
    BEGIN("4 threads fetch/unpin 4 hot pages (race check)");
    g_pool = make_pool(0);
    g_fail = 0;
    pthread_t th[4];
    for (long i = 0; i < 4; i++) {
        pthread_create(&th[i], NULL, stress_worker, (void *)i);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(th[i], NULL);
    }
    int pinned = 0;
    for (int i = 0; i < frames_in_buf; i++) {
        pinned += g_pool->dt[i]->use_count;
    }
    LOG("    failed fetches = %d, leftover pins = %d, hits = %d, misses = %d\n",
        g_fail, pinned, g_pool->stat->hits, g_pool->stat->misses);
    CHECK(g_fail == 0, "no fetch failed");
    CHECK(pinned == 0, "every pin was released");
    free_pool(g_pool);
}

int main(int argc, char **argv) {
    int all = (argc > 1 && strcmp(argv[1], "all") == 0);
    LOG("buffer pool tests (%s)\n", all ? "all, including slow and threaded" : "fast set; run with 'all' for the rest");

    t_bitmap_initially_free();
    t_get_free_marks_used();
    t_get_free_all_distinct();
    t_in_use_sets_bit();
    t_bit_mapping();
    t_make_it_free();
    t_make_it_free_neighbors();
    t_wait_for_next();
    t_miss_loads_page();
    t_hit_same_frame();
    t_hit_clock_bit();
    t_distinct_pages();
    t_unwritten_page_is_zero();
    t_read_from_disk_target();
    t_write_read_roundtrip();
    t_write_no_clobber();
    t_roundtrip_many();
    t_persistence_across_pools();
    t_unpin_decrements();
    t_unpin_sets_dirty();
    t_unpin_clean_keeps_dirty();
    t_unpin_zero_keeps_valid();
    t_clock_all_pinned();
    t_clock_zero_bit_evicts();
    t_clock_high_bit_survives();
    t_clock_sweeps();
    t_clock_force_lowest();
    t_clock_skips_invalid();
    t_clock_flush_dirty();
    t_clock_clean_no_flush();
    t_clock_resets_desc();
    t_clock_clears_page_mapping();
    t_get_free_normal_full();
    t_get_free_force_full();
    t_full_pool_all_pinned();
    t_full_pool_hit_works();
    t_eviction_via_api();
    t_dirty_flush_via_api();
    t_delete_unpinned();
    t_delete_pinned();
    t_delete_then_refetch();
    t_locks_init();
    t_fadvise_helpers();
    t_insert_the_thing();

    if (all) {
        t_background_writer();
        t_stress_threads();
    }

    unlink(PAGE_FILE);
    LOG("\n==== %d tests, %d checks passed, %d checks failed ====\n", tests, passes, failures);
    return failures ? 1 : 0;
}

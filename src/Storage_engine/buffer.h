#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "storage_engine.h"

#define frames_in_buf 1024
#define buffer_page_size 4096
#define max_pages 65536
#define max_tables 16
#define retry_times 3
#define wait_time 15
#define normal_file_name "normal_file"
#define catalog_file "tables.cat"

typedef enum {
    force = 200,
    soft
} frame_retrieve_type;

typedef struct details {
    int valid;
    int dirty;
    char table_name[64];
    uint32_t page_id;
    int use_count;
    int clock_treated_bit;
    int io_lock;
    int io_in_progress;
    pthread_mutex_t latch;
} details;

typedef struct map_out_frames {
    int frame_page_hash[max_tables][max_pages];
    uint8_t bitmaps[frames_in_buf / 8];
    int free_bits;
} map_out_frames;

typedef struct stats {
    int hits;
    int misses;
    int evictions;
    int flushes;
} stats;

typedef struct frames {
    char frames[frames_in_buf][4096];
    details *dt[frames_in_buf];
    map_out_frames *mp;
    stats *stat;
} frames;

int get_the_table_name(const char *table_name) ; 
void make_it_free(map_out_frames *map, int thing, int slot, int page_id);
void in_use(map_out_frames *map, int thing);
int give_the_frame_through_clock(frames *fra, int type);
void write_to_disk(uint32_t page_id, int frame_index, frames *fra);
void read_from_disk(uint32_t page_id, int frame_index, frames *fra);
void wait_for_the_next(int time);
void insert_the_thing(int ind, frames *fra);

extern frames *bp;
int insert_page_in_frame(frames *fra, int page_num , char * table_name);
void not_in_use_get_out(int changed, int frame_id, frames *fra);
void for_the_locks_init(frames *fra);
void *background_dirty_writer(void *arg);
void delete_the_page(int fd, uint32_t page_id, frames *fra, const char *table_name);

#endif
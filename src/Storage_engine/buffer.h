#define frames_in_buf 1024 
#include <time.h> 
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#define frames_in_buf 1024
#define page_size 4096
#define times 3 
#define wait_time 15

typedef enum {
    force = 200 ,
    normal 
} frame_retrieve_type;

typedef struct details{
    int valid ; 
    int dirty ; 
    uint32_t page_id ; 
    int use_count ; 
    int clock_treated_bit ; 
    int io_lock ; 
}details ; 



typedef struct map_out_frames{
    int frame_page_hash[frames_in_buf] ; 
    uint8_t bitmaps[frames_in_buf/8] ; 
    int free_bits ; 
}map_out_frames ; 

typedef struct stats {
    int hits ;       
    int misses ;     
    int evictions ;   
    int flushes ;    
} stats;


typedef struct frames{
    char frames[frames_in_buf][4096] ;
    details * dt[frames_in_buf] ; 
    map_out_frames * mp ; 
    stats * stat 
}frames ; 


void make_it_free(map_out_frames *map , int thing) ;
void make_it_occupied(map_out_frames *map , int thing) ;
int  give_the_frame_through_clock(frames *fra , int type) ;
void write_to_disk(uint32_t page_id , int frame_index , frames *fra) ;
void read_from_disk(uint32_t page_id , int frame_index , frames *fra) ;
void wait_for_the_next(int time) ;





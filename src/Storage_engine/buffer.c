void read_from_disk( uint32_t page_id, int frame_index, frames *fra) {
    int fd = open(/* normal page file */, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        return ;
    }
    pthread_mutex_lock(&fra->dt[frame_index]->latch) ; 
    lseek(fd, page_id * PAGE_SIZE, SEEK_SET);
    read(fd, fra->frames[frame_index], PAGE_SIZE);
    close(fd);
    pthread_mutex_unlock(&fra->dt[frame_index]->latch) ; 
}

void write_to_disk( uint32_t page_id, int frame_index, frames *fra) {
    int fd = open(/* normal page file */, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        return ;
    }
    pthread_mutex_lock(&fra->dt[frame_index]->latch) ; 
    lseek(fd, page_id * PAGE_SIZE, SEEK_SET);
    write(fd, fra->frames[frame_index], PAGE_SIZE);
    close(fd);
    pthread_mutex_unlock(&fra->dt[frame_index]->latch) ; 
}

void for_the_locks_init(){
    for (int i = 0; i < frames_in_buf; i++) {
        pthread_mutex_init(&fra->dt[i]->latch, NULL);
    }
}

int get_which_is_free(frames *fra , int type ){
    int where_to_put  = -1 ; 
    int i = 0 ; 
    int temp = 0 ; 
    while (i < frames_in_buf / 8) {
        temp = fra->mp->bitmaps[i];
        for (int j = 7; j >= 0; j--) {
            int bit = (temp >> j) & 1;
            if (bit == 0) {
                where_to_put = (i * 8) + j;
                break;
            }
        }
        if (where_to_put != -1) {
            break;
        }
        i++ ; 
    }
    if (where_to_put != -1 ){
        pthread_mutex_lock(&fra->dt[where_to_put]->latch) ; 
        in_use(fra->mp, where_to_put) ;
        pthread_mutex_unlock(&fra->dt[where_to_put]->latch) ; 
        return where_to_put ;
    }
    else { 
        if (type == force ){
            where_to_put = give_the_frame_through_clock(fra, force);
            pthread_mutex_lock(&fra->dt[where_to_put]->latch) ; 
            if(where_to_put == -1 ){
                return -1 ; 
            }
            fra->stat->evictions++ ; 
            in_use(fra->mp, where_to_put) ;
            pthread_mutex_unlock(&fra->dt[where_to_put]->latch) ; 
            return where_to_put;
        }
        else { 
            for (int r = 0; r < times; r++) {
                where_to_put = give_the_frame_through_clock(fra , normal );
                if (where_to_put != -1) {
                    pthread_mutex_lock(&fra->dt[where_to_put]->latch) ; 
                    fra->stat->evictions++ ; 
                    in_use(fra->mp, where_to_put) ;
                    pthread_mutex_unlock(&fra->dt[where_to_put]->latch) ; 
                    return where_to_put ; 
                } 
                wait_for_the_next(wait_time);  
            }
            if (where_to_put == -1){
                where_to_put = give_the_frame_through_clock(fra , force );
                if (where_to_put != -1 ){
                    pthread_mutex_lock(&fra->dt[where_to_put]->latch) ; 
                    fra->stat->evictions++ ; 
                    make_it_occupied(fra->mp, where_to_put) ;
                    pthread_mutex_unlock(&fra->dt[where_to_put]->latch) ; 
                    return where_to_put ; 
                }
                else { 
                    return -1 ; 
                }
            }
        }
    }

    return -1  ; 
}

void make_it_free(map_out_frames *map, int thing) {
    int which_num = thing / 8;
    int bit_in_num = thing % 8;
    map->bitmaps[which_num] &= ~(1 << bit_in_num);
    map->frame_page_hash[thing] = -1 ; 
}

void background_dirty_writer(){

}

void *background_dirty_writer(void *arg) {
    frames *fra = (frames *)arg ;
    while (1) {
        sleep(5);  
        for (int i = 0 ; i < frames_in_buf ; i++){
            if (fra->dt[i]->dirty == 1 && fra->dt[i]->use_count == 0) {
                insert_the_thing( i , fra ) ;
            }
        }
    }
    return NULL;
}

void in_use(map_out_frames *map, int thing) {  
    int which_num  = thing / 8;
    int bit_in_num = thing % 8;
    map->bitmaps[which_num] |= (1 << bit_in_num);
}


void wait_for_the_next(int time) {
    struct timespec ts;
    ts.tv_sec  = time / 1000;
    ts.tv_nsec = (time % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


int give_the_frame_through_clock(frames *fra  , int type ){
    int num = INT_MAX ; 
    int ind = -1 ; 
    int count = 0 ; 
    if (type == force ){
        for (int i = 0 ; i < frames_in_buf ; i++ ){
            if (fra->dt[i]->valid == 1 ){
                if ( fra->dt[i]->use_count == 0 ){
                    fra->dt[i]->clock_treated_bit-- ; 
                    if (fra->dt[i]->clock_treated_bit < num  ){
                        num = fra->dt[i]->clock_treated_bit  ; 
                        ind = i ; 
                        count = 1 ; 
                    }
                    else if (fra->dt[i]->clock_treated_bit == num ){
                        count++ ; 
                    }
                }
            }
        }
    }

    else {
        for (int i = 0 ; i < frames_in_buf ; i++ ){
            if (fra->dt[i]->valid == 1 ){
                if ( fra->dt[i]->use_count == 0 ){
                    fra->dt[i]->clock_treated_bit-- ; 
                    if (fra->dt[i]->clock_treated_bit == 0  ){
                        ind = i ; 
                        break ; 
                    }
                }
            }
        }
    }


    if (ind == -1 ){
        return -1 ; 
    }
    pthread_mutex_lock(&fra->dt[ind]->latch) ;  
    if( fra->dt[ind]->dirty == 1 ){
        fra->stat->flushes++ ; 
        page *pg = (page *)fra->frames[ind];
        fra->dt[ind]->dirty = 0 ; 
        fra->dt[ind]->io_lock = 1 ; 
        write_to_disk( pg->header.page_id , ind ,fra ) ; 
        fra->dt[ind]->io_lock = 0 ; 
    }

    fra->dt[ind]->valid = 0  ; 
    fra->dt[ind]->dirty = 0 ; 
    fra->dt[ind]->page_id = /* invalid page id need to put that num */; ;  
    fra->dt[ind]->use_count = 0  ; 
    fra->dt[ind]->clock_treated_bit = 0  ; 
    make_it_free(fra->mp , ind) ; 
    pthread_mutex_unlock(&fra->dt[ind]->latch) ;  
    return ind ;
}


int insert_page_in_frame(frames * fra  , int page_num ){
    int where_to_put_frame ; 
    if (fra->mp->frame_page_hash[page_num] != -1 ){
        fra->stat->hits++ ; 
        where_to_put_frame = fra->mp->frame_page_hash[page_num]  ; 
        pthread_mutex_lock(&fra->dt[where_to_put_frame]->latch) ; 
        fra->dt[where_to_put_frame]->use_count++ ;
        fra->dt[where_to_put_frame]->clock_treated_bit++ ;
        pthread_mutex_unlock(&fra->dt[where_to_put_frame]->latch) ; 
        return where_to_put_frame  ; 
    }
    else { 
        fra->stat->misses++ ; 
        where_to_put_frame = get_which_is_free(fra, normal);
        if (where_to_put_frame == -1) {
            return -1 ;  
        }
        pthread_mutex_lock(&fra->dt[where_to_put_frame]->latch) ; 
        fra->dt[where_to_put_frame]->io_lock = 1 ;
        read_from_disk( page_num , where_to_put_frame , fra);
        fra->dt[where_to_put_frame]->io_lock = 0 ;
        fra->dt[where_to_put_frame]->page_id = page_num;
        fra->dt[where_to_put_frame]->valid = 1;
        fra->dt[where_to_put_frame]->dirty = 0;
        fra->dt[where_to_put_frame]->use_count = 1;
        fra->dt[where_to_put_frame]->clock_treated_bit = 1;
        fra->mp->frame_page_hash[page_num] = where_to_put_frame;
        pthread_mutex_unlock(&fra->dt[where_to_put_frame]->latch) ; 
        return where_to_put_frame ; 
    }
}


void not_in_use_get_out(int changed , int frame_id , frames * fra ){
    pthread_mutex_lock(&fra->dt[frame_id]->latch) ; 
    fra->dt[frame_id]->use_count-- ; 
    fra->dt[frame_id]->dirty = 0 ;
    if (changed == 1 ){
        fra->dt[frame_id]->dirty = 1 ;
    }
    if (fra->dt[frame_id]->use_count == 0 ){
        fra->dt[frame_id]->valid = 0 ;
    }
    pthread_mutex_unlock(&fra->dt[frame_id]->latch) ; 
}

void insert_the_thing( int ind , frames *fra) {
    page *pg = (page *)fra->frames[ind];
    int frame_num = get_which_is_free(fra, normal) ;
    if (frame_num == -1) {
        return ; 
    }
    pthread_mutex_lock(&fra->dt[frame_num]->latch) ;
    memcpy(fra->frames[frame_num], pg, PAGE_SIZE) ;
    fra->dt[frame_num]->page_id = pg->header.page_num ;
    fra->dt[frame_num]->valid = 1 ;
    fra->dt[frame_num]->dirty = 1 ;
    fra->dt[frame_num]->use_count = 0 ;
    fra->dt[frame_num]->clock_treated_bit = 0 ;
    fra->mp->frame_page_hash[pg->header.page_num] = frame_num ;
    fra->dt[frame_num]->io_in_progress = 1 ;
    write_to_disk(pg->header.page_num , frame_num , fra) ;
    fra->dt[frame_num]->io_in_progress = 0 ;
    fra->dt[frame_num]->dirty = 0 ;
    pthread_mutex_unlock(&fra->dt[frame_num]->latch) ;
}

void delete_the_page(int fd, uint32_t page_id, frames *fra) {
    int frame_index = fra->mp->frame_page_hash[page_id];
    pthread_mutex_lock(&fra->dt[frame_index]->latch) ; 
    if (frame_index == -1 ){
        return  ; 
    }
    if (fra->dt[frame_index]->use_count > 0){
        return ;
    }
    fra->dt[frame_index]->io_lock = 1; 
    char zeros[page_size];
    memset(zeros, 0, page_size);
    lseek(fd, page_id * page_size, SEEK_SET);
    write(fd, zeros, page_size);
    fra->dt[frame_index]->io_lock = 0; 
    fra->dt[frame_index]->valid = 0 ;
    fra->dt[frame_index]->dirty = 0 ;
    fra->dt[frame_index]->page_id = INVALID_PAGE_ID ;
    fra->dt[frame_index]->use_count = 0 ;
    fra->dt[frame_index]->clock_treated_bit = 0 ;
    fra->mp->frame_page_hash[page_id] = -1;
    make_it_free(fra->mp, frame_index);
    pthread_mutex_unlock(&fra->dt[frame_index]->latch) ; 
}



void prefetch(int fd, uint32_t page_id) {
    posix_fadvise(fd, page_id * PAGE_SIZE, PAGE_SIZE * 4, POSIX_FADV_WILLNEED);
}

void sequential_hint(int fd) {
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
}

void random_hint(int fd) {
    posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);
}

void done_with_file(int fd) {
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
}

// here now these are the prefetched stuff in the os cache so pretty much i had no idea when to like call them or get them to implment so just wrote and now putting it on the future me to work on it hehe 


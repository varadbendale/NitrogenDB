#include<storage_engine.h>


int db_open(const char *filename) {
    return open(filename, O_RDWR | O_CREAT, 0644);
}

void write(int fd, int page_num, uint8_t *buf) {
    ssize_t size = pwrite(fd, buf, PAGE_SIZE, page_num * PAGE_SIZE);
    if (size != PAGE_SIZE ){
        // error ; 
        exit(1) ; 
    }
}

void read(int fd, int page_num, uint8_t *buf) {
    ssize_t size = pread(fd, buf, PAGE_SIZE, page_num * PAGE_SIZE);
    if (size != PAGE_SIZE ){
        // error ; 
        exit(1) ; 
    }
}


void write_pagezero(pagezero * pg, const char * filename){
    FILE * fp = fopen(filename, "wb");
    fwrite(pg, sizeof(pagezero), 1, fp);
    fclose(fp);
}

void read_pagezero(pagezero * pg, const char * filename){
    FILE * fp = fopen(filename, "rb");
    fread(pg, sizeof(pagezero), 1, fp);
    fclose(fp);
}

void close(int fd) {
    close(fd);
}




#include "storage_engine.h"


int db_open(const char *filename) {
    return open(filename, O_RDWR | O_CREAT, 0644);
}

void db_write(int fd, int page_num, uint8_t *buf) {
    ssize_t size = pwrite(fd, buf, PAGE_SIZE, (off_t)page_num * PAGE_SIZE);
    if (size != PAGE_SIZE) {
        exit(1);
    }
}

void db_read(int fd, int page_num, uint8_t *buf) {
    ssize_t size = pread(fd, buf, PAGE_SIZE, (off_t)page_num * PAGE_SIZE);
    if (size == 0) {
        memset(buf, 0, PAGE_SIZE);
        return;
    }
    if (size != PAGE_SIZE) {
        exit(1);
    }
}




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
        return;
    }
    fread(pg, sizeof(pagezero), 1, fp);
    fclose(fp);
}

void db_close(int fd) {
    close(fd);
}

page *get_page_from_named_file(const char *filename, long offset) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }
    fseek(file, offset, SEEK_SET);
    page *ans = malloc(sizeof(page));
    if (ans == NULL) {
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(ans, 1, sizeof(page), file);
    fclose(file);
    if (bytes_read != sizeof(page)) {
        free(ans);
        return NULL;
    }
    return ans;
}


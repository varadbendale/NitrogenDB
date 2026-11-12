#ifndef B_PLUS_TREES_H
#define B_PLUS_TREES_H

#include "storage_engine.h"
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct root_entry {
    char *table;
    internal_node *root;
    struct root_entry *next;
} root_entry;

static root_entry *root_list = NULL;

#endif
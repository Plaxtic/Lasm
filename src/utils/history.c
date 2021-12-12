#ifndef HISTORY_H
#define HISTORY_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "history.h"

struct history *create_history_struct() {
    struct history *new = malloc(sizeof(struct history));

    if (new == NULL)
        return NULL;

    memset(new->instruction, 0, MAXINSTRUCTIONSIZE);
    new->adr = 0;
    new->idx = 0;
    new->next = NULL;
    new->prev = NULL;

    return new;
}

struct history *add_to_history(struct history *prev) {
    struct history *new = create_history_struct();

    if (new == NULL)
        return NULL;

    new->prev = prev;
    prev->next = new;

    return new;
}

struct history *find_head(struct history *curr) {
    if (curr->next == NULL)
        return curr;

    return find_head(curr->next);
}

struct history *find_by_adr(unsigned long long adr, struct history *head) {
    if (head == NULL || head->adr == adr)
        return head;

    return find_by_adr(adr, head->prev);
}



struct history *load_history(FILE *log) {
    struct history *new = create_history_struct();

    if (new == NULL)
        return new;

    // get last 1000 lines of file
    fseek(log, 0, SEEK_END);

    int count = 0;
    long int pos = ftell(log);
    while (pos > 0) {
        fseek(log, --pos, SEEK_SET); /* seek from begin */
        if (fgetc(log) == '\n') {
            if (count++ == 1000) break;
        }
    }

    // read log into mem 
    char buf[MAXINSTRUCTIONSIZE+24];
    while (fgets(buf, sizeof buf, log) != NULL) {
        buf[strlen(buf)-1] = 0;
        new->adr = strtoul(buf, 0, 10);
        strncpy(new->instruction, strchr(buf, ':')+1, MAXINSTRUCTIONSIZE);
        new = add_to_history(new);
    }

    return new;
}
#endif

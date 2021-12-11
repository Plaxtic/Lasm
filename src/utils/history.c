#ifndef HISTORY_H
#define HISTORY_H

#include <stdlib.h>
#include <string.h>

#include "history.h"


struct history *add_to_history(struct history *prev) {
    struct history *new = malloc(sizeof(struct history));

    if (new == NULL)
        return NULL;

    memset(new->instruction, 0, MAXINSTRUCTIONSIZE);
    new->adr = 0;
    new->idx = 0;
    new->prev = prev;
    prev->next = new;
    new->next = NULL;

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
#endif

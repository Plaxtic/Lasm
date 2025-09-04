#ifndef LABELS_H
#define LABELS_H

#include "labels.h"
#include "history.h"

char *get_last_word(char *op) {
    int len = strlen(op);
    char *p = op;
    
    p += len;

    while (*--p != ' ' && op < p);

    return p > op ? p+1 : op;
}

bool is_jump(char*op) {
    // TODO
    return toupper(op[0]) == 'J';
}

long long get_adr_by_name(struct label *head, char *name){
    if (head == NULL) 
        return -1;

    if (!strcmp(head->name, name))
        return head->adr;

    return get_adr_by_name(head->next, name);
}

int replace_label(struct label *head, char *op, unsigned long long rip) {
    if (!is_jump(op))
        return -1;

    char *last_word = get_last_word(op);
    size_t len = strlen(last_word);
    long long adr = get_adr_by_name(head, last_word);

    if (adr > 0) {
        snprintf(last_word, MAXINSTRUCTIONSIZE-len, "%lld", adr-rip);
        return 0;
    }

    return -1;
}

struct label *addlabel(struct label *prev, char *name, unsigned long long adr) {
    int len = strlen(name)-1;

    if (len > MAXLABELLEN) 
        return NULL;

    if (get_adr_by_name(prev, name) != -1) 
        return NULL;

    struct label *new = malloc(sizeof(struct label));

    if (new != NULL) {
        strncpy(new->name, name, len);
        new->name[len] = 0;
        new->adr = adr;
        new->next = prev;
    }

    return new;
}

bool is_label(char *name) {
    int len = strlen(name);

    if (len <= 1)
        return false;

    if (name[len-1] != ':')
        return false;

    if (len > MAXLABELLEN)
        return false;

    if (isdigit(name[0]))
        return false;

    for (int i = 0; i < len-1; i++)
        if (!isalpha(name[i]) && 
            !isdigit(name[i]) && 
            name[i] != '_')
            return false;

    return true;
}

void free_labels(struct label *head) {
    struct label *current = head;
    struct label *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}
#endif

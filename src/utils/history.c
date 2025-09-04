#include <limits.h>
#include "history.h"


struct history *create_history_struct() {
    struct history *new = malloc(sizeof(struct history));

    if (new == NULL)
        return NULL;

    memset(new->instruction, 0, MAXINSTRUCTIONSIZE);
    new->addr = 0;
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
    if (curr == NULL) return NULL;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    return curr;
}


struct history *find_by_addr(unsigned long long addr, struct history *head) {
    struct history *current = head;
    while (current != NULL) {
        if (current->addr == addr) {
            return current;
        }
        current = current->prev; // Traverse backwards
    }
    return NULL; // Not found
}

// struct history *find_head(struct history *curr) {
//     if (curr->next == NULL)
//         return curr;
// 
//     return find_head(curr->next);
// }

// struct history *find_by_addr(unsigned long long addr, struct history *head) {
//     if (head == NULL || head->addr == addr)
//         return head;
// 
//     return find_by_addr(addr, head->prev);
// }

FILE *get_history_file() {

    // load history log
#ifdef INSTALL
    char history_path[PATH_MAX];
    snprintf(history_path, PATH_MAX, "%s/.lasm_history", getenv("HOME"));
#else
    char history_path[] = ".lasm_history";
#endif

    // if file does not exist, create it
    FILE *log;
    if (access(history_path, F_OK) == 0)
        log = fopen(history_path, "r+");
    else
        log = fopen(history_path, "w");

    return log;
}

struct history *load_history(FILE *log) {

    struct history *new = create_history_struct();
    if (new == NULL)
        return NULL;

    // get last 1000 lines of file
    fseek(log, 0, SEEK_END);

    int count = 0;
    long int pos = ftell(log);
    while (pos > 0) {

        // seek from begin
        fseek(log, --pos, SEEK_SET);
        if (fgetc(log) == '\n') {
            if (count++ == 1000) break;
        }
    }

    // read log into mem 
    char buf[MAXINSTRUCTIONSIZE+24];
    while (fgets(buf, sizeof buf, log) != NULL) {
        buf[strlen(buf)-1] = 0;
        new->addr = strtoul(buf, 0, 10);
        snprintf(new->instruction, MAXINSTRUCTIONSIZE, "%s", strchr(buf, ':')+1);
        new = add_to_history(new);
    }
    return new;
}

void free_history(struct history *head) {
    struct history *current = head;
    struct history *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAXLABELLEN 24

struct label {
    char name[MAXLABELLEN];
    unsigned long long adr;
    struct label *next;
};

char *get_last_word(char *op);
bool is_jump(char*op);
long long get_adr_by_name(struct label *head, char *name);
int replace_label(struct label*, char*, unsigned long long);
struct label *addlabel(struct label*, char*, unsigned long long);
bool is_label(char *);
void free_labels(struct label *head);

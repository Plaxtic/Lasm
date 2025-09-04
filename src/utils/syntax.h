#ifndef SYNTAX_H
#define SYNTAX_H

#include <stdio.h>
#include <string.h>
#include <keystone/keystone.h>

struct syntax {
    int code;
    char *name;
};

extern struct syntax synax_table[];

void print_syntax_options();
int get_syntax(char *option);
int set_syntax(ks_engine *ks, int code);

#endif


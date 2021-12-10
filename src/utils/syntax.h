struct syntax {
    int code;
    char *name;
} syntax_table[] = {
    {KS_OPT_SYNTAX_INTEL, "intel"},
    {KS_OPT_SYNTAX_ATT, "att"},
    {KS_OPT_SYNTAX_NASM, "nasm"},
//    {KS_OPT_SYNTAX_MASM, "masm"},                /*  unsupported  */
    {KS_OPT_SYNTAX_GAS, "gas"},
    {KS_OPT_SYNTAX_RADIX16, "radix"},
    {0, NULL}
};

void print_syntax_options() {
    for (int i = 0; syntax_table[i].name != NULL; ++i) 
        fprintf(stderr, "\t%s\n", syntax_table[i].name);
}

int get_syntax(char *option) {

    for (int i = 0;; ++i) {
        if (syntax_table[i].name == NULL)
            return -1;
        if (!strcmp(syntax_table[i].name, option))
            return syntax_table[i].code;
    }
}


int set_syntax(ks_engine *ks, int code) {
    if (ks_option(ks, KS_OPT_SYNTAX, code) != KS_ERR_OK)
        return -1;

    return 0;
}


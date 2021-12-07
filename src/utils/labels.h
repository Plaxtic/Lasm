#define MAXLABELLEN 20

struct label {
    char name[MAXLABELLEN];
    unsigned long long adr;
    struct label *next;
};

long long get_adr_by_name(struct label *head, char *name){
    if (head == NULL) 
        return -1;

    if (!strcmp(head->name, name))
        return head->adr;

    return get_adr_by_name(head->next, name);
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

    if (name[len-1] != ':')
        return false;

    if (len > MAXLABELLEN)
        return false;

    if (isdigit(name[0]))
        return false;

    for (int i = 0; i < len-1; i++)
        if (!isalpha(name[i]) && !isdigit(name[i]))
            return false;

    return true;
}

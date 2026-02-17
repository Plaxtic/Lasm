#include "bytecode.h"
#include "app.h"

struct assembled_bytes *new_bytecode(uint8_t bytes[MAXBYTECODESIZ], size_t nbytes) {

    // init struct
    struct assembled_bytes *new = malloc(sizeof(struct assembled_bytes));

    // fill struct
    new->nbytes = nbytes;
    memcpy(new->bytecode, bytes, nbytes);

    return new;
}

int add_bytecode(struct shell_context *ctx, uint8_t *bytes, size_t nbytes) {

    // create struct
    struct assembled_bytes *new = new_bytecode(bytes, nbytes);
    if (new == NULL) {
        perror("struct create fail");
        return -1;
    }

    // stick on to head
    new->next = ctx->bytecode_head;
    ctx->bytecode_head = new;
    return 0;
}

uint8_t *get_n_bytecode(struct shell_context *ctx, int num_instr, size_t *total_bytes) {

    // calculate buffersize for malloc
    struct assembled_bytes *curr = ctx->bytecode_head;
    *total_bytes = 0;
    for (int i = 0; i < num_instr; i++) {
        if (curr == NULL) {
            fprintf(stderr, "too many instructions!\n");
            return NULL;
        }
        *total_bytes += curr->nbytes;
        curr = curr->next;
    }

    // allocate and save head
    uint8_t *buffer = malloc(*total_bytes);
    uint8_t *ret_bytecode = buffer;

    // fill buffer
    curr = ctx->bytecode_head;
    while (num_instr-- > 0) {
        memcpy(buffer, curr->bytecode, curr->nbytes);
        buffer += curr->nbytes;
        curr = curr->next;
    }

    return ret_bytecode;
}

void print_n_instr_bytecode(struct shell_context *ctx, int num_instr) {

    size_t total_bytes;
    uint8_t *bytecode = get_n_bytecode(ctx, num_instr, &total_bytes);
    if (bytecode == NULL) {
        return;
    }

    // print the bytes
    for (int i = 0; i < total_bytes; i++)
        printf("%02x", bytecode[i]);
    puts("");
}


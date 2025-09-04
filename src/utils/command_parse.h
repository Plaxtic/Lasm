#ifndef COMMAND_PARSE_H
#define COMMAND_PARSE_H

#include <stdbool.h>
#include "app.h"
#include "windows.h"
#include "wrappers.h"

#define MAXLABELLEN 24

bool is_buggy_char(const char *);
int handle_loops(struct shell_context *, unsigned long long *);
int process_instruction(struct shell_context *ctx);
int handle_step_command(struct shell_context *ctx);
int handle_assembly(struct shell_context *ctx);
int handle_label_creation(struct shell_context *ctx);
int execute_assembly(struct shell_context *ctx, uint8_t *bytes, size_t nbytes);
void manage_history(struct shell_context *ctx);

#endif

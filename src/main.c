#include <linux/limits.h>

#include "utils/ipc.h"
#include "utils/history.h"
#include "utils/windows.h"
#include "utils/labels.h"
#include "utils/syntax.h"
#include "utils/wrappers.h"
#include "utils/command_parse.h"

int main(int argc, char **argv) {
    struct shell_context ctx;
    unsigned long long loop_begin = 0;
    int return_value = EXIT_SUCCESS;

    // initialize shell context
    if (shell_init(&ctx, argc, argv) != 0)
        return EXIT_FAILURE;

    // main loop
    while (1) {

        // clear and jump to top if at bottom
        if (ctx.current_line > LINES-3)
            jump_to_bottom(&ctx);

        // get register state 
        get_regs(ctx.child, &ctx.regs_after);

        // display everything
        update_ui(&ctx);

        // perform loops
        handle_loops(&ctx, &loop_begin);

        // get instruction
        ctx.history_head = get_instruction(&ctx, 42);
        if (ctx.history_head == NULL) 
            break;
        
        // update instruction pointer
        ctx.history_head->addr = ctx.regs_after.rip;

        // Process the instruction
        return_value = process_instruction(&ctx);
        if (return_value != 0)
            break;

        // Manage history
        manage_history(&ctx);
    }
    shell_cleanup(&ctx);

    return return_value;
}

#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "command_parse.h"

bool is_buggy_char(const char *instruction) {
    if (!instruction || !instruction[0])
        return false;
        
    // Skip obvious non-instructions
    if (instruction[0] == ':')  // Just colon
        return true;
        
    // Could add other obvious invalid patterns:
    // if (strspn(instruction, " \t") == strlen(instruction))  // Just whitespace
    //     return false;
        
    return false;
}

#endif

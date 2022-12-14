//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H
#define ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

// PUSH num value - adds {num} bytes to the stack with values set from bytecode itself
// POP num - pops {num} bytes from the stack
// SWAP pos1 pos2 num - swaps {num} bytes from positions (stack.size() - pos)
// COPY_PUSH pos, num - pushes {num} bytes, and copies {num} bytes at stack.size() - pos
// BUILTIN_CALL num_fn - calls builtin function. Arguments are gotten from stack.
// GOTO pos - moves execution pointer to position in bytecode. If pos == UINT32_MAX, then stop execution and exit
// COND_GOTO pos_false - gets 4 bytes from the top of the stack, if >0, then just continue executing, if == 0 then goto to pos_false
// REL_GOTO - gets pos number from the top of the stack, pops it from the stack, and goes to it
// GET_ABSOLUTE_POS rel_pos - pushes stack.size() - rel_pos to the top of the stack
// NOOP - operation that does nothing. Is needed, so that batch_push_commands doesn't batch together a while loop expression with a var declaration above it, etc.
enum class ByteCode:uint8_t {
    PUSH,
    POP,
    SWAP,
    COPY_PUSH,
    BUILTIN_CALL,
    GOTO,
    COND_GOTO,
    REL_GOTO,
    GET_ABSOLUTE_POS,
    NOOP,
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

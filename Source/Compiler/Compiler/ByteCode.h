//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H
#define ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

// PUSH num value - adds {num} bytes to the stack with values set from bytecode itself
// POP num - pops {num} bytes from the stack
// SWAP pos1 pos2 num - swaps {num} bytes from positions
// COPY_PUSH pos, num - pushes {num} bytes, and copies {num} bytes from {pos}
// BUILTIN_CALL num_fn - calls builtin function. Arguments are gotten from stack.
// GOTO pos - moves execution pointer to position in bytecode. If pos == UINT32_MAX, then stop execution and exit
// COND_GOTO cond_result_pos pos_true pos_false - gets 4 bytes from stack's cond_result_pos, if > 0, then goto to pos_true, if == 0 then goto to pos_false
// REL_GOTO - gets pos number from the top of the stack, pops it from the stack, and goes to it
// PUSH_STACK_SCOPE - for user functions
// POP_STACK_SCOPE - for user functions
// PUSH_CURRENT_STACK_LEVEL - pushes to the back of the stack 4 byte stack level num
// GET_ABSOLUTE_POS - in the stack should be two 4 byte nums: relative var pos and stack_level. It will these two numbers, do some math, pop them and push absolute position
enum class ByteCode:uint8_t {
    PUSH,
    POP,
    SWAP,
    COPY_PUSH,
    BUILTIN_CALL,
    GOTO,
    COND_GOTO,
    REL_GOTO,
    PUSH_STACK_SCOPE,
    POP_STACK_SCOPE,
    PUSH_CURRENT_STACK_LEVEL,
    GET_ABSOLUTE_POS,
    START_ARGUMENTS,
    END_ARGUMENTS,
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

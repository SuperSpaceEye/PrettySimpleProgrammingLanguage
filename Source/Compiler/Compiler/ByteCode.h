//
// Created by spaceeye on 25.10.22.
//

#ifndef ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H
#define ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

// PUSH num - adds {num} bytes to the stack with values set from bytecode itself
// POP num - pops {num} bytes from the stack
// SWAP pos1 pos2 num - swaps {num} bytes from positions
// COPY_PUSH pos, num - pushes {num} bytes, and copies {num} bytes from {pos}
// BUILTIN_CALL num_fn - calls builtin function. Arguments are gotten from stack.
// GOTO pos - moves execution pointer to position in bytecode
// EXIT - stops the execution of bytecode
// COND_GOTO cond_result_pos pos_true pos_false - gets 4 bytes from stack's cond_result_pos, if > 0, then goto to pos_true, if == 0 then goto to pos_false
enum class ByteCode:uint8_t {
    PUSH,
    POP,
    SWAP,
    COPY_PUSH,
    BUILTIN_CALL,
    GOTO,
    EXIT,
    COND_GOTO,
};

#endif //ANOTHERPROGRAMMINGLANGUAGE_BYTECODE_H

#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
    // Decrement ESP to allocate space for 4 bytes
    REG(R_ESP) -= DATA_BYTE;

    // Write operand value to the stack
    swaddr_write(REG(R_ESP), DATA_BYTE, op_src->val);

    // For disassembly output
    print_asm_template1();
}

// helper for different operand sources (register/memory/immediate)
make_instr_helper(r)     // push r32
//make_instr_helper(rm)    // push r/m32
//make_instr_helper(i)     // push imm32

#include "cpu/exec/template-end.h"

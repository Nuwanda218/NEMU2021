#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute() {
    // read value from stack top
    DATA_TYPE val = MEM_R(REG(R_ESP));
    // write to destination operand
    OPERAND_W(op_dest, val);
    // move ESP up
    REG(R_ESP) += DATA_BYTE;
    print_asm_template1();
}

make_instr_helper(r)   // pop r16/r32
make_instr_helper(rm)  // pop r/m16 or r/m32

#include "cpu/exec/template-end.h"

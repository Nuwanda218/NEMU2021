#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    if (cpu.eflags.ZF) {           // Jump if zero
        printf("[DEBUG] je: eip=0x%x, offset=%d, target=0x%x\n",
               cpu.eip, (int32_t)op_src->val, cpu.eip + (int32_t)op_src->val);
        cpu.eip += op_src->val;
    } else {
        printf("[DEBUG] je: eip=0x%x, ZF=0, not jumping\n", cpu.eip);
    }
    print_asm_template1();
}

make_instr_helper(si)

#include "cpu/exec/template-end.h"

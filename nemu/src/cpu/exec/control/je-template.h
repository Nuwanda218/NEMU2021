#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    if (cpu.eflags.ZF) {
        int offset = op_src->val;
        printf("ZF is set, current EIP: %x, offset: %x\n", cpu.eip, offset);
        cpu.eip += offset;
        printf("Jumping to address: %x\n", cpu.eip);
    } else {
        printf("ZF is not set, not jumping\n");
    }
    print_asm_template1();
}
make_instr_helper(si)

#include "cpu/exec/template-end.h"

#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    int32_t target = cpu.eip + op_src->simm + DATA_BYTE; // 跳转目标 = 当前eip + 偏移 + 指令长度
    if (cpu.eflags.ZF) {           // Jump if zero
        printf("[DEBUG] je: eip=0x%x, ZF=1, offset=%d, target=0x%x\n",
               cpu.eip, op_src->simm, target);
        cpu.eip = target;
    } else {
        printf("[DEBUG] je: eip=0x%x, ZF=0, not jumping\n", cpu.eip);
        cpu.eip += DATA_BYTE; // 没跳转也要跳过立即数
    }

    print_asm_template1();
}

make_instr_helper(si)

#include "cpu/exec/template-end.h"

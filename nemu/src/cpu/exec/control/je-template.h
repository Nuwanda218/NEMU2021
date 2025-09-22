#include "cpu/exec/template-start.h"

#define instr je

/*
 * Jump if Equal (ZF == 1)
 * EIP ← EIP + SignExtended(imm)
 */
static void do_execute() {
    if (cpu.eflags.ZF) {               // Zero Flag 为 1 时跳转
        cpu.eip += op_src->val;        // op_src->val 已经 sign-extend
    }
    // 打印反汇编信息，显示目标地址
    print_asm("je %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)   // 生成 je_si_b / je_si_v

#include "cpu/exec/template-end.h"

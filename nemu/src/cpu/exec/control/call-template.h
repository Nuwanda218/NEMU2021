#include "cpu/exec/template-start.h"

#define instr call

/* 通用执行过程:
 *  1. 将返回地址(eip + 指令长度)压入栈
 *  2. eip 跳转到相对偏移后的目标地址
 */
static void do_execute() {
    // 1. push return address
    cpu.esp -= DATA_BYTE;
    swaddr_write(cpu.esp, DATA_BYTE, cpu.eip + op_src->size + 1); // +1: opcode 本身

    // 2. 跳转 (rel 是有符号立即数)
    cpu.eip += op_src->val;

    print_asm_template1();
}

/* 使用 decode_i_* 来解析立即数 */
make_instr_helper(i)   // 生成 call_i_b/w/l

#include "cpu/exec/template-end.h"

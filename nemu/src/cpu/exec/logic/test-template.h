#include "cpu/exec/template-start.h"

#define instr test

/* 执行逻辑：
 * result = op_dest & op_src;
 * 只更新 ZF、SF、PF，清除 CF、OF
 */
static void do_execute() {
    DATA_TYPE result = op_dest->val & op_src->val;

    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;

    cpu.eflags.ZF = (result == 0);
    cpu.eflags.SF = MSB(result);

    // 计算偶校验：低 8 位中 1 的个数是偶数时 PF = 1
    uint8_t low8 = (uint8_t)result;
    low8 ^= low8 >> 4;
    low8 ^= low8 >> 2;
    low8 ^= low8 >> 1;
    cpu.eflags.PF = !(low8 & 1);

    print_asm_template2();
}

/* 三种寻址模式：
   test r/m, r
   test r/m, imm
   test a,   imm
*/
make_instr_helper(rm2r)
make_instr_helper(i2rm)
make_instr_helper(i2a)

#include "cpu/exec/template-end.h"

#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
    DATA_TYPE src = op_src->val;
    DATA_TYPE dest = op_dest->val;
    DATA_TYPE result = dest - src;

    /* ZF, SF, PF */
    update_eflags_pf_zf_sf(result);

    /* CF：无符号借位 */
    cpu.eflags.CF = (uint32_t)dest < (uint32_t)src;

    /* OF：有符号溢出
       条件：符号不同并且 result 符号与 dest 不同 */
    cpu.eflags.OF =
        (( (int32_t)dest ^ (int32_t)src ) & ((int32_t)dest ^ (int32_t)result)) >> 31;

    print_asm_template2();
}

make_instr_helper(rm2r)   // cmp r/m, r
make_instr_helper(r2rm)   // cmp r, r/m
make_instr_helper(i2a)    // cmp imm, rAX
make_instr_helper(i2rm)   // cmp imm, r/m
make_instr_helper(i2r)    // cmp imm, r

#include "cpu/exec/template-end.h"

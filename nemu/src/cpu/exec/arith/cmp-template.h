#include "cpu/exec/template-start.h"
 
#define instr cmp
 
static void do_execute(){
    //被减数为左边的目标操作数，减数为右边的操作数
    DATA_TYPE result = op_dest->val - op_src->val;
    
    update_eflags_pf_zf_sf((DATA_TYPE_S)result);  //利用函数更新PF,SF,ZF
    cpu.eflags.CF = result > op_dest->val;  //通过比较结果和被减数判断是否借位
    //减法发生溢出只需判断是否被减数和减数不同号，被减数和结果不同号，再用MSB宏提取最高位即可
    cpu.eflags.OF = MSB((op_dest->val ^ op_src->val) & (op_dest->val ^ result));
 
    print_asm_template2();
}
 
make_instr_helper(i2a);
make_instr_helper(i2rm);
 
//有符号立即数需要位扩展，只在2与4字节中发生
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
 
make_instr_helper(r2rm)
make_instr_helper(rm2r)
 
#include "cpu/exec/template-end.h"
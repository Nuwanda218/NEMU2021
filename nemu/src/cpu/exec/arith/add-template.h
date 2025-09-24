#include "cpu/exec/template-start.h"
 
#define instr add
 
static void do_execute(){
    DATA_TYPE result = op_src->val + op_dest->val; //记录两个操作数相加后的结果
    OPERAND_W(op_dest, result); //相加后的结果是储存在目标操作书中的
    
    update_eflags_pf_zf_sf((DATA_TYPE_S)result); //根据相加后的结果更新标志位
    cpu.eflags.CF = op_dest->val > result; //通过相加后的结果是否小于目标操作数判断借位
    //通过提取符号位，确定操作数同号，但是结果异号来判断溢出
    cpu.eflags.OF = MSB(~(op_src->val ^ op_dest->val)&(op_dest->val ^ result));
 
    print_asm_template2();
}
 
make_instr_helper(i2a);
make_instr_helper(i2rm);
 
//特殊处理有符号数的位扩展
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
 
make_instr_helper(r2rm)
make_instr_helper(rm2r)
 
#include "cpu/exec/template-end.h"
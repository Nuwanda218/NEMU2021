#include "cpu/exec/template-start.h"
 
#define instr pop
 
static void do_execute(){
    //向译码出的对象操作数中写入栈顶的数据
    OPERAND_W(op_src,swaddr_read(cpu.esp, 4, R_SS));
    cpu.esp += 4;  //栈顶加4进行地址回退
    print_asm_template1();
}
 
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(r)
make_instr_helper(rm)
#endif
 
#include "cpu/exec/template-end.h"
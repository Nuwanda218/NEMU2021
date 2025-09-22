#include "cpu/exec/helper.h"
 
//不需区分操作数位数，只需分析指令，因此不需要template.h文件
make_helper(ret){
    cpu.eip = swaddr_read(cpu.esp, 4) - 1;  //使eip跳转到esp中存放的返回地址
    cpu.esp += 4;  //栈回退4位
 
    print_asm("ret");
 
    return 1;
}
 
make_helper(ret_i){
    uint16_t imm = instr_fetch(eip + 1, 2);  //还需要读取ret后面所跟的16位的参数
    cpu.eip = swaddr_read(cpu.esp, 4) - 1-2;  
    cpu.esp += 4 + imm; 
 
    print_asm("ret $0x%x", imm);
 
    return 3;
}
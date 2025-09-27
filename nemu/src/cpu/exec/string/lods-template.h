#include "cpu/exec/template-start.h"
 
#define instr lods
 
make_helper(concat(lods_,SUFFIX)){
    REG(R_EAX) = MEM_R(cpu.esi); //将esi寄存器中的数据加载到eax寄存器中
    //根据方向标志位判断esi寄存器的更新方向
    cpu.esi += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);
 
    print_asm("lods" str(SUFFIX) " %%ds:(%%esi),%%%s", REG_NAME(R_EAX));
    return 1;
}
 
#include "cpu/exec/template-end.h"
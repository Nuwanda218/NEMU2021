//不涉及操作数类型，在.c文件中定义make_helper函数框架即可
#include "cpu/exec/helper.h"
 
make_helper(leave){
    cpu.esp = cpu.ebp;  //栈顶指向栈基
    cpu.ebp = swaddr_read(cpu.esp, 4, R_SS);  //栈基回到存放好的上一级函数的栈基
    cpu.esp += 4;  //栈顶去除存放栈基的部分内存
 
    print_asm("leave");
    return 1;
}
// 头部引入文件start引入需要使用到的宏定义
#include "cpu/exec/template-start.h"
 
#define instr call  //构造指令名称
 
make_helper(concat(call_i_, SUFFIX)){  //定义call指令跳转相对偏移量的操作
    int len = concat(decode_i_, SUFFIX)(eip + 1); //decode函数中解码立即数的操作获取偏移量长度
    reg_l(R_ESP) -= DATA_BYTE;  //esp-4操作，相当于压地址入栈的第一步，栈顶地址减4
    MEM_W(reg_l(R_ESP), cpu.eip+len+1);  //将下一步的指令地址写入栈顶
 
    cpu.eip += (DATA_TYPE_S)op_src->val; //加上立即数偏移量，实现跳转操作
    print_asm("call: 0x%x", cpu.eip + len + 1);  //打印调试信息，查看跳转的具体地址
    return len + 1;  //返回指令长度
}
 
make_helper(concat(call_rm_, SUFFIX)){  //定义call指令直接跳转内存或寄存器地址的操作
    int len = concat(decode_rm_, SUFFIX)(eip + 1);  //decode函数中解码寄存器或地址获取指令长度
    reg_l(R_ESP) -= DATA_BYTE;
    MEM_W(reg_l(R_ESP), cpu.eip + len + 1);
 
    cpu.eip = (DATA_TYPE_S)op_src->val - len - 1;  //跳转到目标寄存器地址，修正当前指令的影响
    print_asm("call: %s", op_src->str);
    return len + 1;
} 
 
//尾部再引入文件end清除宏定义，防止污染后续代码文件
#include "cpu/exec/template-end.h"
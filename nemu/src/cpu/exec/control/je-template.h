#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
    if (cpu.eflags.ZF) {           // 检查零标志
        int offset = op_src->val;  // 获取偏移量，确保这是正确的相对偏移量
        cpu.eip += offset;         // 应用偏移量到EIP
        printf("ZF is set, current EIP: %x, offset: %x\n", cpu.eip - offset, offset);
        printf("Jumping to address: %x\n", cpu.eip);
    } else {
        printf("ZF is not set, not jumping\n");
    }
    print_asm_template1();
}

make_instr_helper(si)  // 根据宏生成指令处理函数

#include "cpu/exec/template-end.h"
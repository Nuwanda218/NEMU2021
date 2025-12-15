#include "cpu/exec/template-start.h"
 
#define instr lgdt
 
make_helper(concat(lgdt_rm_, SUFFIX)) {
    int len = concat(decode_rm_, SUFFIX)(eip + 1);
    uint32_t addr = op_src->addr;
    uint8_t sreg = op_src->sreg;
    cpu.gdtr.limit = swaddr_read(addr, 2, sreg);
    if (ops_decoded.is_operand_size_16) {
        uint32_t base = swaddr_read(addr + 2, 4, sreg) & 0x00ffffff;
        cpu.gdtr.base = base;
    } else {
        cpu.gdtr.base = swaddr_read(addr + 2, 4, sreg);
    }
    print_asm("lgdt %s", op_src->str);
    return len + 1; 
}
 
#include "cpu/exec/template-end.h"

#include "cpu/exec/template-start.h"

make_helper(concat(ret_, SUFFIX)) {
    uint32_t new_eip = swaddr_read(cpu.esp, DATA_BYTE);
    cpu.esp += DATA_BYTE;
    cpu.eip = new_eip;
    print_asm("ret");
    return 0;
}
#include "cpu/exec/template-end.h"

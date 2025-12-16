#include "mov_cr.h"
 
int mov_cr2r (swaddr_t eip) {
    
    uint8_t modrm = instr_fetch(eip + 1, 1);
    
    int cr_idx = (modrm >> 3) & 0x7;
    int reg_idx = modrm & 0x7;
    
    uint32_t cr_val = 0;
    switch (cr_idx) {
        case 0 :  cr_val = cpu.cr0.val;
                  break;
        default : panic("mov_cr2r: CR%d is not supported.", cr_idx); 
                  break;
    }
 
    reg_l(reg_idx) = cr_val;
 
    print_asm("mov %%cr%d, reg[%d]", cr_idx, reg_idx);
 
    return 2;
}
 
int mov_r2cr (swaddr_t eip) { 
    uint8_t modrm = instr_fetch(eip + 1, 1);
    int cr_idx = (modrm >> 3) & 0x7;
    int reg_idx = modrm & 0x7;
    
    uint32_t cr_val = reg_l(reg_idx);
        switch (cr_idx) {
        case 0 :  cpu.cr0.val = cr_val ;
                  break;
        case 3 :  cpu.cr3.val = cr_val ;
                  break;
        default : panic("mov_r2cr: CR%d is not supported.", cr_idx); 
                  break;
    }
 
    print_asm("mov reg[%d], %%cr%d", reg_idx, cr_idx);
    return 2;
}
 
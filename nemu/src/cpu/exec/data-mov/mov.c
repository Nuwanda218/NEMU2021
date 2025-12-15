#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "mov-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "mov-template.h"
#undef DATA_BYTE

/* for instruction encoding overloading */

make_helper_v(mov_i2r)
make_helper_v(mov_i2rm)
make_helper_v(mov_r2rm)
make_helper_v(mov_rm2r)
make_helper_v(mov_a2moffs)
make_helper_v(mov_moffs2a)

int mov_r2sreg(swaddr_t eip) {
    
    uint8_t modrm = instr_fetch(eip + 1, 1);
    uint8_t sreg_idx = (modrm >> 3) & 0x7;
    uint8_t reg_idx = modrm & 0x7;
    uint16_t selector = reg_w(reg_idx);
    
    cpu.seg_regs[sreg_idx].val = selector;
 
    if (cpu.cr0.protect_enable) {
        
        uint32_t index = (selector >> 3) & 0x1FFF;
        bool ti = (selector >> 2) & 0x1;
        
        
        if (ti) {
            panic("LDT not supported");
        }
        
        if (index * 8 + 7 > cpu.gdtr.limit) {
            panic("GDT index %d out of bounds", index);
        }
        
        // 读取描述符
        uint32_t descriptor_addr = cpu.gdtr.base + index * 8;
        uint32_t desc_low = lnaddr_read(descriptor_addr, 4);
        uint32_t desc_high = lnaddr_read(descriptor_addr + 4, 4);
        
        
        // ============ 修正开始 ============
        
        // 提取limit低16位（desc_low的低16位）
        uint32_t limit_low = desc_low & 0xFFFF;
        
        // 提取limit高4位（desc_high的第16-19位，即字节6的低4位）
        // desc_high: | 字节7 | 字节6 | 字节5 | 字节4 |
        uint32_t limit_high = (desc_high >> 16) & 0xF;  // 修正：>>16 而不是直接&0xF
        
        // 组合20位limit
        uint32_t limit = (limit_high << 16) | limit_low;
        
        // 提取G位（desc_high的第23位，即字节6的第7位）
        uint32_t g_bit = (desc_high >> 23) & 0x1;  // 修正：检查第23位
        
        // 提取base
        uint32_t base_low = (desc_low >> 16) & 0xFFFF;   // desc_low的高16位
        uint32_t base_mid = desc_high & 0xFF;            // desc_high的低8位（字节4）
        uint32_t base_high = (desc_high >> 24) & 0xFF;   // desc_high的高8位（字节7）
        uint32_t base = base_low | (base_mid << 16) | (base_high << 24);
        
        // 处理粒度
        if (g_bit) {
            limit = (limit << 12) | 0xFFF;
        }
        
        // ============ 修正结束 ============
        
        
        // 设置段寄存器的隐藏部分
        cpu.seg_regs[sreg_idx].base = base;
        cpu.seg_regs[sreg_idx].limit = limit;
        
    } else {
        // 实模式
        cpu.seg_regs[sreg_idx].base = (uint32_t)selector << 4;
        cpu.seg_regs[sreg_idx].limit = 0xFFFF;
    }
    
    const char* sreg_name[] = {"ES", "CS", "SS", "DS", "FS", "GS"};
    print_asm("mov %%%s, %%%s", "ax", sreg_name[sreg_idx]);
    
    return 2;
}



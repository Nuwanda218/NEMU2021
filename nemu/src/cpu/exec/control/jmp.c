#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jmp-template.h"
#undef DATA_BYTE

int ljmp_ptr16_32(swaddr_t eip) {
    // 读取32位偏移量
    uint32_t offset = instr_fetch(eip + 1, 4);
    
    // 读取16位段选择子
    uint16_t selector = instr_fetch(eip + 5, 2);
 
    cpu.seg_regs[1].val = selector;  // 使用符号常量CS，而不是硬编码1
    
    if (cpu.cr0.protect_enable) {
        // ---- 保护模式：从GDT加载描述符信息 ----
        uint32_t index = (selector >> 3) & 0x1FFF;
        bool ti = (selector >> 2) & 0x1;
        
        if (ti) {
            panic("ljmp: LDT not supported");
        }
        
        if (index * 8 + 7 > cpu.gdtr.limit) {
            panic("ljmp: GDT index %d out of bounds", index);
        }
        
        // 从GDT读取描述符
        uint32_t descriptor_addr = cpu.gdtr.base + index * 8;
        uint32_t desc_low = lnaddr_read(descriptor_addr, 4);
        uint32_t desc_high = lnaddr_read(descriptor_addr + 4, 4);
        
        // ============ 修正开始 ============
        
        // 提取limit低16位
        uint32_t limit_low = desc_low & 0xFFFF;
        
        // 提取limit高4位（desc_high的第16-19位）
        uint32_t limit_high = (desc_high >> 16) & 0xF;
        
        // 组合20位limit
        uint32_t limit = (limit_high << 16) | limit_low;
        
        // 提取G位（desc_high的第23位）
        uint32_t g_bit = (desc_high >> 23) & 0x1;
        
        // 提取base
        uint32_t base_low = (desc_low >> 16) & 0xFFFF;
        uint32_t base_mid = desc_high & 0xFF;
        uint32_t base_high = (desc_high >> 24) & 0xFF;
        uint32_t base = base_low | (base_mid << 16) | (base_high << 24);
        
        // 处理粒度
        if (g_bit) {
            limit = (limit << 12) | 0xFFF;
        }
        
        // ============ 修正结束 ============
        
        cpu.seg_regs[1].base = base;
        cpu.seg_regs[1].limit = limit;
        
        // 调试输出
        #ifdef DEBUG
        printf("[LJMP CS] selector=0x%04x\n", selector);
        printf("  limit_low=0x%04x, limit_high=0x%x, G=%d\n",
               limit_low, limit_high, g_bit);
        printf("  limit=0x%08x, base=0x%08x\n", limit, base);
        #endif
        
    } else {
        // 实模式
        cpu.seg_regs[1].base = (uint32_t)selector << 4;
        cpu.seg_regs[1].limit = 0xFFFF;
    }
 
    cpu.eip = offset - 7;
    
    print_asm("ljmp $0x%04x, $0x%08x", selector, offset);
    
    return 7;
}
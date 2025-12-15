#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include "cpu/reg.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);


lnaddr_t seg_translate(swaddr_t offset, size_t len, uint8_t sreg) {
	sreg = sreg & 0x3;
    if (sreg >= NR_SEG_REGS) {
        panic("seg_translate: Invalid segment register index %d", sreg);
    }
 
    // 直接从段寄存器中获取 base 和 limit
    lnaddr_t base = cpu.seg_regs[sreg].base;
    uint32_t limit = cpu.seg_regs[sreg].limit;
 
    // 检查偏移量是否超出段界限
    if (cpu.cr0.protect_enable && (offset > limit || (offset + len - 1) > limit)) {    
            panic("seg_translate: Segment limit exceeded. Offset: 0x%x, Limit: 0x%x, Len: %zu, Segment: %d",
              offset, limit, len, sreg);
    }
 
    // 实模式和保护模式下的地址转换逻辑现在统一了
    return base + offset;
}

 
/* Memory accessing interfaces */
 
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
  int cache_L1_way_1_index = read_cache_L1(addr);
  uint32_t block_bias = addr & (CACHE_B - 1);
  uint8_t ret[BURST_LEN << 1];
  //printf("%d\n", block_bias);
  if (block_bias + len > CACHE_B) {
    int cache_L1_way_2_index = read_cache_L1(addr + CACHE_B - block_bias);
    memcpy(ret, cache_L1[cache_L1_way_1_index].data + block_bias, CACHE_B - block_bias);
    memcpy(ret  + CACHE_B - block_bias, cache_L1[cache_L1_way_2_index].data, len - (CACHE_B - block_bias));
  } else {
    memcpy(ret, cache_L1[cache_L1_way_1_index].data + block_bias, len);
  }
  int tmp = 0;
  uint32_t result = unalign_rw(ret + tmp, 4) & (~0u >> ((4 - len) << 3));
  return result;
}
 
void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
  write_cache_L1(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate (addr, len, sreg); 
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
  lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}




#ifndef __REG_H__
#define __REG_H__

#include "common.h"
//为了使用CR0
#include "../../../lib-common/x86-inc/mmu.h"
#include "../../../lib-common/x86-inc/cpu.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

enum {
    R_ES,  // 编译器自动赋值为 0
    R_CS,  // 编译器自动赋值为 1
    R_SS,  // 编译器自动赋值为 2
    R_DS,  // 编译器自动赋值为 3
    NR_SEG_REGS  // 编译器自动赋值为 4
};
/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

 

typedef struct {
     union{
	union {
		uint32_t _32;
		uint16_t _16;
		uint8_t _8[2];
     } gpr[8];

     /* Do NOT change the order of the GPRs' definitions. */

     struct {
	     int32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
     	};
     };

     swaddr_t eip;
     
     union {
		struct {
			uint32_t CF		:1;
			uint32_t pad0	:1;
			uint32_t PF		:1;
			uint32_t pad1	:1;
			uint32_t AF		:1;
			uint32_t pad2	:1;
			uint32_t ZF		:1;
			uint32_t SF		:1;
			uint32_t TF		:1;
			uint32_t IF		:1;
			uint32_t DF		:1;
			uint32_t OF		:1;
			uint32_t IOPL	:2;
			uint32_t NT		:1;
			uint32_t pad3	:1;
			uint16_t pad4;
		};
		uint32_t val;
	} eflags;

	/*GDTR 结构*/
    struct {
		uint16_t limit;
		uint32_t base;
	}gdtr;	
 
    CR0 cr0;
 
	struct {
    	//SegSelector seg;
    	uint16_t val;
		uint32_t base;  // 隐藏部分 (Cache)：段基地址
    	uint32_t limit; // 隐藏部分 (Cache)：段界限
	} seg_regs[NR_SEG_REGS];


} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
	assert(index >= 0 && index < 8);
	return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

// Display CPU register state
void isa_reg_display(void);


#endif

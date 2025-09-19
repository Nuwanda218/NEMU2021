#ifndef __CMP_H__
#define __CMP_H__

#include "cpu/exec/helper.h"

/* 声明带有自动选择 8/16/32 位版本的 helper 函数 */
make_helper(cmp_i2a_v);     // 立即数和累加器比较
make_helper(cmp_i2rm_v);    // 立即数和内存/寄存器比较
make_helper(cmp_r2rm_v);    // 寄存器和内存/寄存器比较
make_helper(cmp_rm2r_v);    // 内存/寄存器和寄存器比较

#endif

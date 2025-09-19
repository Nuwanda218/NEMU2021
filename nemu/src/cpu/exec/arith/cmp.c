#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "cmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "cmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "cmp-template.h"
#undef DATA_BYTE

/* 自动根据操作数大小选择 16/32 位版本 */
make_helper(cmp_r2rm_v) { return (ops_decoded.is_operand_size_16 ? cmp_r2rm_w(eip) : cmp_r2rm_l(eip)); }
make_helper(cmp_rm2r_v) { return (ops_decoded.is_operand_size_16 ? cmp_rm2r_w(eip) : cmp_rm2r_l(eip)); }
make_helper(cmp_i2rm_v) { return (ops_decoded.is_operand_size_16 ? cmp_i2rm_w(eip) : cmp_i2rm_l(eip)); }
make_helper(cmp_i2a_v)  { return (ops_decoded.is_operand_size_16 ? cmp_i2a_w(eip)  : cmp_i2a_l(eip)); }

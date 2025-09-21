#include "cpu/exec/helper.h"

// 为 8/16/32 位分别生成 helper
#define DATA_BYTE 1
#include "test-template.h"
#undef DATA_BYTE

#define DATA_BYTE 2
#include "test-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "test-template.h"
#undef DATA_BYTE

// 根据 is_operand_size_16 选择 _w 还是 _l 版本
make_helper_v(test_rm2r)
make_helper_v(test_i2rm)
make_helper_v(test_i2a)

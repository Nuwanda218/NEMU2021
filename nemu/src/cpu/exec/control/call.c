#include "cpu/exec/helper.h"

/* 将 DATA_BYTE 分别定义为 2 和 4 以实例化 w/l 版本
 * 8086 的 16bit 模式很少用，如果只做 32 位，可只保留 l
 */
#define DATA_BYTE 2
#include "call-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "call-template.h"
#undef DATA_BYTE

/* 根据操作数前缀自动选择 16 或 32 位 */
make_helper_v(call_i)

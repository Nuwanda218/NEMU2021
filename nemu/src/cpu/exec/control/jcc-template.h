#include "cpu/exec/template-start.h"
 
//条件跳转指令译码过程复杂，make_instr_helper无法使用，需要重新定义
#define make_jcc_helper(cc) \
    make_helper(concat4(j, cc, _, SUFFIX)){ \
        int len = concat(decode_si_, SUFFIX)(eip + 1); \
        print_asm(str(concat(j,cc)) " %x",cpu.eip+op_src->val+1+len+(DATA_BYTE ==4)); \
        cpu.eip += (concat(check_cc_, cc)() ? op_src->val : 0);\
        return len + 1; \
    }
 
make_jcc_helper(e)

make_jcc_helper(be)

make_jcc_helper(le)
 
make_jcc_helper(l)

make_jcc_helper(g)

make_jcc_helper(ne)

make_jcc_helper(ge)

make_jcc_helper(a)

make_jcc_helper(s)


#include "cpu/exec/template-end.h"
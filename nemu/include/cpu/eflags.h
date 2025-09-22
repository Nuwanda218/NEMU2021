#ifndef __EFLAGS_H__
#define __EFLAGS_H__

#include "common.h"

void update_eflags_pf_zf_sf(uint32_t);
//je条件的判断，检测ZF标志位
static inline bool check_cc_e(){
        return cpu.eflags.ZF;
}

#endif

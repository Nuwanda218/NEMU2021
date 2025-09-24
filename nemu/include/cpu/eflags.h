#ifndef __EFLAGS_H__
#define __EFLAGS_H__

#include "common.h"

void update_eflags_pf_zf_sf(uint32_t);
//je条件的判断，检测ZF标志位
static inline bool check_cc_e(){
        return cpu.eflags.ZF;
}

//jbe条件的判断，检测CF和ZF标志位
static inline bool check_cc_be(){
        return cpu.eflags.CF | cpu.eflags.ZF;
}

//ne条件的判断，返回非运算的ZF标志位即可
static inline bool check_cc_ne(){
        return !cpu.eflags.ZF;
}

//le条件的判断，检测OF与SF位的异或结果确定是否小于
static inline bool check_cc_le(){
        return (cpu.eflags.OF ^ cpu.eflags.SF) | cpu.eflags.ZF;
}


//l条件的判断
static inline bool check_cc_l(){
        return cpu.eflags.OF ^ cpu.eflags.SF;
}

//g条件的判断
static inline bool check_cc_g(){
        return !((cpu.eflags.OF ^ cpu.eflags.SF) | cpu.eflags.ZF);
}

//ge条件的判断
static inline bool check_cc_ge(){
        return !(cpu.eflags.OF ^ cpu.eflags.SF);
}



#endif

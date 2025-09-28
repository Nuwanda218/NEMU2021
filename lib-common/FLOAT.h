#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "trap.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
        //除以2^16，即右移16位，实现定点数转换回整形
        return (a >> 16);
}
 
static inline FLOAT int2F(int a) {
        //乘2^16，即左移16位，实现整形转换定点数
        return (a << 16);
}
 
static inline FLOAT F_mul_int(FLOAT a, int b) {
        //不需要调用int2F函数，因为两个定点数相乘后要除2^16，和函数转换时候的乘2^16消掉了
        return a * b;
}
 
static inline FLOAT F_div_int(FLOAT a, int b) {
        //直接除就行
        return a / b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT sqrt(FLOAT);
FLOAT pow(FLOAT, FLOAT);

// used when calling printf/sprintf to format a FLOAT argument
#define FLOAT_ARG(f) (long long)f

void init_FLOAT_vfprintf(void);

#endif

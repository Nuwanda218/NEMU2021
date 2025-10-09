#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#ifndef MEMORY_CACHE_H
#define MEMORY_CACHE_H

#include "common.h"

// 声明缓存相关函数
void init_cache();
int read_cache_L1(hwaddr_t addr);
void write_cache_L1(hwaddr_t addr, size_t len, uint32_t data);

#endif // MEMORY_CACHE_H

#define CACHE_b 6  //b
#define CACHE_L1_e 3  //一级高速缓存的e
#define CACHE_L1_s 7  //一级高速缓存的s
#define CACHE_L2_e 4  //二级高速缓存的e
#define CACHE_L2_s 12 //二级高速缓存的s
#define CACHE_L1_CAP (64 * 1024)  //一级高速缓存的C
#define CACHE_L2_CAP (4 * 1024 * 1024) //二级高速缓存的C

#define CACHE_B (1 << CACHE_b)
#define CACHE_L1_E (1 << CACHE_L1_e)
#define CACHE_L1_S (1 << CACHE_L1_s)
#define CACHE_L2_E (1 << CACHE_L2_e)
#define CACHE_L2_S (1 << CACHE_L2_s)
 
//一级高速缓存L1的定义
typedef struct{
    uint8_t data[CACHE_B]; //字节数组存储块内数据
    uint32_t tag;  //标签位
    bool validVal;  //有效位
} L1;
//缓存块数组表示整个一级高速缓存
L1 cache_L1[CACHE_L1_S * CACHE_L1_E];
 
//二级高速缓存L2的定义，与一级高速缓存相比多了一个脏标签
typedef struct{
    uint8_t data[CACHE_B];
    uint32_t tag;
    bool validVal;
    bool dirtyVal;
} L2;
 
L2 cache_L2[CACHE_L2_S * CACHE_L2_E];
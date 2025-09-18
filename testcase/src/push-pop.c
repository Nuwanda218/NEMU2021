#include "trap.h"

uint32_t pushpop(uint32_t v) {
    uint32_t out;
    __asm__ volatile (
        "push %1\n\t"
        "pop  %0\n\t"
        : "=r"(out)
        : "r"(v)
    );
    return out;
}

int data[] = {0, 1, 0xffffffff, 0x7fffffff, 0x80000000};
#define N (sizeof(data)/sizeof(data[0]))

int main() {
    for (int i = 0; i < N; i++) {
        nemu_assert(pushpop(data[i]) == data[i]);
    }
    return 0;
}

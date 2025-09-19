#include "trap.h"

/* Test function for push and pop operations */
uint32_t pushpop(uint32_t v) {
    uint32_t out;
    __asm__ volatile (
        "push %1\n\t"    /* Push input value to stack */
        "pop  %0\n\t"    /* Pop value from stack to output */
        : "=r"(out)      /* Output operand */
        : "r"(v)         /* Input operand */
    );
    return out;
}

/* Test data */
int data[] = {0, 1, 0xffffffff, 0x7fffffff, 0x80000000};
#define N (sizeof(data)/sizeof(data[0]))

int main() {
    int i;
    /* Iterate over test data and verify push/pop correctness */
    for (i = 0; i < N; i++) {
        nemu_assert(pushpop(data[i]) == data[i]);
    }
    return 0;
}

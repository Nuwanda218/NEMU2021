#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <stdio.h>
#include <string.h>


#include "monitor/monitor.h"   // for nemu_state
#include "cpu/reg.h"

#define NR_WP 32

extern int nemu_state;
extern CPU_state cpu;          // CPU registers

static WP wp_pool[NR_WP];
 WP *head = NULL;
static WP *free_ = NULL;   


/* Initialize the watchpoint pool */
void init_wp_pool() {
    int i;
    for (i = 0; i < NR_WP; i++) {
        wp_pool[i].NO = i;
        wp_pool[i].next = &wp_pool[i + 1];
        wp_pool[i].expr[0] = '\0';    // clear expression string
        wp_pool[i].last_val = 0;      // reset last value
    }
    wp_pool[NR_WP - 1].next = NULL;

    head = NULL;     // list of active watchpoints
    free_ = wp_pool; // list of free watchpoints
}

/* Allocate a new watchpoint from free list */
WP* new_wp() {
    if (free_ == NULL) {
        printf("No free watchpoints available!\n");
        return NULL;
    }

    WP *wp = free_;         // take from free list head
    free_ = free_->next;    // advance free list

    // reset data fields
    wp->expr[0] = '\0';
    wp->last_val = 0;

    // insert into active list
    wp->next = head;
    head = wp;

    return wp;
}

/* Free a watchpoint and return it to free list */
void free_wp(WP *wp) {
    if (wp == NULL) return;

    // remove from active list
    if (head == wp) {
        head = wp->next;
    } else {
        WP *prev = head;
        while (prev && prev->next != wp) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = wp->next;
        }
    }

    // add to free list head
    wp->next = free_;
    free_ = wp;
    wp->expr[0] = '\0';
    wp->last_val = 0;
}

/* Add a new watchpoint with given expression */
void add_watchpoint(const char *expr_str) {
    bool success = true;
    uint32_t val = expr((char *)expr_str, &success);
    if (!success) {
        printf("Invalid expression: %s\n", expr_str);
        return;
    }
    WP *wp = new_wp();
    if (!wp) return;

    strncpy(wp->expr, expr_str, sizeof(wp->expr) - 1);
    wp->expr[sizeof(wp->expr) - 1] = '\0';
    wp->last_val = val;

    printf("Watchpoint %d set on \"%s\", initial value = %u\n",
           wp->NO, wp->expr, wp->last_val);
}

/* Delete watchpoint by number */
void delete_watchpoint(int no) {
    WP *p = head;
    while (p) {
        if (p->NO == no) {
            free_wp(p);
            printf("Watchpoint %d deleted.\n", no);
            return;
        }
        p = p->next;
    }
    printf("No such watchpoint: %d\n", no);
}

/* Print all active watchpoints */
void info_watchpoints() {
    if (!head) {
        printf("No watchpoints.\n");
        return;
    }
    printf("Num\tExpr\tValue\n");
	WP *p;
    for (p = head; p; p = p->next) {
        printf("%d\t%s\t%u\n", p->NO, p->expr, p->last_val);
    }
}

/* Check all watchpoints after each instruction */
bool check_watchpoints() {
    WP *wp = head;
    bool success;
    
   

    while (wp) {
        uint32_t new_val = expr(wp->expr, &success);
         printf("[wp] wp%d expr=%s val=%u last=%u\n", wp->NO, wp->expr, new_val, wp->last_val);
        if (!success) {
            printf("Fail to evaluate expression for watchpoint %d: %s\n",
                   wp->NO, wp->expr);
            WP *bad = wp;
            wp = wp->next;
            delete_watchpoint(bad->NO);
            continue;
        }

        /*  detect  0 -> 1  （表达式刚刚成立） */
        if (new_val == 1 && wp->last_val == 0) {
            printf("Hint watchpoint %d at address 0x%08x\n",
                   wp->NO, (uint32_t)cpu.eip);   // 打印当前指令地址
            wp->last_val = new_val;

            /* 一次性断点：触发即删除 */
            WP *to_del = wp;
            wp = wp->next;
            delete_watchpoint(to_del->NO);
            nemu_state = STOP;
            return true;
        }

        /* 其它情况只更新值，不触发 */
        wp->last_val = new_val;
        wp = wp->next;
    }
    return false;
}

/* TODO: Implement the functionality of watchpoint */



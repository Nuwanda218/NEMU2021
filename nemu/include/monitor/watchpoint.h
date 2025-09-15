#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#include <stdint.h>  



typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	/* TODO: Add more members if necessary */
	char expr[256];     // expression string
    uint32_t last_val;  // last evaluated value
} WP;

extern WP *head;

void init_wp_pool(void);
WP* new_wp(void);
void free_wp(WP *wp);

// New functions
bool check_watchpoints(void);
void add_watchpoint(const char *expr_str);
void delete_watchpoint(int no);
void info_watchpoints(void);


#endif

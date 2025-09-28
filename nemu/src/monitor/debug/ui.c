#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "cpu/exec/helper.h"
#include "cpu/reg.h" 

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
    int step = 1;   
    if (args != NULL) {
        step = atoi(args);   
        if (step <= 0) step = 1; 
    }
    cpu_exec(step);  
    return 0;
}

/* Extend info command to show watchpoints */
static int cmd_info(char *args) {
    if (args == NULL) {
        printf("Usage: info r - display registers\n");
        printf("       info w - display watchpoints\n");
        return 0;
    }

    if (strcmp(args, "r") == 0) {
        
        isa_reg_display();
    } else if (strcmp(args, "w") == 0) {
        
        info_watchpoints();
    } else {
        printf("Unknown info command: %s\n", args);
        printf("Supported: info r, info w\n");
    }

    return 0;
}

static int cmd_x(char *args) {
    char *arg1 = strtok(args, " ");
    char *arg2 = strtok(NULL, " ");
    int len;
    swaddr_t address;
    int i;

    if (!arg1 || !arg2) {
        printf("Usage: x N EXPR\n");
        return 0;
    }

    if (sscanf(arg1, "%d", &len) != 1 || len <= 0) {
        printf("Invalid number: %s\n", arg1);
        return 0;
    }

    bool success;
    address = expr(arg2, &success);
    if (!success) {
        printf("Invalid expression: %s\n", arg2);
        return 0;
    }

    printf("0x%08x:", address);
    for (i = 0; i < len; i++) {
        uint32_t val = swaddr_read(address, 4);
        printf(" 0x%08x", val);
        address += 4;
    }
    printf("\n");

    return 0;
}

static int cmd_p(char *args) {
    if (args == NULL) {
        printf("Usage: p EXPR\n");
        return 0;
    }

    bool success = true;
    int32_t result = expr(args, &success);  

    if (success) {
        printf("%s = %d (0x%x)\n", args, result, result);
    } else {
        printf("Evaluation failed for expression: %s\n", args);
    }

    return 0;
}

/* Set a new watchpoint */
static int cmd_w(char *args) {
    if (args == NULL) {
        printf("Usage: w EXPR\n");
        return 0;
    }
    add_watchpoint(args);
    return 0;
}

/* Delete a watchpoint by number */
static int cmd_d(char *args) {
    if (args == NULL) {
        printf("Usage: d N\n");
        return 0;
    }
    int no = atoi(args);
    delete_watchpoint(no);
    return 0;
}

/* Add display backtrace */
static int cmd_bt(char *args) {
	const char* find_fun_name(uint32_t eip);
	struct {
		swaddr_t prev_ebp;
		swaddr_t ret_addr;
		uint32_t args[4];
	} sf;

	uint32_t ebp = cpu.ebp;
	uint32_t eip = cpu.eip;
	int i = 0;
	while(ebp != 0) {
		sf.args[0] = swaddr_read(ebp + 8, 4);
		sf.args[1] = swaddr_read(ebp + 12, 4);
		sf.args[2] = swaddr_read(ebp + 16, 4);
		sf.args[3] = swaddr_read(ebp + 20, 4);

		printf("#%d 0x%08x in %s (0x%08x 0x%08x 0x%08x 0x%08x)\n", i, eip, find_fun_name(eip), sf.args[0], sf.args[1], sf.args[2], sf.args[3]);
		i ++;
		eip = swaddr_read(ebp + 4, 4);
		ebp = swaddr_read(ebp, 4);
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
    { "si", "Step execute N instructions (usage: si [N], default N=1)", cmd_si },
	{ "info", "Display program status or watchpoints (usage: info r|w)", cmd_info },
	{ "x", "Scan memory. Print N 4-byte values starting at the address computed by EXPR (usage: x N EXPR, default N=1)", cmd_x },
	{ "p", "Evaluate and print the value of an expression (usage: p EXPR)", cmd_p },
	{ "w", "Set a watchpoint (usage: w EXPR)", cmd_w },
	{ "d", "Delete a watchpoint by number (usage: d N)", cmd_d },
    { "bt", "Print the StackFrame Chain", cmd_bt},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}

#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


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

static int cmd_info(char *args) {
    char *arg = strtok(NULL, " ");
    int i;                          /* C89 requirment */

    if (arg && strcmp(arg, "r") == 0) {
        for (i = 0; i < 8; i++) {
            printf("%s 0x%08x %d\n", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
        }
        return 0;
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
	{ "info", "Display program status (usage: info r)", cmd_info },
	{ "x", "Scan memory. Print N 4-byte values starting at the address computed by EXPR (usage: x N EXPR, default N=1)", cmd_x },
	{ "p", "Evaluate and print the value of an expression (usage: p EXPR)", cmd_p },


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

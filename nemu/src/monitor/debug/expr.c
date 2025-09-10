#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
enum {
	NOTYPE = 256, EQ

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"\\-", '-'},                   // minus
    {"==", EQ},                     // equal
    {"0[xX][0-9a-fA-F]+", 'H'},    // hex number
    {"[0-9]+", 'D'},                // decimal number
    {"\\$[a-zA-Z]+", 'R'},          // registers
    {"\\(", '('},                   // left parenthesis
    {"\\)", ')'}                    // right parenthesis
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

/* Check register value */
static bool get_reg_val(const char *reg, uint32_t *val) {
    if(strcmp(reg, "$eax")==0) *val = cpu.eax;
    else if(strcmp(reg, "$ecx")==0) *val = cpu.ecx;
    else if(strcmp(reg, "$edx")==0) *val = cpu.edx;
    else if(strcmp(reg, "$ebx")==0) *val = cpu.ebx;
    else if(strcmp(reg, "$esp")==0) *val = cpu.esp;
    else if(strcmp(reg, "$ebp")==0) *val = cpu.ebp;
    else if(strcmp(reg, "$esi")==0) *val = cpu.esi;
    else if(strcmp(reg, "$edi")==0) *val = cpu.edi;
    else return false;
    return true;
}

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
    				case NOTYPE:
        				// Ignore spaces
        				break;
    				case EQ:
    				case '+': case '-': case '*': case '/': case '(': case ')':
        				tokens[nr_token].type = rules[i].token_type;
        				strncpy(tokens[nr_token].str, substr_start, substr_len);
        				tokens[nr_token].str[substr_len] = '\0';
        				nr_token++;
        				break;
    				case 'D':  // decimal number
    				case 'H':  // hex number
        				tokens[nr_token].type = 'N';  // 统一数字类型
        				strncpy(tokens[nr_token].str, substr_start, substr_len);
        				tokens[nr_token].str[substr_len] = '\0';
        				nr_token++;
        				break;
    				case 'R':  // register
        				tokens[nr_token].type = 'R';
        				strncpy(tokens[nr_token].str, substr_start, substr_len);
        				tokens[nr_token].str[substr_len] = '\0';
        				nr_token++;
        					break;
    				default:
        				panic("Unknown token type in make_token");
	    }


				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

/* Recursive evaluation */
static uint32_t eval(int p, int q, bool *success) {
	if (p > q) { *success = false; return 0; }
	else if (p == q) {
		if (tokens[p].type == 'D') {
			uint32_t val;
			if (tokens[p].str[0]=='0' && (tokens[p].str[1]=='x' || tokens[p].str[1]=='X'))
				sscanf(tokens[p].str, "%x", &val);
			else
				sscanf(tokens[p].str, "%u", &val);
			return val;
		} else if (tokens[p].type == 'R') {
			uint32_t val;
			if (!get_reg_val(tokens[p].str, &val)) { *success = false; return 0; }
			return val;
		} else { *success = false; return 0; }
	} else if (tokens[p].type == '(' && tokens[q].type == ')') {
		return eval(p+1, q-1, success);
	} else {
		// find main operator
		int op = -1;
		int level = 0;
		int i;
		for (i = p; i <= q; i++) {
			if (tokens[i].type == '(') level++;
			else if (tokens[i].type == ')') level--;
			else if (level == 0) {
				if (tokens[i].type == '+' || tokens[i].type == '-') op = i;
			}
		}
		if (op == -1) { *success = false; return 0; }
		uint32_t val1 = eval(p, op-1, success);
		if (!*success) return 0;
		uint32_t val2 = eval(op+1, q, success);
		if (!*success) return 0;
		switch(tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			default: *success = false; return 0;
		}
	}
}



uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	*success = true;

    if (nr_token == 1 && tokens[0].type == 'N') {
        if (tokens[0].str[1] == 'x' || tokens[0].str[0] == '0') {
        
            return strtoul(tokens[0].str, NULL, 0);
        } else {
            return (uint32_t)atoi(tokens[0].str);
        }
    }

	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token-1, success);
}


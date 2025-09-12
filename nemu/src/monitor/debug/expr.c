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
#include "memory/memory.h"  
enum {
	NOTYPE = 256, 
	EQ, NEQ, // "==" (provided in the framework, can ignore for now)
    AND, OR,
    LT, GT, LE, GE,
    NUM,   // number (decimal or hex)
    REG,    // register
    DEREF,     // unary * (memory dereference)
    NOT        // logical !

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
    {"\\|\\|", OR},                 /* === Added: logical OR === */
    {"&&", AND},                    /* === Added: logical AND === */
	{"\\+", '+'},					// plus
	{"\\-", '-'},                   // minus
	{"\\*", '*'},                   // multiply
    {"\\/", '/'},                   // divide
    {"==", EQ},                     // equal
    {"!=", NEQ},                    /* === Added: not equal === */
    {"<=", LE}, {">=", GE},         /* === Added: <= >= === */
    {"<",  LT}, {">",  GT},         /* === Added: <  > === */
    {"!",  NOT},                    /* === Added: logical NOT === */
    {"0[xX][0-9a-fA-F]+", NUM},  // hex number
    {"[0-9]+", NUM},              // decimal number
    {"\\$[a-z]+", REG},        // registers
    {"\\(", '('},                 // left parenthesis
    {"\\)", ')'}                  // right parenthesis
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

Token tokens[128];
int nr_token = 0;

/* Check register value */

/* Check register value: support 32-bit, 16-bit, and 8-bit registers */
static bool get_reg_val(const char *reg, uint32_t *val) {
    if (strcmp(reg, "$eax") == 0) *val = cpu.eax;
    else if (strcmp(reg, "$ax") == 0) *val = cpu.eax & 0xFFFF;
    else if (strcmp(reg, "$al") == 0) *val = cpu.eax & 0xFF;
    else if (strcmp(reg, "$ah") == 0) *val = (cpu.eax >> 8) & 0xFF;

    else if (strcmp(reg, "$ebx") == 0) *val = cpu.ebx;
    else if (strcmp(reg, "$bx") == 0) *val = cpu.ebx & 0xFFFF;
    else if (strcmp(reg, "$bl") == 0) *val = cpu.ebx & 0xFF;
    else if (strcmp(reg, "$bh") == 0) *val = (cpu.ebx >> 8) & 0xFF;

    else if (strcmp(reg, "$ecx") == 0) *val = cpu.ecx;
    else if (strcmp(reg, "$cx") == 0) *val = cpu.ecx & 0xFFFF;
    else if (strcmp(reg, "$cl") == 0) *val = cpu.ecx & 0xFF;
    else if (strcmp(reg, "$ch") == 0) *val = (cpu.ecx >> 8) & 0xFF;

    else if (strcmp(reg, "$edx") == 0) *val = cpu.edx;
    else if (strcmp(reg, "$dx") == 0) *val = cpu.edx & 0xFFFF;
    else if (strcmp(reg, "$dl") == 0) *val = cpu.edx & 0xFF;
    else if (strcmp(reg, "$dh") == 0) *val = (cpu.edx >> 8) & 0xFF;

    else if (strcmp(reg, "$esi") == 0) *val = cpu.esi;
    else if (strcmp(reg, "$di") == 0) *val = cpu.edi & 0xFFFF; // 16-bit alias
    else if (strcmp(reg, "$edi") == 0) *val = cpu.edi;
    else if (strcmp(reg, "$si") == 0) *val = cpu.esi & 0xFFFF; // 16-bit alias

    else if (strcmp(reg, "$esp") == 0) *val = cpu.esp;
    else if (strcmp(reg, "$sp") == 0) *val = cpu.esp & 0xFFFF; // 16-bit alias

    else if (strcmp(reg, "$ebp") == 0) *val = cpu.ebp;
    else if (strcmp(reg, "$bp") == 0) *val = cpu.ebp & 0xFFFF; // 16-bit alias

    else if (strcmp(reg, "$eip") == 0) *val = cpu.eip;

    else return false;

    return true;
}



/* Mark '*' as DEREF if it is a unary operator (memory dereference) */
static void mark_deref() {
    int i;
    for (i = 0; i < nr_token; i++) {
        if (tokens[i].type == '*') {
            if (i == 0 ||
                tokens[i-1].type == '+'  || tokens[i-1].type == '-'  ||
                tokens[i-1].type == '*'  || tokens[i-1].type == '/'  ||
                tokens[i-1].type == '('  || tokens[i-1].type == AND  ||
                tokens[i-1].type == OR   || tokens[i-1].type == EQ   ||
                tokens[i-1].type == NEQ  || tokens[i-1].type == LT   ||
                tokens[i-1].type == LE   || tokens[i-1].type == GT   ||
                tokens[i-1].type == GE   || tokens[i-1].type == NOT) {
                tokens[i].type = DEREF;
            }
        }
    }
}


static bool make_token(char *e) {
	int position = 0;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		bool matched = false;
		int i;
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
				matched = true;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

                  /* bounds checks */
                if (substr_len >= (int)sizeof(tokens[0].str)) {
                    printf("token too long at position %d: \"%.*s\"\n", position - substr_len, substr_len, substr_start);
                    return false;
                }
                if (nr_token >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
                    printf("too many tokens (increase tokens[] size)\n");
                    return false;
                }

				switch(rules[i].token_type) {
    				case NOTYPE:
        				// Ignore spaces
        				break;
                    /* logical/relational operators */
                    case AND: case OR:
                    case EQ: case NEQ:
                    case LT: case GT: case LE: case GE:
                    /* arithmetic and parentheses and logical NOT */
                    case '+': case '-': case '*': case '/':
                    case '(': case ')':
                        tokens[nr_token].type = rules[i].token_type;
                        strncpy(tokens[nr_token].str, substr_start, substr_len);
                        tokens[nr_token].str[substr_len] = '\0';
                        nr_token++;
                        break;
                    case NOT:
                    tokens[nr_token].type = NOT;  
                    strncpy(tokens[nr_token].str, substr_start, substr_len);
                    tokens[nr_token].str[substr_len] = '\0';
                    nr_token++;
                        break;
    				case NUM:
                        // Decimal or hex number
        				tokens[nr_token].type = NUM;  
        				strncpy(tokens[nr_token].str, substr_start, substr_len);
        				tokens[nr_token].str[substr_len] = '\0';
        				nr_token++;
        				break;

    				case REG:
                        // Register
						tokens[nr_token].type = REG;
    					strncpy(tokens[nr_token].str, substr_start, substr_len);
   					 	tokens[nr_token].str[substr_len] = '\0';
    					nr_token++;
   						break;

    				default:
                        /* Defensive logging before panic to ease debugging */
                        printf("Unknown token type %d matched for \"%.*s\"\n", rules[i].token_type, substr_len, substr_start);
                        panic("Unknown token type in make_token");
                }


				break;// Once matched, break the for-loop
			}
		}

		if (!matched) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
    mark_deref(); /* === Added: must run after tokenization === */
	return true; 
}

// Check if the entire token range [p, q] is wrapped by a pair of matching parentheses
static bool check_parentheses(int p, int q) {
    if (tokens[p].type != '(' || tokens[q].type != ')') return false;
    int level = 0;
	int i;
    for (i = p; i <= q; i++) {
        if (tokens[i].type == '(') level++;
        else if (tokens[i].type == ')') level--;
        // If the outermost pair does not cover the entire range
        if (level == 0 && i < q) return false;
    }
    return level == 0;
}

/* Recursive evaluation */
static int32_t eval(int p, int q, bool *success) {
	if (p > q) { *success = false; return 0; }

	// 1. Single token
    if (p == q) {
        if (tokens[p].type == NUM) {
            return (int32_t)strtol(tokens[p].str, NULL, 0);
        } else if (tokens[p].type == REG) {
            uint32_t val;
            if (!get_reg_val(tokens[p].str, &val)) {
                *success = false;
                return 0;
            }
            return (int32_t)val;
        } else {
            *success = false;
            return 0;
        }
	}
    
	// 2. Check parentheses
    if (check_parentheses(p, q)) {
        return eval(p + 1, q - 1, success);
    }
	  
	 /* handle unary operators */
  if (tokens[p].type == DEREF) {
    // Strip parentheses around subexpression
    if (check_parentheses(p + 1, q)) {
        p = p + 1;
        q = q - 1;
    }

    // Evaluate address inside DEREF
    bool addr_success = true;
    int32_t addr = eval(p + 1, q, &addr_success);  // 注意 p+1
    if (!addr_success) {
        *success = false;
        return 0;
    }

    int32_t val = swaddr_read(addr, 4);
    return val;
}





   if (tokens[p].type == '-' &&
    (p == 0 || tokens[p-1].type == '(' || tokens[p-1].type == AND || 
     tokens[p-1].type == OR || tokens[p-1].type == EQ || tokens[p-1].type == NEQ ||
     tokens[p-1].type == LT || tokens[p-1].type == LE || tokens[p-1].type == GT ||
     tokens[p-1].type == GE)) {
    int32_t val = eval(p + 1, q, success);
    return -val;
}

    if (tokens[p].type == NOT) {
        int32_t val = eval(p + 1, q, success);
        return !val;
    }

	// 4. Find main operator at the outermost level
    int op = -1;
    int level = 0;
	
	  // Operator precedence from lowest to highest: OR > AND > EQ/NEQ > < <= > >= > + - > * /
     int precedence[][2] = {
        {OR, OR}, {AND, AND}, {EQ, NEQ}, {LT, GE}, {'+', '-'}, {'*', '/'}
    };
    int pri;
    for (pri = 0; pri < 6; pri++) {
        level = 0;
        int i;
        for (i = q; i >= p; i--) {
            if (tokens[i].type == ')') level++;
            else if (tokens[i].type == '(') level--;
            else if (level == 0) {
                if (tokens[i].type == precedence[pri][0] || tokens[i].type == precedence[pri][1]) {
                    op = i;
                    break;
                }
            }
        }
        if (op != -1) break;
    }
	if (op == -1) { *success = false; return 0; }

	 // 5. Recursively evaluate left and right subexpressions
    int32_t val1 = eval(p, op - 1, success);
    if (!*success) return 0;
    int32_t val2 = eval(op + 1, q, success);
    if (!*success) return 0;

	 // 6. Apply the operator
    switch (tokens[op].type) {
        case OR:  return val1 || val2;
        case AND: return val1 && val2;
        case EQ:  return val1 == val2;
        case NEQ: return val1 != val2;
        case LT:  return val1 <  val2;
        case LE:  return val1 <= val2;
        case GT:  return val1 >  val2;
        case GE:  return val1 >= val2;
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': 
            if (val2 == 0) { *success = false; return 0; }
            return val1 / val2;
        default:
            *success = false;
            return 0;
    }
}



int32_t expr(char *e, bool *success) {
	// 1. Tokenize the input expression
    if (!make_token(e)) {
        *success = false;
        return 0;
    }

    *success = true;

	// 2. Handle single token (number or register)
    if (nr_token == 1) {
        if (tokens[0].type == NUM) {
            return (int32_t)strtol(tokens[0].str, NULL, 0);  // parse decimal or hex
        } else if (tokens[0].type == REG) {
            uint32_t val;
            if (!get_reg_val(tokens[0].str, &val)) {
                *success = false;
                return 0;
            }
            return (int32_t)val;
        }
    }
    
	 // 3. Evaluate the full expression recursively
    int32_t result = eval(0, nr_token - 1, success);

	/* TODO: Insert codes to evaluate the expression. */
	return result;
}

// [PA1 stage2 mandatory task 3]
// Run test cases for arithmetic expression lexical analysis
// Print all tokens of the current expression
static void print_tokens() {
    printf("Tokens:\n");
	int i;
    for (i = 0; i < nr_token; i++) {
        printf("  %d: type=%d, str=\"%s\"\n", i, tokens[i].type, tokens[i].str);
    }
}

// Run some test expressions
void test_expr() {
    
    cpu.eax = 0x100;

   
    swaddr_write(0x100, 4, 42);

    const char *tests[] = {
        "*0x100",        
        "*($eax)",       
        "*($eax + 4)",   
        "*0x100 + 10",   
        "-*0x100",      
        NULL
    };
    int i;
    for (i = 0; tests[i] != NULL; i++) {
        bool success = true;
        printf("\n==== Test %d: \"%s\" ====\n", i + 1, tests[i]);

        /* Evaluate the expression */
        int32_t result = expr((char *)tests[i], &success);

        /* Print tokens after parsing for debug */
        print_tokens();

        /* Show evaluation result */
        if (success) {
            printf("Result = %d (0x%x)\n", result, result);
        } else {
            printf("Evaluation failed!\n");
        }
    }
}


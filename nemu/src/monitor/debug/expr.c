#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256,

  /* TODO: Add more token types */
	TK_LS, TK_LE, TK_EQ, TK_NE, TK_GE, TK_GT,
	TK_NOT, TK_AND,TK_OR,
	TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_MOD,
	TK_LP, TK_RP,
	TK_NUM, TK_REG, TK_REF
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},	// spaces
  {"&&", TK_AND},		// and
  {"\\|\\|", TK_OR},	// or
  {"<=", TK_LE},		// less equal
  {">=", TK_GE},		// greater equal
  {"==", TK_EQ},		// equal
  {"!=", TK_NE},		// not equal
  {"<", TK_LS},			// less
  {">", TK_GT},			// greater
  {"!", TK_NOT},		// not
  {"\\(", TK_LP},		// (
  {"\\)", TK_RP},		// )
  {"@", TK_REF},		// @
  {"\\+", TK_ADD},		// add
  {"-", TK_SUB},		// sub
  {"\\*", TK_MUL},		// mul
  {"/", TK_DIV},		// div
  {"%", TK_MOD},		// mod
  {"\\$[a-z]{3,3}", TK_REG},
  {"(0[xX][0-9a-fA-F]+)|([1-9][0-9]+)|([0-7]+)", TK_NUM},
  /* {"[a-zA-Z_][0-9a-zA-Z_]*", TK_EXP} */
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

  for (i = 0; i < NR_REGEX; i ++) {
	ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, sizeof(error_msg));
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
	int type;
	uint32_t value;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  if (e == NULL) return false;
  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

		int type = rules[i].token_type;
		if (type == TK_NOTYPE) continue;
		if (nr_token < 32) tokens[nr_token].type = type;
		else assert(0); // Out of range of length of regexp tokens
        if (type == TK_NUM || type == TK_REG) {
			char p = e[position];
			e[position] = '\0';
			if(type == TK_NUM) {
				int v;
				if (substr_start[0] == '0') {
					if (substr_start[1] == 'x' || substr_start[1] == 'X')
						sscanf(substr_start, "%x", &v);
					else sscanf(substr_start, "%o", &v);
				}
				else sscanf(substr_start, "%d", &v);
				tokens[nr_token].value = v;
			}
			else {
				int i;
				for (i = R_EAX; i <= R_EDI; ++i) {
					if (strcmp(substr_start + 1, regsl[i]) == 0) {
						tokens[nr_token].value = i;
						break;
					}
				}
				if (i <= R_EDI) ;
				else if (strcmp(substr_start, "$eip") == 0)
					tokens[nr_token].value = i;
				else return false;
			}
			e[position] = p;
		}
		switch (type) {
			default: ;//TODO();
        }
		++nr_token;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int prior(int type) {
	int p;
	switch(type) {
		case TK_OR: p = 0; break;
		case TK_AND: p = 1; break;
		case TK_LE: case TK_LS: case TK_EQ: case TK_NE: case TK_GE:	case TK_GT:
			p = 2; break;
		case TK_ADD: case TK_SUB: p = 3; break;
		case TK_MUL: case TK_DIV: case TK_MOD: p = 4; break;
		case TK_REF: p = 5; break;
		case TK_NOT: p = 6; break;
		case TK_LP: p = 7; break;
		case TK_RP: p = 8; break;
		case TK_NUM: case TK_REG: p = 9; break; 
		default: p = 0xf;
	}
	return p;
}

static uint32_t cal(int type, uint32_t a, uint32_t b) {
	uint32_t result = a;
	switch(type) {
		case TK_ADD: result += b; break;
		case TK_SUB: result -= b; break;
		case TK_MUL: result *= b; break;
		case TK_DIV: result /= b; break;
		case TK_MOD: result %= b; break;
		case TK_AND: result = a && b; break;
		case TK_OR:  result = a || b; break;
		case TK_NOT: result = !a; break;
		case TK_LE: result = a <= b; break;
		case TK_LS: result = a < b; break;
		case TK_EQ: result = a == b; break;
		case TK_NE: result = a != b; break;
		case TK_GE: result = a >= b; break;
		case TK_GT: result = a > b; break;
		case TK_REG: result = (a <= R_EDI ? reg_l(a) : cpu.eip); break;
		case TK_REF: result = vaddr_read(a, 4); break;
		default: ;
	}
	return result;
} 

uint32_t expr_cal(bool *suc, int begin, int end) {
	int par = 0, op = end, result = 0, num = 0;printf("%d,%d\n",begin,end);
	if (begin >= end) { *suc = false; return 0; }
	else if (begin + 1 == end) {
		int type = tokens[begin].type;
		*suc = true;
		if (type == TK_NUM) result = tokens[begin].value;
		else if (type == TK_REG) result = cal(type, tokens[begin].value, 0);
		else *suc = false;
		return result;
	}
	for (int i = begin; i < end; ++i) {
		int type = tokens[i].type;
		if (type == TK_LP) ++par;
		else if (type == TK_RP) --par;
		else if (par == 0 && prior(type) < 7) {
			if (op == end) op = i;
			else if (prior(type) < prior(tokens[op].type)) { op = i;num = 1; }
			else if (prior(type) == prior(tokens[op].type)) ++num;
		}
	}
	if (par == 0) {
		if (op == end) result = expr_cal(suc, begin + 1, end - 1);
		else {
			if (op != begin) result = expr_cal(suc, begin, op);
			if (!*suc) return 0;
			for (int i = op + 1; i <= end; ++i) {
				if (i == end || prior(tokens[i].type) == prior(tokens[op].type)) {
					int y = expr_cal(suc, op + 1, i);
					if (!*suc) return 0;
					if (tokens[op].type == TK_REF || tokens[op].type == TK_NOT)
						result = cal(tokens[op].type, y, 0);
					else {
						result = cal(tokens[op].type, result, y);
						op = i;
					}
				}
			}
		}
	}
	else *suc = false;
	return result;
}

uint32_t expr(char *e, bool *success) {
	if (!make_token(e)) {
		*success = false;
		return 0;
	}
	/* TODO: Insert codes to evaluate the expression. */
	
	return expr_cal(success, 0, nr_token);
}

/*
||,&&,<,+,*,@,!,(,)
H -> H||I | I
I -> I&&J | J
J -> J<E | E

E -> E+T | T
T -> T*F | F
F -> @P | !P | P
P -> (H) | e


static char opg_table[][10] = {
// ||,&&,<,+,*,@,!,(,),e
	{ '>', '<', '<', '<', '<', '<', '<', '<', '>', '<' },
	{ '>', '>', '<', '<', '<', '<', '<', '<', '>', '<' },
	{ '>', '>', '>', '<', '<', '<', '<', '<', '>', '<' },
	{ '>', '>', '>', '>', '<', '<', '<', '<', '>', '<' },
	{ '>', '>', '>', '>', '>', '<', '<', '<', '>', '<' },
	{ '>', '>', '>', '>', '>', ' ', '<', '<', '>', '<' },
	{ '>', '>', '>', '>', '>', ' ', '<', '<', '>', '<' },
	{ '<', '<', '<', '<', '<', '<', '<', '<', '=', '<' },
	{ '>', '>', '>', '>', '>', ' ', ' ', ' ', '>', ' ' },
	{ '>', '>', '>', '>', '>', ' ', ' ', ' ', '>', ' ' }
};

uint32_t opg_handle(bool *success) {
	int top = -1;
	Token stack[32];
	printf("%d\n", tokens[0].value);
	for (int i = 1; i < nr_token; ++i) {
		int j = top + 1, p;
		do {
			--j;
			p = prior(stack[j].type);
		} while (j >= 0 && p != 0xf);
		if (j <0 || opg_table[p][i] == '<') {
			++top;
			stack[top].type = tokens[i].type;
			if (tokens[i].type == TK_REG)
				stack[top].value = cal(TK_REG, tokens[i].value, 0);
			else stack[top].value = tokens[i].value;
		}
		else if (opg_table[p][i] == '>') {
			if (tokens[i].type == TK_RP) {
				stack[top-2].type = stack[top-1].type;
				stack[top-2].value = stack[top-1].value;
				top -=2;
			}
			else if (stack[top-1].type == TK_REF || stack[top-1].type == TK_NOT) {
				--top;
				stack[top].value = cal(stack[top].type, stack[top].value, 0);
				stack[top].type = TK_NUM;
			}
			else if (prior(stack[top-1].type) != 0xf) {
				top -=2;
				stack[top].value = cal(stack[top+1].type, stack[top+1].value, stack[top+2].value);
				stack[top].type = TK_NUM;
			}
			else assert(0);
		}
		else assert(0);
	}
	return stack[top].value;
}
*/

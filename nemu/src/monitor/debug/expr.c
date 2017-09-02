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
	TK_NUM, TK_REG
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
/*
static int find_op(int begin, int end) {
	int pos = end, par = 0;
	int type = tokens[begin].token_type;
	if (type == TK_LP) ++par;
	if (type != TK_NUM || type != TK_EXP)
}
static uint32_t eval(int begin, int end) {
	int type;
	if (begin >= end) panic("Error in function eval.");
	type = tokens[begin].token_type;
	if (type == TK_NUM) {
		
	}
	else if ()
}
*/
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return nr_token;

  return 0;
}

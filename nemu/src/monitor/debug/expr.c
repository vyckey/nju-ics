#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,

  /* TODO: Add more token types */
	TK_NOT, TK_LS, TK_LE, TK_EQ, TK_NE, TK_GE, TK_GT,
	TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_MOD,
	TK_LP, TK_RP,
	TK_DOLAR,
	TK_NUM, TK_EXP
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},		// spaces
  {"<=", TK_LE},			// less
  {">=", TK_GE},
  {"==", TK_EQ},		// equal
  {"!=", TK_NE},		// not equal
  {"<", TK_LS},
  {">", TK_GT},
  {"!", TK_NOT},
  {"\\(", TK_LP},
  {"\\)", TK_RP},
  {"\\+", TK_ADD},			// add
  {"-", TK_SUB},			// sub
  {"\\*", TK_MUL},			// mul
  {"/", TK_DIV},			// div
  {"%", TK_MOD},
  {"\\$", TK_DOLAR},
  {"{0[xX]}?[0-9]+", TK_NUM},
  {"[a-zA-Z_][0-9a-zA-Z_]*", TK_EXP}
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
  char str[32];
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

		if (rules[i].token_type == TK_NOTYPE) continue;
		if (nr_token < 32) tokens[nr_token].type = rules[i].token_type;
		else panic("Out of range of length of regexp tokens\n");
        switch (rules[i].token_type) {
			case TK_NUM: case TK_EXP:
				if (substr_len >= sizeof(tokens[0].str))
					panic("Too long match string\n");
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				tokens[nr_token].str[substr_len] = '\0';
				break;
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return nr_token;

  return 0;
}

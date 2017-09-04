#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

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

static void cmd_error(int error_no, const char *cmd) {
	switch (error_no) {
		case 0:
			printf("Unknown command '%s'\n", cmd);
			break;
		case 1:
			printf("Unknown argments in command '%s'\n", cmd);
			break;
		case 2:
			printf("Invalid argments in command '%s'\n", cmd);
			break;
		case 3:
			printf("Too few argments in command '%s'\n", cmd);
			break;
		default:
			printf("Unknown error_no\n");
	}
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
	int n;
	char *arg1 = strtok(NULL, " ");
	n = ((arg1 == NULL) ? 1 : atoi(arg1));
	if (n < 1) cmd_error(2, "si");
	else cpu_exec(n);
	return 0;
}

static int cmd_info(char *args) {
	char *arg1 = strtok(NULL, " ");
	if (arg1 == NULL) cmd_error(3, "info");
	else if (strcmp(arg1, "r") == 0) {
		printf("eip\t0x%08x\n", cpu.eip);
		for (int i = R_EAX; i <= R_EDI; ++i) {
			printf("%s\t%d\t0x%08x\n", regsl[i], reg_l(i), reg_l(i));
		}
	}
	else if (strcmp(arg1, "w") == 0) {
		list_wp();
	}
	else cmd_error(1, "info");
	return 0;
}


static int cmd_p(char *args) {
	bool success;
	uint32_t value = expr(args, &success);
	if (success) printf("%d\n", value);
	else printf("Invalid expr\n");
	return 0;
}


static int cmd_x(char *args) {
	return 0;
}


static int cmd_w(char *args) {
	char *str = malloc(sizeof(char) * (strlen(args) + 1));
	strcpy(str,args);
	if (new_wp(str)) printf("Had add watchpoint.\n");
	else printf("Failed.\n");
	return 0;
}


static int cmd_d(char *args) {
	char *arg1 = strtok(NULL, " ");
	unsigned int no = atoi(arg1);
	if (arg1) {
		if (free_wp(no)) printf("Had delete watchpoint %d.\n", no);
		else printf("Failed\n");
	}
	else clear_wp();
	return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *usage;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "help [command]", "Display informations about all supported commands", cmd_help },
  { "c", NULL, "Continue the execution of the program", cmd_c },
  { "q", NULL, "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
	{ "si", "si [n] (n is number of instructions)", "Step one instruction exactly.", cmd_si },
	{ "info", "info r/w", "Describe register or watchpoint status information.", cmd_info },
	{ "p", "p EXP", "Print value of expression EXP.", cmd_p },
	{ "x", "x n EXP (n is the number of 4B)", "Print value of address.", cmd_x },
	{ "w", "w EXP", "Set a watchpoint for an expression.", cmd_w },
	{ "d", "d n (n is the id of watchpoint)", "Delete nth watchpoint.", cmd_d }
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        if (cmd_table[i].usage) printf("Usage: %s\n", cmd_table[i].usage);
		return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  } 
  
  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);
    
	/* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }
    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
     if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
       }
    } 

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

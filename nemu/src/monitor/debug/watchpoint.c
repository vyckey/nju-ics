#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include <stdlib.h>

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
	wp_pool[i].expr_str = NULL;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *str) {
	WP *cur = NULL;
	if (free_) {
		cur = free_;
		free_ = free_->next;
		cur->next = head;
		head = cur;
		head->expr_str = str;
	}
	return cur;
}

void free_wp(WP *wp) {
	if (wp) {
		WP *pre = head;
		if (head == wp) head = wp->next;
		else {
			while (pre && pre->next != wp) pre = pre->next;
			if (pre) pre->next = wp->next;
			else assert(0); // error
		}
		wp->next = free_;
		free_ = wp;
		if (free_->expr_str) free(free_->expr_str);
	}
}

void travel_wp(void (*pfunc)(WP*)) {
	WP *p = head;
	while (p) {
		pfunc(p);
		p = p->next;
	}
}

void wp_clear(WP *p) { if (p->expr_str) free(p->expr_str); }
void wp_update(WP *p) { }

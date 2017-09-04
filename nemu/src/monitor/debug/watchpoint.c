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

void free_wp(unsigned int no) {
	WP *pre, *p;
	pre = p = head;
	while (p) {
		if (p->NO == no) {
			if (pre == head) head = p->next;
			else pre->next = p->next;
			p->next = free_;
			free_ = p;
			if (free_->expr_str) free(free_->expr_str);
			free_->expr_str = NULL;
		}
		pre = p;
		p = p->next;
	}
}

void clear_wp() {
	WP *p = head;
	while (p) {
		if (p->expr_str) {
			free(p->expr_str);
			p->expr_str = NULL;
		}
		p = p->next;
	}
	init_wp_pool();
}

void list_wp() {
	WP *p = head;
	while (p) {
		bool b;
		printf("Watchpoint %d\nExpr: %s\nLast value: 0x%x\nCurrent value: %x\n", p->NO, p->expr_str, p->last_value, expr(p->expr_str, &b));
		p = p->next;
	}
}

void update_wp() {
	WP *p = head;
	while (p) {
		bool b;
		uint32_t new_value = expr(p->expr_str, &b);
		if (p->last_value != new_value) {
			printf("Watchpoint %d\nExpr: %s\nLast value: 0x%x\nCurrent value: %x\n", p->NO, p->expr_str, p->last_value, new_value);
			p->last_value = new_value;
		}
		p = p->next;
	}
}

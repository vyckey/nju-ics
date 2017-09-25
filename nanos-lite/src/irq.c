#include "common.h"
#include "proc.h"

_RegSet* do_syscall(_RegSet *r);
_RegSet* schedule(_RegSet *prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
	_RegSet *ret = NULL;
 	switch (e.event) {
		case _EVENT_SYSCALL: do_syscall(r); /*ret = schedule(r);*/ break;
		case _EVENT_TRAP: Log("trap event..."); ret = schedule(r); break;
		case _EVENT_IRQ_TIME: Log("time interupt..."); ret = schedule(r); break;
		default: panic("Unhandled event ID = %d", e.event);
	}

  return ret;
}

void init_irq(void) {
  _asye_init(do_event);
}

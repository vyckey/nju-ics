#include "cpu/exec.h"

make_EHelper(call_rel32) {
	print_asm("call %x", id_dest->imm);
}

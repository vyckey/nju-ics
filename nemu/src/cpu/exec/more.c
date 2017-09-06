#include "cpu/exec.h"

make_EHelper(call_rel32) {
	decoding.is_jmp = true;
	print_asm("call %x", id_dest->imm + *eip);
}

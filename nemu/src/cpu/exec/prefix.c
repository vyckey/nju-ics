#include "cpu/exec.h"

make_EHelper(real);

make_EHelper(operand_size) {
	printf("0x%x\n", *eip);
  decoding.is_operand_size_16 = true;
  exec_real(eip);
  decoding.is_operand_size_16 = false;
}

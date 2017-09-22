#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  rtl_li(&t0, id_dest->addr);
  rtl_lm(&t1, &t0, 2);
  cpu.idtr.limit = t1;
  rtl_addi(&t0, &t0, 2);
  rtl_lm(&t1, &t0, 4);
  cpu.idtr.base = t1;

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  switch (id_dest->reg) {
    case 0: cpu.cr0 = id_src->val; break;
    case 3: cpu.cr3 = id_src->val; break;
    default: panic("Invalid CR exec");
  }printf("fdsfs %x %x\n", cpu.eax, id_src->val);

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  switch (id_dest->reg) {
    case 0: operand_write(id_dest, &cpu.cr0); break;
    case 3: operand_write(id_dest, &cpu.cr3); break;
    default: panic("Invalid CR exec");
  }

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  void raise_intr(uint8_t NO, vaddr_t ret_addr);
  // no implement all, read i386
  raise_intr(id_dest->val, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  // no implement all, read i386
  rtl_pop(&t0);
  decoding.jmp_eip = t0;
  decoding.is_jmp = 1;
  rtl_pop(&t0);
  cpu.cs = t0;
  rtl_pop(&t0);
  cpu.eflags_val = t0;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  uint32_t data = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &data);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

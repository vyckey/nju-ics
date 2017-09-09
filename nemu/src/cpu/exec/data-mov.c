#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val, id_dest->width);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val, id_dest->width);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_lr(&t0, R_ESP, width);
  for (int i = R_EAX; i <= R_EDI; ++i) {
    if (i == R_ESP) rtl_mv(&t1, &t0);
    else rtl_lr(&t1, i, width);
    rtl_push(&t1, width);
  }

  print_asm("pusha");
}

make_EHelper(popa) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  for (int i = R_EDI; i >= R_EAX; --i) {
    if (i != R_ESP) {
      rtl_pop(&t0, width);
      rtl_sr(i, width, &t0);
    }
  }

  print_asm("popa");
}

make_EHelper(leave) {
  TODO();
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_pop(&t0, width);
  rtl_sr(R_EBP, width, &t0);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {//cwd
    rtl_lr_w(&t0, R_EAX);
    if (t0 < 0) rtl_li(&t1, 0xffff);
    else rtl_li(&t1, 0);
    rtl_sr_w(R_EDX, &t1);
  }
  else {//cdq
    rtl_lr_l(&t0, R_EAX);
    if (t0 < 0) rtl_li(&t1, 0xffffffff);
    else rtl_li(&t1, 0);
    rtl_sr_w(R_EDX, &t1);printf("fdsafdsai\n");
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  rtl_lr_w(&t0, R_EAX);
  if (decoding.is_operand_size_16) {
    rtl_sext(&t0, &t0, 1);
    rtl_sr_w(R_EAX, &t0);
  }
  else {
    rtl_sext(&t0, &t0, 2);
    rtl_sr_l(R_EAX, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

make_EHelper(xchg) {
  rtlreg_t h, l;
  h = (-1 << (id_dest->width << 3));
  l = ~h;
  t0 = id_dest->val;
  id_dest->val = (id_dest->val & h) | (id_src->val & l);
  id_src->val = (id_src->val & h) | (t0 & l);
  operand_write(id_dest, &id_dest->val);
  operand_write(id_src, &id_src->val);
}
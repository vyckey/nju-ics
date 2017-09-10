#include "cpu/exec.h"

static inline void logic_set_eflags(const rtlreg_t* result, int width) {
  rtlreg_t t;
  rtl_update_ZFSF(result, width);
  rtl_li(&t, 0);
  rtl_set_CF(&t);
  rtl_set_OF(&t);
}

static inline void shift_set_cf(const rtlreg_t* result, int n) {
  rtlreg_t t = (*result >> n) & 0x1;
  rtl_set_CF(&t);
}
make_EHelper(test) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
  
  logic_set_eflags(&t0, id_dest->width);
  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  logic_set_eflags(&t0, id_dest->width);
  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  logic_set_eflags(&t0, id_dest->width);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  logic_set_eflags(&t0, id_dest->width);
  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);
  if (t1 != 0) shift_set_cf(&t0, (t1 & 0x1f) - 1);
  if (t1 == 1) rtl_li(&t2, 0);
  else rtl_li(&t2, 0);
  rtl_set_OF(&t2);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);
  if (t1 != 0) shift_set_cf(&t0, 32 - (t1 & 0x1f));
  if (t1 == 1) {
    rtl_msb(&t2, &t0, id_dest->width);
    rtl_get_CF(&t3);
    t3 = (t2 != t3);
  }
  else rtl_li(&t3, 0);
  rtl_set_OF(&t3);
  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);
  if (t1 != 0) shift_set_cf(&t0, (t1 & 0x1f) - 1);
  if (t1 == 1) rtl_msb(&t2, &t0, id_dest->width);
  else rtl_li(&t2, 0);
  rtl_set_OF(&t2);
  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}

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
  if (t1 == 1) {
    rtl_li(&t2, 0);
    rtl_set_OF(&t2);
  }
  //else rtl_li(&t2, 0);
  
  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU

  printf("shl before zf:%d val:%x width:%d\n", cpu.eflags._ZF, t2, id_dest->width);
  rtl_update_ZFSF(&t2, id_dest->width);
  printf("shl before zf:%d val:%x\n", cpu.eflags._ZF, t2);
  if (t1 != 0) shift_set_cf(&t0, 32 - (t1 & 0x1f));
  if (t1 == 1) {
    rtl_msb(&t2, &t0, id_dest->width);
    rtl_get_CF(&t3);
    t3 = (t2 != t3);
    rtl_set_OF(&t3);
  }
  //else rtl_li(&t3, 0);
  
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
  if (t1 == 1) {
    rtl_msb(&t2, &t0, id_dest->width);
    rtl_set_OF(&t2);
  }
  //else rtl_li(&t2, 0);
  
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

make_EHelper(rcl) {
  rtl_get_CF(&t0);
  rtl_msb(&t1, &id_dest->val, id_dest->width);
  rtl_rotate_cl(&t2, &id_dest->val, &id_src->val, id_dest->width);
  operand_write(id_dest, &t2);
  if (id_src->val == 1) {
    t1 = (t0 != t1);
    rtl_set_OF(&t1);
  }
}

make_EHelper(rcr) {
  rtl_li(&t2, id_dest->width*8 - id_src->val);
  rtl_rotate_cl(&t2, &id_dest->val, &t2, id_dest->width);
  operand_write(id_dest, &t2);
}

make_EHelper(rol) {
  rtl_rotate_l(&t0, &id_dest->val, &id_src->val, id_dest->width);
  operand_write(id_dest, &t0);
}

make_EHelper(ror) {
  rtl_msb(&t0, &id_dest->val, id_dest->width);
  rtl_li(&t2, id_dest->width*8 - id_src->val);
  rtl_rotate_l(&t2, &id_dest->val, &t2, id_dest->width);
  operand_write(id_dest, &t2);
  if (id_src->val == 1) {
    rtl_msb(&t1, &t2, id_dest->width);
    t3 = (t0 != t1);
    rtl_set_OF(&t3);
  }
}
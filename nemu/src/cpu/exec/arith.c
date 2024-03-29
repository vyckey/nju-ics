#include "cpu/exec.h"

make_EHelper(add) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_add(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_xor(&t2, &t0, &t1);
  rtl_not(&t2);
  rtl_xor(&t3, &t0, &id_dest->val);
  rtl_and(&t3, &t2, &t3);
  rtl_msb(&t3, &t3, id_dest->width);
  rtl_set_OF(&t3);

  rtl_sltu(&t3, &id_dest->val, &t0);
  rtl_set_CF(&t3);
  print_asm_template2(add);
}

make_EHelper(sub) {
  rtl_mv(&t0, &id_dest->val);
  rtl_mv(&t1, &id_src->val);
  rtl_sub(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_xor(&t2, &t0, &t1);
  rtl_xor(&t3, &t0, &id_dest->val);
  rtl_and(&t3, &t2, &t3);
  rtl_msb(&t3, &t3, id_dest->width);
  rtl_set_OF(&t3);

  rtl_sltu(&t3, &t0, &id_dest->val);
  rtl_set_CF(&t3);
  print_asm_template2(sub);
}

make_EHelper(cmp) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);
  rtl_sub(&t3, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&t3, id_dest->width);
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t3);
  rtl_and(&t2, &t0, &t1);
  rtl_msb(&t2, &t2, id_dest->width);
  rtl_set_OF(&t2);

  rtl_sltu(&t3, &id_dest->val, &t3);
  rtl_set_CF(&t3);
  print_asm_template2(cmp);
}

make_EHelper(inc) {
  rtl_mv(&t0, &id_dest->val);
  rtl_addi(&t1, &t0, 1);
  rtl_mv(&id_dest->val, &t1);
  operand_write(id_dest, &t1);
  rtl_update_ZFSF(&t1, id_dest->width);

  /*rtl_eq0(&t2, &t1);
  rtl_set_OF(&t2);
  rtl_set_CF(&t2);*/
  print_asm_template1(inc);
}

make_EHelper(dec) {
  rtl_mv(&t0, &id_dest->val);
  rtl_subi(&t1, &t0, 1);
  rtl_mv(&id_dest->val, &t1);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);

  /*rtl_eq0(&t2, &t0);
  rtl_set_OF(&t2);
  rtl_set_CF(&t2);*/
  print_asm_template1(dec);
}

make_EHelper(neg) {
  rtl_li(&t0, 0);
  rtl_mv(&t1, &id_dest->val);
  rtl_sub(&id_dest->val, &t0, &t1);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  rtl_set_OF(&t0);
  rtl_neq0(&t0, &t1);
  rtl_set_CF(&t0);
  print_asm_template1(neg);
}

make_EHelper(adc) {
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_mv(&t3, &t2);
  rtl_get_CF(&t1);
  rtl_add(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t3, &id_dest->val);
  rtl_sltu(&t3, &t2, &t3);
  rtl_or(&t0, &t0, &t3);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  if (id_src->width == 1 && id_dest->width > 1) {
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  }
  rtl_get_CF(&t1);
  rtl_add(&t2, &id_src->val, &t1);
  rtl_mv(&t3, &t2);
  rtl_sub(&t2, &id_dest->val, &t2);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t3, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);
  rtl_or(&t0, &t0, &t3);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mv(&t3, &t0);
  rtl_imul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      rtl_update_ZFSF(&t1, 2);
      t3 = (t3 * id_dest->val != (t1 & 0xff));
      break;
    case 2:
      rtl_update_ZFSF(&t1, 4);
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      t3 = (t3 * id_dest->val != (t1 & 0xffff));
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      rtl_update_SF(&t1, 4);
      t2 = !((t0 == 0) && (t1 == 0));
      rtl_update_ZF(&t2, 4);
      t3 = (t0 !=0 && t0 != -1);
      break;
    default: assert(0);
  }

  rtl_set_CF(&t3);
  rtl_set_OF(&t3);

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);
  t2 = (id_dest->val * id_src->val != t1);
  if (id_dest->width == 4) t2 = (t0 !=0 && t0 != -1);
  rtl_set_CF(&t2);
  rtl_set_OF(&t2);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_src2->val, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_src2->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_li(&t1, 0);
      rtl_lr_w(&t0, R_AX);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_li(&t1, 0);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_div(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  switch (id_dest->width) {
    case 1:
      rtl_lr_w(&t0, R_AX);
      rtl_sext(&t0, &t0, 2);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_idiv(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(idiv);
}

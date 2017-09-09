#include "cpu/exec.h"

make_EHelper(mov);//seg 8c,8e

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

/* arith.c */
make_EHelper(add);
make_EHelper(sub);
make_EHelper(cmp);
make_EHelper(inc);
make_EHelper(dec);
make_EHelper(neg);
make_EHelper(adc);
make_EHelper(sbb);
make_EHelper(mul);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);//6b, 69
make_EHelper(div);
make_EHelper(idiv);

/* cc. c*/


/* control.c */
make_EHelper(jmp);//rel?,m,ptr
make_EHelper(jcc);//jcxz/jecxz
make_EHelper(jmp_rm);
make_EHelper(call);//m,ptr
make_EHelper(ret);//far
make_EHelper(call_rm);

/* data-mov.c */
make_EHelper(push);//seg,m
make_EHelper(pop);//seg,m
make_EHelper(pusha);
make_EHelper(popa);
make_EHelper(leave);//todo
make_EHelper(cltd);//cwd/cdq
make_EHelper(cwtl);//cbw/cwde
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(lea);
make_EHelper(xchg);

/* exec.c */
//make_EHelper(real)

/* logic.c */
make_EHelper(test);
make_EHelper(and);
make_EHelper(xor);
make_EHelper(or);
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(setcc);
make_EHelper(not);

/* prefix.c */
//make_EHelper(real);
//make_EHelper(operand_size)

/* special.c */
make_EHelper(nop);
//make_EHelper(inv);
//make_EHelper(nemu_trap);

/* system.c*/
make_EHelper(lidt);//?
make_EHelper(mov_r2cr);//other
make_EHelper(mov_cr2r);
make_EHelper(int);//?
make_EHelper(iret);
make_EHelper(in);
make_EHelper(out);

/*
matrix-mul.c
max.c
min3.c
mov-c.c
movsx.c
mul-longlong.c
*/ 
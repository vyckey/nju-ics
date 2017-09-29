#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  uint32_t sign = SIGN(a) ^ SIGN(b);
  FLOAT m = UNSIGN_F(a) * (UNSIGN_F(b) >> 16);
  return (sign != 0 ? SIGN_F(m) : m);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  uint32_t sign = SIGN(a) ^ SIGN(b);
  //FLOAT f = (UNSIGN_F(a) / UNSIGN_F(b)) << 16;
  FLOAT a_abs, b_abs, q, r, d;
  a_abs = UNSIGN_F(a);
  b_abs = UNSIGN_F(b);
  q = a_abs / b_abs;
  r = a_abs - q * b_abs;
  d = 0;
  for (int i = 0; i < 16; ++i) {
    r <<= 1;
    d <<= 1;
    if (r >= b_abs) {
      r -= b_abs;
      ++d;
    }
  }
  FLOAT f = (q << 16) | (d & 0xffff);
  return (sign != 0 ? SIGN_F(f) : f);
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  uint32_t *p = (void*)&a;
  FLOAT base = (*p & 0x007fffff) | 0x00800000, e = ((*p >> 23) & 0xff) - 127;
  FLOAT f;
  if (e >= 0) f = base >> 7 << e;
  else f = base >> 7 >> (-e);
  return SIGN(*p) | f;
}

FLOAT Fabs(FLOAT a) {
  return UNSIGN_F(a);
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);
printf("Fpow start %#x\n", f2F(1e-4));
  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
    //printf("t%#x t2%#x dt%#x %#x\n", t, t2, dt, F_div_F(x, t2));
  } while(Fabs(dt) > f2F(1e-4));
printf("Fpow end\n");

  return t;
}

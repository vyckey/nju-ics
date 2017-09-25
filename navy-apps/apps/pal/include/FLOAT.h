#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;
#define SIGN(f) ((f) & 0x80000000)
#define UNSIGN_F(f) ((f) & 0x7fffffff)
#define SIGN_F(f) ((f) | 0x80000000)

static inline int F2int(FLOAT a) {
  assert(0);
  int t = UNSIGN_F(a) >> 16;
  if (SIGN(a)) return -t;
  return t;
}

static inline FLOAT int2F(int a) {
  assert(0);
  FLOAT f = (a >= 0) ? a : (-a);
  f << 16;
  if (a < 0) SIGN_F(f);
  return f;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  assert(0);
  int m = (b >= 0) ? b : -b;
  FLOAT f = a * m;
  if (b < 0) return (SIGN(f) ? UNSIGN_F(f) : SIGN_F(f));
  return f;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  assert(0);
  int m = (b >= 0) ? b : -b;
  FLOAT f = a / m;
  if (b < 0) return (SIGN(f) ? UNSIGN_F(f) : SIGN_F(f));
  return f;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif

#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/rat.h"
#include "cixl/scope.h"

static uint64_t gcd(uint64_t a, uint64_t b) {
  while (b) {
    uint64_t t = b;
    b = a % b;
    a = t;
  }

  return a;
}

static void simplify(struct cx_rat *rat) {
  uint64_t m = gcd(rat->num, rat->den);
  rat->num /= m;
  rat->den /= m;
}

struct cx_rat *cx_rat_init(struct cx_rat *rat, uint64_t num, uint64_t den, bool neg) {
  rat->num = num;
  rat->den = den;
  rat->neg = neg;
  return rat;
}

int64_t cx_rat_int(struct cx_rat *rat) {
  int64_t n = rat->num / rat->den;
  return rat->neg ? -n : n;
}

void cx_rat_add(struct cx_rat *dst, struct cx_rat *src) {
  uint64_t
    den = gcd(dst->den, src->den),
    dst_num = dst->num * den / dst->den,
    src_num = src->num * den / src->den;

  if (dst->neg == src->neg) {
    dst->num = dst_num + src_num;
  } else {
    if (src_num > dst_num) {
      dst->num = src_num - dst_num;
      dst->neg = src->neg;
    } else {
      dst->num = dst_num - src_num;
    }
  }
  
  dst->den = den;
  simplify(dst);
}

void cx_rat_mul(struct cx_rat *dst, struct cx_rat *src) {
  dst->num = dst->num*src->num;
  dst->den = dst->den*src->den;
  dst->neg ^= src->neg;
  simplify(dst);
}

enum cx_cmp cx_cmp_rat(const void *x, const void *y) {
  const struct cx_rat *xv = x, *yv = y;

  uint64_t
    den = gcd(xv->den, yv->den),
    xn = xv->num * den / xv->den,
    yn = xv->num * den / xv->den;

  if (xn < yn) { return CX_CMP_LT; }
  return (xn > yn) ? CX_CMP_GT : CX_CMP_EQ;
}

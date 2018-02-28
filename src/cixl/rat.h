#ifndef CX_RAT_H
#define CX_RAT_H

#include <stdbool.h>
#include <stdint.h>

#include "cixl/cmp.h"

struct cx;
struct cx_type;

struct cx_rat {
  uint64_t num, den;
  bool neg;
};

struct cx_rat *cx_rat_init(struct cx_rat *rat, uint64_t num, uint64_t den, bool neg);

int64_t cx_rat_int(struct cx_rat *rat);

void cx_rat_add(struct cx_rat *dst, struct cx_rat *src);
void cx_rat_mul(struct cx_rat *dst, struct cx_rat *src);

enum cx_cmp cx_cmp_rat(const void *x, const void *y);

#endif

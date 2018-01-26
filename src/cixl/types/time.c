#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/types/time.h"

struct cx_time *cx_time_init(struct cx_time *time, int32_t months, int64_t ns) {
  time->months = months;
  time->ns = ns;
  return time;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_time *xt = &x->as_time, *yt = &y->as_time;
  return xt->months == yt->months && xt->ns == yt->ns;
}


static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  const struct cx_time *xt = &x->as_time, *yt = &y->as_time;
  
  if (xt->months < yt->months ||
      (xt->months == yt->months && xt->ns < yt->ns)) {
    return CX_CMP_LT;
  }

  if (xt->months > yt->months ||
      (xt->months == yt->months && xt->ns > yt->ns)) {
    return CX_CMP_GT;
  }

  return CX_CMP_EQ;
}

static bool ok_imp(struct cx_box *v) {
  struct cx_time *t = &v->as_time;
  return t->months || t->ns;
}

static void fprint_ns(int64_t ns, FILE *out) {
  int32_t h = ns / CX_HOUR;
  ns %= CX_HOUR;
  int32_t m = ns / CX_MIN;
  ns %= CX_MIN;
  int32_t s = ns / CX_SEC;
  ns %= CX_SEC;
  
  fprintf(out, "%02" PRId32 ":%02" PRId32 ":%02" PRId32 ".%" PRId64, h, m, s, ns);
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputs("([", out);
  
  struct cx_time *t = &v->as_time;
  
  int32_t y = t->months / 12, m = t->months % 12, d = t->ns / CX_DAY; 
  fprintf(out, "%" PRId32 " %" PRId32 " %" PRId32, y, m, d);
  int64_t ns = t->ns % CX_DAY;
  
  if (ns) {
    fputc(' ', out);
    
    int32_t h = ns / CX_HOUR;
    ns %= CX_HOUR;
    int32_t m = ns / CX_MIN;
    ns %= CX_MIN;
    int32_t s = ns / CX_SEC;
    ns %= CX_SEC;
  
    fprintf(out, "%" PRId32 " %" PRId32 " %" PRId32 " %" PRId64, h, m, s, ns);
  }

  fputs("] time)", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputs("Time(", out);
  struct cx_time *t = &v->as_time;
  
  if (t->months) {
    int32_t y = t->months / 12, m = t->months % 12, d = t->ns / CX_DAY; 
    fprintf(out, "%04" PRId32 "-%02" PRId32 "-%02" PRId32, y, m, d);
    int64_t ns = t->ns % CX_DAY;

    if (ns) {
      fputc(' ', out);
      fprint_ns(ns, out);
    }
  } else {
    fprint_ns(t->ns, out);
  }

  fputc(')', out);
}

static void print_imp(struct cx_box *v, FILE *out) {
  struct cx_time *t = &v->as_time;
  
  if (t->months) {
    int32_t y = t->months / 12, m = t->months % 12, d = t->ns / CX_DAY; 
    fprintf(out, "%04" PRId32 "-%02" PRId32 "-%02" PRId32, y, m+1, d+1);
    int64_t ns = t->ns % CX_DAY;

    if (ns) {
      fputc(' ', out);
      fprint_ns(ns, out);
    }
  } else {
    fprint_ns(t->ns, out);
  }
}

struct cx_type *cx_init_time_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Time", cx->cmp_type);
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  return t;
}


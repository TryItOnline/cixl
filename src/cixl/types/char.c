#include <ctype.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/char.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_char == y->as_char;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_char;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_char = src->as_char;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  unsigned char c = v->as_char;
  
  switch (c) {
  case '\n':
    fputs("\\\\n", out);
    break;
  case '\r':
    fputs("\\\\r", out);
    break;
  case ' ':
    fputs("\\\\s", out);
    break;
  case '\t':
    fputs("\\\\t", out);
    break;
  default:
    if (isgraph(c)) {
      fprintf(out, "\\%c", v->as_char);
    } else {
      fprintf(out, "\\\\%d", v->as_char);
    }
  }
}

struct cx_type *cx_init_char_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Char", cx->any_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->write = dump_imp;
  t->dump = dump_imp; 
  return t;
}

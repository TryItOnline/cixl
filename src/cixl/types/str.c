#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/str.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool len_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = strlen(v.as_ptr);
  return true;
}

static bool for_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    str = *cx_test(cx_pop(scope, false));

  bool ok = false;

  for (char *c = str.as_ptr; *c; c++) {
    cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = *c;
    if (!cx_call(&act, scope)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_deinit(&str);
  cx_box_deinit(&act);
  return ok;
}

static bool map_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    str = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (char *c = str.as_ptr; *c; c++) {
    cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = *c;
    if (!cx_call(&act, scope)) { goto exit; }
    struct cx_box *res = cx_pop(scope, true);

    if (!res) {
      cx_error(cx, cx->row, cx->col, "Missing result for '%c'", *c);
      goto exit;
    }

    if (res->type != cx->char_type) {
      cx_error(cx, cx->row, cx->col, "Expected type Char, got %s", res->type->id);
      goto exit;
    }

    *c = res->as_char;
  }

  *cx_push(scope) = str;
  ok = true;
 exit:
  cx_box_deinit(&act);
  return ok;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return strcmp(x->as_ptr, y->as_ptr) == 0;
}

static bool ok_imp(struct cx_box *v) {
  char *s = v->as_ptr;
  return s[0];
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'", (char *)v->as_ptr);
}

static void deinit_imp(struct cx_box *v) {
  free(v->as_ptr);
}

struct cx_type *cx_init_str_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Str", cx->any_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;
  
  cx_add_func(cx, "len", cx_arg(t))->ptr = len_imp;
  cx_add_func(cx, "for", cx_arg(t), cx_arg(cx->any_type))->ptr = for_imp;
  cx_add_func(cx, "map", cx_arg(t), cx_arg(cx->any_type))->ptr = map_imp;

  return t;
}

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/type.h"

struct cx_box *cx_box_new(struct cx_type *type) {
  return cx_box_init(malloc(sizeof(struct cx_box)), type);
}

struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type) {
  box->type = type;
  box->undef = false;
  return box;
}

struct cx_box *cx_box_deinit(struct cx_box *box) {
  if (box->type->deinit) { box->type->deinit(box); }
  return box;
}

bool cx_eqval(struct cx_box *x, struct cx_box *y, struct cx_scope *scope) {
  return x->type->eqval ? x->type->eqval(x, y, scope) : cx_equid(x, y);
}

bool cx_equid(struct cx_box *x, struct cx_box *y) {
  return cx_test(x->type->equid)(x, y);
}

bool cx_ok(struct cx_box *x) {
  return x->type->ok ? x->type->ok(x) : true;
}

bool cx_call(struct cx_box *box, struct cx_scope *scope) {
  if (!box->type->call) {
    cx_copy(cx_push(scope), box);
    return true;
  }
  
  return box->type->call(box, scope);
}

struct cx_box *cx_copy(struct cx_box *dst, struct cx_box *src) {
  if (src->type->copy && !src->undef) {
    dst->type = src->type;
    dst->undef = src->undef;
    src->type->copy(dst, src);
  }
  else {
    *dst = *src;
  }

  return dst;
}

struct cx_box *cx_clone(struct cx_box *dst, struct cx_box *src) {
  if (!src->type->clone) { return cx_copy(dst, src); }
  dst->type = src->type;
  dst->undef = src->undef;
  src->type->clone(dst, src);
  return dst;
}

void cx_fprint(struct cx_box *box, FILE *out) {
  cx_test(box->type->fprint)(box, out);
}

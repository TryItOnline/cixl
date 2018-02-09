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
  return box;
}

struct cx_box *cx_box_deinit(struct cx_box *box) {
  if (box->type->deinit) { box->type->deinit(box); }
  return box;
}

bool cx_box_emit(struct cx_box *box, FILE *out) {
  return cx_test(box->type->emit)(box, out);
}

bool cx_eqval(struct cx_box *x, struct cx_box *y) {
  return x->type->eqval ? x->type->eqval(x, y) : cx_equid(x, y);
}

bool cx_equid(struct cx_box *x, struct cx_box *y) {
  return cx_test(x->type->equid)(x, y);
}

enum cx_cmp cx_cmp(const struct cx_box *x, const struct cx_box *y) {
  return cx_test(x->type->cmp)(x, y);
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

struct cx_box *cx_copy(struct cx_box *dst, const struct cx_box *src) {
  if (src->type->copy) {
    dst->type = src->type;
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
  src->type->clone(dst, src);
  return dst;
}

struct cx_iter *cx_iter(struct cx_box *box) {
  return cx_test(box->type->iter)(box);
}

bool cx_write(struct cx_box *box, FILE *out) {
  if (!box->type->write) {
    struct cx *cx = box->type->cx;
    cx_error(cx, cx->row, cx->col,
	     "Write not implemented for type: %s",
	     box->type->id);
    return false;
  }
  
  box->type->write(box, out);
  return true;
}

void cx_dump(struct cx_box *box, FILE *out) {
  cx_test(box->type->dump)(box, out);
}

void cx_print(struct cx_box *box, FILE *out) {
  if (box->type->print)
    box->type->print(box, out);
  else {
    cx_dump(box, out);
  }
}

enum cx_cmp cx_cmp_box(const void *x, const void *y) {
  return cx_cmp(x, y);
}

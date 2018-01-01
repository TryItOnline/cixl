#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/type.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

struct cx_type *cx_type_init(struct cx_type *type, struct cx *cx, const char *id) {
  type->cx = cx;
  type->id = strdup(id);
  type->tag = cx->next_type_tag;
  cx->next_type_tag *= 2;
  type->tags = type->tag;
  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  
  type->eqval = NULL;
  type->equid = NULL;
  type->ok = NULL;
  type->call = NULL;
  type->copy = NULL;
  type->fprint = NULL;
  type->deinit = NULL;

  return type;
}

struct cx_type *cx_type_deinit(struct cx_type *type) {
  free(type->id);
  cx_set_deinit(&type->parents);
  return type;  
}

void cx_derive(struct cx_type *child, struct cx_type *parent) {
  *(struct cx_type **)cx_test(cx_set_insert(&child->parents, parent)) = parent;
  child->tags |= parent->tag;
}

bool cx_is(struct cx_type *child, struct cx_type *parent) {
  if (child->tags & parent->tag) { return true; }
  
  cx_do_set(&child->parents, struct cx_type *, pt) {
    if (cx_is(*pt, parent)) {
      child->tags |= parent->tag;
      return true;
    }
  }

  return false;
}

static bool type_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->meta_type)->as_ptr = v.type;
  return true;
}

static bool is_imp(struct cx_scope *scope) {
  struct cx_type
    *y = cx_test(cx_pop(scope, false))->as_ptr,
    *x = cx_test(cx_pop(scope, false))->as_ptr;

  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_is(x, y);
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_type *type = value->as_ptr;
  fputs(type->id, out);
}

struct cx_type *cx_init_meta_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Type", cx->any_type);
  t->equid = equid_imp;
  t->fprint = fprint_imp;
  
  cx_add_func(cx, "type", cx_arg(cx->any_type))->ptr = type_imp;
  cx_add_func(cx, "is", cx_arg(t), cx_arg(t))->ptr = is_imp;

  return t;
}

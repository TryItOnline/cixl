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
  type->trait = false;
  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  cx_set_init(&type->children, sizeof(struct cx_type *), cx_cmp_ptr);
  
  type->new = NULL;
  type->eqval = NULL;
  type->equid = NULL;
  type->cmp = NULL;
  type->ok = NULL;
  type->call = NULL;
  type->copy = NULL;
  type->clone = NULL;
  type->iter = NULL;
  type->write = NULL;
  type->dump = NULL;
  type->print = NULL;
  type->deinit = NULL;

  type->type_deinit = NULL;
  return type;
}

struct cx_type *cx_type_reinit(struct cx_type *type) {
  cx_do_set(&type->parents, struct cx_type *, t) {
    cx_set_delete(&(*t)->children, t);
  }
  
  cx_set_clear(&type->parents);

  cx_do_set(&type->children, struct cx_type *, t) {
    cx_set_delete(&(*t)->parents, t);
  }
  
  cx_set_clear(&type->children);
  return type;
}

struct cx_type *cx_type_deinit(struct cx_type *type) {
  if (type->type_deinit) { type->type_deinit(type); }  
  cx_set_deinit(&type->parents);
  cx_set_deinit(&type->children);
  free(type->id);
  return type;  
}

void cx_derive(struct cx_type *child, struct cx_type *parent) {
  struct cx_type **tp = cx_set_insert(&child->parents, &parent);
  if (tp) { *tp = parent; }
  
  tp = cx_set_insert(&parent->children, &child);
  if (tp) { *tp = child; }

  cx_do_set(&parent->parents, struct cx_type *, t) { cx_derive(child, *t); }
  cx_do_set(&child->children, struct cx_type *, t) { cx_derive(*t, parent); }
}

bool cx_is(const struct cx_type *child, const struct cx_type *parent) {
  return child == parent || cx_set_get(&child->parents, &parent);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_type *type = value->as_ptr;
  fputs(type->id, out);
}

struct cx_type *cx_init_meta_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Type", cx->any_type);
  t->equid = equid_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  return t;
}

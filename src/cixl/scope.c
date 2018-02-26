#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/vect.h"

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent) {
  struct cx_scope *scope = cx_malloc(&cx->scope_alloc);
  scope->cx = cx;
  scope->parent = parent ? cx_scope_ref(parent) : NULL;
  cx_vec_init(&scope->stack, sizeof(struct cx_box));
  cx_env_init(&scope->vars, &cx->var_alloc);
  scope->safe = cx->scopes.count ? cx_scope(cx, 0)->safe : true;
  scope->nrefs = 0;
  return scope;
}

struct cx_scope *cx_scope_ref(struct cx_scope *scope) {
  scope->nrefs++;
  return scope;
}

void cx_scope_deref(struct cx_scope *scope) {
  cx_test(scope->nrefs);
  scope->nrefs--;
  
  if (!scope->nrefs) {
    if (scope->parent) { cx_scope_deref(scope->parent); }

    cx_do_vec(&scope->stack, struct cx_box, b) { cx_box_deinit(b); }
    cx_vec_deinit(&scope->stack);
    
    cx_env_deinit(&scope->vars);

    cx_free(&scope->cx->scope_alloc, scope);
  }
}

struct cx_box *cx_push(struct cx_scope *scope) {
  return cx_vec_push(&scope->stack);
}

struct cx_box *cx_pop(struct cx_scope *scope, bool silent) {
  if (!scope->stack.count) {
    if (!silent) {
      cx_error(scope->cx, scope->cx->row, scope->cx->col, "Stack is empty");
    }

    return NULL;
  }

  return cx_vec_pop(&scope->stack);
}

struct cx_box *cx_peek(struct cx_scope *scope, bool silent) {
  if (!scope->stack.count) {
    if (silent) { return NULL; }
    cx_error(scope->cx, scope->cx->row, scope->cx->col, "Stack is empty");
  }

  return cx_vec_peek(&scope->stack, 0);
}

void cx_stackdump(struct cx_scope *scope, FILE *out) {
  cx_vect_dump(&scope->stack, out);
  fputc('\n', out);
}

struct cx_box *cx_get_var(struct cx_scope *scope, struct cx_sym id, bool silent) {
  struct cx_box *val = cx_env_get(&scope->vars, id);

  if (!val) {
    if (scope->parent) { return cx_get_var(scope->parent, id, silent); }

    if (!silent) {
      struct cx *cx = scope->cx;
      cx_error(cx, cx->row, cx->col, "Unknown var: %s", id.id);
    }
    
    return NULL;
  }

  return val;
}

struct cx_box *cx_put_var(struct cx_scope *scope, struct cx_sym id, bool force) {
  struct cx_box *val = cx_env_get(&scope->vars, id);

  if (val) {
    if (!force) {
      struct cx *cx = scope->cx;
      cx_error(cx, cx->row, cx->col, "Attempt to rebind var: %s", id.id);
      return NULL;
    }
      
    cx_box_deinit(val);
  } else {
    val = cx_env_put(&scope->vars, id);
  }

  return val;
}

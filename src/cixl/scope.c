#include "cixl/box.h"
#include "cixl/catch.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent) {
  struct cx_scope *scope = cx_malloc(&cx->scope_alloc);
  scope->cx = cx;
  scope->parent = parent ? cx_scope_ref(parent) : NULL;
  cx_vec_init(&scope->stack, sizeof(struct cx_box));
  scope->stack.alloc = &cx->stack_items_alloc;
  cx_env_init(&scope->vars, &cx->var_alloc);
  cx_vec_init(&scope->catches, sizeof(struct cx_catch));
  cx_vec_init(&scope->var_scopes, sizeof(struct cx_scope *));
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
    cx_do_vec(&scope->catches, struct cx_catch, c) { cx_catch_deinit(c); }
    
    cx_vec_deinit(&scope->catches);
    cx_vec_deinit(&scope->var_scopes);
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

struct cx_box *cx_get_var(struct cx_scope *scope, struct cx_sym id, bool silent) {
  if (scope->var_scopes.count) {
    struct cx_scope *var_scope = *(struct cx_scope **)cx_vec_pop(&scope->var_scopes);
    struct cx_box *v = cx_get_var(var_scope, id, silent);
    *(struct cx_scope **)cx_vec_push(&scope->var_scopes) = var_scope;
    return v;
  }
  
  struct cx_var *v = cx_env_get(&scope->vars, id);

  if (!v) {
    if (scope->parent) { return cx_get_var(scope->parent, id, silent); }

    if (!silent) {
      struct cx *cx = scope->cx;
      cx_error(cx, cx->row, cx->col, "Unknown var: %s", id.id);
    }
    
    return NULL;
  }

  return &v->value;
}

struct cx_box *cx_put_var(struct cx_scope *scope, struct cx_sym id, bool force) {
  struct cx_var *v = cx_env_get(&scope->vars, id);

  if (v) {
    if (!force) {
      struct cx *cx = scope->cx;
      cx_error(cx, cx->row, cx->col, "Attempt to rebind var: %s", id.id);
      return NULL;
    }
      
    cx_box_deinit(&v->value);
    return &v->value;
  }

  return cx_env_put(&scope->vars, id);
}

void cx_reset(struct cx_scope *scope) {
  cx_do_vec(&scope->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&scope->stack);
}

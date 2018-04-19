#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/tok.h"

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent) {
  struct cx_scope *scope = cx_malloc(&cx->scope_alloc);
  scope->cx = cx;

  cx_vec_init(&scope->parents, sizeof(struct cx_scope *));

  if (parent) {
    *(struct cx_scope **)cx_vec_push(&scope->parents) = cx_scope_ref(parent);
  }

  cx_vec_init(&scope->stack, sizeof(struct cx_box));
  scope->stack.alloc = &cx->stack_items_alloc;
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
    cx_env_deinit(&scope->vars);

    cx_do_vec(&scope->stack, struct cx_box, b) { cx_box_deinit(b); }
    cx_vec_deinit(&scope->stack);
    
    cx_do_vec(&scope->parents, struct cx_scope *, ps) { cx_scope_deref(*ps); }
    cx_vec_deinit(&scope->parents);

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
  struct cx_var *v = cx_env_get(&scope->vars, id);

  if (!v) {
    cx_do_vec(&scope->parents, struct cx_scope *, ps) {
      struct cx_box *v = cx_get_var(*ps, id, true);
      if (v) { return v; }
    }

    if (!silent) {
      struct cx *cx = scope->cx;
      cx_error(cx, cx->row, cx->col, "Unknown var: %s", id.id);
    }
    
    return NULL;
  }

  return &v->value;
}

struct cx_box *cx_put_var(struct cx_scope *scope, struct cx_sym id) {
  struct cx_var *v = cx_env_get(&scope->vars, id);

  if (v) {
    cx_box_deinit(&v->value);
    return &v->value;
  }

  return cx_env_put(&scope->vars, id);
}

void cx_stash(struct cx_scope *s) {
  struct cx *cx = s->cx;
  struct cx_stack *out = cx_stack_new(cx);
  out->imp = s->stack;
  cx_vec_init(&s->stack, sizeof(struct cx_box));
  s->stack.alloc = &cx->stack_items_alloc;
  struct cx_type *t = NULL;
  
  if (out->imp.count) {
    struct cx_box *v = cx_vec_start(&out->imp);
    t = v->type;
    
    for (; v != cx_vec_end(&out->imp); v++) {
      size_t i = cx_min(cx_min(t->is.count, v->type->is.count)-1, t->tag);
      
      for (struct cx_type
	     **t_is = cx_vec_get(&t->is, i),
	     **v_is = cx_vec_get(&v->type->is, i);
	   t_is >= (struct cx_type **)cx_vec_start(&t->is);
	   t_is--, v_is--) {
	if (*t_is && *v_is) {
	  t = *t_is;
	  break;
	}
      }	   
    }
  }
    
  cx_box_init(cx_push(s),
	      t ? cx_type_get(cx->stack_type, t) : cx->stack_type)->as_ptr = out;
}

void cx_reset(struct cx_scope *s) {
  cx_do_vec(&s->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&s->stack);
}

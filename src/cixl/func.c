#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/type.h"

static const void *get_imp_arg_tags(const void *value) {
  struct cx_func_imp *const *imp = value;
  return &(*imp)->arg_tags;
}

enum cx_cmp cmp_arg_tags(const void *x, const void *y) {
  const cx_type_tag_t *xv = x, *yv = y;
  if (*xv < *yv) { return CX_CMP_LT; }
  return (*xv > *yv) ? CX_CMP_GT : CX_CMP_EQ;
}

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs) {
  func->cx = cx;
  func->id = strdup(id);
  cx_set_init(&func->imps, sizeof(struct cx_func_imp *), cmp_arg_tags);
  func->imps.key = get_imp_arg_tags;
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_do_set(&func->imps, struct cx_func_imp *, i) { free(cx_func_imp_deinit(*i)); }
  cx_set_deinit(&func->imps);
  return func; 
}

struct cx_func_imp *cx_func_imp_init(struct cx_func_imp *imp,
				     struct cx_func *func,
				     cx_type_tag_t arg_tags) {
  imp->func = func;
  imp->arg_tags = arg_tags;
  imp->ptr = NULL;
  imp->bin = NULL;
  cx_vec_init(&imp->args, sizeof(struct cx_func_arg));
  cx_vec_init(&imp->toks, sizeof(struct cx_tok));
  return imp;
}

struct cx_func_imp *cx_func_imp_deinit(struct cx_func_imp *imp) {
  cx_vec_deinit(&imp->args);
  cx_do_vec(&imp->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&imp->toks);
  if (imp->bin) { cx_bin_unref(imp->bin); }
  return imp;
}

struct cx_func_arg cx_arg(struct cx_type *type) {
  return (struct cx_func_arg) { type, -1 };
}

struct cx_func_arg cx_narg(int n) {
  return (struct cx_func_arg) { NULL, n };
}

struct cx_func_imp *cx_func_add_imp(struct cx_func *func,
				    int nargs,
				    struct cx_func_arg *args) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_func_arg));
  cx_type_tag_t arg_tags = 0;
  
  if (nargs) {
    cx_vec_grow(&imp_args, nargs);

    for (int i=0; i < nargs; i++) {
      struct cx_func_arg a = args[i];
      *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
      if (a.type) { arg_tags += a.type->tag; }
    }
  }
    
  struct cx_func_imp **found = cx_set_get(&func->imps, &arg_tags);
  
  if (found) {
    cx_set_delete(&func->imps, &arg_tags);
    free(cx_func_imp_deinit(*found));
  }

  struct cx_func_imp *imp = cx_func_imp_init(malloc(sizeof(struct cx_func_imp)),
					     func,
					     arg_tags);
  *(struct cx_func_imp **)cx_set_insert(&func->imps, &arg_tags) = imp;
  imp->args = imp_args;
  return imp;
}

bool cx_func_imp_match(struct cx_func_imp *imp, struct cx_vec *stack) {
  if (!imp->args.count) { return true; }
  
  struct cx_func_arg *i = (struct cx_func_arg *)cx_vec_end(&imp->args)-1;
  struct cx_box *j = (struct cx_box *)cx_vec_end(stack)-1;
  
  for (; i >= (struct cx_func_arg *)imp->args.items &&
	 j >= (struct cx_box *)stack->items;
       i--, j--) {

    struct cx_type *t = i->type
      ? i->type
      : ((struct cx_box *)cx_vec_get(stack,
				     stack->count-imp->args.count+i->narg))->type;
      
    if (!cx_is(j->type, t)) { return false; }
  }

  return true;
}

bool cx_func_imp_eval(struct cx_func_imp *imp, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func_imp *prev = cx->func_imp;
  cx->func_imp = imp;
  struct cx_bin_func *bin = cx_bin_get_func(cx->bin, imp);
  bool ok = false;

  if (bin) {
    ok = cx_eval(cx, cx->bin, cx_vec_get(&cx->bin->ops, bin->start_op));
  } else {
    if (!imp->bin) {
      imp->bin = cx_bin_new();
      
      if (!cx_compile(cx,
		      cx_vec_start(&imp->toks),
		      cx_vec_end(&imp->toks),
		      imp->bin)) {
	cx_error(cx, cx->row, cx->col, "Failed compiling func imp");
      }
    }
    
    ok = cx_eval(cx, imp->bin, NULL);
  }
  
  cx->func_imp = prev;
  return ok;
}

bool cx_func_imp_call(struct cx_func_imp *imp, struct cx_scope *scope) {
  if (imp->ptr) {
    imp->ptr(scope);
    return true;
  }
  
  cx_begin(scope->cx, false);
  bool ok = cx_func_imp_eval(imp, scope);
  cx_end(scope->cx);
  return ok;
}

struct cx_func_imp *cx_func_get_imp(struct cx_func *func, struct cx_vec *stack) {
  cx_do_set(&func->imps, struct cx_func_imp *, imp) {
    if (cx_func_imp_match(*imp, stack)) { return *imp; }
  }

  return NULL;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_func_imp *imp = cx_func_get_imp(value->as_ptr, &scope->stack);

  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", imp->func->id);
    return -1;
  }

  return cx_func_imp_call(imp, scope);
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_func *func = value->as_ptr;
  fprintf(out, "Func(%s)", func->id);
}

struct cx_type *cx_init_func_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Func", cx->any_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->fprint = fprint_imp;
  return t;
}

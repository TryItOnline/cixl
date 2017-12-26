#include <stdlib.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/type.h"

static const void *get_imp_id(const void *value) {
  struct cx_func_imp *const *imp = value;
  return &(*imp)->id;
}

struct cx_func *cx_func_init(struct cx_func *func, const char *id, int nargs) {
  func->id = strdup(id);
  cx_set_init(&func->imps, sizeof(struct cx_func_imp *), cx_cmp_str);
  func->imps.key = get_imp_id;
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_do_set(&func->imps, struct cx_func_imp *, i) { free(cx_func_imp_deinit(*i)); }
  cx_set_deinit(&func->imps);
  return func; 
}

struct cx_func_imp *cx_func_imp_init(struct cx_func_imp *imp, char *id) {
  imp->id = id;
  imp->ptr = NULL;
  cx_vec_init(&imp->args, sizeof(struct cx_func_arg));
  cx_vec_init(&imp->toks, sizeof(struct cx_tok));
  return imp;
}

struct cx_func_imp *cx_func_imp_deinit(struct cx_func_imp *imp) {
  free(imp->id);
  cx_vec_deinit(&imp->args);
  cx_do_vec(&imp->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&imp->toks);
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
  struct cx_buf id;
  cx_buf_open(&id);
  
  for (int i=0; i < nargs; i++) {
    struct cx_func_arg a = args[i];
    *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
    
    if (i) { fputc(' ', id.stream); }
    
    if (a.type) {
      fputs(a.type->id, id.stream);
    } else {
      fprintf(id.stream, "%d", a.narg);
    }
  }
    
  cx_buf_close(&id);
  struct cx_func_imp **found = cx_set_get(&func->imps, &id.data);
  
  if (found) {
    cx_set_delete(&func->imps, &id.data);
    free(cx_func_imp_deinit(*found));
  }

  struct cx_func_imp *imp = cx_func_imp_init(malloc(sizeof(struct cx_func_imp)),
					     id.data);
  *(struct cx_func_imp **)cx_set_insert(&func->imps, &id.data) = imp;
  imp->args = imp_args;
  return imp;
}

struct cx_func_imp *cx_func_get_imp(struct cx_func *func, struct cx_vec *stack) {
  cx_do_set(&func->imps, struct cx_func_imp *, imp) {
    bool match = true;
    
    for (int i=func->nargs-1, j=stack->count-1;
	 i >= 0 && j >= 0;
	 i--, j--) {
      struct cx_func_arg *imp_arg = cx_vec_get(&(*imp)->args, i);

      struct cx_type *imp_type = imp_arg->type
	? imp_arg->type
	: ((struct cx_box *)cx_vec_get(stack, imp_arg->narg))->type;
      
      struct cx_box *arg = cx_vec_get(stack, j);
      
      if (!cx_type_is(arg->type, imp_type)) {
	match = false;
	break;
      }
    }

    if (match) { return *imp; }
  }

  return NULL;
}

bool cx_funcall(struct cx_func *func, struct cx_scope *scope, int row, int col) {
  struct cx *cx = scope->cx;
  struct cx_func_imp *imp = cx_func_get_imp(func, &scope->stack);

  if (!imp) {
    cx_error(cx, row, col, "Func not applicable: '%s'", func->id);
    return -1;
  }

  if (imp->ptr) {
    imp->ptr(scope);
    return true;
  }
  
  struct cx_scope *func_scope = cx_begin(cx, false);
  struct cx_func_imp *prev = cx->func_imp;
  cx->func_imp = imp;
  bool ok = cx_eval(cx, &imp->toks, 0);
  cx->func_imp = prev;
  if (cx_scope(cx, 0) == func_scope) { cx_end(cx); }
  return ok;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  return cx_funcall(value->as_ptr, scope, cx->row, cx->col);
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_func *func = value->as_ptr;
  fprintf(out, "Func(%s)", func->id);
}

struct cx_type *cx_init_func_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Func", cx->any_type, NULL);
  t->equid = equid_imp;
  t->call = call_imp;
  t->fprint = fprint_imp;
  return t;
}

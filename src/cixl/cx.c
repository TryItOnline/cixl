#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/bool.h"
#include "cixl/box.h"
#include "cixl/coro.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/lambda.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/type.h"

static const void *get_type_id(const void *value) {
  struct cx_type *const *type = value;
  return &(*type)->id;
}

static const void *get_macro_id(const void *value) {
  struct cx_macro *const *macro = value;
  return &(*macro)->id;
}

static const void *get_func_id(const void *value) {
  struct cx_func *const *func = value;
  return &(*func)->id;
}

static ssize_t let_eval(struct cx_macro_eval *eval,
			struct cx *cx,
			struct cx_vec *toks,
			ssize_t pc) {
  struct cx_scope *s = cx_begin(cx, true);
  cx_eval(cx, &eval->toks, 1);
  struct cx_box *val = cx_pop(s, false);
  
  if (!val) {
    cx_end(cx);
    return -1;
  }

  struct cx_tok *id = cx_vec_get(&eval->toks, 0);
  struct cx_box *var = cx_set(s->parent, id->data);
  *var = *val;
  cx_end(cx);
  
  return pc+1;
}

static bool let_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_macro_eval *eval = cx_macro_eval_new(let_eval);

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &eval->toks, false)) {
    cx_error(cx, row, col, "Missing let id");
    cx_macro_eval_unref(eval);
    return false;
  }

  struct cx_tok *id = cx_vec_peek(&eval->toks, 0);

  if (id->type != CX_TID) {
    cx_error(cx, row, col, "Invalid let id");
    cx_macro_eval_unref(eval);
    return false;
  }
  
  if (!cx_parse_end(cx, in, &eval->toks)) {
    cx_error(cx, row, col, "Empty let");
    cx_macro_eval_unref(eval);
    return false;
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO, eval, row, col);
  return true;
}

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx *cx,
			 struct cx_vec *toks,
			 ssize_t pc) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_scope *ps = cx_scope(cx, 1);

  for (int j = eval->toks.count-1; j >= 0; j--) {
    struct cx_tok *t = cx_vec_get(&eval->toks, j);
    *cx_set(s, t->data) = *cx_pop(ps, false);
  }
  
  return pc+1;
}

static bool func_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func id");
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID) {
    cx_error(cx, row, col, "Invalid func id");
    cx_tok_deinit(&id);
    cx_vec_deinit(&toks);
    return false;
  }

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func args");
    cx_tok_deinit(&id);
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok args = *(struct cx_tok *)cx_vec_pop(&toks);

  if (args.type != CX_TGROUP) {
    cx_error(cx, row, col, "Invalid func args");
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&toks), CX_TMACRO, eval, row, col);
  
  struct cx_vec func_args;
  cx_vec_init(&func_args, sizeof(struct cx_func_arg));

  if (!cx_eval_args(cx, args.data, &eval->toks, &func_args)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;  
  }

  cx_tok_deinit(&args);

  if (!cx_parse_end(cx, in, &toks)) {
    cx_tok_deinit(&id);
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;
  }

  _cx_add_func(cx,
	       id.data,
	       func_args.count,
	       (void *)func_args.items)->toks = toks;

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);
  return true;
}

static void dup_imp(struct cx_scope *scope) {
  struct cx_box *vp = cx_ok(cx_peek(scope, false));
  struct cx_box v = *vp;
  cx_box_copy(cx_push(scope), &v);
}

static void zap_imp(struct cx_scope *scope) {
  cx_box_deinit(cx_ok(cx_pop(scope, false)));
}

static void cls_imp(struct cx_scope *scope) {
  cx_vec_clear(&scope->stack);
}

static void call_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_call(&x, scope);
  cx_box_deinit(&x);
}

static void eqval_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_ok(cx_pop(scope, false)),
    x = *cx_ok(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_eqval(&x, &y);
}

static void equid_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_ok(cx_pop(scope, false)),
    x = *cx_ok(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_equid(&x, &y);
}

static void test_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  
  if (x.type != cx->bool_type || !x.as_bool) {
    cx_error(cx, cx->row, cx->col, "Test failed");
  }
}

struct cx *cx_init(struct cx *cx) {
  cx->coro = NULL;
  cx->toks = NULL;
  cx->pc = cx->stop_pc = -1;  
  cx->row = cx->col = -1;
  
  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;(){}[]");

  cx_set_init(&cx->types, sizeof(struct cx_type *), cx_cmp_str);
  cx->types.key = get_type_id;
  
  cx_set_init(&cx->macros, sizeof(struct cx_macro *), cx_cmp_str);
  cx->macros.key = get_macro_id;

  cx_set_init(&cx->funcs, sizeof(struct cx_func *), cx_cmp_str);
  cx->funcs.key = get_func_id;

  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx_vec_init(&cx->errors, sizeof(struct cx_error));

  cx_add_macro(cx, "let:", let_parse);
  cx_add_macro(cx, "func:", func_parse);
  
  cx->any_type = cx_add_type(cx, "A", NULL);

  cx->meta_type = cx_init_meta_type(cx);
  cx->bool_type = cx_init_bool_type(cx);
  cx->int_type = cx_init_int_type(cx);
  cx->func_type = cx_init_func_type(cx);
  cx->lambda_type = cx_init_lambda_type(cx);
  cx->coro_type = cx_init_coro_type(cx);

  cx_add_func(cx, "dup", cx_arg(cx->any_type))->ptr = dup_imp;
  cx_add_func(cx, "zap", cx_arg(cx->any_type))->ptr = zap_imp;
  cx_add_func(cx, "cls")->ptr = cls_imp;

  cx_add_func(cx, "call", cx_arg(cx->any_type))->ptr = call_imp;
  cx_add_func(cx, "=", cx_arg(cx->any_type), cx_arg(cx->any_type))->ptr = eqval_imp;
  cx_add_func(cx, "==", cx_arg(cx->any_type), cx_arg(cx->any_type))->ptr = equid_imp;
  cx_add_func(cx, "test", cx_arg(cx->bool_type))->ptr = test_imp;
  
  cx->main = cx_begin(cx, false);
  return cx;
}

struct cx *cx_deinit(struct cx *cx) {
  cx_set_deinit(&cx->separators);
  
  cx_do_vec(&cx->scopes, struct cx_scope *, s) { cx_scope_unref(*s); }
  cx_vec_deinit(&cx->scopes);

  cx_do_set(&cx->macros, struct cx_macro *, m) { free(cx_macro_deinit(*m)); }
  cx_set_deinit(&cx->macros);

  cx_do_set(&cx->funcs, struct cx_func *, f) { free(cx_func_deinit(*f)); }
  cx_set_deinit(&cx->funcs);

  cx_do_set(&cx->types, struct cx_type *, t) { free(cx_type_deinit(*t)); }
  cx_set_deinit(&cx->types);

  cx_do_vec(&cx->errors, char *, e) { free(*e); }
  cx_vec_deinit(&cx->errors);
  return cx;
}

void cx_add_separators(struct cx *cx, const char *cs) {
  for (const char *c = cs; *c; c++) {
    *(char *)cx_ok(cx_set_insert(&cx->separators, c)) = *c;
  }
}

bool cx_is_separator(struct cx *cx, char c) {
  return cx_set_get(&cx->separators, &c);
}

struct cx_type *cx_add_type(struct cx *cx, const char *id, ...) {
  struct cx_type **t = cx_ok(cx_set_insert(&cx->types, &id));

  if (!t) {
    cx_error(cx, cx->row, cx->col, "Duplicate type: '%s'", id);
    return NULL;
  }
  
  *t = cx_type_init(malloc(sizeof(struct cx_type)), id);
    
  va_list parents;
  va_start(parents, id);				
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_type_add_parent(*t, pt); }
  va_end(parents);					
  return *t;
}

struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent) {
  struct cx_type **t = cx_set_get(&cx->types, &id);

  if (!t && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id);
  }

  return t ? *t : NULL;
}

struct cx_func_imp *_cx_add_func(struct cx *cx,
				 const char *id,
				 int nargs,
				 struct cx_func_arg *args) {
  struct cx_func **f = cx_set_get(&cx->funcs, &id);

  if (f) {
    if ((*f)->nargs != nargs) {
      cx_error(cx,
	       cx->row, cx->col,
	       "Wrong number of args for func '%s': %d/%d",
	       id, nargs, (*f)->nargs);
    }
  } else {
    f = cx_set_insert(&cx->funcs, &id);
    *f = cx_func_init(malloc(sizeof(struct cx_func)), id, nargs);
  }
  
  return cx_func_add_imp(*f, nargs, args);
}

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent) {
  struct cx_func **f = cx_set_get(&cx->funcs, &id);

  if (!f && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown func: '%s'", id);
  }

  return f ? *f : NULL;
}


struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp) {
  struct cx_macro **m = cx_ok(cx_set_insert(&cx->macros, &id));

  if (!m) {
    cx_error(cx, cx->row, cx->col, "Duplicate macro: '%s'", id);
    return NULL;
  }

  *m = cx_macro_init(malloc(sizeof(struct cx_macro)), id, imp); 
  return *m;
}

struct cx_macro *cx_get_macro(struct cx *cx, const char *id, bool silent) {
  struct cx_macro **m = cx_set_get(&cx->macros, &id);

  if (!m && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown macro: '%s'", id);
  }
  
  return m ? *m : NULL;
}

struct cx_scope *cx_scope(struct cx *cx, size_t i) {
  return *(struct cx_scope **)cx_vec_peek(&cx->scopes, i);
}

void cx_push_scope(struct cx *cx, struct cx_scope *scope) {
  *(struct cx_scope **)cx_vec_push(&cx->scopes) = cx_scope_ref(scope);
}

struct cx_scope *cx_pop_scope(struct cx *cx, bool silent) {
  if (cx->scopes.count == 1) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "No open scopes"); }
    return NULL;
  }
  
  struct cx_scope *s = *(struct cx_scope **)cx_vec_pop(&cx->scopes);

  if (s->stack.count) {
    struct cx_box *v = cx_vec_pop(&s->stack);
    *cx_push(cx_scope(cx, 0)) = *v;   
  }

  cx_scope_unref(s);
  return s;
}

struct cx_scope *cx_begin(struct cx *cx, bool child) {
  struct cx_scope *s = cx_scope_new(cx, child ? cx_scope(cx, 0) : NULL);
  cx_push_scope(cx, s);
  return s;
}

void cx_end(struct cx *cx) {
  cx_pop_scope(cx, false);
}

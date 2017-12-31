#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/timer.h"
#include "cixl/types/bin.h"
#include "cixl/types/bool.h"
#include "cixl/types/char.h"
#include "cixl/types/int.h"
#include "cixl/types/lambda.h"
#include "cixl/types/nil.h"
#include "cixl/types/vect.h"
#include "cixl/types/str.h"
#include "cixl/util.h"

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

static bool trait_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing trait id");
    goto exit2;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid trait id");
    goto exit1;
  }

  if (!cx_parse_end(cx, in, &toks)) { goto exit1; }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TTYPE()) {
      cx_error(cx, row, col, "Invalid trait arg");
      goto exit1;
    }
  }

  struct cx_type *trait = cx_add_type(cx, id.as_ptr);

  if (!trait) { goto exit1; }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *child = t->as_ptr;
    cx_derive(child, trait);
  }

  ok = true;
 exit1:
  cx_tok_deinit(&id);
 exit2: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static ssize_t let_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OSCOPE, tok_idx)->as_scope.child = true;  

  if (!cx_compile(cx, cx_vec_get(&eval->toks, 1), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling let");
    return -1;
  }
  
  cx_op_init(cx_vec_push(&bin->ops), CX_OUNSCOPE, tok_idx);
  struct cx_tok *id = cx_vec_get(&eval->toks, 0);
  struct cx_op * op = cx_op_init(cx_vec_push(&bin->ops), CX_OSET, tok_idx);
  op->as_set.id = id->as_ptr;
  op->as_set.force = false;
  op->as_set.parent = false;
  return tok_idx+1;
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

  if (id->type != CX_TID()) {
    cx_error(cx, row, col, "Invalid let id");
    cx_macro_eval_unref(eval);
    return false;
  }
  
  if (!cx_parse_end(cx, in, &eval->toks)) {
    cx_error(cx, row, col, "Empty let");
    cx_macro_eval_unref(eval);
    return false;
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  for (int i = eval->toks.count-1; i >= 0; i--) {
    struct cx_tok *t = cx_vec_get(&eval->toks, i);
    struct cx_op *op = cx_op_init(cx_vec_push(&bin->ops), CX_OSET, tok_idx);
    op->as_set.id = t->as_ptr;
    op->as_set.parent = true;
    op->as_set.force = true;
  }
  
  return tok_idx+1;
}

static bool func_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func id");
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid func id");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func args");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok args = *(struct cx_tok *)cx_vec_pop(&toks);

  if (args.type != CX_TGROUP()) {
    cx_error(cx, row, col, "Invalid func args");
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&toks), CX_TMACRO(), row, col)->as_ptr = eval;
  
  struct cx_vec func_args;
  cx_vec_init(&func_args, sizeof(struct cx_func_arg));

  if (!cx_eval_args(cx, &args.as_vec, &eval->toks, &func_args)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;  
  }

  cx_tok_deinit(&args);

  if (!cx_parse_end(cx, in, &toks)) {
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;
  }

  _cx_add_func(cx,
	       id.as_ptr,
	       func_args.count,
	       (void *)func_args.items)->toks = toks;

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);
  return true;
}

static bool nil_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->nil_type);
  return true;
}

static bool cls_imp(struct cx_scope *scope) {
  cx_vec_clear(&scope->stack);
  return true;
}

static bool dup_imp(struct cx_scope *scope) {
  cx_copy(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool zap_imp(struct cx_scope *scope) {
  cx_box_deinit(cx_test(cx_pop(scope, false)));
  return true;
}

static bool eqval_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_eqval(&x, &y);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool equid_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_equid(&x, &y);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool ok_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_ok(&v);
  cx_box_deinit(&v);
  return true;
}

static bool not_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = !cx_ok(&v);
  cx_box_deinit(&v);
  return true;
}

static bool if_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  cx_call(cx_ok(&c) ? &x : &y, scope);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool compile_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));

  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = cx_parse_str(cx, in.as_ptr, &toks);
  if (!ok) { goto exit; }
  
  struct cx_bin *bin = cx_bin_new();

  if (!(ok = cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin))) {
    goto exit;
  }
  
  cx_box_init(cx_push(scope), cx->bin_type)->as_ptr = bin;
 exit:
  cx_box_deinit(&in);
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

static bool call_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  return ok;
}

static bool recall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (!cx->func_imp) {
    cx_error(cx, cx->row, cx->col, "Nothing to recall");
    return false;
  }

  if (!cx_scan_args(cx, cx->func_imp->func)) { return false; }
  
  if (!cx_func_imp_match(cx->func_imp, &scope->stack)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }

  return cx_eval(cx, cx->func_imp->bin, NULL);
}

static bool clock_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_timer_t timer;
  cx_timer_reset(&timer);
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = cx_timer_ns(&timer);
  return ok;
}

static bool test_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  bool ok = true;
  
  if (!cx_ok(&v)) {
    cx_error(cx, cx->row, cx->col, "Test failed");
    ok = false;
  }

  cx_box_deinit(&v);
  return ok;
}

struct cx *cx_init(struct cx *cx) {
  cx->next_type = 1;
  cx->func_imp = NULL;
  cx->bin = NULL;
  cx->op = NULL;
  cx->stop = false;
  cx->row = cx->col = -1;
  
  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;,|%_?!(){}");

  cx_set_init(&cx->types, sizeof(struct cx_type *), cx_cmp_str);
  cx->types.key = get_type_id;
  
  cx_set_init(&cx->macros, sizeof(struct cx_macro *), cx_cmp_str);
  cx->macros.key = get_macro_id;

  cx_set_init(&cx->funcs, sizeof(struct cx_func *), cx_cmp_str);
  cx->funcs.key = get_func_id;

  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx_vec_init(&cx->errors, sizeof(struct cx_error));

  cx_add_macro(cx, "trait:", trait_parse);
  cx_add_macro(cx, "let:", let_parse);
  cx_add_macro(cx, "func:", func_parse);
  
  cx->opt_type = cx_add_type(cx, "Opt");
  cx->any_type = cx_add_type(cx, "A", cx->opt_type);
  cx->nil_type = cx_init_nil_type(cx);
  cx->meta_type = cx_init_meta_type(cx);
  
  cx->bool_type = cx_init_bool_type(cx);
  cx->int_type = cx_init_int_type(cx);
  cx->char_type = cx_init_char_type(cx);
  cx->str_type = cx_init_str_type(cx);
  cx->vect_type = cx_init_vect_type(cx);
  cx->bin_type = cx_init_bin_type(cx);
  cx->func_type = cx_init_func_type(cx);
  cx->lambda_type = cx_init_lambda_type(cx);

  cx_add_func(cx, "nil")->ptr = nil_imp;
  
  cx_add_func(cx, "|")->ptr = cls_imp;
  cx_add_func(cx, "%", cx_arg(cx->opt_type))->ptr = dup_imp;
  cx_add_func(cx, "_", cx_arg(cx->opt_type))->ptr = zap_imp;
  
  cx_add_func(cx, "=", cx_arg(cx->any_type), cx_narg(0))->ptr = eqval_imp;
  cx_add_func(cx, "==", cx_arg(cx->any_type), cx_narg(0))->ptr = equid_imp;

  cx_add_func(cx, "?", cx_arg(cx->opt_type))->ptr = ok_imp;
  cx_add_func(cx, "!", cx_arg(cx->opt_type))->ptr = not_imp;

  cx_add_func(cx, "if",
	      cx_arg(cx->opt_type),
	      cx_arg(cx->any_type),
	      cx_arg(cx->any_type))->ptr = if_imp;
  
  cx_add_func(cx, "compile", cx_arg(cx->str_type))->ptr = compile_imp;
  cx_add_func(cx, "call", cx_arg(cx->any_type))->ptr = call_imp;
  cx_add_func(cx, "recall", cx_arg(cx->any_type))->ptr = recall_imp;

  cx_add_func(cx, "clock", cx_arg(cx->any_type))->ptr = clock_imp;
  cx_add_func(cx, "test", cx_arg(cx->opt_type))->ptr = test_imp;
  
  cx->main = cx_begin(cx, false);
  return cx;
}

void cx_init_math(struct cx *cx) {
  cx_test(cx_eval_str(cx,
		      "func: fib-rec(a b n Int) "
		      "$n? if {, recall $b, $a + $b, -- $n} $a;"));

  cx_test(cx_eval_str(cx,
		      "func: fib(n Int) "
		      "fib-rec 0 1 $n;"));
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
    *(char *)cx_test(cx_set_insert(&cx->separators, c)) = *c;
  }
}

bool cx_is_separator(struct cx *cx, char c) {
  return cx_set_get(&cx->separators, &c);
}

struct cx_type *_cx_add_type(struct cx *cx, const char *id, ...) {
  struct cx_type **t = cx_test(cx_set_insert(&cx->types, &id));

  if (!t) {
    cx_error(cx, cx->row, cx->col, "Duplicate type: '%s'", id);
    return NULL;
  }
  
  *t = cx_type_init(malloc(sizeof(struct cx_type)), cx, id);
    
  va_list parents;
  va_start(parents, id);				
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_derive(*t, pt); }
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
    *f = cx_func_init(malloc(sizeof(struct cx_func)), cx, id, nargs);
  }
  
  return cx_func_add_imp(*f, nargs, args);
}

bool cx_add_mixl_func(struct cx *cx,
		      const char *id,
		      const char *args,
		      const char *body) {
  char *in = cx_fmt("func: %s(%s) %s;", id, args, body);
  bool ok = cx_eval_str(cx, in);
  free(in);
  return ok;
}

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent) {
  struct cx_func **f = cx_set_get(&cx->funcs, &id);

  if (!f && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown func: '%s'", id);
  }

  return f ? *f : NULL;
}


struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp) {
  struct cx_macro **m = cx_test(cx_set_insert(&cx->macros, &id));

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

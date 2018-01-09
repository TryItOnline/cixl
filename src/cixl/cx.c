#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/timer.h"
#include "cixl/types/bin.h"
#include "cixl/types/bool.h"
#include "cixl/types/char.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/int.h"
#include "cixl/types/lambda.h"
#include "cixl/types/nil.h"
#include "cixl/types/rat.h"
#include "cixl/types/rec.h"
#include "cixl/types/str.h"
#include "cixl/types/sym.h"
#include "cixl/types/time.h"
#include "cixl/types/vect.h"
#include "cixl/util.h"
#include "cixl/var.h"

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

  struct cx_tok id_tok = *(struct cx_tok *)cx_vec_pop(&toks);
  struct cx_type *type = NULL;

  if (id_tok.type == CX_TTYPE()) {
    type = id_tok.as_ptr;
    
    if (!type->trait) {
      cx_error(cx, row, col, "Attempt to redefine %s as trait", type->id);
      goto exit1;
    }
  }

  if (id_tok.type != CX_TID() && id_tok.type != CX_TTYPE()) {
    cx_error(cx, row, col, "Invalid trait id");
    goto exit1;
  }

  if (!cx_parse_end(cx, in, &toks, false)) {
    cx_error(cx, cx->row, cx->col, "Missing trait end");
    goto exit1;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TTYPE()) {
      cx_error(cx, row, col, "Invalid trait arg");
      goto exit1;
    }
  }

  if (type) {
    cx_do_set(&cx->types, struct cx_type *, i) {
      struct cx_type *t = *i;
      if (cx_set_delete(&t->parents, &type->id)) {
	t->tags ^= type->tag;
      }
    }
  } else {
    type = cx_add_type(cx, id_tok.as_ptr);
    if (!type) { goto exit1; }
    type->trait = true;
  }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *child = t->as_ptr;
    cx_derive(child, type);
  }

  ok = true;
 exit1:
  cx_tok_deinit(&id_tok);
 exit2: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  for (int i = eval->toks.count-1; i >= 0; i--) {
    struct cx_tok *t = cx_vec_get(&eval->toks, i);

    if (t->type == CX_TID()) {
      struct cx_op *op = cx_op_init(cx_vec_push(&bin->ops), CX_OSET(), tok_idx);
      op->as_set.type = NULL;
      op->as_set.id = cx_sym(cx, t->as_ptr);
      op->as_set.pop_parent = true;
      op->as_set.set_parent = false;
      op->as_set.force = true;
    } else {
      cx_op_init(cx_vec_push(&bin->ops), CX_OZAP(), tok_idx)->as_zap.parent = true;
    }
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

  if (!cx_parse_end(cx, in, &toks, true)) {
    cx_error(cx, cx->row, cx->col, "Missing func end");
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

static ssize_t repeat_eval(struct cx_macro_eval *eval,
			   struct cx_bin *bin,
			   size_t tok_idx,
			   struct cx *cx) {
  if (!cx_compile(cx, cx_vec_start(&eval->toks), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling repeat");
    return -1;
  }

  return tok_idx+1;
}

static bool repeat_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, true)) {
    cx_error(cx, cx->row, cx->col, "Missing repeat prefix");
    goto exit;
  }
  
  if (!cx_parse_end(cx, in, &toks, true)) {
    cx_error(cx, cx->row, cx->col, "Missing repeat end");
    goto exit;
  }
  
  struct cx_tok *prefix = cx_vec_get(&toks, 0);
  struct cx_macro_eval *eval = cx_macro_eval_new(repeat_eval);

  for (struct cx_tok *t = cx_vec_get(&toks, 1), *pt = NULL;
       t != cx_vec_end(&toks);
       pt = t, t++) {
    if (!pt || pt->type == CX_TCUT()) {
        if (prefix->type == CX_TGROUP()) {
	  cx_do_vec(&prefix->as_vec, struct cx_tok, tt) {
	    cx_tok_copy(cx_vec_push(&eval->toks), tt);
	  }
	} else {
	  cx_tok_copy(cx_vec_push(&eval->toks), prefix);
	}
    }
    
    cx_tok_copy(cx_vec_push(&eval->toks), t);
  }
					  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  ok = true;
 exit: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool cls_imp(struct cx_scope *scope) {
  cx_do_vec(&scope->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&scope->stack);
  return true;
}

static bool copy_imp(struct cx_scope *scope) {
  cx_copy(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool clone_imp(struct cx_scope *scope) {
  cx_clone(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool flip_imp(struct cx_scope *scope) {
  if (scope->stack.count < 2) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Nothing to flip");
    return false;
  }

  struct cx_box *ptr = cx_vec_peek(&scope->stack, 0), tmp = *ptr;
  *ptr = *(ptr-1);
  *(ptr-1) = tmp;
  return true;
}

static bool eqval_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_eqval(&x, &y, scope);
  
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
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_peek(scope, false));
  if (v.type == cx->bool_type) { return true; }
  cx_pop(scope, false);
  cx_box_init(cx_push(scope), cx->bool_type)->as_bool = cx_ok(&v);
  cx_box_deinit(&v);
  return true;
}

static bool not_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box *v = cx_test(cx_peek(scope, false));

  if (v->type == cx->bool_type) {
    v->as_bool = !v->as_bool;
    return true;
  }
  
  bool ok = cx_ok(v);
  cx_box_deinit(v);  
  cx_pop(scope, false);
  cx_box_init(cx_push(scope), cx->bool_type)->as_bool = !ok;
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
  
  if (!cx->fimp) {
    cx_error(cx, cx->row, cx->col, "Nothing to recall");
    return false;
  }

  if (!cx_scan_args(cx, cx->fimp->func)) { return false; }
  
  if (!cx_fimp_match(cx->fimp, &scope->stack, scope)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }

  return cx_fimp_eval(cx->fimp, scope);
}

static bool upcall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (!cx->fimp || !cx->fimp->i) {
    cx_error(cx, cx->row, cx->col, "Nothing to upcall");
    return false;
  }

  struct cx_func *func = cx->fimp->func;
  if (!cx_scan_args(cx, func)) { return false; }

  struct cx_fimp *imp = cx_func_get_imp(func,
					&scope->stack,
					func->imps.count-cx->fimp->i,
					scope);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Upcall not applicable");
    return false;
  }

  return cx_fimp_call(imp, scope);
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
  cx->next_sym_tag = cx->next_type_tag = 1;
  cx->fimp = NULL;
  cx->bin = NULL;
  cx->op = NULL;
  cx->stop = false;
  cx->row = cx->col = -1;
  
  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;,|_?!()[]{}<>");

  cx_set_init(&cx->syms, sizeof(struct cx_sym), cx_cmp_str);
  cx->consts.key_offs = offsetof(struct cx_sym, id);

  cx_set_init(&cx->types, sizeof(struct cx_type *), cx_cmp_str);
  cx->types.key = get_type_id;
  
  cx_set_init(&cx->macros, sizeof(struct cx_macro *), cx_cmp_str);
  cx->macros.key = get_macro_id;

  cx_set_init(&cx->funcs, sizeof(struct cx_func *), cx_cmp_str);
  cx->funcs.key = get_func_id;

  cx_set_init(&cx->consts, sizeof(struct cx_var), cx_cmp_sym);
  cx->consts.key_offs = offsetof(struct cx_var, id);

  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx_vec_init(&cx->errors, sizeof(struct cx_error));

  cx_add_macro(cx, "trait:", trait_parse);
  cx_add_macro(cx, "func:", func_parse);
  cx_add_macro(cx, "repeat:", repeat_parse);
  
  cx->opt_type = cx_add_type(cx, "Opt");
  cx->opt_type->trait = true;

  cx->any_type = cx_add_type(cx, "A", cx->opt_type);
  cx->any_type->trait = true;

  cx->num_type = cx_add_type(cx, "Num", cx->any_type);
  cx->num_type->trait = true;
  
  cx->rec_type = cx_add_type(cx, "Rec", cx->any_type);
  cx->rec_type->trait = true;

  cx->nil_type = cx_init_nil_type(cx);
  cx->meta_type = cx_init_meta_type(cx);
  
  cx->bool_type = cx_init_bool_type(cx);
  cx->int_type = cx_init_int_type(cx);
  cx->rat_type = cx_init_rat_type(cx);
  cx->char_type = cx_init_char_type(cx);
  cx->str_type = cx_init_str_type(cx);
  cx->sym_type = cx_init_sym_type(cx);
  cx->time_type = cx_init_time_type(cx);
  cx->vect_type = cx_init_vect_type(cx);
  cx->bin_type = cx_init_bin_type(cx);
  cx->func_type = cx_init_func_type(cx);
  cx->fimp_type = cx_init_fimp_type(cx);
  cx->lambda_type = cx_init_lambda_type(cx);
  
  cx_add_func(cx, "|")->ptr = cls_imp;
  cx_add_func(cx, "%", cx_arg(cx->opt_type))->ptr = copy_imp;
  cx_add_func(cx, "%%", cx_arg(cx->opt_type))->ptr = clone_imp;
  cx_add_func(cx, "~", cx_arg(cx->opt_type))->ptr = flip_imp;
  
  cx_add_func(cx, "=", cx_arg(cx->opt_type), cx_narg(0))->ptr = eqval_imp;
  cx_add_func(cx, "==", cx_arg(cx->opt_type), cx_narg(0))->ptr = equid_imp;

  cx_add_func(cx, "?", cx_arg(cx->opt_type))->ptr = ok_imp;
  cx_add_func(cx, "!", cx_arg(cx->opt_type))->ptr = not_imp;

  cx_add_func(cx, "if",
	      cx_arg(cx->opt_type),
	      cx_arg(cx->any_type),
	      cx_arg(cx->any_type))->ptr = if_imp;
  
  cx_add_func(cx, "compile", cx_arg(cx->str_type))->ptr = compile_imp;
  cx_add_func(cx, "call", cx_arg(cx->any_type))->ptr = call_imp;
  cx_add_func(cx, "recall")->ptr = recall_imp;
  cx_add_func(cx, "upcall")->ptr = upcall_imp;

  cx_add_func(cx, "clock", cx_arg(cx->any_type))->ptr = clock_imp;
  cx_add_func(cx, "test", cx_arg(cx->opt_type))->ptr = test_imp;
  
  cx->main = cx_begin(cx, NULL);
  return cx;
}

struct cx *cx_deinit(struct cx *cx) {
  cx_set_deinit(&cx->separators);
  
  cx_do_vec(&cx->scopes, struct cx_scope *, s) { cx_scope_unref(*s); }
  cx_vec_deinit(&cx->scopes);

  cx_do_set(&cx->consts, struct cx_var, v) { cx_var_deinit(v); }
  cx_set_deinit(&cx->consts);

  cx_do_set(&cx->macros, struct cx_macro *, m) { free(cx_macro_deinit(*m)); }
  cx_set_deinit(&cx->macros);

  cx_do_set(&cx->funcs, struct cx_func *, f) { free(cx_func_deinit(*f)); }
  cx_set_deinit(&cx->funcs);

  cx_do_set(&cx->types, struct cx_type *, t) { free(cx_type_deinit(*t)); }
  cx_set_deinit(&cx->types);

  cx_do_set(&cx->syms, struct cx_sym, s) { cx_sym_deinit(s); }
  cx_set_deinit(&cx->syms);

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

struct cx_rec_type *cx_add_rec_type(struct cx *cx, const char *id) {
  struct cx_type **found = cx_set_get(&cx->types, &id);
  if (found) { return NULL; }
  
  if (found) {
    struct cx_rec_type *t = cx_baseof(*found, struct cx_rec_type, imp);
    cx_rec_type_reinit(t);
    return t;
  }
  
  struct cx_rec_type *t = cx_rec_type_new(cx, id);
  *(struct cx_type **)cx_test(cx_set_insert(&cx->types, &id)) = &t->imp;
  cx_derive(&t->imp, cx->rec_type);
  return t;
}

struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent) {
  struct cx_type **t = cx_set_get(&cx->types, &id);

  if (!t && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id);
  }

  return t ? *t : NULL;
}

struct cx_fimp *_cx_add_func(struct cx *cx,
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

struct cx_box *cx_get_const(struct cx *cx, struct cx_sym id, bool silent) {
  struct cx_var *var = cx_set_get(&cx->consts, &id);

  if (!var) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "Unknown const: '%s'", id); }
    return NULL;
  }

  return &var->value;
}

struct cx_box *cx_set_const(struct cx *cx, struct cx_sym id, bool force) {
  struct cx_var *var = cx_set_get(&cx->consts, &id);

  if (var) {
    if (!force) {
      cx_error(cx, cx->row, cx->col, "Attempt to rebind const: '%s'", id);
      return NULL;
    }
      
    cx_box_deinit(&var->value);
  } else {
    var = cx_var_init(cx_set_insert(&cx->consts, &id), id);
  }

  return &var->value;
}

struct cx_sym cx_sym(struct cx *cx, const char *id) {
  struct cx_sym *s = cx_set_get(&cx->syms, &id);
  return s ? *s : *cx_sym_init(cx_set_insert(&cx->syms, &id), id, cx->next_sym_tag++);
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

struct cx_scope *cx_begin(struct cx *cx, struct cx_scope *parent) {
  struct cx_scope *s = cx_scope_new(cx, parent);
  cx_push_scope(cx, s);
  return s;
}

void cx_end(struct cx *cx) {
  cx_pop_scope(cx, false);
}

bool cx_funcall(struct cx *cx, const char *id) {
  struct cx_func *func = cx_get_func(cx, id, false);
  if (!func) { return false; }
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_fimp *imp = cx_func_get_imp(func, &s->stack, 0, s);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
  
  return cx_fimp_call(imp, s);
}

bool cx_load(struct cx *cx, const char *path) {
  FILE *f = fopen(path , "r");

  if (!f) {
    cx_error(cx, cx->row, cx->col, "Failed opening file '%s': %d", path, errno);
    return false;
  }
  
  fseek(f, 0, SEEK_END);
  size_t len = ftell(f);
  rewind(f);

  char *buf = malloc(len+1);
  
  if (fread(buf, len, 1 , f) != 1) {
    cx_error(cx, cx->row, cx->col, "Failed reading file '%s': %d", path, errno);
  }
  
  fclose(f);
  buf[len] = 0;
  bool ok = cx_eval_str(cx, buf);
  free(buf);
  return ok;
}

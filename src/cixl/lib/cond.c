#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/bool.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/cond.h"
#include "cixl/op.h"
#include "cixl/scope.h"

static ssize_t switch_eval(struct cx_macro_eval *eval,
			   struct cx_bin *bin,
			   size_t tok_idx,
			   struct cx *cx) {
  size_t start = bin->ops.count;
  
  for (struct cx_tok *t = cx_vec_start(&eval->toks);
       t != cx_vec_end(&eval->toks);
       t++) {
    struct cx_vec *ts = &t->as_vec;
    struct cx_tok *c = cx_vec_start(ts);
    
    if (c->type == CX_TGROUP()) {
      struct cx_vec *cts = &c->as_vec;
      cx_compile(cx, cx_vec_start(cts), cx_vec_end(cts), bin);
    } else {
      cx_compile(cx, cx_vec_start(ts), cx_vec_get(ts, 1), bin);
    }
    
    size_t ei = bin->ops.count;
    cx_op_init(bin, CX_OELSE(), tok_idx);
    cx_compile(cx, cx_vec_get(ts, 1), cx_vec_end(ts), bin);
    cx_op_init(bin, CX_OJUMP(), tok_idx);

    struct cx_op *eop = cx_vec_get(&bin->ops, ei);
    eop->as_else.nops = bin->ops.count-ei-1;
  }

  for (struct cx_op *op = cx_vec_get(&bin->ops, start);
       op != cx_vec_end(&bin->ops);
       op++) {    
    if (op->type == CX_OJUMP()) {
      op->as_jump.nops = (struct cx_op *)cx_vec_end(&bin->ops)-op-1;
    }
  }
  
  return tok_idx+1;
}

static bool switch_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_macro_eval *eval = cx_macro_eval_new(switch_eval);

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_end(cx, in, &eval->toks, true)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing switch end"); }
    goto error;
  }

  for (struct cx_tok *t = cx_vec_start(&eval->toks);
       t != cx_vec_end(&eval->toks);
       t++) {
    if (t->type != CX_TGROUP()) {
      cx_error(cx, row, col, "Invalid switch clause: %s", t->type->id);
      goto error;
    }
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
 error:
  cx_macro_eval_deref(eval);
  return false;  
}

static bool int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = v.as_bool ? 1 : 0;
  return true;
}

static bool eqval_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_eqval(&x, &y);
  
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

static struct cx_sym cmp_sym(struct cx *cx, enum cx_cmp cmp) {
  switch (cmp) {
  case CX_CMP_LT:
    return cx_sym(cx, "<");
  case CX_CMP_EQ:
    return cx_sym(cx, "=");
  default:
    break;
  }

  return cx_sym(cx, ">");
}

static bool cmp_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope), cx->sym_type)->as_sym = cmp_sym(cx, cx_cmp(&x, &y));
  return true;
}

static bool lt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_cmp(&x, &y) == CX_CMP_LT;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool gt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_cmp(&x, &y) == CX_CMP_GT;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool lte_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  enum cx_cmp cmp = cx_cmp(&x, &y);
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cmp == CX_CMP_LT || cmp == CX_CMP_EQ;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool gte_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  enum cx_cmp cmp = cx_cmp(&x, &y);
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cmp == CX_CMP_GT || cmp == CX_CMP_EQ;
  
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

static bool and_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  if (!cx_call(&x, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&x);
  struct cx_box *xc = cx_pop(scope, false);
  
  if (!xc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    cx_box_deinit(&y);
    return false;    
  }

  struct cx_box xcv = *xc;
  
  if (!cx_ok(&xcv)) {
    cx_box_deinit(&y);
    *cx_push(scope) = xcv;
    return true;
  }
  
  cx_box_deinit(&xcv);
  
  if (!cx_call(&y, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&y);
  struct cx_box *yc = cx_pop(scope, false);

  if (!yc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    return false;    
  }

  struct cx_box ycv = *yc;
  *cx_push(scope) = ycv;
  return true;
}

static bool or_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  if (!cx_call(&x, scope)) {
    cx_box_deinit(&x);
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&x);
  struct cx_box *xc = cx_pop(scope, false);

  if (!xc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    cx_box_deinit(&y);
    return false;    
  }

  struct cx_box xcv = *xc;
  
  if (cx_ok(&xcv)) {
    cx_box_deinit(&y);
    *cx_push(scope) = xcv;
    return true;
  }
  
  cx_box_deinit(&xcv);
  
  if (!cx_call(&y, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&y);
  struct cx_box *yc = cx_pop(scope, false);

  if (!yc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    return false;    
  }
  
  struct cx_box ycv = *yc;
  *cx_push(scope) = ycv;
  return true;
}

static bool if_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  if (cx_ok(&c)) { cx_call(&x, scope); }
  cx_box_deinit(&x);
  cx_box_deinit(&c);
  return true;
}

static bool else_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  if (!cx_ok(&c)) { cx_call(&x, scope); }
  cx_box_deinit(&x);
  cx_box_deinit(&c);
  return true;
}

static bool if_else_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  cx_call(cx_ok(&c) ? &x : &y, scope);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  cx_box_deinit(&c);
  return true;
}

cx_lib(cx_init_cond, "cx/cond", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc", "A", "Opt");
    cx_use(cx, "cx/sym", "Sym");

    cx->bool_type = cx_init_bool_type(lib);
    
    cx_box_init(cx_set_const(lib, cx_sym(cx, "t"), false),
		cx->bool_type)->as_bool = true;
    cx_box_init(cx_set_const(lib, cx_sym(cx, "f"), false),
		cx->bool_type)->as_bool = false;

    cx_add_macro(lib, "switch:", switch_parse);
  
    cx_add_cfunc(lib, "int",
		 cx_args(cx_arg("v", cx->bool_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 int_imp);

    cx_add_cfunc(lib, "=",
		 cx_args(cx_arg("x", cx->opt_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 eqval_imp);
  
    cx_add_cfunc(lib, "==",
		 cx_args(cx_arg("x", cx->opt_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 equid_imp);

    cx_add_cfunc(lib, "<=>",
		 cx_args(cx_arg("x", cx->cmp_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->sym_type)),
		 cmp_imp);
  
    cx_add_cfunc(lib, "<",
		 cx_args(cx_arg("x", cx->cmp_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 lt_imp);
  
    cx_add_cfunc(lib, ">",
		 cx_args(cx_arg("x", cx->cmp_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 gt_imp);
  
    cx_add_cfunc(lib, "<=",
		 cx_args(cx_arg("x", cx->cmp_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 lte_imp);
  
    cx_add_cfunc(lib, ">=",
		 cx_args(cx_arg("x", cx->cmp_type), cx_narg("y", 0)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 gte_imp);
  
    cx_add_cfunc(lib, "?",
		 cx_args(cx_arg("v", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 ok_imp);
  
    cx_add_cfunc(lib, "!",
		 cx_args(cx_arg("v", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->bool_type)),
		 not_imp);
  
    cx_add_cfunc(lib, "and",
		 cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 and_imp);
  
    cx_add_cfunc(lib, "or",
		 cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 or_imp);
  
    cx_add_cfunc(lib, "if",
		 cx_args(cx_arg("cnd", cx->opt_type), cx_arg("act", cx->any_type)),
		 cx_args(),
		 if_imp);

    cx_add_cfunc(lib, "else",
		 cx_args(cx_arg("cnd", cx->opt_type), cx_arg("act", cx->any_type)),
		 cx_args(),
		 else_imp);

    cx_add_cfunc(lib, "if-else",
		 cx_args(cx_arg("cnd", cx->opt_type),
			 cx_arg("tact", cx->any_type), cx_arg("fact", cx->any_type)),
		 cx_args(),
		 if_else_imp);
  })

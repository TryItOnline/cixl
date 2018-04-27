#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/meta.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static ssize_t lib_eval(struct cx_macro_eval *eval,
			    struct cx_bin *bin,
			    size_t tok_idx,
			    struct cx *cx) {
  struct cx_tok *t = cx_vec_start(&eval->toks);
  struct cx_lib *lib = t->as_lib;
  struct cx_op *op = cx_op_new(bin, CX_OLIBDEF(), tok_idx);
  op->as_libdef.lib = lib;
  op->as_libdef.init = lib->inits.count;
  size_t start_pc = bin->ops.count;
  cx_op_new(bin, CX_OPUSHLIB(), tok_idx)->as_pushlib.lib = lib;
  t++;

  cx_push_lib(cx, lib);
  bool ok = cx_compile(cx, t, cx_vec_end(&eval->toks), bin);
  cx_pop_lib(cx);
  
  if (!ok) {
    cx_error(cx, cx->row, cx->col, "Failed compiling lib");
    return -1;
  }

  cx_op_new(bin, CX_OPOPLIB(), tok_idx);
  cx_lib_push_init(lib, cx_lib_ops(bin, start_pc, bin->ops.count-start_pc));  
  return tok_idx+1;
}

static bool lib_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(lib_eval);

  if (!cx_parse_tok(cx, in, &eval->toks)) {
    cx_error(cx, row, col, "Missing lib id");
    cx_macro_eval_deref(eval);
    return false;
  }

  struct cx_tok *id = cx_vec_peek(&eval->toks, 0);

  if (id->type != CX_TID()) {
    cx_error(cx, id->row, id->col, "Invalid lib id: %s", id->type->id);
    cx_macro_eval_deref(eval);
    return false;
  }

  struct cx_lib *lib = cx_add_lib(cx, id->as_ptr);
  cx_tok_deinit(id);
  cx_tok_init(id, CX_TLIB(), id->row, id->col)->as_lib = lib;
  struct cx_lib *prev = *cx->lib;
  cx_push_lib(cx, lib);
  cx_lib_use(prev);
  
  if (!cx_parse_end(cx, in, &eval->toks)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing lib: end"); }
    
    cx_pop_lib(cx);
    cx_macro_eval_deref(eval);
    return false;
  }

  cx_pop_lib(cx);
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static ssize_t use_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  cx_op_new(bin, CX_OUSE(), tok_idx);
  return tok_idx+1;
}

static bool use_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(use_eval);
  
  if (!cx_parse_end(cx, in, &eval->toks)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing use: end"); }
    cx_macro_eval_deref(eval);
    return false;
  }

  cx_do_vec(&eval->toks, struct cx_tok, t) {
    if (t->type != CX_TID() && t->type != CX_TGROUP()) {
      cx_error(cx, t->row, t->col, "Invalid lib: %s", t->type->id);
      cx_macro_eval_deref(eval);
      return false;
    }
    
    if (t->type == CX_TID()) {
      if (!cx_use(cx, t->as_ptr)) {
	cx_macro_eval_deref(eval);
	return false;
      }

    } else {
      struct cx_tok *tt = cx_vec_get(&t->as_vec, 0);

      if (tt->type != CX_TID()) {
	cx_error(cx, tt->row, tt->col, "Invalid id: %s", tt->type->id);
	cx_macro_eval_deref(eval);
	return false;
      }

      const char *lib = tt->as_ptr;
      tt++;

      struct cx_vec ids;
      cx_vec_init(&ids, sizeof(const char *));

      for (; tt != cx_vec_end(&t->as_vec); tt++) {
	if (tt->type != CX_TID()) {
	  cx_error(cx, tt->row, tt->col, "Invalid id: %s", tt->type->id);
	  cx_vec_deinit(&ids);
	  cx_macro_eval_deref(eval);
	  return false;
	}
	
	*(char **)cx_vec_push(&ids) = tt->as_ptr;
      }

      if (!cx_vuse(cx, lib, ids.count, (const char **)ids.items)) {
	cx_vec_deinit(&ids);
	cx_macro_eval_deref(eval);
	return false;
      }

      cx_vec_deinit(&ids);
    }        
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool this_lib_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->lib_type)->as_lib = *(s->cx->lib-1);
  return true;
}

static bool lib_id_imp(struct cx_call *call) {
  struct cx_box *lib = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->sym_type)->as_sym = lib->as_lib->id;
  return true;
}

static bool get_lib_imp(struct cx_call *call) {
  struct cx_box *id = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_lib *lib = cx_get_lib(s->cx, id->as_sym.id, true);
  
  if (lib) {
    cx_box_init(cx_push(s), s->cx->lib_type)->as_lib = lib;
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

cx_lib(cx_init_meta, "cx/meta") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Lib", "Opt", "Str", "Sym")) {
    return false;
  }
    
  cx_add_macro(lib, "lib:", lib_parse);
  cx_add_macro(lib, "use:", use_parse);

  cx_add_cfunc(lib, "this-lib",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->lib_type)),
	       this_lib_imp);

  cx_add_cfunc(lib, "id",
	       cx_args(cx_arg("lib", cx->lib_type)),
	       cx_args(cx_arg(NULL, cx->sym_type)),
	       lib_id_imp);

  cx_add_cfunc(lib, "get-lib",
	       cx_args(cx_arg("id", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->lib_type))),
	       get_lib_imp);

  return true;
}

#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/meta.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static ssize_t include_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  if (!cx_compile(cx, cx_vec_start(&eval->toks), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling include");
    return -1;
  }

  return tok_idx+1;
}

static bool include_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  struct cx_vec fns;
  cx_vec_init(&fns, sizeof(struct cx_tok));
  
  if (!cx_parse_end(cx, in, &fns, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing include: end"); }
    goto exit1;
  }

  struct cx_macro_eval *eval = cx_macro_eval_new(include_eval);

  cx_do_vec(&fns, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid include token: %s", t->type->id);
      goto exit2;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid filename: %s", t->as_box.type->id);
      goto exit2;
    }

    char *full_path = cx_get_path(cx, t->as_box.as_str->data);
    bool ok = cx_load_toks(cx, full_path, &eval->toks);
    free(full_path);
    free(*(char **)cx_vec_pop(&cx->load_paths));
    if (!ok) { goto exit2; }
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  ok = true;
  goto exit1;
 exit2:
  cx_macro_eval_deref(eval);
 exit1: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
}

static ssize_t use_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  cx_op_init(bin, CX_OUSE(), tok_idx);
  return tok_idx+1;
}

static bool use_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(use_eval);
  
  if (!cx_parse_end(cx, in, &eval->toks, false)) {
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

      if (!_cx_use(cx, lib, ids.count, (const char **)ids.items)) {
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

cx_lib(cx_init_meta, "cx/meta", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/str", "Str");
    
    cx_add_macro(lib, "include:", include_parse);
    cx_add_macro(lib, "use:", use_parse);
  })

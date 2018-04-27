#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/const.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static ssize_t define_eval(struct cx_macro_eval *eval,
			   struct cx_bin *bin,
			   size_t tok_idx,
			   struct cx *cx) {
  for (struct cx_tok *t = cx_vec_start(&eval->toks);
       t != cx_vec_end(&eval->toks);
       t++) {
    struct cx_op *op = cx_op_new(bin, CX_OPUTCONST(), tok_idx);
    op->as_putconst.id = cx_sym(cx, t->as_ptr);
    op->as_putconst.lib = *cx->lib;
  }

  return tok_idx+1;
}

static bool define_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  struct cx_macro_eval *eval = cx_macro_eval_new(define_eval);
  
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks)) {
    cx_error(cx, row, col, "Missing define id");
    goto exit1;
  }

  struct cx_tok id_tok = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id_tok.type != CX_TID() && id_tok.type != CX_TGROUP()) {
    cx_error(cx, id_tok.row, id_tok.col, "Invalid define id");
    goto exit1;
  }

  if (!cx_parse_end(cx, in, &toks)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing define end"); }
    goto exit1;
  }

  struct cx_scope *s = cx_begin(cx, NULL);
  if (!cx_eval_toks(cx, &toks)) { goto exit2; }

  bool put(struct cx_tok *id, struct cx_type *type) {
    struct cx_box *src = cx_pop(s, true);

    if (!src) {
      cx_error(cx, row, col, "Missing value for id: %s", id);
      return false;
    }

    if (type && !cx_is(src->type, type)) {
      cx_error(cx, row, col,
	       "Expected type %s, actual: %s",
	       type->id, src->type->id);
      
      return false;
    }
    
    struct cx_box *dst = cx_put_const(*cx->lib, cx_sym(cx, id->as_ptr), false);

    if (dst) {
      *dst = *src;
      cx_tok_copy(cx_vec_push(&eval->toks), id);
    }

    return true;
  }

  if (id_tok.type == CX_TID()) {
    if (!put(&id_tok, NULL)) { goto exit2; }
    ok = true;
  } else {
    struct cx_vec *id_toks = &id_tok.as_vec, ids, types;
    cx_vec_init(&ids, sizeof(struct cx_tok));
    cx_vec_init(&types, sizeof(struct cx_type *));
    
    bool push_type(struct cx_type *type) {
      if (ids.count == types.count) {
	cx_error(cx, cx->row, cx->col, "Missing define id");
	return false;
      }
      
      for (struct cx_tok *id = cx_vec_get(&ids, types.count);
	   id != cx_vec_end(&ids);
	   id++) {
	*(struct cx_type **)cx_vec_push(&types) = type;	
      }

      return true;
    }
    
    cx_do_vec(id_toks, struct cx_tok, t) {
      if (t->type == CX_TID()) {
	char *id = t->as_ptr;
	if (isupper(id[0])) {
	  struct cx_type *type = cx_get_type(cx, id, false);
	  if (!type || !push_type(type)) { goto exit3; }
	} else {
	  *(struct cx_tok *)cx_vec_push(&ids) = *t;
	}
      } else {
	goto exit3;
      }
    }

    if (ids.count > types.count && !push_type(NULL)) { goto exit3; }
    struct cx_tok *id = cx_vec_peek(&ids, 0);
    struct cx_type **type = cx_vec_peek(&types, 0);
    
    for (; id >= (struct cx_tok *)ids.items; id--, type--) {
      if (!put(id, *type)) { goto exit3; }
    }

    ok = true;
  exit3:
    cx_vec_deinit(&ids);
    cx_vec_deinit(&types);
  }
  
  cx_tok_deinit(&id_tok);
  if (ok && s->stack.count) { cx_error(cx, row, col, "Too many values in define"); }
 exit2:
  cx_reset(s);
  cx_end(cx);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    
    if (ok) {
      cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
    } else {
      cx_macro_eval_deref(eval);
    }
    
    return ok;
  }
}

cx_lib(cx_init_const, "cx/const") {
  //struct cx *cx = lib->cx;
    
  cx_add_macro(lib, "define:", define_parse);
  return true;
}

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/types/lambda.h"
#include "cixl/util.h"
#include "cixl/vec.h"

bool cx_eval(struct cx *cx, struct cx_bin *bin, struct cx_op *start) {
  if (!bin->ops.count) { return true; }
  struct cx_bin *prev_bin = cx->bin;
  struct cx_op *prev_op = cx->op;

  cx->bin = bin;
  cx->op = start ? start : cx_vec_start(&bin->ops);
  bool ok = false;
  struct cx_op *end = cx_vec_end(&cx->bin->ops);
    
  while (cx->op != end && !cx->stop) {
    if (!cx_eval_next(cx)) { goto exit; }
  }

  ok = true;
 exit:
  cx->bin = prev_bin;
  cx->op = prev_op;
  cx->stop = false;
  return ok;
}

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_str(cx, in, &toks)) { goto exit1; }

  if (!toks.count) {
    ok = true;
    goto exit1;
  }

  struct cx_bin *bin = cx_bin_new();
  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin)) { goto exit2; }
  if (!cx_eval(cx, bin, NULL)) { goto exit2; }
  ok = true;
 exit2:
  cx_bin_unref(bin);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

bool cx_eval_next(struct cx *cx) {
  struct cx_op *op = cx->op++;
  struct cx_tok *tok = cx_vec_get(&cx->bin->toks, op->tok_idx);
  cx->row = tok->row;
  cx->col = tok->col;
  return op->type->eval(op, tok, cx) && !cx->errors.count;
}

bool cx_scan_args(struct cx *cx, struct cx_func *func) {
  int row = cx->row, col = cx->col;
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_op *end = cx_vec_end(&cx->bin->ops);
  
  while (cx->op != end) {
    if (cx_scope(cx, 0) == s && s->stack.count - s->cut_offs >= func->nargs) {
      break;
    }

    if (!cx_eval_next(cx)) { return false; }
  }
  
  if (s->stack.count - s->cut_offs < func->nargs) {
    cx_error(cx, row, col, "Not enough args for func: '%s'", func->id);
    return false;
  }

  s->cut_offs = 0;
  return true;
}

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));

  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      cx_tok_copy(cx_vec_push(&tmp_ids), t);
    } else if (t->type == CX_TLITERAL()) {
      struct cx_box *v = &t->as_box;

      if (v->type != cx->int_type || v->as_int >= func_args->count) {
	cx_error(cx, t->row, t->col, "Invalid arg");
	cx_vec_deinit(&tmp_ids);
	return false;
      }

      if (!tmp_ids.count) {
	cx_error(cx, t->row, t->col, "Missing args for type: %d", v->as_int);
	cx_vec_deinit(&tmp_ids);
	return false;
      }
      
      cx_do_vec(&tmp_ids, struct cx_tok, id) {
	*(struct cx_tok *)cx_vec_push(ids) = *id;
	*(struct cx_func_arg *)cx_vec_push(func_args) = cx_narg(v->as_int);      
      }
    } else if (t->type == CX_TTYPE()) {
      struct cx_type *type = t->as_ptr;

      if (!tmp_ids.count) {
	cx_error(cx, t->row, t->col, "Missing args for type: %s", type->id);
	cx_vec_deinit(&tmp_ids);
	return false;
      }

      cx_do_vec(&tmp_ids, struct cx_tok, id) {
	*(struct cx_tok *)cx_vec_push(ids) = *id;
	*(struct cx_func_arg *)cx_vec_push(func_args) = cx_arg(type);      
      }
      
      cx_vec_clear(&tmp_ids);
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      cx_vec_deinit(&tmp_ids);
      return false;
    }
  }
  
  cx_vec_deinit(&tmp_ids);
  return true;
}

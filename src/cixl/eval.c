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

static ssize_t cut_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_cut_op, tok_idx);
  return tok_idx+1;
}

cx_tok_type(cx_cut_tok, {
    type.compile = cut_compile;
  });

cx_tok_type(cx_end_tok);

static ssize_t func_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_funcall_op *op = &cx_op_init(cx_vec_push(&bin->ops),
					 cx_funcall_op,
					 tok_idx)->as_funcall;
  
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);  
  op->func = tok->as_ptr;
  op->imp = (op->func->imps.members.count == 1)
    ? *(struct cx_func_imp **)cx_vec_start(&op->func->imps.members)
    : NULL;
  
  return tok_idx+1;
}

cx_tok_type(cx_func_tok, {
    type.compile = func_compile;
  });

static ssize_t group_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_scope_op, tok_idx)->as_scope.child = true;
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_vec toks = tok->as_vec;
  cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin);
  cx_op_init(cx_vec_push(&bin->ops), cx_unscope_op, tok_idx);
  return tok_idx+1;
}

static void group_copy(struct cx_tok *dst, struct cx_tok *src) {
  struct cx_vec
    *src_body = &src->as_vec,
    *dst_body = &dst->as_vec;

  cx_vec_init(dst_body, sizeof(struct cx_tok));
  
  if (src_body->count) {
    cx_vec_grow(dst_body, src_body->count);
    
    cx_do_vec(src_body, struct cx_tok, t) {
      cx_tok_copy(cx_vec_push(dst_body), t);
    }
  }
}

static void group_deinit(struct cx_tok *tok) {
  struct cx_vec *body = &tok->as_vec;
  cx_do_vec(body, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(body);
}

cx_tok_type(cx_group_tok, {
    type.compile = group_compile;
    type.copy = group_copy;
    type.deinit = group_deinit;
  });

static ssize_t id_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  char *id = tok->as_ptr;
  
  if (id[0] != '$') {
    cx_error(cx, tok->row, tok->col, "Unknown id: '%s'", id);
    return -1;
  }

  cx_op_init(cx_vec_push(&bin->ops), cx_get_op, tok_idx)->as_get.id = id+1;
  return tok_idx+1;
}

static void id_copy(struct cx_tok *dst, struct cx_tok *src) {
    dst->as_ptr = strdup(src->as_ptr);
}

static void id_deinit(struct cx_tok *tok) {
    free(tok->as_ptr);
}

cx_tok_type(cx_id_tok, {
    type.compile = id_compile;
    type.copy = id_copy;
    type.deinit = id_deinit;
  });

static ssize_t lambda_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);

  size_t i = bin->ops.count;
  cx_op_init(cx_vec_push(&bin->ops),
	     cx_lambda_op,
	     tok_idx)->as_lambda.start_op = i+1;
  
  struct cx_vec toks = tok->as_vec;
  cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin);
  cx_op_init(cx_vec_push(&bin->ops), cx_stop_op, tok_idx);
  struct cx_op *op = cx_vec_get(&bin->ops, i);
  op->as_lambda.num_ops = bin->ops.count - op->as_lambda.start_op;
  return tok_idx+1;
}

static void lambda_copy(struct cx_tok *dst, struct cx_tok *src) {
  group_copy(dst, src);
}

static void lambda_deinit(struct cx_tok *tok) {
  group_deinit(tok);
}

cx_tok_type(cx_lambda_tok, {
    type.compile = lambda_compile;
    type.copy = lambda_copy;
    type.deinit = lambda_deinit;
  });

static ssize_t literal_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_push_op, tok_idx);
  return tok_idx+1;
}

static void literal_copy(struct cx_tok *dst, struct cx_tok *src) {
  cx_copy(&dst->as_box, &src->as_box);
}

static void literal_deinit(struct cx_tok *tok) {
  cx_box_deinit(&tok->as_box);
}

cx_tok_type(cx_literal_tok, {
    type.compile = literal_compile;
    type.copy = literal_copy;
    type.deinit = literal_deinit;
  });

static ssize_t macro_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_macro_op, tok_idx);
  return tok_idx+1;
}

static void macro_copy(struct cx_tok *dst, struct cx_tok *src) {
  dst->as_ptr = cx_macro_eval_ref(src->as_ptr);
}
  
static void macro_deinit(struct cx_tok *tok) {
  cx_macro_eval_unref(tok->as_ptr);
}

cx_tok_type(cx_macro_tok, {
    type.compile = macro_compile;
    type.copy = macro_copy;
    type.deinit = macro_deinit;
  });

static ssize_t type_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_type *type = tok->as_ptr;
  tok->type = cx_literal_tok();
  cx_box_init(&tok->as_box, cx->meta_type)->as_ptr = type;    
  cx_op_init(cx_vec_push(&bin->ops), cx_push_op, tok_idx);
  return tok_idx+1;
}

cx_tok_type(cx_type_tok, {
    type.compile = type_compile;
  });

cx_tok_type(cx_ungroup_tok);
cx_tok_type(cx_unlambda_tok);

bool cx_eval(struct cx *cx, struct cx_bin *bin, struct cx_op *start) {
  if (!bin->ops.count) { return true; }
  struct cx_bin *prev_bin = cx->bin;
  struct cx_op *prev_op = cx->op;

  cx->bin = bin;
  cx->op = start ? start : cx_vec_start(&bin->ops);
  bool ok = false;
  
  while (cx->op != cx_vec_end(&cx->bin->ops) && !cx->stop) {
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
  return op->eval(tok, op, cx);
}

bool cx_scan_args(struct cx *cx, struct cx_func *func) {
  int row = cx->row, col = cx->col;
  struct cx_scope *s = cx_scope(cx, 0);

  while (cx->op != cx_vec_end(&cx->bin->ops)) {
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
    if (t->type == cx_id_tok()) {
      cx_tok_copy(cx_vec_push(&tmp_ids), t);
    } else if (t->type == cx_literal_tok()) {
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
    } else if (t->type == cx_type_tok()) {
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

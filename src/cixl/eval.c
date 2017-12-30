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

static bool cut_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  s->cut_offs = s->stack.count;
  return true;
}

cx_tok_type(cx_cut_tok, {
    type.compile = cut_compile;
    type.eval = cut_eval;
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

static bool func_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_func *func = tok->as_ptr;
  int row = cx->row, col = cx->col;
  
  if (!cx_scan_args(cx, func)) { return false; }
    
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_func_imp *imp = cx_func_get_imp(func, &s->stack);

  if (!imp) {
    cx_error(cx, row, col, "Func not applicable: '%s'", func->id);
    return false;
  }

  return cx_func_imp_call(imp, s);
}

cx_tok_type(cx_func_tok, {
    type.compile = func_compile;
    type.eval = func_eval;
  });

static ssize_t group_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_scope_op, tok_idx)->as_scope.child = true;
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  cx_compile(cx, &tok->as_vec, bin);
  cx_op_init(cx_vec_push(&bin->ops), cx_unscope_op, tok_idx);
  return tok_idx+1;
}

static bool group_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_vec *body = &tok->as_vec;
  cx_begin(cx, true);
  bool ok = cx_eval(cx, body, cx_vec_start(body));
  cx_end(cx);
  return ok;
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
    type.eval = group_eval;
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

static bool id_eval(struct cx_tok *tok, struct cx *cx) {
  char *id = tok->as_ptr;

  if (id[0] != '$') {
    cx_error(cx, tok->row, tok->col, "Unknown id: '%s'", id);
    return -1;
  }
  
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *v = cx_get(s, id+1, false);
  if (!v) { return false; }
  cx_copy(cx_push(s), v);
  return true;
}

static void id_copy(struct cx_tok *dst, struct cx_tok *src) {
    dst->as_ptr = strdup(src->as_ptr);
}

static void id_deinit(struct cx_tok *tok) {
    free(tok->as_ptr);
}

cx_tok_type(cx_id_tok, {
    type.compile = id_compile;
    type.eval = id_eval;
    type.copy = id_copy;
    type.deinit = id_deinit;
  });

static bool lambda_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *lambda = tok->as_box.as_ptr;
  if (lambda->scope) { cx_scope_unref(lambda->scope); }
  lambda->scope = cx_scope_ref(scope);
  cx_copy(cx_push(scope), &tok->as_box);
  return true;
}

static void lambda_copy(struct cx_tok *dst, struct cx_tok *src) {
  cx_copy(&dst->as_box, &src->as_box);
}

static void lambda_deinit(struct cx_tok *tok) {
  cx_box_deinit(&tok->as_box);
}

cx_tok_type(cx_lambda_tok, {
    type.eval = lambda_eval;
    type.copy = lambda_copy;
    type.deinit = lambda_deinit;
  });

static ssize_t literal_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  cx_op_init(cx_vec_push(&bin->ops),
	     cx_push_op,
	     tok_idx)->as_push.value = &tok->as_box;
  return tok_idx+1;
}

static bool literal_eval(struct cx_tok *tok, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)), &tok->as_box);
  return true;
}

static void literal_copy(struct cx_tok *dst, struct cx_tok *src) {
  cx_copy(&dst->as_box, &src->as_box);
}

static void literal_deinit(struct cx_tok *tok) {
  cx_box_deinit(&tok->as_box);
}

cx_tok_type(cx_literal_tok, {
    type.compile = literal_compile;
    type.eval = literal_eval;
    type.copy = literal_copy;
    type.deinit = literal_deinit;
  });

static ssize_t macro_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), cx_macro_op, tok_idx);
  return tok_idx+1;
}

static bool macro_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_macro_eval *eval = tok->as_ptr;
  return eval->imp(eval, cx);
}

static void macro_copy(struct cx_tok *dst, struct cx_tok *src) {
  dst->as_ptr = cx_macro_eval_ref(src->as_ptr);
}
  
static void macro_deinit(struct cx_tok *tok) {
  cx_macro_eval_unref(tok->as_ptr);
}

cx_tok_type(cx_macro_tok, {
    type.compile = macro_compile;
    type.eval = macro_eval;
    type.copy = macro_copy;
    type.deinit = macro_deinit;
  });

static ssize_t type_compile(size_t tok_idx, struct cx_bin *bin, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_type *type = tok->as_ptr;
  tok->type = cx_literal_tok();
  cx_box_init(&tok->as_box, cx->meta_type)->as_ptr = type;    
  cx_op_init(cx_vec_push(&bin->ops),
	     cx_push_op,
	     tok_idx)->as_push.value = &tok->as_box;
  return tok_idx+1;
}

static bool type_eval(struct cx_tok *tok, struct cx *cx) {
  cx_box_init(cx_push(cx_scope(cx, 0)), cx->meta_type)->as_ptr = tok->as_ptr;
  return true;
}

cx_tok_type(cx_type_tok, {
    type.compile = type_compile;
    type.eval = type_eval;
  });

cx_tok_type(cx_ungroup_tok);
cx_tok_type(cx_unlambda_tok);

static bool eval_tok(struct cx *cx) {
  struct cx_tok *t = cx->pc++;

  if (!t->type->eval) { cx_error(cx, t->row, t->col, "Unexpected token"); }

  cx->row = t->row;
  cx->col = t->col;

  return t->type->eval(t, cx);
}

bool cx_eval(struct cx *cx, struct cx_vec *toks, struct cx_tok *pc) {
  struct cx_tok *prev_pc = cx->pc;
  struct cx_vec *prev_toks = cx->toks;
    
  cx->pc = pc;
  cx->toks = toks;
  bool ok = false;
  
  while (cx->pc != cx_vec_end(toks) && cx->pc != cx->stop_pc) {
    if (!eval_tok(cx)) { goto exit; }
    if (cx->errors.count) { goto exit; }
  }

  ok = true;
 exit:
  cx->toks = prev_toks;
  cx->pc = prev_pc;
  cx->stop_pc = NULL;
  return ok;
}

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = cx_parse_str(cx, in, &toks);
  if (ok && toks.count) { ok = cx_eval(cx, &toks, cx_vec_start(&toks)); }
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

bool cx_scan_args(struct cx *cx, struct cx_func *func) {
  int row = cx->row, col = cx->col;

  while (cx->pc != cx_vec_end(cx->toks)) {
    struct cx_scope *s = cx_scope(cx, 0);
    if (s->stack.count - s->cut_offs >= func->nargs) { break; }
    if (!eval_tok(cx)) { return false; }
  }

  struct cx_scope *s = cx_scope(cx, 0);
  
  if (s->stack.count - s->cut_offs < func->nargs) {
    cx_error(cx, row, col, "Not enough args for func: '%s'", func->id);
    return false;
  }

  s->cut_offs = 0;
  return true;
}

bool cx_eval_next(struct cx *cx) {
  struct cx_op *op = cx->op++;
  struct cx_tok *tok = cx_vec_get(&cx->bin->toks, op->tok_idx);
  cx->row = tok->row;
  cx->col = tok->col;
  return op->eval(tok, op, cx);
}

bool cx_scan_args2(struct cx *cx, struct cx_func *func) {
  int row = cx->row, col = cx->col;

  while (cx->op != cx_vec_end(&cx->bin->ops)) {
    struct cx_scope *s = cx_scope(cx, 0);
    if (s->stack.count - s->cut_offs >= func->nargs) { break; }
    if (!cx_eval_next(cx)) { return false; }
  }

  struct cx_scope *s = cx_scope(cx, 0);
  
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

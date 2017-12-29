#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/types/lambda.h"
#include "cixl/util.h"
#include "cixl/vec.h"

static bool cls_eval(struct cx_tok *tok, struct cx *cx) {
  cx_vec_clear(&cx_scope(cx, 0)->stack);
  return true;
}

cx_tok_type(cx_cls_tok, {
    type.eval = cls_eval;
  });

static bool cut_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  s->cut_offs = s->stack.count;
  return true;
}

cx_tok_type(cx_cut_tok, {
    type.eval = cut_eval;
  });

static bool dup_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);

  if (!s->stack.count) {
    cx_error(cx, tok->row, tok->col, "Nothing to dup");
    return false;
  }

  cx_copy(cx_push(s), cx_peek(s, true));
  return true;
}

cx_tok_type(cx_dup_tok, {
    type.eval = dup_eval;
  });

cx_tok_type(cx_end_tok);

static bool false_eval(struct cx_tok *tok, struct cx *cx) {
  cx_box_init(cx_push(cx_scope(cx, 0)), cx->bool_type)->as_bool = false;
  return true;
}

cx_tok_type(cx_false_tok, {
    type.eval = false_eval;
  });

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

  tok->type = cx_func_imp_tok();
  tok->as_ptr = imp;
  
  return cx_func_imp_call(imp, s);
}

cx_tok_type(cx_func_tok, {
    type.eval = func_eval;
  });

static bool func_imp_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_func_imp *imp = tok->as_ptr;
  struct cx_func *func = imp->func;
  int row = cx->row, col = cx->col;

  if (!cx_scan_args(cx, func)) { return false; }
  
  struct cx_scope *s = cx_scope(cx, 0);
  
  if (!cx_func_imp_match(imp, &s->stack)) {
    imp = cx_func_get_imp(func, &s->stack);
    
    if (!imp) {
      cx_error(cx, row, col, "Func not applicable: '%s'", func->id);
      return -1;
    }

    tok->as_ptr = imp;
  }
  
  return cx_func_imp_call(imp, s);
}

cx_tok_type(cx_func_imp_tok, {
    type.eval = func_imp_eval;
  });

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
    type.eval = group_eval;
    type.copy = group_copy;
    type.deinit = group_deinit;
  });

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
    type.eval = id_eval;
    type.copy = id_copy;
    type.deinit = id_deinit;
  });

static bool lambda_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  
  struct cx_lambda *lambda = cx_lambda_init(malloc(sizeof(struct cx_lambda)),
					    scope,
					    &tok->as_vec);

  struct cx_box *v = cx_box_init(cx_push(scope), cx->lambda_type);
  v->as_ptr = lambda;
  return true;
}

static void lambda_copy(struct cx_tok *dst, struct cx_tok *src) {
  group_copy(dst, src);
}

static void lambda_deinit(struct cx_tok *tok) {
  group_deinit(tok);
}

cx_tok_type(cx_lambda_tok, {
    type.eval = lambda_eval;
    type.copy = lambda_copy;
    type.deinit = lambda_deinit;
  });

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
    type.eval = literal_eval;
    type.copy = literal_copy;
    type.deinit = literal_deinit;
  });

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
    type.eval = macro_eval;
    type.copy = macro_copy;
    type.deinit = macro_deinit;
  });

static bool nil_eval(struct cx_tok *tok, struct cx *cx) {
  cx_box_init(cx_push(cx_scope(cx, 0)), cx->nil_type);
  return true;
}

cx_tok_type(cx_nil_tok, {
    type.eval = nil_eval;
  });

static bool true_eval(struct cx_tok *tok, struct cx *cx) {
  cx_box_init(cx_push(cx_scope(cx, 0)), cx->bool_type)->as_bool = true;
  return true;
}

cx_tok_type(cx_true_tok, {
    type.eval = true_eval;
  });

static bool type_eval(struct cx_tok *tok, struct cx *cx) {
  cx_box_init(cx_push(cx_scope(cx, 0)), cx->meta_type)->as_ptr = tok->as_ptr;
  return true;
}

cx_tok_type(cx_type_tok, {
    type.eval = type_eval;
  });

cx_tok_type(cx_ungroup_tok);
cx_tok_type(cx_unlambda_tok);

static bool zap_eval(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);

  if (!s->stack.count) {
    cx_error(cx, tok->row, tok->col, "Nothing to zap");
    return false;
  }
  
  cx_box_deinit(cx_pop(cx_scope(cx, 0), false));
  return true;
}

cx_tok_type(cx_zap_tok, {
    type.eval = zap_eval;
  });

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

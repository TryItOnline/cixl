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

static bool eval_id(struct cx_tok *tok, struct cx *cx) {
  char *id = tok->data;

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

static bool eval_literal(struct cx_tok *tok, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)), tok->data);
  return true;
}

static bool eval_macro(struct cx_tok *tok, struct cx *cx) {
  struct cx_macro_eval *eval = tok->data;
  return eval->imp(eval, cx);
}

static bool eval_lambda(struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  
  struct cx_lambda *lambda = cx_lambda_init(malloc(sizeof(struct cx_lambda)),
					    scope,
					    tok->data);

  struct cx_box *v = cx_box_init(cx_push(scope), cx->lambda_type);
  v->as_ptr = lambda;
  return true;
}

static bool eval_func(struct cx_tok *tok, struct cx *cx) {
  struct cx_func *func = tok->data;
  int row = cx->row, col = cx->col;
  
  if (!cx_scan_args(cx, func)) { return false; }
    
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_func_imp *imp = cx_func_get_imp(func, &s->stack);

  if (!imp) {
    cx_error(cx, row, col, "Func not applicable: '%s'", func->id);
    return false;
  }

  tok->type = CX_TFUNC_IMP;
  tok->data = imp;
  
  return cx_func_imp_call(imp, s);
}

static bool eval_func_imp(struct cx_tok *tok, struct cx *cx) {
  struct cx_func_imp *imp = tok->data;
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

    tok->data = imp;
  }
  
  return cx_func_imp_call(imp, s);
}

static bool eval_group(struct cx_tok *tok, struct cx *cx) {
  struct cx_vec *body = tok->data;
  cx_begin(cx, true);
  bool ok = cx_eval(cx, body, 0);
  cx_end(cx);
  return ok;
}

static bool eval_tok(struct cx *cx) {
  struct cx_tok *t = cx_vec_get(cx->toks, cx->pc++);
  struct cx_scope *s = cx_scope(cx, 0);

  cx->row = t->row;
  cx->col = t->col;
  
  switch (t->type) {
  case CX_TCUT:
    s->cut_offs = s->stack.count;
    return true;
  case CX_TFUNC:
    return eval_func(t, cx);
  case CX_TFUNC_IMP:
    return eval_func_imp(t, cx);
  case CX_TGROUP:
    return eval_group(t, cx);
  case CX_TID:
    return eval_id(t, cx);
  case CX_TLAMBDA:
    return eval_lambda(t, cx);
  case CX_TLITERAL:
    return eval_literal(t, cx);
  case CX_TMACRO:
    return eval_macro(t, cx);
  case CX_TNIL:
    cx_box_init(cx_push(s), s->cx->nil_type);
    return true;
  case CX_TTYPE:
    cx_box_init(cx_push(s), s->cx->meta_type)->as_ptr = t->data;
    return true;
  case CX_TTRUE:
  case CX_TFALSE:
    cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = t->type == CX_TTRUE;
    return true;
  default:
    cx_error(cx, t->row, t->col, "Unexpected token: %d", t->type);
    break;
  }

  return false;
}

bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t pc) {
  ssize_t prev_pc = cx->pc;
  struct cx_vec *prev_toks = cx->toks;
    
  cx->pc = pc;
  cx->toks = toks;
  bool ok = false;
  
  while (cx->pc < toks->count && cx->pc != cx->stop_pc) {
    if (!eval_tok(cx)) { goto exit; }
    if (cx->errors.count) { goto exit; }
  }

  ok = true;
 exit:
  cx->toks = prev_toks;
  cx->pc = prev_pc;
  cx->stop_pc = -1;
  return ok;
}

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = cx_parse_str(cx, in, &toks) && cx_eval(cx, &toks, 0);
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

bool cx_scan_args(struct cx *cx, struct cx_func *func) {
  int row = cx->row, col = cx->col;

  while (cx->pc < cx->toks->count) {
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
    switch (t->type) {
    case CX_TID: {
      cx_tok_copy(cx_vec_push(&tmp_ids), t);
      break;
    }
      
    case CX_TLITERAL: {
      struct cx_box *v = t->data;

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

      break;
    }

    case CX_TTYPE: {
      struct cx_type *type = t->data;

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
      
      break;
    }      
      
    default:
	cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
	cx_vec_deinit(&tmp_ids);
	return false;
    }
  }
  
  cx_vec_deinit(&tmp_ids);
  return true;
}

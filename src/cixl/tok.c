#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/macro.h"
#include "cixl/op.h"
#include "cixl/tok.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/vec.h"

struct cx_tok_type *cx_tok_type_init(struct cx_tok_type *type, const char *id) {
  type->id = id;
  type->compile = NULL;
  type->copy = NULL;
  type->deinit = NULL;
  return type;
}

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   struct cx_tok_type *type,
			   int row, int col) {
  tok->type = type;
  tok->row = row;
  tok->col = col;
  return tok;
}

struct cx_tok *cx_tok_deinit(struct cx_tok *tok) {
  if (tok->type->deinit) { tok->type->deinit(tok); }
  return tok;
}

void cx_tok_copy(struct cx_tok *dst, struct cx_tok *src) {
  *dst = *src;
  if (src->type->copy) { src->type->copy(dst, src); }
}

static ssize_t cut_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OCUT(), tok_idx);
  return tok_idx+1;
}

cx_tok_type(CX_TCUT, {
    type.compile = cut_compile;
  });

cx_tok_type(CX_TEND);

static bool inline_fimp(struct cx_fimp *imp,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  size_t i = bin->ops.count;

  struct cx_op *op = cx_op_init(cx_vec_push(&bin->ops),
				CX_OFIMP(),
				tok_idx);
  op->as_fimp.imp = imp;
  op->as_fimp.start_op = i+1;
  if (!cx_fimp_compile(imp, tok_idx, bin)) { return false; }
  op = cx_vec_get(&bin->ops, i);
  op->as_fimp.num_ops = bin->ops.count - op->as_fimp.start_op;
  return true;
}

static ssize_t fimp_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {  
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);  
  struct cx_fimp *imp = tok->as_ptr;

  if (!imp->ptr &&
      !cx_bin_get_func(bin, imp) &&
      !inline_fimp(imp, bin, tok_idx, cx)) {
    return -1;
  }

  struct cx_funcall_op *op = &cx_op_init(cx_vec_push(&bin->ops),
					 CX_OFUNCALL(),
					 tok_idx)->as_funcall;
  op->func = imp->func;
  op->imp = imp;
  op->jit_imp = NULL;
	   
  return tok_idx+1;
}

cx_tok_type(CX_TFIMP, {
    type.compile = fimp_compile;
  });

static ssize_t func_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {  
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);  
  struct cx_func *func = tok->as_ptr;
  struct cx_fimp *imp = (func->imps.count == 1)
    ? *(struct cx_fimp **)cx_vec_start(&func->imps)
    : NULL;

  if (imp &&
      !imp->ptr &&
      !cx_bin_get_func(bin, imp) &&
      !inline_fimp(imp, bin, tok_idx, cx)) {
    return -1;
  }

  struct cx_funcall_op *op = &cx_op_init(cx_vec_push(&bin->ops),
					 CX_OFUNCALL(),
					 tok_idx)->as_funcall;
  op->func = func;
  op->imp = imp;
  op->jit_imp = NULL;
	   
  return tok_idx+1;
}

cx_tok_type(CX_TFUNC, {
    type.compile = func_compile;
  });

static ssize_t group_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OBEGIN(), tok_idx)->as_begin.child = true;
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_vec *toks = &tok->as_vec;

  if (toks->count) {
    if (!cx_compile(cx, cx_vec_start(toks), cx_vec_end(toks), bin)) {
      tok = cx_vec_get(&bin->toks, tok_idx);  
      cx_error(cx, tok->row, tok->col, "Failed compiling group");
      return -1;
    }
  }
  
  cx_op_init(cx_vec_push(&bin->ops),
	     CX_OEND(),
	     tok_idx)->as_end.push_result = true;
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

cx_tok_type(CX_TGROUP, {
    type.compile = group_compile;
    type.copy = group_copy;
    type.deinit = group_deinit;
  });

static ssize_t id_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  char *id = tok->as_ptr;
  
  if (id[0] == '#') {
    cx_op_init(cx_vec_push(&bin->ops),
	       CX_OGET_CONST(),
	       tok_idx)->as_get_const.id = cx_sym(cx, id+1);
  } else if (id[0] == '$') {
    cx_op_init(cx_vec_push(&bin->ops),
	       CX_OGET_VAR(),
	       tok_idx)->as_get_var.id = cx_sym(cx, id+1);
  } else {
    cx_error(cx, tok->row, tok->col, "Unknown id: '%s'", id);
    return -1;
  }

  return tok_idx+1;
}

static void id_copy(struct cx_tok *dst, struct cx_tok *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void id_deinit(struct cx_tok *tok) {
  free(tok->as_ptr);
}

cx_tok_type(CX_TID, {
    type.compile = id_compile;
    type.copy = id_copy;
    type.deinit = id_deinit;
  });

static ssize_t lambda_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);

  size_t i = bin->ops.count;
  cx_op_init(cx_vec_push(&bin->ops),
	     CX_OLAMBDA(),
	     tok_idx)->as_lambda.start_op = i+1;
  
  struct cx_vec *toks = &tok->as_vec;

  if (toks->count) {
    if (!cx_compile(cx, cx_vec_start(toks), cx_vec_end(toks), bin)) {
      tok = cx_vec_get(&bin->toks, tok_idx);  
      cx_error(cx, tok->row, tok->col, "Failed compiling lambda");
      return -1;
    }
  }
  
  cx_op_init(cx_vec_push(&bin->ops), CX_OSTOP(), tok_idx);
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

cx_tok_type(CX_TLAMBDA, {
    type.compile = lambda_compile;
    type.copy = lambda_copy;
    type.deinit = lambda_deinit;
  });

static ssize_t literal_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OPUSH(), tok_idx);
  return tok_idx+1;
}

static void literal_copy(struct cx_tok *dst, struct cx_tok *src) {
  cx_copy(&dst->as_box, &src->as_box);
}

static void literal_deinit(struct cx_tok *tok) {
  cx_box_deinit(&tok->as_box);
}

cx_tok_type(CX_TLITERAL, {
    type.compile = literal_compile;
    type.copy = literal_copy;
    type.deinit = literal_deinit;
  });

static ssize_t macro_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_macro_eval *eval = tok->as_ptr;
  return eval->imp(eval, bin, tok_idx, cx);
}

static void macro_copy(struct cx_tok *dst, struct cx_tok *src) {
  dst->as_ptr = cx_macro_eval_ref(src->as_ptr);
}
  
static void macro_deinit(struct cx_tok *tok) {
  cx_macro_eval_unref(tok->as_ptr);
}

cx_tok_type(CX_TMACRO, {
    type.compile = macro_compile;
    type.copy = macro_copy;
    type.deinit = macro_deinit;
  });

static ssize_t stash_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OSTASH(), tok_idx);
  return tok_idx+1;
}

cx_tok_type(CX_TSTASH, {
    type.compile = stash_compile;
  });

static ssize_t type_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_type *type = tok->as_ptr;
  tok->type = CX_TLITERAL();
  cx_box_init(&tok->as_box, cx->meta_type)->as_ptr = type;    
  cx_op_init(cx_vec_push(&bin->ops), CX_OPUSH(), tok_idx);
  return tok_idx+1;
}

cx_tok_type(CX_TTYPE, {
    type.compile = type_compile;
  });

cx_tok_type(CX_TUNGROUP);
cx_tok_type(CX_TUNLAMBDA);
cx_tok_type(CX_TUNVECT);

static ssize_t zap_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OZAP(), tok_idx);
  return tok_idx+1;
}

cx_tok_type(CX_TZAP, {
    type.compile = zap_compile;
  });

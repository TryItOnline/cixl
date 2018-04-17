#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/macro.h"
#include "cixl/op.h"
#include "cixl/stack.h"
#include "cixl/tok.h"
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

cx_tok_type(CX_TEND);
cx_tok_type(CX_TFIMP);

static ssize_t group_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_vec *toks = &tok->as_vec;

  if (toks->count) {
    struct cx_op *op = cx_op_init(bin, CX_OBEGIN(), tok_idx);
    op->as_begin.child = true;
    op->as_begin.fimp = NULL;
    
    if (!cx_compile(cx, cx_vec_start(toks), cx_vec_end(toks), bin)) {
      tok = cx_vec_get(&bin->toks, tok_idx);  
      cx_error(cx, tok->row, tok->col, "Failed compiling group");
      goto exit;
    }
    
    cx_op_init(bin, CX_OEND(), tok_idx);
  }

 exit:
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

static ssize_t lib_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_lib *lib = tok->as_lib;
  tok->type = CX_TLITERAL();
  cx_box_init(&tok->as_box, cx->lib_type)->as_lib = lib;    
  cx_copy(&cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value, &tok->as_box);
  return tok_idx+1;
}

cx_tok_type(CX_TLIB, {
    type.compile = lib_compile;
  });

static ssize_t id_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  char *id = tok->as_ptr;
  
  if (id[0] == '#') {
    cx_op_init(bin,
	       CX_OGETCONST(),
	       tok_idx)->as_getconst.id = cx_sym(cx, id+1);    
  } else if (id[0] == '$') {
    cx_op_init(bin, CX_OGETVAR(), tok_idx)->as_getvar.id = cx_sym(cx, id+1);
  } else if (isupper(id[0])) {
    struct cx_type *t = cx_get_type(cx, id, false);
    if (!t) { return -1; }
    struct cx_box *v = &cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value;
    cx_box_init(v, cx->meta_type)->as_ptr = t;
  } else {
    bool ref = id[0] == '&';
    if (ref) { id++; }
    char *imp_id = strchr(id, '<');

    if (imp_id > id) {      
      *imp_id = 0;
      imp_id++;
      imp_id[strlen(imp_id)-1] = 0;
    }
    
    int row = cx->row, col = cx->col;
    struct cx_func *f = cx_get_func(cx, id, false);
    if (!f) { return -1; }
    struct cx_fimp *imp = NULL;
    
    if (imp_id > id) {
      struct cx_fimp **found = cx_set_get(&f->imps, &imp_id);
      
      if (!found) {
	cx_error(cx, row, col, "Fimp not found: %s", imp_id);
	return -1;
      }
      
      imp = *found;
    }
    
    if (ref) {
      struct cx_box *v = &cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value;

      if (imp) {
	cx_box_init(v, cx->fimp_type)->as_ptr = imp;
      } else {
	cx_box_init(v, cx->func_type)->as_ptr = f;
      }
    } else {
      if (imp) {
	if (!imp->ptr && !cx_fimp_inline(imp, tok_idx, bin, cx)) { return -1; }
      } else {
	struct cx_fimp *imp = (f->imps.members.count == 1)
	  ? *(struct cx_fimp **)cx_vec_start(&f->imps.members)
	  : NULL;
	
	if (imp && !imp->ptr && !cx_fimp_inline(imp, tok_idx, bin, cx)) {
	  return -1;
	}
      }

      struct cx_funcall_op *op = &cx_op_init(bin,
					     CX_OFUNCALL(),
					     tok_idx)->as_funcall;
      op->func = f;
      op->imp = imp;
    }
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
  cx_op_init(bin,
	     CX_OLAMBDA(),
	     tok_idx)->as_lambda.start_op = i+1;
  
  struct cx_vec *toks = &tok->as_vec;

  if (toks->count) {
    if (!cx_compile(cx, cx_vec_start(toks), cx_vec_end(toks), bin)) {
      tok = cx_vec_get(&bin->toks, tok_idx);  
      cx_error(cx, tok->row, tok->col, "Failed compiling lambda");
      goto exit;
    }
  }
  
  struct cx_op *op = cx_vec_get(&bin->ops, i);
  op->as_lambda.nops = bin->ops.count - op->as_lambda.start_op;
  
 exit:
  return tok_idx+1;
}

cx_tok_type(CX_TLAMBDA, {
    type.compile = lambda_compile;
    type.copy = group_copy;
    type.deinit = group_deinit;
  });

static ssize_t literal_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&bin->toks, tok_idx);
  cx_copy(&cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value, &t->as_box);
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
  cx_macro_eval_deref(tok->as_ptr);
}

cx_tok_type(CX_TMACRO, {
    type.compile = macro_compile;
    type.copy = macro_copy;
    type.deinit = macro_deinit;
  });

static ssize_t stack_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);    
  struct cx_vec *toks = &tok->as_vec;

  if (toks->count) {
    struct cx_op *op = cx_op_init(bin, CX_OBEGIN(), tok_idx);
    op->as_begin.child = true;
    op->as_begin.fimp = NULL;
    
    if (!cx_compile(cx, cx_vec_start(toks), cx_vec_end(toks), bin)) {
      tok = cx_vec_get(&bin->toks, tok_idx);  
      cx_error(cx, tok->row, tok->col, "Failed compiling stack");
      return -1;
    }

    cx_op_init(bin, CX_OSTASH(), tok_idx);
    cx_op_init(bin, CX_OEND(), tok_idx);
  } else {
    struct cx_box *v = &cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value;
    cx_box_init(v, cx->stack_type)->as_ptr = cx_stack_new(cx);
  }
  
  return tok_idx+1;
}

cx_tok_type(CX_TSTACK, {
    type.compile = stack_compile;
    type.copy = group_copy;
    type.deinit = group_deinit;
  });

static ssize_t type_compile(struct cx_bin *bin, size_t tok_idx, struct cx *cx) {
  struct cx_tok *tok = cx_vec_get(&bin->toks, tok_idx);
  struct cx_type *type = tok->as_ptr;
  tok->type = CX_TLITERAL();
  cx_box_init(&tok->as_box, cx->meta_type)->as_ptr = type;    
  cx_copy(&cx_op_init(bin, CX_OPUSH(), tok_idx)->as_push.value, &tok->as_box);
  return tok_idx+1;
}

cx_tok_type(CX_TTYPE, {
    type.compile = type_compile;
  });

cx_tok_type(CX_TUNGROUP);
cx_tok_type(CX_TUNLAMBDA);
cx_tok_type(CX_TUNSTACK);

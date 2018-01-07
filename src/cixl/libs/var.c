#include <ctype.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/var.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/rec.h"

static ssize_t let_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  cx_op_init(cx_vec_push(&bin->ops), CX_OSCOPE(), tok_idx)->as_scope.child = true;  

  if (!cx_compile(cx, cx_vec_get(&eval->toks, 1), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling let");
    return -1;
  }
  
  void set(const char *id, struct cx_type *type) {
    struct cx_op * op = cx_op_init(cx_vec_push(&bin->ops), CX_OSET(), tok_idx);
    op->as_set.id = cx_sym(cx, id);
    op->as_set.type = type;
    op->as_set.force = false;
    op->as_set.pop_parent = false;
    op->as_set.set_parent = true;
  }

  struct cx_tok *id_tok = cx_vec_get(&eval->toks, 0);
  
  if (id_tok->type == CX_TID()) {
    set(id_tok->as_ptr, NULL);
  } else {
    struct cx_vec *toks = &id_tok->as_vec, ids, types;
    cx_vec_init(&ids, sizeof(struct cx_tok));
    cx_vec_init(&types, sizeof(struct cx_type *));
    bool ok = false;
    
    bool push_type(struct cx_type *type) {
      if (ids.count == types.count) {
	cx_error(cx, cx->row, cx->col, "Missing let id");
	return false;
      }
      
      for (struct cx_tok *id = cx_vec_get(&ids, types.count);
	   id != cx_vec_end(&ids);
	   id++) {
	*(struct cx_type **)cx_vec_push(&types) = type;	
      }

      return true;
    }
    
    cx_do_vec(toks, struct cx_tok, t) {
      if (t->type == CX_TID()) {
	*(struct cx_tok *)cx_vec_push(&ids) = *t;
      } else if (t->type == CX_TTYPE() && !push_type(t->as_ptr)) {
	goto exit;
      }
    }

    if (ids.count > types.count && !push_type(NULL)) { goto exit; }
    struct cx_tok *id = cx_vec_peek(&ids, 0);
    struct cx_type **type = cx_vec_peek(&types, 0);
    
    for (; id >= (struct cx_tok *)ids.items; id--, type--) {
      set(id->as_ptr, *type);
    }
    
    ok = true;
    
    exit:
    cx_vec_deinit(&ids);
    cx_vec_deinit(&types);
    if (!ok) { return -1; }
  }

  cx_op_init(cx_vec_push(&bin->ops),
	     CX_OUNSCOPE(),
	     tok_idx)->as_unscope.push_result = false;

  return tok_idx+1;
}

static bool let_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_macro_eval *eval = cx_macro_eval_new(let_eval);

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &eval->toks, false)) {
    cx_error(cx, row, col, "Missing let id");
    goto error;
  }

  struct cx_tok *id = cx_vec_peek(&eval->toks, 0);

  if (id->type != CX_TID() && id->type != CX_TGROUP()) {
    cx_error(cx, id->row, id->col, "Invalid let id");
    goto error;
  }

  if (!cx_parse_end(cx, in, &eval->toks, true)) { goto error; }
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
 error:
  cx_macro_eval_unref(eval);
  return false;  
}

static bool islet_imp(struct cx_scope *scope) {
  struct cx_sym s = cx_test(cx_pop(scope, false))->as_sym;
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_islet(scope, s);
  return true;
}

static bool unlet_imp(struct cx_scope *scope) {
  struct cx_sym s = cx_test(cx_pop(scope, false))->as_sym;
  cx_unlet(scope, s);
  return true;
}

void cx_init_var(struct cx *cx) {
  cx_add_macro(cx, "let:", let_parse);

  cx_add_func(cx, "islet", cx_arg(cx->sym_type))->ptr = islet_imp;
  cx_add_func(cx, "unlet", cx_arg(cx->sym_type))->ptr = unlet_imp;
}

#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/fimp.h"
#include "cixl/lib.h"
#include "cixl/lib/var.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

static ssize_t let_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  if (!cx_compile(cx, cx_vec_get(&eval->toks, 1), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling let");
    return tok_idx+1;
  }
  
  void put(const char *id, struct cx_type *type) {
    struct cx_op * op = cx_op_init(bin, CX_OPUTVAR(), tok_idx);
    op->as_putvar.id = cx_sym(cx, id);
    op->as_putvar.type = type;
  }

  struct cx_tok *id_tok = cx_vec_get(&eval->toks, 0);
  
  if (id_tok->type == CX_TID()) {
    put(id_tok->as_ptr, NULL);
  } else {
    struct cx_vec *toks = &id_tok->as_vec, ids, types;
    cx_vec_init(&ids, sizeof(struct cx_tok));
    cx_vec_init(&types, sizeof(struct cx_type *));
    
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
	char *id = t->as_ptr;

	if (isupper(id[0])) {
	  struct cx_type *type = cx_get_type(cx, id, false);
	  if (!type || !push_type(type)) { goto exit; }
	} else {
	  *(struct cx_tok *)cx_vec_push(&ids) = *t;
	}
      } else {
	cx_error(cx, t->row, t->col, "Invalid let tok: %s", t->type->id);
	goto exit;
      }
    }

    if (ids.count > types.count && !push_type(NULL)) { goto exit; }
    struct cx_tok *id = cx_vec_peek(&ids, 0);
    struct cx_type **type = cx_vec_peek(&types, 0);
    
    for (; id >= (struct cx_tok *)ids.items; id--, type--) {
      put(id->as_ptr, *type);
    }    
  exit:
    cx_vec_deinit(&ids);
    cx_vec_deinit(&types);
  }

  return tok_idx+1;
}

static bool let_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_macro_eval *eval = cx_macro_eval_new(let_eval);

  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &eval->toks)) {
    cx_error(cx, row, col, "Missing let id");
    goto error;
  }

  struct cx_tok *id = cx_vec_peek(&eval->toks, 0);

  if (id->type != CX_TID() && id->type != CX_TGROUP()) {
    cx_error(cx, id->row, id->col, "Invalid let id");
    goto error;
  }

  if (!cx_parse_end(cx, in, &eval->toks)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing let end"); }
    goto error;
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
 error:
  cx_macro_eval_deref(eval);
  return false;  
}

static bool let_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 1));
  struct cx_sym id = cx_test(cx_call_arg(call, 0))->as_sym;
  struct cx_scope *s = call->scope;
  struct cx_box *var = cx_put_var(s, id);
  cx_copy(var, v);
  return true;
}

static bool var_imp(struct cx_call *call) {
  struct cx_sym id = cx_test(cx_call_arg(call, 0))->as_sym;
  struct cx_scope *s = call->scope;
  struct cx_box *v = cx_get_var(s, id, true);

  if (!v) {
    cx_box_init(cx_push(s), s->cx->nil_type);
  } else {
    cx_copy(cx_push(s), v);
  }
  
  return true;
}

cx_lib(cx_init_var, "cx/var") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Opt", "Sym")) {
    return false;
  }
    
  cx_add_macro(lib, "let:", let_parse);

  cx_add_cfunc(lib, "let",
	       cx_args(cx_arg("id", cx->sym_type), cx_arg("val", cx->any_type)),
	       cx_args(),
	       let_imp);
  
  cx_add_cfunc(lib, "var",
	       cx_args(cx_arg("id", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       var_imp);

  return true;
}

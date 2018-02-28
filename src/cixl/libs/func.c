#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lambda.h"
#include "cixl/lib.h"
#include "cixl/libs/func.h"
#include "cixl/libs/stack.h"
#include "cixl/op.h"
#include "cixl/scope.h"

static bool parse_args(struct cx *cx, struct cx_vec *toks, struct cx_vec *args) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));
  bool ok = false;
  
  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *id = t->as_ptr;

      if (strncmp(id, "Arg", 3) == 0 && isdigit(id[3])) {
	int i = strtoimax(id+3, NULL, 10);

	if (tmp_ids.count) {
	  cx_do_vec(&tmp_ids, struct cx_tok, id) {
	    *(struct cx_arg *)cx_vec_push(args) = cx_narg(id->as_ptr, i);      
	  }
	  
	  cx_vec_clear(&tmp_ids);
	} else {
	  *(struct cx_arg *)cx_vec_push(args) = cx_narg(NULL, i);      
	}	
      } else {
	*(struct cx_tok *)cx_vec_push(&tmp_ids) = *t;
      }
    } else if (t->type == CX_TLITERAL()) {
      *(struct cx_arg *)cx_vec_push(args) = cx_varg(&t->as_box);        
    } else if (t->type == CX_TTYPE()) {
      struct cx_type *type = t->as_ptr;

      if (tmp_ids.count) {
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  *(struct cx_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, type);      
	}
      
	cx_vec_clear(&tmp_ids);
      } else {
	*(struct cx_arg *)cx_vec_push(args) = cx_arg(NULL, type);
      }
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      goto exit;
    }
  }

  cx_do_vec(&tmp_ids, struct cx_tok, id) {
    *(struct cx_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, cx->any_type);      
  }
  
  ok = true;
 exit:
  cx_vec_deinit(&tmp_ids);
  return ok;
}

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  struct cx_tok *f = cx_vec_get(&eval->toks, 0);
  
  cx_op_init(bin,
	     CX_OFUNCDEF(),
	     tok_idx)->as_funcdef.imp = f->as_ptr;

  if (!cx_fimp_inline(f->as_ptr, tok_idx, bin, cx)) { return -1; }
  return tok_idx+1;
}

static bool func_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func id");
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid func id: %s", id.type->id);
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func args");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok args = *(struct cx_tok *)cx_vec_pop(&toks);

  if (args.type != CX_TGROUP()) {
    cx_error(cx, row, col, "Invalid func args: %s", args.type->id);
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_vec func_args;
  cx_vec_init(&func_args, sizeof(struct cx_arg));

  if (!parse_args(cx, &args.as_vec, &func_args)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;  
  }

  cx_tok_deinit(&args);

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func rets");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok rets = *(struct cx_tok *)cx_vec_pop(&toks);

  if (rets.type != CX_TGROUP()) {
    cx_error(cx, row, col, "Invalid func rets: %s", rets.type->id);
    cx_tok_deinit(&id);
    cx_tok_deinit(&rets);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_vec func_rets;
  cx_vec_init(&func_rets, sizeof(struct cx_arg));

  if (!parse_args(cx, &rets.as_vec, &func_rets)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&rets);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_rets);
    return false;
  }

  cx_tok_deinit(&rets);
  
  if (!cx_parse_end(cx, in, &toks, true)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing func end"); }

    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;
  }

  struct cx_fimp *imp = cx_add_func(cx,
				    id.as_ptr,
				    func_args.count,
				    (void *)func_args.items,
				    func_rets.count,
				    (void *)func_rets.items);

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);
  cx_vec_deinit(&func_rets);

  if (!imp) {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }    
    cx_vec_deinit(&toks);
    return false;
  }
  
  imp->toks = toks;
  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TFIMP(), row, col)->as_ptr = imp;
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool imps_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  struct cx_stack *is = cx_stack_new(cx);

  for (struct cx_fimp **i = cx_vec_peek(&f->imps, 0);
       i >= (struct cx_fimp **)f->imps.items;
       i--) {
    cx_box_init(cx_vec_push(&is->imp), cx->fimp_type)->as_ptr = *i;
  }
  
  cx_box_init(cx_push(scope), scope->cx->stack_type)->as_ptr = is;
  return true;
}

static struct cx_call *get_fimp_call(struct cx *cx) {
  for (struct cx_call *c = cx_vec_peek(&cx->calls, 0);
       c >= (struct cx_call *)cx->calls.items;
       c--) {
    if (!c->target->ptr) { return c; }
  }

  return NULL;
}

static bool recall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_call *call = get_fimp_call(cx);
  
  if (!call) {
    cx_error(cx, cx->row, cx->col, "Nothing to recall");
    return false;
  }

  call->recalls++;  
  return true;
}

static bool upcall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_call *call = get_fimp_call(cx);
  
  if (!call) {
    cx_error(cx, cx->row, cx->col, "Nothing to upcall");
    return false;
  }

  struct cx_fimp *imp = call->target;
  struct cx_func *func = imp->func;
  
  if (!imp->idx) {
    cx_error(cx, cx->row, cx->col, "No more fimps");
    return false;
  }
  
  imp = cx_func_match(func, scope, func->imps.count - imp->idx);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Upcall not applicable");
    return false;
  }
  
  return cx_fimp_call(imp, scope);  
}

cx_lib(cx_init_func, "cx/func", {
    if (!cx_use(cx, "cx/func/types", false) ||
	!cx_use(cx, "cx/stack/types", false)) {
      return false;
    }

  cx_add_macro(cx, "func:", func_parse);

  cx_add_cfunc(cx, "imps",
	       cx_args(cx_arg("f", cx->func_type)),
	       cx_args(cx_arg(NULL, cx->stack_type)),
	       imps_imp);
  
  cx_add_cfunc(cx, "recall", cx_args(), cx_args(), recall_imp);
  cx_add_cfunc(cx, "upcall", cx_args(), cx_args(), upcall_imp);

  return true;
  })

cx_lib(cx_init_func_types, "cx/func/types", {
    cx->func_type = cx_init_func_type(cx);
    cx->fimp_type = cx_init_fimp_type(cx);
    cx->lambda_type = cx_init_lambda_type(cx);
    return true;
  })

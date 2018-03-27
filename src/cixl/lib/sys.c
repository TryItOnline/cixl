#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/sys.h"
#include "cixl/link.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/stack.h"

static bool link_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  struct cx_vec fns;
  cx_vec_init(&fns, sizeof(struct cx_tok));
  
  if (!cx_parse_end(cx, in, &fns, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing link end"); }
    goto exit;
  }

  cx_do_vec(&fns, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid link token: %s", t->type->id);
      goto exit;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid link filename: %s", t->as_box.type->id);
      goto exit;
    }

    char *p = t->as_box.as_str->data;
    int pl = strlen(p);
    char fp[pl+4];
    strcpy(fp, p);
    strcpy(fp+pl, ".so");
    
    dlerror();
    void *h = dlopen(fp, RTLD_LAZY | RTLD_GLOBAL);
    
    if (!h) {
      cx_error(cx, t->row, t->col,
	       "Linked library not found: %s\n%s",
	       p, dlerror());
      
      goto exit;
    }

    cx_link_init(cx_vec_push(&cx->links), cx, p, h);
  }
  
  ok = true;
 exit: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
}

static ssize_t init_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  cx_do_vec(&eval->toks, struct cx_tok, t) {
    cx_op_init(bin, CX_OINIT(), tok_idx)->as_init.id = cx_str_ref(t->as_box.as_str);
  }
  
  return tok_idx+1;
}

static bool init_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(init_eval);

  if (!cx_parse_end(cx, in, &eval->toks, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing init end"); }
    cx_macro_eval_deref(eval);
    return false;
  }

  cx_do_vec(&eval->toks, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid init token: %s", t->type->id);
      cx_macro_eval_deref(eval);
      return false;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid init id: %s", t->as_box.type->id);
      cx_macro_eval_deref(eval);
      return false;
    }

    char *id = t->as_box.as_str->data;
    
    if (!cx_dlinit(cx, id)) {
      cx_macro_eval_deref(eval);
      return false;
    }
  }

  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool home_dir_imp(struct cx_scope *scope) {
  const char *d = cx_home_dir();
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(d, strlen(d));
  return true;
}

static bool make_dir_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  bool ok = cx_make_dir(p.as_str->data);
  cx_box_deinit(&p);
  return ok;
}

cx_lib(cx_init_sys, "cx/sys") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Int", "Stack", "Str")) {
    return false;
  }

  cx_box_init(cx_put_const(cx_test(cx_get_lib(cx, "cx/sys", false)),
			   cx_sym(cx, "args"), false),
	      cx->stack_type)->as_ptr = cx_stack_new(cx);

  cx_add_macro(lib, "link:", link_parse);
  cx_add_macro(lib, "init:", init_parse);

  cx_add_cfunc(lib, "home-dir",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       home_dir_imp);

  cx_add_cfunc(lib, "make-dir",
	       cx_args(cx_arg("p", cx->str_type)),
	       cx_args(),
	       make_dir_imp);

  return true;
}

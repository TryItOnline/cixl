#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/call.h"
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
  
  if (!cx_parse_end(cx, in, &fns)) {
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

    char *id = t->as_box.as_str->data;
    int idl = strlen(id);
    char fid[idl+4];
    strcpy(fid, id);
    strcpy(fid+idl, ".so");
    
    dlerror();
    void *h = dlopen(fid, RTLD_NOW | RTLD_GLOBAL);
    
    if (!h) {
      cx_error(cx, t->row, t->col,
	       "Failed linking: %s\n%s",
	       id, dlerror());
      
      goto exit;
    }

    cx_link_init(cx_vec_push(&cx->links), cx, id, h);
  }
  
  ok = true;
 exit: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
}

static bool init_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_end(cx, in, &toks)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing init end"); }
    goto exit;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid init token: %s", t->type->id);
      goto exit;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col, "Invalid init id: %s", t->as_box.type->id);
      goto exit;
    }

    struct cx_str *id = t->as_box.as_str;
    if (!cx_dlinit(cx, id->data)) { goto exit; }
    *(struct cx_str **)cx_vec_push(&cx->inits) = cx_str_ref(id);
  }

  ok = true;
 exit: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool home_dir_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  const char *d = cx_home_dir();
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(d, strlen(d));
  return true;
}

static bool make_dir_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  return cx_make_dir(p->as_str->data);
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

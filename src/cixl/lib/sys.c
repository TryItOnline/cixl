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
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/stack.h"

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

    char *fn = t->as_box.as_str->data;
    
    dlerror();
    void *h = dlopen(fn, RTLD_LAZY | RTLD_GLOBAL);
    
    if (!h) {
      cx_error(cx, t->row, t->col,
	       "Linked library not found: %s\n%s",
	       fn, dlerror());
      
      goto exit;
    }

    *(void **)cx_vec_push(&cx->dlibs) = h;
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
  bool ok = false;
  
  struct cx_vec fns;
  cx_vec_init(&fns, sizeof(struct cx_tok));
  
  if (!cx_parse_end(cx, in, &fns, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing init end"); }
    goto exit;
  }

  cx_do_vec(&fns, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid init token: %s", t->type->id);
      goto exit;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid init id: %s", t->as_box.type->id);
      goto exit;
    }

    char *id = t->as_box.as_str->data;
    char *fid = cx_fmt("cx_init_%s", id);

    dlerror();
    void *fp = dlsym(NULL, fid);
    
    if (!fp) {
      cx_error(cx, t->row, t->col, "Init not found: %s\ns%s", id, dlerror());
      free(fid);
      goto exit;
    }
 
    bool (*f)(struct cx *) = NULL;
    *(void **)(&f) = dlsym(NULL, fid);
    free(fid);
    if (!f(cx)) { goto exit; }
  }
  
  ok = true;
 exit: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
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

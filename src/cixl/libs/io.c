#include <errno.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/io.h"
#include "cixl/scope.h"
#include "cixl/types/file.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/str.h"

static bool print_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));
  
  cx_print(&v, out.as_file->ptr);
  cx_box_deinit(&v);
  cx_box_deinit(&out);
  return true;
}

static bool ask_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  fputs(p.as_str->data, stdout);
  cx_box_deinit(&p);
  
  struct cx_buf out;
  cx_buf_open(&out);
  
  while (true) {
    char c = getc(stdin);
    if (c == '\n' || c == EOF) { break; }
    fputc(c, out.stream);
  }

  cx_buf_close(&out);
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(out.data);
  free(out.data);
  return true;
}

static bool load_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  bool ok = cx_load(scope->cx, p.as_str->data);
  cx_box_deinit(&p);
  return ok;
}

static bool fopen_imp(struct cx_scope *scope) {
  struct cx_box
     m = *cx_test(cx_pop(scope, false)),
     p = *cx_test(cx_pop(scope, false));

  struct cx_type *ft = NULL;
  struct cx *cx = scope->cx;
  bool ok = false;

  if (strchr(m.as_sym.id, 'r')) {
    ft = strchr(m.as_sym.id, '+') ? cx->rwfile_type : cx->rfile_type;
  } else if (strchr(m.as_sym.id, 'w') || strchr(m.as_sym.id, 'a')) {
    ft = strchr(m.as_sym.id, '+') ? cx->rwfile_type : cx->wfile_type;
  } else {
    cx_error(cx, cx->row, cx->col, "Invalid fopen mode: %s", m.as_ptr);
    goto exit;
  }

  FILE *f = fopen(p.as_str->data, m.as_sym.id);
  
  if (!f) {
    cx_error(cx, cx->row, cx->col,
	     "Failed opening file '%s': %d",
	     p.as_str->data, errno);
    
    goto exit;
  }
  
  cx_box_init(cx_push(scope), ft)->as_file = cx_file_new(f);
  ok = true;
 exit:
  cx_box_deinit(&p);
  return ok;
}

static bool flush_imp(struct cx_scope *scope) {
  struct cx_box *f = cx_test(cx_pop(scope, false));
  bool ok = false;
  
  if (fflush(f->as_file->ptr)) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Failed flushing file: %d", errno);
    goto exit;
  }

  ok = true;
 exit:
  cx_box_deinit(f);
  return ok;
}

static bool read_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));

  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_tok(cx, in.as_file->ptr, &toks, true)) {
    if (!cx->errors.count) { cx_box_init(cx_push(scope), cx->nil_type); }
    goto exit1;
  }
  
  struct cx_bin bin;
  cx_bin_init(&bin);
  
  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), &bin)) { goto exit2; }
  if (!cx_eval(cx, &bin, NULL)) { goto exit2; }
  ok = true;
 exit2:
  cx_bin_deinit(&bin);
 exit1:
  cx_box_deinit(&in);
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

static bool write_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));
  
  bool ok = cx_write(&v, out.as_file->ptr);
  fputc('\n', out.as_file->ptr);
  cx_box_deinit(&v);
  cx_box_deinit(&out);
  return ok;
}

void cx_init_io(struct cx *cx) {
  cx_box_init(cx_set_const(cx, cx_sym(cx, "in"), false),
	      cx->rfile_type)->as_file = cx_file_new(stdin);

  cx_box_init(cx_set_const(cx, cx_sym(cx, "out"), false),
	      cx->wfile_type)->as_file = cx_file_new(stdout);

  cx_add_cfunc(cx, "print",
	       cx_args(cx_arg("out", cx->wfile_type), cx_arg("v", cx->any_type)),
	       cx_rets(),
	       print_imp);
    
  cx_add_cfunc(cx, "ask",
	       cx_args(cx_arg("prompt", cx->str_type)), cx_rets(),
	       ask_imp);
  
  cx_add_cfunc(cx, "load",
	       cx_args(cx_arg("path", cx->str_type)), cx_rets(),
	       load_imp);  

  cx_add_cfunc(cx, "fopen",
	       cx_args(cx_arg("path", cx->str_type), cx_arg("mode", cx->sym_type)),
	       cx_rets(cx_ret(cx->file_type)),
	       fopen_imp);  

  cx_add_cfunc(cx, "flush",
	       cx_args(cx_arg("file", cx->wfile_type)), cx_rets(),
	       flush_imp);

  cx_add_cfunc(cx, "read",
	       cx_args(cx_arg("f", cx->rfile_type)), cx_rets(cx_ret(cx->opt_type)),
	       read_imp);

  cx_add_cfunc(cx, "write",
	       cx_args(cx_arg("f", cx->wfile_type), cx_arg("v", cx->opt_type)),
	       cx_rets(),
	       write_imp);

  cx_add_func(cx, "say",
	      cx_args(cx_arg("v", cx->any_type)), cx_rets(),
	      "#out print $v "
	      "#out print \\\\n");
}

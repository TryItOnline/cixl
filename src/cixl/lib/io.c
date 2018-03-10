#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/file.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/io.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static ssize_t include_eval(struct cx_macro_eval *eval,
			    struct cx_bin *bin,
			    size_t tok_idx,
			    struct cx *cx) {
  if (!cx_compile(cx, cx_vec_start(&eval->toks), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling include");
  }

  return tok_idx+1;
}

static bool include_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  struct cx_vec fns;
  cx_vec_init(&fns, sizeof(struct cx_tok));
  
  if (!cx_parse_end(cx, in, &fns, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing include: end"); }
    goto exit1;
  }

  struct cx_macro_eval *eval = cx_macro_eval_new(include_eval);

  cx_do_vec(&fns, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid include token: %s", t->type->id);
      goto exit2;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid filename: %s", t->as_box.type->id);
      goto exit2;
    }

    char *full_path = cx_get_path(cx, t->as_box.as_str->data);
    bool ok = cx_load_toks(cx, full_path, &eval->toks);
    free(full_path);
    free(*(char **)cx_vec_pop(&cx->load_paths));
    if (!ok) { goto exit2; }
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  ok = true;
  goto exit1;
 exit2:
  cx_macro_eval_deref(eval);
 exit1: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
}

struct line_iter {
  struct cx_iter iter;
  struct cx_file *in;
  char *line;
  size_t len; 
};

static bool line_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct line_iter *it = cx_baseof(iter, struct line_iter, iter);

  if (!cx_get_line(&it->line, &it->len, cx_file_ptr(it->in))) {
    iter->done = true;
    return false;
  }
  
  cx_box_init(out, scope->cx->str_type)->as_str = cx_str_new(it->line);
  return true;
}

static void *line_deinit(struct cx_iter *iter) {
  struct line_iter *it = cx_baseof(iter, struct line_iter, iter);
  cx_file_deref(it->in);
  if (it->line) { free(it->line); }
  return it;
}

static cx_iter_type(line_iter, {
    type.next = line_next;
    type.deinit = line_deinit;
  });

static struct cx_iter *line_iter_new(struct cx_file *in) {
  struct line_iter *it = malloc(sizeof(struct line_iter));
  cx_iter_init(&it->iter, line_iter());
  it->in = cx_file_ref(in);
  it->line = NULL;
  it->len = 0;
  return &it->iter;
}

static bool print_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));
  
  cx_print(&v, cx_file_ptr(out.as_file));
  cx_box_deinit(&v);
  cx_box_deinit(&out);
  return true;
}

static bool ask_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  fputs(p.as_str->data, stdout);
  cx_box_deinit(&p);
  char *line = NULL;
  size_t len = 0;
  
  if (!cx_get_line(&line, &len, stdin)) { return false; }
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(line);
  free(line);
  return true;
}

static bool load_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  struct cx_bin *bin = cx_bin_new();
  bool ok = cx_load(scope->cx, p.as_str->data, bin) && cx_eval(bin, 0, scope->cx);
  cx_bin_deref(bin);
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
  
  cx_box_init(cx_push(scope), ft)->as_file = cx_file_new(cx, fileno(f), NULL, f);
  ok = true;
 exit:
  cx_box_deinit(&p);
  return ok;
}

static bool flush_imp(struct cx_scope *scope) {
  struct cx_box *f = cx_test(cx_pop(scope, false));
  bool ok = false;
  
  if (f->as_file->_ptr && fflush(f->as_file->_ptr)) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Failed flushing file: %d", errno);
    goto exit;
  }

  ok = true;
 exit:
  cx_box_deinit(f);
  return ok;
}

static bool close_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box *f = cx_test(cx_pop(scope, false));
  bool ok = cx_file_close(f->as_file);
  if (!ok) { cx_error(cx, cx->row, cx->col, "File not open"); }
  cx_box_deinit(f);
  return ok;
}

static bool read_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));

  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_tok(cx, cx_file_ptr(in.as_file), &toks, true)) {
    if (!cx->errors.count) { cx_box_init(cx_push(scope), cx->nil_type); }
    goto exit1;
  }
  
  struct cx_bin bin;
  cx_bin_init(&bin);
  
  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), &bin)) { goto exit2; }
  if (!cx_eval(&bin, 0, cx)) { goto exit2; }
  ok = true;
 exit2:
  cx_bin_deinit(&bin);
 exit1:
  cx_box_deinit(&in);
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

static bool read_bytes_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  bool ok = false;

  struct cx_box
    nbytes = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_buf *o = out.as_buf;
  cx_vec_grow(&o->data, o->data.count+nbytes.as_int);  
  int rbytes = read(in.as_file->fd, o->data.items, nbytes.as_int);

  if (!rbytes || (rbytes == -1 && errno == ECONNREFUSED)) {
    cx_box_init(cx_push(scope), cx->nil_type);
    ok = true;
    goto exit;
  }

  if (rbytes == -1 && errno == EAGAIN) { rbytes = 0; }

  if (rbytes == -1) {
    cx_error(cx, cx->row, cx->col, "Failed reading: %d", errno);
    goto exit;
  }

  o->data.count += rbytes;
  cx_box_init(cx_push(scope), cx->int_type)->as_int = rbytes;
  ok = true;
 exit:
  cx_box_deinit(&out);
  cx_box_deinit(&in);
  return ok;
}

static bool write_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));

  FILE *f = cx_file_ptr(out.as_file);
  bool ok = cx_write(&v, f);
  fputc('\n', f);
  cx_box_deinit(&v);
  cx_box_deinit(&out);
  return ok;
}

static bool lines_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = line_iter_new(in.as_file);
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;
  cx_box_deinit(&in);
  return true;
}

cx_lib(cx_init_io, "cx/io") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Cmp", "Char", "Iter", "Opt", "Str", "Sym") ||
      !cx_use(cx, "cx/buf", "Buf")) {
    return false;
  }

  cx->file_type = cx_init_file_type(lib, "File");
  cx->rfile_type = cx_init_file_type(lib, "RFile", cx->file_type, cx->seq_type);
  cx->rfile_type->iter = cx_file_iter;
    
  cx->wfile_type = cx_init_file_type(lib, "WFile", cx->file_type);
  cx->rwfile_type = cx_init_file_type(lib, "RWFile", 
				      cx->rfile_type, cx->wfile_type);
  cx->rwfile_type->iter = cx_file_iter;

  cx_box_init(cx_put_const(lib, cx_sym(cx, "in"), false), cx->rfile_type)->as_file =
    cx_file_new(cx, fileno(stdin), NULL, stdin);
    
  cx_box_init(cx_put_const(lib, cx_sym(cx, "out"), false), cx->wfile_type)->as_file =
    cx_file_new(cx, fileno(stdout), NULL, stdout);
    
  cx_add_macro(lib, "include:", include_parse);

  cx_add_cfunc(lib, "print",
	       cx_args(cx_arg("out", cx->wfile_type), cx_arg("v", cx->any_type)),
	       cx_args(),
	       print_imp);
    
  cx_add_cfunc(lib, "ask",
	       cx_args(cx_arg("prompt", cx->str_type)), cx_args(),
	       ask_imp);
  
  cx_add_cfunc(lib, "load",
	       cx_args(cx_arg("path", cx->str_type)), cx_args(),
	       load_imp);  

  cx_add_cfunc(lib, "fopen",
	       cx_args(cx_arg("path", cx->str_type), cx_arg("mode", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx->file_type)),
	       fopen_imp);  

  cx_add_cfunc(lib, "flush",
	       cx_args(cx_arg("file", cx->wfile_type)), cx_args(),
	       flush_imp);

  cx_add_cfunc(lib, "close",
	       cx_args(cx_arg("file", cx->wfile_type)), cx_args(),
	       close_imp);

  cx_add_cfunc(lib, "read",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       read_imp);

  cx_add_cfunc(lib, "read-bytes",
	       cx_args(cx_arg("in", cx->rfile_type),
		       cx_arg("out", cx->buf_type),
		       cx_arg("nbytes", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       read_bytes_imp);

  cx_add_cfunc(lib, "write",
	       cx_args(cx_arg("f", cx->wfile_type), cx_arg("v", cx->opt_type)),
	       cx_args(),
	       write_imp);

  cx_add_cfunc(lib, "lines",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       lines_imp);

  cx_add_cxfunc(lib, "say",
		cx_args(cx_arg("v", cx->any_type)), cx_args(),
		"#out $v print\n"
		"#out @@n print");

  return true;
}

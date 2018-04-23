#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/call.h"
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
  
  if (!cx_parse_end(cx, in, &fns)) {
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
  struct cx *cx = scope->cx;
  struct line_iter *it = cx_baseof(iter, struct line_iter, iter);
  FILE *fptr = cx_file_ptr(it->in);
    
  if (!cx_get_line(&it->line, &it->len, cx_file_ptr(it->in))) {
    if (feof(fptr)) {
      iter->done = true;
      return false;
    } else if (errno == EAGAIN) {
      cx_box_init(out, cx->nil_type);
    } else {
      cx_error(cx, cx->row, cx->col, "Failed reading line: %d", errno);
      return false;
    }
  } else {
    cx_box_init(out, cx->str_type)->as_str = cx_str_new(it->line, strlen(it->line));
  }
  
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

struct reverse_iter {
  struct cx_iter iter;
  struct cx_file *in;
  ssize_t offs;
};

static bool reverse_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct reverse_iter *it = cx_baseof(iter, struct reverse_iter, iter);
  FILE *fptr = cx_file_ptr(it->in);
  it->offs--;
    
  if (fseek(fptr, it->offs, SEEK_SET)) {
    cx_error(cx, cx->row, cx->col, "Failed seeking: %d", errno);
    return false;
  }

  int c = fgetc(fptr);

  if (c == EOF) {
    if (errno == EAGAIN) {
      cx_box_init(out, cx->nil_type);
      return true;
    }
      
    cx_error(cx, cx->row, cx->col, "Failed reading: %d", errno);
    return false;
  }

  cx_box_init(out, cx->char_type)->as_char = c;
  if (!it->offs) { iter->done = true; }
  return true;
}

static void *reverse_deinit(struct cx_iter *iter) {
  struct reverse_iter *it = cx_baseof(iter, struct reverse_iter, iter);
  cx_file_deref(it->in);
  return it;
}

static cx_iter_type(reverse_iter, {
    type.next = reverse_next;
    type.deinit = reverse_deinit;
  });

static struct cx_iter *reverse_iter_new(struct cx_file *in) {
  struct reverse_iter *it = malloc(sizeof(struct reverse_iter));
  cx_iter_init(&it->iter, reverse_iter());
  it->in = cx_file_ref(in);
  FILE *fptr = cx_file_ptr(in);
  fseek(fptr, 0, SEEK_END);
  it->offs = ftell(cx_file_ptr(in));
  it->iter.done = !it->offs;
  return &it->iter;
}

struct read_iter {
  struct cx_iter iter;
  struct cx_box in;
  struct cx_vec toks;
  struct cx_bin bin;
};

static bool read_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct read_iter *it = cx_baseof(iter, struct read_iter, iter);
  FILE *fptr = cx_file_ptr(it->in.as_file);
  cx_do_vec(&it->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_clear(&it->toks);
  
  if (!cx_parse_tok(cx, fptr, &it->toks)) {
    if (errno == EAGAIN) {
      cx_box_init(out, cx->nil_type);
      return true;
    }
    
    if (feof(fptr)) { iter->done = true; }
    return false;
  }

  cx_bin_clear(&it->bin);
  struct cx_tok *t = cx_vec_start(&it->toks);
  if (!cx_compile(cx, t, t+1, &it->bin)) { return false; }
  if (!cx_eval(&it->bin, 0, -1, cx)) { return false; }
  struct cx_box *v = cx_pop(scope, true);

  if (!v) {
    cx_error(cx, cx->row, cx->col, "Missing read value");
    return false;
  }
  
  *out = *v;
  return true;
}

static void *read_deinit(struct cx_iter *iter) {
  struct read_iter *it = cx_baseof(iter, struct read_iter, iter);
  cx_box_deinit(&it->in);
  cx_do_vec(&it->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&it->toks);
  cx_bin_deinit(&it->bin);
  return it;
}

static cx_iter_type(read_iter, {
    type.next = read_next;
    type.deinit = read_deinit;
  });

static struct cx_iter *read_iter_new(struct cx_box *in) {
  struct read_iter *it = malloc(sizeof(struct read_iter));
  cx_iter_init(&it->iter, read_iter());
  cx_copy(&it->in, in);
  cx_vec_init(&it->toks, sizeof(struct cx_tok));
  cx_bin_init(&it->bin);
  return &it->iter;
}

static bool print_imp(struct cx_call *call) {
  struct cx_box
    *v = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));
  
  cx_print(v, cx_file_ptr(out->as_file));
  return true;
}

static bool load_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_bin *bin = cx_bin_new();
  struct cx_lib *lib = cx_pop_lib(s->cx);
  bool ok = cx_load(s->cx, p->as_str->data, bin) && cx_eval(bin, 0, -1, s->cx);
  cx_push_lib(s->cx, lib);
  cx_bin_deref(bin);
  return ok;
}

static bool fopen_imp(struct cx_call *call) {
  struct cx_box
    *m = cx_test(cx_call_arg(call, 1)),
    *p = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_type *ft = NULL;

  if (strchr(m->as_sym.id, 'r')) {
    ft = strchr(m->as_sym.id, '+') ? s->cx->rwfile_type : s->cx->rfile_type;
  } else if (strchr(m->as_sym.id, 'w') || strchr(m->as_sym.id, 'a')) {
    ft = strchr(m->as_sym.id, '+') ? s->cx->rwfile_type : s->cx->wfile_type;
  } else {
    cx_error(s->cx, s->cx->row, s->cx->col, "Invalid fopen mode: %s", m->as_sym.id);
    return false;
  }

  FILE *f = fopen(p->as_str->data, m->as_sym.id);

  if (f) {
    cx_box_init(cx_push(s), ft)->as_file = cx_file_new(s->cx, fileno(f), NULL, f);
  } else {
    if (errno != ENOENT) {
      cx_error(s->cx, s->cx->row, s->cx->col,
	       "Failed opening file '%s': %d",
	       p->as_str->data, errno);
      
      return false;
    }

    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool unblock_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  bool ok = cx_file_unblock(f->as_file);
  return ok;
}

static bool attach_imp(struct cx_call *call) {
  struct cx_box
    *dst = cx_test(cx_call_arg(call, 1)),
    *src = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  
  if (dup2(dst->as_file->fd, src->as_file->fd) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed attaching file: %d", errno);
    return false;
  }

  return true;
}

static bool flush_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  if (f->as_file->_ptr && fflush(f->as_file->_ptr)) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed flushing file: %d", errno);
    return false;
  }

  return true;
}

static bool seek_imp(struct cx_call *call) {
  struct cx_box
    *pos = cx_test(cx_call_arg(call, 1)),
    *f = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  
  if (fseek(cx_file_ptr(f->as_file), pos->as_int, SEEK_SET) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed seeking: %d", errno);
    return false;
  }

  return true;
}

static bool tell_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  off_t pos = ftell(cx_file_ptr(f->as_file));
  
  if (pos == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed telling: %d", errno);
    return false;
  }

  cx_box_init(cx_push(s), s->cx->int_type)->as_int = pos;
  return true;
}

static bool eof_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = feof(cx_file_ptr(f->as_file));
  return true;
}

static bool close_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  if (!cx_file_close(f->as_file)) {
    cx_error(s->cx, s->cx->row, s->cx->col, "File not open");
    return false;
  }

  return true;
}

static bool read_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->iter_type)->as_iter = read_iter_new(in);
  return true;
}

static bool read_char_imp(struct cx_call *call) {
  struct cx_file *f = cx_test(cx_call_arg(call, 0))->as_file;
  struct cx_scope *s = call->scope;
  int c = fgetc(cx_file_ptr(f));
  
  if (c == EOF) {
    if (errno == EAGAIN) {
      cx_box_init(cx_push(s), s->cx->nil_type);
      return true;
    }

    cx_error(s->cx, s->cx->row, s->cx->col, "Failed reading char: %d", errno);
    return false;
  }

  cx_box_init(cx_push(s), s->cx->int_type)->as_int = c;
  return true;
}

static bool write_imp(struct cx_call *call) {
  struct cx_box
    *v = cx_test(cx_call_arg(call, 1)),
    *out = cx_test(cx_call_arg(call, 0));

  return cx_write(v, cx_file_ptr(out->as_file));
}

static bool lines_imp(struct cx_call *call) {
  struct cx_file *in = cx_test(cx_call_arg(call, 0))->as_file;
  struct cx_scope *s = call->scope;
  struct cx_iter *it = line_iter_new(in);
  
  cx_box_init(cx_push(s),
	      cx_type_get(s->cx->iter_type, s->cx->str_type))->as_iter = it;

  return true;
}

static bool reverse_imp(struct cx_call *call) {
  struct cx_file *in = cx_test(cx_call_arg(call, 0))->as_file;
  struct cx_scope *s = call->scope;
  struct cx_iter *it = reverse_iter_new(in);
  
  cx_box_init(cx_push(s),
	      cx_type_get(s->cx->iter_type, s->cx->char_type))->as_iter = it;

  return true;
}

cx_lib(cx_init_io, "cx/io") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Cmp", "Char", "Int", "Iter", "Opt", "Str", "Sym")) {
    return false;
  }

  cx->file_type = cx_init_file_type(lib, "File");
  cx->rfile_type = cx_init_file_type(lib, "RFile",
				     cx->file_type,
				     cx_type_get(cx->seq_type, cx->char_type));
  cx->rfile_type->iter = cx_file_iter;
    
  cx->wfile_type = cx_init_file_type(lib, "WFile", cx->file_type);
  cx->rwfile_type = cx_init_file_type(lib, "RWFile", 
				      cx->rfile_type, cx->wfile_type);
  cx->rwfile_type->iter = cx_file_iter;

  cx_box_init(cx_put_const(lib, cx_sym(cx, "in"), false), cx->rfile_type)->as_file =
    cx_file_new(cx, fileno(stdin), NULL, stdin);
    
  cx_box_init(cx_put_const(lib, cx_sym(cx, "out"), false), cx->wfile_type)->as_file =
    cx_file_new(cx, fileno(stdout), NULL, stdout);

  cx_box_init(cx_put_const(lib, cx_sym(cx, "error"), false),
	      cx->wfile_type)->as_file =
    cx_file_new(cx, fileno(stderr), NULL, stderr);

  cx_add_macro(lib, "include:", include_parse);

  cx_add_cfunc(lib, "print",
	       cx_args(cx_arg("v", cx->any_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       print_imp);
    
  cx_add_cfunc(lib, "load",
	       cx_args(cx_arg("path", cx->str_type)), cx_args(),
	       load_imp);  

  cx_add_cfunc(lib, "fopen",
	       cx_args(cx_arg("path", cx->str_type), cx_arg("mode", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->file_type))),
	       fopen_imp);  

  cx_add_cfunc(lib, "unblock",
	       cx_args(cx_arg("f", cx->file_type)), cx_args(),
	       unblock_imp);

  cx_add_cfunc(lib, "attach",
	       cx_args(cx_arg("src", cx->file_type), cx_arg("dst", cx->file_type)),
	       cx_args(),
	       attach_imp);

  cx_add_cfunc(lib, "flush",
	       cx_args(cx_arg("f", cx->wfile_type)), cx_args(),
	       flush_imp);

  cx_add_cfunc(lib, "seek",
	       cx_args(cx_arg("f", cx->file_type), cx_arg("pos", cx->int_type)),
	       cx_args(),
	       seek_imp);

  cx_add_cfunc(lib, "tell",
	       cx_args(cx_arg("f", cx->file_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       tell_imp);

  cx_add_cfunc(lib, "eof",
	       cx_args(cx_arg("f", cx->file_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       eof_imp);

  cx_add_cfunc(lib, "close",
	       cx_args(cx_arg("f", cx->file_type)), cx_args(),
	       close_imp);

  cx_add_cfunc(lib, "read",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       read_imp);

  cx_add_cfunc(lib, "read-char",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       read_char_imp);

  cx_add_cfunc(lib, "write",
	       cx_args(cx_arg("f", cx->wfile_type), cx_arg("v", cx->opt_type)),
	       cx_args(),
	       write_imp);

  cx_add_cfunc(lib, "lines",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->iter_type, cx->str_type))),
	       lines_imp);

  cx_add_cfunc(lib, "reverse",
	       cx_args(cx_arg("f", cx->rfile_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->iter_type, cx->char_type))),
	       reverse_imp);

  return true;
}

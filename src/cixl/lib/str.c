#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/char.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/str.h"
#include "cixl/scope.h"
#include "cixl/str.h"

typedef bool (*cx_split_t)(unsigned char c);

struct cx_split_iter {
  struct cx_iter iter;
  struct cx_iter *in;
  cx_split_t split;
  struct cx_buf out;
};

bool split_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_split_iter *it = cx_baseof(iter, struct cx_split_iter, iter);
  struct cx *cx = scope->cx;
  struct cx_box c;

  while (true) {
    if (!cx_iter_next(it->in, &c, scope)) {
      iter->done = true;
      fflush(it->out.stream);
      if (it->out.data[0]) { break; }
      return false;
    }

    if (c.type != cx->char_type) {
      cx_error(cx, cx->row, cx->col, "Expected type Char, actual: %s", c.type->id);
      return false;
    }
    
    if (it->split(c.as_char)) {
      fflush(it->out.stream);
      if (it->out.data[0]) { break; }
    } else {
      fputc(c.as_char, it->out.stream);
    }
  }

  cx_buf_close(&it->out);
  cx_box_init(out, cx->str_type)->as_str = cx_str_new(it->out.data);
  free(it->out.data);
  cx_buf_open(&it->out);
  return true;
}

void *split_deinit(struct cx_iter *iter) {
  struct cx_split_iter *it = cx_baseof(iter, struct cx_split_iter, iter);
  cx_iter_deref(it->in);
  cx_buf_close(&it->out);
  free(it->out.data);
  return it;
}

cx_iter_type(split_iter, {
    type.next = split_next;
    type.deinit = split_deinit;
  });

struct cx_iter *cx_split_iter_new(struct cx_iter *in, cx_split_t split) {
  struct cx_split_iter *it = malloc(sizeof(struct cx_split_iter));
  cx_iter_init(&it->iter, split_iter());
  it->in = in;
  it->split = split;
  cx_buf_open(&it->out);
  return &it->iter;
}

bool split_lines(unsigned char c) { return c == '\r' || c == '\n'; }

static bool lines_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_split_iter_new(cx_iter(&in), split_lines);
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;
  cx_box_deinit(&in);
  return true;
}

bool split_words(unsigned char c) {
  return !isalpha(c);
}

static bool words_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_split_iter_new(cx_iter(&in), split_words);
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;
  cx_box_deinit(&in);
  return true;
}

static bool char_upper_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_char = toupper(v->as_char);
  return true;
}

static bool char_lower_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_char = tolower(v->as_char);
  return true;
}

static bool char_int_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  cx_box_init(v, scope->cx->int_type)->as_int = v->as_char;
  return true;
}

static bool int_char_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box *v = cx_test(cx_peek(scope, false));
  
  if (v->as_int < 0 || v->as_int > 255) {
    cx_error(cx, cx->row, cx->col, "Invalid char: %" PRId64, v->as_int);
    return false;
  }
  
  cx_box_init(v, cx->char_type)->as_char = v->as_int;
  return true;
}

static bool int_str_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  char *s = cx_fmt("%" PRId64, v->as_int);
  cx_box_init(v, scope->cx->str_type)->as_str = cx_str_new(s);
  free(s);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  size_t len = v->as_str->len;
  cx_box_deinit(v);
  cx_box_init(v, scope->cx->int_type)->as_int = len;
  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    i = *cx_test(cx_pop(scope, false)),
    s = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  if (i.as_int < 0 || i.as_int >= s.as_str->len) {
    cx_error(cx, cx->row, cx->col, "Index out of bounds: %" PRId64, i.as_int);
    goto exit;
  }
  
  cx_box_init(cx_push(scope), cx->char_type)->as_char = *(s.as_str->data+i.as_int);
  ok = true;
 exit:
  cx_box_deinit(&s);
  return ok;
}

static bool seq_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_iter(&in);
  bool ok = false;
  struct cx_buf out;
  cx_buf_open(&out);
  struct cx_box c;
  
  while (cx_iter_next(it, &c, scope)) {
    if (c.type != cx->char_type) {
      cx_error(cx, cx->row, cx->col, "Expected type Char, actual: %s", c.type->id);
      cx_buf_close(&out);
      goto exit;
    }
    
    fputc(c.as_char, out.stream);
  }

  cx_buf_close(&out);
  cx_box_init(cx_push(scope), cx->str_type)->as_str = cx_str_new(out.data);
  ok = true;
 exit:
  free(out.data);
  cx_box_deinit(&in);
  cx_iter_deref(it);
  return ok;
}

static bool str_int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  int64_t iv = strtoimax(s->data, NULL, 10);
  
  if (!iv && (!s->data[0] || s->data[0] != '0' || s->data[1])) {
    cx_box_init(cx_push(scope), cx->nil_type);
  } else {
    cx_box_init(cx_push(scope), cx->int_type)->as_int = iv;
  }
  
  cx_box_deinit(&v);
  return true;
}

static bool str_sub_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    y = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope),
	      scope->cx->int_type)->as_int = cx_str_dist(x.as_str->data,
							 y.as_str->data);
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool str_upper_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  for (char *c = s.as_str->data; *c; c++) { *c = toupper(*c); }
  cx_box_deinit(&s);
  return true;
}

static bool str_lower_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  for (char *c = s.as_str->data; *c; c++) { *c = tolower(*c); }
  cx_box_deinit(&s);
  return true;
}

cx_lib(cx_init_str, "cx/str", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/iter");
    cx_use(cx, "cx/str/types");

    cx_add_cfunc(lib, "lines",
		 cx_args(cx_arg("in", cx->seq_type)),
		 cx_args(cx_arg(NULL, cx->iter_type)),
		 lines_imp);

    cx_add_cfunc(lib, "words",
		 cx_args(cx_arg("in", cx->seq_type)),
		 cx_args(cx_arg(NULL, cx->iter_type)),
		 words_imp);

    cx_add_cfunc(lib, "upper",
		 cx_args(cx_arg("c", cx->char_type)),
		 cx_args(cx_arg(NULL, cx->char_type)),
		 char_upper_imp);

    cx_add_cfunc(lib, "lower",
		 cx_args(cx_arg("c", cx->char_type)),
		 cx_args(cx_arg(NULL, cx->char_type)),
		 char_lower_imp);

    cx_add_cfunc(lib, "int",
		 cx_args(cx_arg("c", cx->char_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 char_int_imp);

    cx_add_cfunc(lib, "char",
		 cx_args(cx_arg("v", cx->int_type)),
		 cx_args(cx_arg(NULL, cx->char_type)),
		 int_char_imp);
  
    cx_add_cfunc(lib, "str",
		 cx_args(cx_arg("v", cx->int_type)),
		 cx_args(cx_arg(NULL, cx->str_type)),
		 int_str_imp);

    cx_add_cfunc(lib, "len",
		 cx_args(cx_arg("s", cx->str_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 len_imp);

    cx_add_cfunc(lib, "get",
		 cx_args(cx_arg("s", cx->str_type), cx_arg("i", cx->int_type)),
		 cx_args(cx_arg(NULL, cx->char_type)),
		 get_imp);

    cx_add_cfunc(lib, "str",
		 cx_args(cx_arg("s", cx->seq_type)),
		 cx_args(cx_arg(NULL, cx->str_type)),
		 seq_imp);

    cx_add_cfunc(lib, "int",
		 cx_args(cx_arg("s", cx->str_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 str_int_imp);

    cx_add_cfunc(lib, "-",
		 cx_args(cx_arg("x", cx->str_type), cx_arg("y", cx->str_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 str_sub_imp);

    cx_add_cfunc(lib, "upper",
		 cx_args(cx_arg("s", cx->str_type)), cx_args(),
		 str_upper_imp);

    cx_add_cfunc(lib, "lower",
		 cx_args(cx_arg("s", cx->str_type)), cx_args(),
		 str_lower_imp);
  })

cx_lib(cx_init_str_types, "cx/str/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");

    cx->char_type = cx_init_char_type(lib);
    cx->str_type = cx_init_str_type(lib);
  });

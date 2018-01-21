#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/libs/str.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/iter.h"
#include "cixl/types/str.h"

static bool char_upper_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = toupper(v.as_char);
  return true;
}

static bool char_lower_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = tolower(v.as_char);
  return true;
}

static bool char_int_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v.as_char;
  return true;
}

static bool int_char_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  
  if (v.as_int < 0 || v.as_int > 255) {
    cx_error(cx, cx->row, cx->col, "Invalid char: %" PRId64, v.as_int);
    return false;
  }
  
  cx_box_init(cx_push(scope), cx->char_type)->as_char = v.as_int;
  return true;
}

static bool int_str_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  char *s = cx_fmt("%" PRId64, v.as_int);
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(s);
  free(s);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v.as_str->len;
  cx_box_deinit(&v);
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

void cx_init_str(struct cx *cx) {
  cx_add_cfunc(cx, "upper",
	       cx_args(cx_arg("c", cx->char_type)), cx_rets(cx_ret(cx->char_type)),
	       char_upper_imp);

  cx_add_cfunc(cx, "lower",
	       cx_args(cx_arg("c", cx->char_type)), cx_rets(cx_ret(cx->char_type)),
	       char_lower_imp);

  cx_add_cfunc(cx, "int",
	       cx_args(cx_arg("c", cx->char_type)), cx_rets(cx_ret(cx->int_type)),
	       char_int_imp);

  cx_add_cfunc(cx, "char",
	       cx_args(cx_arg("v", cx->int_type)), cx_rets(cx_ret(cx->char_type)),
	       int_char_imp);
  
  cx_add_cfunc(cx, "str",
	       cx_args(cx_arg("v", cx->int_type)), cx_rets(cx_ret(cx->str_type)),
	       int_str_imp);

  cx_add_cfunc(cx, "len",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_rets(cx_ret(cx->int_type)),
	       len_imp);

  cx_add_cfunc(cx, "get",
	       cx_args(cx_arg("s", cx->str_type), cx_arg("i", cx->int_type)),
	       cx_rets(cx_ret(cx->char_type)),
	       get_imp);

  cx_add_cfunc(cx, "str",
	       cx_args(cx_arg("s", cx->seq_type)),
	       cx_rets(cx_ret(cx->str_type)),
	       seq_imp);

  cx_add_cfunc(cx, "int",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_rets(cx_ret(cx->opt_type)),
	       str_int_imp);
  
  cx_add_func(cx, "upper",
	      cx_args(cx_arg("s", cx->str_type)),
	      cx_rets(cx_ret(cx->str_type)),
	      "$s map &upper str");

  cx_add_func(cx, "lower",
	      cx_args(cx_arg("s", cx->str_type)),
	      cx_rets(cx_ret(cx->str_type)),
	      "$s map &lower str");
}

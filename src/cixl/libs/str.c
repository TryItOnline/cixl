#include <string.h>
#include <inttypes.h>

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

static bool iterable_imp(struct cx_scope *scope) {
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
  cx_iter_unref(it);
  return ok;
}

void cx_init_str(struct cx *cx) {
  cx_add_func(cx, "len", cx_arg(cx->str_type))->ptr = len_imp;
  cx_add_func(cx, "get", cx_arg(cx->str_type), cx_arg(cx->int_type))->ptr = get_imp;
  cx_add_func(cx, "str", cx_arg(cx->iterable_type))->ptr = iterable_imp;
  
  cx_test(cx_eval_str(cx,
		      "func: upper(s Str) "
		      "  $s map &upper str;"));

  cx_test(cx_eval_str(cx,
		      "func: lower(s Str) "
		      "  $s map &lower str;"));
}

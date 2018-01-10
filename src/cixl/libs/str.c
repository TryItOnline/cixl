#include <string.h>
#include <inttypes.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/libs/str.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
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

static bool for_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    str = *cx_test(cx_pop(scope, false));

  bool ok = false;

  for (char *c = str.as_str->data; *c; c++) {
    cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = *c;
    if (!cx_call(&act, scope)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_deinit(&str);
  cx_box_deinit(&act);
  return ok;
}

static bool map_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    str = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (char *c = str.as_str->data; *c; c++) {
    cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = *c;
    if (!cx_call(&act, scope)) { goto exit; }
    struct cx_box *res = cx_pop(scope, true);

    if (!res) {
      cx_error(cx, cx->row, cx->col, "Missing result for '%c'", *c);
      goto exit;
    }

    if (res->type != cx->char_type) {
      cx_error(cx, cx->row, cx->col, "Expected type Char, got %s", res->type->id);
      goto exit;
    }

    *c = res->as_char;
  }

  *cx_push(scope) = str;
  ok = true;
 exit:
  cx_box_deinit(&act);
  return ok;
}

void cx_init_str(struct cx *cx) {
  cx_add_func(cx, "len", cx_arg(cx->str_type))->ptr = len_imp;
  cx_add_func(cx, "get", cx_arg(cx->str_type), cx_arg(cx->int_type))->ptr = get_imp;
  
  cx_add_func(cx, "for", cx_arg(cx->str_type), cx_arg(cx->any_type))->ptr = for_imp;
  cx_add_func(cx, "map", cx_arg(cx->str_type), cx_arg(cx->any_type))->ptr = map_imp;

  cx_test(cx_eval_str(cx,
		      "func: upper(s Str) "
		      "  $s map &upper;"));

  cx_test(cx_eval_str(cx,
		      "func: lower(s Str) "
		      "  $s map &lower;"));
}

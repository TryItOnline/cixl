#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/libs/io.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/str.h"

static bool say_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  puts(s.as_str->data);
  cx_box_deinit(&s);
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
  return true;
}

static bool load_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_load(scope->cx, p.as_str->data);
  cx_box_deinit(&p);
  return true;
}

static bool dump_imp(struct cx_scope *scope) {
  cx_fprint_stack(scope, stderr);
  return true;
}

void cx_init_io(struct cx *cx) {
  cx_add_func(cx, "say", cx_arg(cx->str_type))->ptr = say_imp;
  cx_add_func(cx, "ask", cx_arg(cx->str_type))->ptr = ask_imp;
  cx_add_func(cx, "load", cx_arg(cx->str_type))->ptr = load_imp;  
  cx_add_func(cx, "dump")->ptr = dump_imp;
}

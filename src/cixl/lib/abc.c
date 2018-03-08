#include "cixl/bool.h"
#include "cixl/char.h"
#include "cixl/cx.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/iter.h"
#include "cixl/lambda.h"
#include "cixl/lib.h"
#include "cixl/nil.h"
#include "cixl/lib/abc.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/sym.h"

cx_lib(cx_init_abc, "cx/abc") { 
  struct cx *cx = lib->cx;

  cx->opt_type = cx_add_type(lib, "Opt");
  cx->opt_type->trait = true;

  cx->any_type = cx_add_type(lib, "A", cx->opt_type);
  cx->any_type->trait = true;

  cx->cmp_type = cx_add_type(lib, "Cmp", cx->any_type);
  cx->cmp_type->trait = true;  

  cx->seq_type = cx_add_type(lib, "Seq", cx->any_type);
  cx->seq_type->trait = true;

  cx->num_type = cx_add_type(lib, "Num", cx->cmp_type);
  cx->num_type->trait = true;

  cx->lib_type = cx_init_lib_type(lib);
  cx->meta_type = cx_init_meta_type(lib);

  cx->bool_type = cx_init_bool_type(lib);
  cx_box_init(cx_put_const(lib, cx_sym(cx, "t"), false),
	      cx->bool_type)->as_bool = true;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "f"), false),
	      cx->bool_type)->as_bool = false;

  cx->nil_type = cx_init_nil_type(lib);

  cx->int_type = cx_init_int_type(lib);
  cx_box_init(cx_put_const(lib, cx_sym(cx, "nil"), false), cx->nil_type);

  cx->char_type = cx_init_char_type(lib);
  cx->str_type = cx_init_str_type(lib);
  cx->sym_type = cx_init_sym_type(lib);
  cx->func_type = cx_init_func_type(lib);
  cx->fimp_type = cx_init_fimp_type(lib);
  cx->lambda_type = cx_init_lambda_type(lib);
  cx->iter_type = cx_init_iter_type(lib);
  cx->stack_type = cx_init_stack_type(lib);

  return true;
}

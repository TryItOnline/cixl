#include "cixl/cx.h"
#include "cixl/libs/io.h"
#include "cixl/libs/iter.h"
#include "cixl/libs/math.h"
#include "cixl/libs/rec.h"
#include "cixl/libs/stack.h"
#include "cixl/libs/str.h"
#include "cixl/libs/table.h"
#include "cixl/libs/time.h"
#include "cixl/libs/type.h"
#include "cixl/libs/var.h"
#include "cixl/repl.h"
#include "cixl/set.h"
#include "cixl/tests.h"
#include "cixl/vec.h"

int main() {
  cx_vec_tests();
  cx_set_tests();
  cx_tests();

  struct cx cx;
  cx_init(&cx);
  cx_init_io(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_type(&cx);
  cx_init_rec(&cx);
  cx_init_stack(&cx);
  cx_init_str(&cx);
  cx_init_table(&cx);
  cx_init_time(&cx);
  cx_init_var(&cx);
  cx_repl(&cx, stdin, stdout);
  cx_deinit(&cx);
  return 0;
}

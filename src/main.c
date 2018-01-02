#include "cixl/cx.h"
#include "cixl/libs/io.h"
#include "cixl/libs/math.h"
#include "cixl/libs/str.h"
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
  cx_init_math(&cx);
  cx_init_str(&cx);
  cx_repl(&cx, stdin, stdout);
  cx_deinit(&cx);
  return 0;
}

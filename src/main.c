#include "cixl/cx.h"
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
  cx_init_math(&cx);
  cx_repl(&cx, stdin, stdout);
  cx_deinit(&cx);
  return 0;
}

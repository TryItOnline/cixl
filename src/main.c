#include "cixl/cx.h"
#include "cixl/repl.h"
#include "cixl/set.h"
#include "cixl/tests.h"
#include "cixl/vec.h"

bool eval(struct cx *cx, size_t start) {
  bool _eval(size_t start) {
    cx->emit_pc = start;
    //struct cx_scope *scope = cx_scope(cx, 0);

    while (!cx->stop) {
      switch (cx->emit_pc) {

      }
    }

    cx->stop = false;
    return true;
  }

  bool (*prev)(size_t start) = cx->emit_eval;
  cx->emit_eval = _eval;
  bool ok = _eval(start);
  cx->emit_eval = prev;
  return ok;
}

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

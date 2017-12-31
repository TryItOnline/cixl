#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/tests.h"

static void run(struct cx *cx, const char *in) {
  cx_vec_clear(&cx_scope(cx, 0)->stack);
  
  if (!cx_eval_str(cx, in)) {
    printf("%s\n", in);
    
    cx_do_vec(&cx->errors, struct cx_error, e) {
      printf("Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
      cx_error_deinit(e);
    }
    
    cx_vec_clear(&cx->errors);
  }
}

static void type_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "42 type Int = test");
  run(&cx, "Int is A test");
  run(&cx, "! (A is Int) test");

  cx_deinit(&cx);
}

static void int_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "42 test");
  run(&cx, "0! test");
  run(&cx, "21 + 21 = 42 test");
  run(&cx, "7 + 14, 7 + 14 + = 42 test");
  run(&cx, "1 = 2! test");

  cx_deinit(&cx);
}

static void str_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "'foo' test");
  run(&cx, "''! test");
  run(&cx, "'foo' = 'foo' test");
  run(&cx, "'foo' = 'bar' ! test");
  run(&cx, "'foo' == 'foo' ! test");
  run(&cx, "'foo' map &upper = 'FOO' test");
  
  cx_deinit(&cx);
}

static void vect_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "1 2 3 (4 5 vect) len 2 = test");
  run(&cx, "(1 2 3 vect) pop 3 = test");
  run(&cx, "(1 2 3 vect) % 4 push len 4 = test");
  run(&cx, "(1 2 vect) for {2 *} + 6 = test");
  
  cx_deinit(&cx);
}

static void stack_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "7 14 % + 28 = test");
  run(&cx, "7 14 % _ + 21 = test");

  cx_deinit(&cx);
}

static void group_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "(7 14 21) 21 = test");

  cx_deinit(&cx);
}

static void func_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "func: foo() 42; foo = 42 test");
  run(&cx, "func: foo() 42; &foo call = 42 test");
  run(&cx, "func: bar(x A) $x + 35; bar 7 42 = test");
  run(&cx, "func: baz(x y Int z 0) $x + $y + $z; baz 1 3 5 9 = test");
  
  cx_deinit(&cx);
}

static void coro_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "{1 2 yield 3} call call 3 = test");
  run(&cx, "{let: x 42; yield $x} call call 42 = test");
  run(&cx, "func: foo() 1 2 yield 3; foo call 3 = test");

  cx_deinit(&cx);
}

static void math_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

  run(&cx, "fib 50 = 12586269025 test");

  cx_deinit(&cx);
}

void cx_tests() {
  type_tests();
  int_tests();
  str_tests();
  vect_tests();
  stack_tests();
  group_tests();
  func_tests();
  coro_tests();
  math_tests();
}

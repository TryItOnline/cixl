#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/math.h"
#include "cixl/libs/str.h"
#include "cixl/libs/rec.h"
#include "cixl/libs/time.h"
#include "cixl/libs/type.h"
#include "cixl/libs/var.h"
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

static void comment_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);
  
  run(&cx, "1 //testing testing\n+ 2 = 3 test");
  run(&cx, "1 /*testing\ntesting*/+ 2 = 3 test");

  cx_deinit(&cx);
}

static void type_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_type(&cx);

  run(&cx, "42 type Int = test");
  run(&cx, "Int is A test");
  run(&cx, "! (A is Int) test");

  cx_deinit(&cx);
}

static void stack_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

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

static void if_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_str(&cx);
  
  run(&cx, "#t if 42 = 42 test");
  run(&cx, "#f else 42 = 42 test");
  run(&cx, "#t if-else `yes `no = `yes test");
  run(&cx, "#f if-else `yes `no = `no test");
  run(&cx, "'foo' %%, $ if &upper");
  cx_deinit(&cx);
}

static void let_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);
  cx_init_str(&cx);
  cx_init_var(&cx);

  run(&cx, "(let: foo 42; $foo 42 = test)");
  run(&cx, "(let: (x y z) 1 2, 3 + 4; $x + $y + $z 10 = test)");
  run(&cx, "(let: (foo Int bar Str) 7 '35'; $foo +, $bar int = 42 test)");

  run(&cx, "(is-var `foo !test "
           " put-var `foo 42 "
           " is-var `foo test "
           " get-var `foo = 42 test)");

  run(&cx,
      "(let: foo 42; "
      " unlet `foo "
      " let: foo 'foo'; "
      " $foo = 'foo' test)");
  
  cx_deinit(&cx);
}

static void func_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);
  
  run(&cx, "func: foo() 42; foo = 42 test");
  run(&cx, "func: foo() 42; &foo call = 42 test");
  run(&cx, "func: bar(x A) $x + 35; bar 7 42 = test");
  run(&cx, "func: baz(x y Int z T0) $x + $y + $z; baz 1 3 5 9 = test");

  run(&cx,
      "func: maybe-add(x y Num) $x + $y; "
      "func: maybe-add(x y Int) $x = 42 if-else 42 {upcall $x $y}; "
      "maybe-add 1 2 3 = test "
      "maybe-add 42 2 42 = test");

  run(&cx,
      "func: answer(0) 0; "
      "func: answer(x Int) $x; "
      "func: answer(42) 'correct'; "
      "answer 0 0 = test "
      "answer 1 1 = test "
      "answer 42 'correct' = test");

  cx_deinit(&cx);
}

static void int_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "42 test");
  run(&cx, "0! test");
  run(&cx, "1 = 2! test");
  run(&cx, "42 str '42' = test");

  cx_deinit(&cx);
}

static void char_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);
  
  run(&cx, "\\a upper \\A = test");
  run(&cx, "\\0 int + 7 char \\7 = test");
  
  cx_deinit(&cx);
}

static void str_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_str(&cx);
  
  run(&cx, "'foo' test");
  run(&cx, "''! test");
  run(&cx, "'foo' = 'foo' test");
  run(&cx, "'foo' = 'bar' ! test");
  run(&cx, "'foo' == 'foo' ! test");
  run(&cx, "'foo' upper = 'FOO' test");
  run(&cx, "'foobar' 3 get \\b = test");
  run(&cx, "'42' int 42 = test");
  
  cx_deinit(&cx);
}

static void sym_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "`foo = `foo test");
  run(&cx, "`foo = `bar !test");
  run(&cx, "'foo' sym = `foo test");
  run(&cx, "`foo str = 'foo' test");
  run(&cx, "Sym new, Sym new = !test");
    
  cx_deinit(&cx);
}

static void rat_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

  run(&cx, "1 / 2, 5 / 2 *, 5 / 4 = test");
  run(&cx, "1 / 2, 5 / 2 +, 3 / 1 = test");
  
  cx_deinit(&cx);
}

static void time_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_time(&cx);

  run(&cx, "now <= now test");
  run(&cx, "[1 0 0 24] time days = 367 test");
  run(&cx, "2m =, 120s test");
  run(&cx, "1 days ,+ 1h * 2h = 50 test");
  run(&cx, "1 months ,+ 1 days * 3 days = 94");
  
  cx_deinit(&cx);
}

static void guid_tests() {
  struct cx cx;
  cx_init(&cx);

  run(&cx, "Guid new, Guid new = !test");
  
  cx_deinit(&cx);
}

static void vect_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

  run(&cx, "1 2 3 [4 5] len 2 = test");
  run(&cx, "[1 2 3] pop 3 = test");
  run(&cx, "[1 2 3] % 4 push<Vect A> len 4 = test");
  run(&cx, "[1 2] for {2 *} + 6 = test");
  
  cx_deinit(&cx);
}

static void math_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

  run(&cx, "21 +<Int Int> 21 = 42 test");
  run(&cx, "7 + 14, 7 + 14 + = 42 test");
  run(&cx, "fib 50 = 12586269025 test");

  cx_deinit(&cx);
}

static void rec_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_rec(&cx);
  cx_init_var(&cx);

  run(&cx,
      "rec: Foo() x Int y Str; "
      "(let: foo Foo new; "
      " $foo !test "
      " $foo put `x 42 "
      " $foo test "
      " $foo get `x = 42 test "
      " $foo get `y = #nil test)");

  run(&cx,
      "let: (bar baz) Foo new %%; "
      "$bar put `x 42 "
      "$baz put `x 42 "
      "$bar = $baz test "
      "$bar put `y 'bar' "
      "$bar = $baz !test "
      "$baz put `y 'baz' "
      "$bar = $baz !test");
  
  run(&cx,
      "func: =(a b Foo) $a get `x =, $b get `x; "
      "$bar = $baz test");
  
  cx_deinit(&cx);
}

static void compile_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_math(&cx);

  run(&cx, "Bin new %, $ compile '1 + 2' call = 3 test");
  cx_deinit(&cx);
}

void cx_tests() {
  comment_tests();
  type_tests();
  stack_tests();
  group_tests();
  if_tests();
  let_tests();
  func_tests();
  int_tests();
  char_tests();
  str_tests();
  sym_tests();
  rat_tests();
  time_tests();
  guid_tests();
  vect_tests();
  math_tests();
  rec_tests();
  compile_tests();
}

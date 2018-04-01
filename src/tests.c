#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/set.h"
#include "cixl/stack.h"
#include "cixl/scope.h"
#include "cixl/vec.h"

static void run(struct cx *cx, const char *in) {
  cx_vec_clear(&cx_scope(cx, 0)->stack);
  cx_eval_str(cx, in);

  if (cx->errors.count) {
    printf("%s\n", in);
    cx_dump_errors(cx, stdout);
  }
}

static void init_cx(struct cx *cx) {
  cx_init(cx);
  cx_init_libs(cx);
  cx_use(cx, "cx/meta", "use:");
}

static void func_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/func cx/cond cx/error cx/math cx/stack cx/var;");
  run(&cx, "func: foo0()(_ Int) 42; foo0 42 = check");
  run(&cx, "func: foo2(x y)(_ Int) $x $y +; 1 2 foo2 3 = check");
  run(&cx, "func: foo3(op Func _ _ Int)(_ Int) $op call; &- 49 7 foo3 42 = check");
  run(&cx, "func: foo1(x Int)(result Int) let: result $x 2 *;; 21 foo1 42 = check");

  run(&cx,
      "func: fortytwo(_ Int)(#f) _;"
      "func: fortytwo(42)(#t);"
      "21 fortytwo !check "
      "42 fortytwo check");
						       
  run(&cx, "(let: x 42; func: foo0()(_ Int) $x;) &foo0 call 42 = check");
  run(&cx, "func: foo1(x A)(_ Int) $x 35 +; 7 foo1<A> 42 = check");
  run(&cx, "func: foo3(x y Int z Arg0)(_ Int) $x $y $z + +; 1 3 5 foo3 9 = check");

  run(&cx,
      "func: answer(0)(_ Int) 0; "
      "func: answer(x Int)(_ Int) $x; "
      "func: answer(42)(_ Sym) `correct; "
      "0 answer 0 = check "
      "1 answer 1 = check "
      "42 answer `correct = check");

  cx_deinit(&cx);
}

static void ref_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/math cx/ref cx/stack;");
  run(&cx, "#nil ref % 42 set deref 42 = check");
  run(&cx, "41 ref % {++ %} set-call 42 = check");
  
  cx_deinit(&cx);
}

static void pair_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/math cx/pair cx/stack;");
  run(&cx, "1 2. % rezip unzip - 1 = check");
  run(&cx, "1 2. %% rezip unzip - -1 = check");
  
  cx_deinit(&cx);
}


static void stack_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/iter cx/math cx/stack;");
  run(&cx, "Stack new % 1 push % 2 push [1 2] = check");
  run(&cx, "1 2 [3 4 5] len 3 = check");
  run(&cx, "[1 2 3] pop 3 = check");
  run(&cx, "[1 2 3] % 4 push<Stack Opt> len 4 = check");
  run(&cx, "[1 2] {2 *} for + 6 = check");
  run(&cx, "[1 2] [3 4] < check");
  run(&cx, "[1 2 3] [1 2] > check");
  run(&cx, "[3 2 1] % #nil sort {} for + - -4 = check");
  run(&cx, "[1 2 3] % {~ <=>} sort {} for + - 0 = check");
  run(&cx, "7 14 % + 28 = check");
  run(&cx, "7 14 % _ + 21 = check");

  cx_deinit(&cx);
}

static void table_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx,
      "use: cx/cond cx/error cx/iter cx/math cx/pair cx/stack "
      "cx/table cx/var cx/type;");

  run(&cx, "(let: t Table new;"
           " $t 1 'foo' put"
           " $t 2 'bar' put"
           " $t 1 'baz' put"
           " $t 1 get 'baz' = check"
           " $t len 2 = check"
           " $t 2 delete"
           " $t len 1 = check)");

  run(&cx, "[1 'foo'. 2 'bar'.] table stack len 2 = check");
  
  cx_deinit(&cx);
}

static void math_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/math;");
  run(&cx, "21 21 +<Int Int> 42 = check");
  run(&cx, "50 fib 12586269025 = check");

  cx_deinit(&cx);
}

static void rec_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/func cx/rec cx/stack cx/type cx/var;");

  run(&cx,
      "rec: Foo() x Int y Str; "
      "(let: foo Foo new; "
      " $foo !check "
      " $foo `x 42 put "
      " $foo check "
      " $foo `x get 42 = check "
      " $foo `y get #nil = check)");

  run(&cx,
      "let: (bar baz) Foo new %%; "
      "$bar `x 42 put "
      "$baz `x 42 put "
      "$bar $baz = check "
      "$bar `y 'bar' put "
      "$bar $baz = !check "
      "$baz `y 'baz' put "
      "$bar $baz = !check");
  
  run(&cx,
      "func: =(a b Foo)(_ Bool) $a `x get $b `x get =; "
      "$bar $baz = check");
  
  cx_deinit(&cx);
}

static void compile_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/bin cx/cond cx/error cx/func cx/math cx/stack cx/type;");
  run(&cx, "Bin new % '1 2 +' compile call 3 = check");
  cx_deinit(&cx);
}

int main() {
  func_tests();
  ref_tests();
  pair_tests();
  stack_tests();
  table_tests();
  math_tests();
  rec_tests();
  compile_tests();
  return 0;
}

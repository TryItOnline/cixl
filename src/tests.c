#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/set.h"
#include "cixl/stack.h"
#include "cixl/scope.h"
#include "cixl/vec.h"

static void vec_tests() {
  const int reps = 100;
  
  void push_pop_tests() {
    struct cx_vec vec;
    cx_vec_init(&vec, sizeof(int));
    
    for (int i = 0; i < reps; i++) {
      *(int *)cx_vec_push(&vec) = i;
    }
    
    cx_test(vec.count == reps);
    
    for (int i = 0; i < reps; i++) {
      cx_vec_pop(&vec);
  }

    cx_test(!vec.count);
    cx_vec_deinit(&vec);
  }
  
  void do_tests() {
    struct cx_vec vec;
    cx_vec_init(&vec, sizeof(int));
    
    for (int i = 0; i < reps; i++) {
      *(int *)cx_vec_push(&vec) = i;
    }
    
    int expected = 0;
    
    cx_do_vec(&vec, int, actual) {
      cx_test(*actual == expected++);
    }    
    
    cx_vec_deinit(&vec);
  }

  push_pop_tests();
  do_tests();
}

void set_tests() {
  const int reps = 100;
  
  void insert_delete_tests() {
    struct cx_set set;
    cx_set_init(&set, sizeof(int64_t), cx_cmp_int);
    
    for (int64_t i = 0; i < reps; i++) {
      void *p = cx_set_insert(&set, &i);
      cx_test(p);
      *(int64_t *)p = i;
    }
    
    cx_test(set.members.count == reps);
    
    for (int64_t i = 0; i < reps; i++) {
      cx_test(cx_set_delete(&set, &i));
    }
    
    cx_test(!set.members.count);
    cx_set_deinit(&set);
  }
  
  insert_delete_tests();
}

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

static void comment_tests() {
  struct cx cx;
  init_cx(&cx);
  
  run(&cx, "use: cx/error cx/math;");
  run(&cx, "1 //foo bar\n2 + 3 = check");
  run(&cx, "1 /*foo \n bar*/2 + 3 = check");

  cx_deinit(&cx);
}

static void error_tests() {
  struct cx cx;
  init_cx(&cx);
  
  run(&cx, "use: cx/abc cx/cond cx/error cx/pair cx/stack;");
  run(&cx, "catch: (A `error ~.) 42 throw 21; `error 42. = check");
  run(&cx, "catch: (Int `int ~.) (A `a ~.) (42 throw) 21; `int 42. = check");

  cx_deinit(&cx);
}

static void lib_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: "
      "(cx/abc Int Lambda Sym) "
      "(cx/cond =) "
      "(cx/const define:) "
      "(cx/func call func:) "
      "(cx/meta lib: cx-lib id use:) "
      "(cx/error check);");
  
  run(&cx, "lib: foo define: (bar Int) 42;; use: (foo #bar); #bar 42 = check");

  run(&cx, "lib: foo func: bar()(_ Int) 42;; use: (foo bar); bar 42 = check");

  run(&cx,
      "lib: foo func: baz(_ Lambda)(_ Sym) call;; "
      "use: (foo baz); "
      "{cx-lib id} baz `lobby = check");

  cx_deinit(&cx);
}

static void type_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/type;");
  run(&cx, "42 type Int = check");
  run(&cx, "Int A is check");
  run(&cx, "A Int is !check");
  run(&cx, "trait: StrInt Str Int; 42 StrInt is check");
  cx_deinit(&cx);
}

static void group_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error;");
  run(&cx, "(7 14 21) 21 = check");

  cx_deinit(&cx);
}

static void if_tests() {
  struct cx cx;
  init_cx(&cx);
  
  run(&cx, "use: cx/abc cx/cond cx/error cx/sym;");
  run(&cx, "#t 42 if 42 = check");
  run(&cx, "#f 42 else 42 = check");
  run(&cx, "#t `yes `no if-else `yes = check");
  run(&cx, "#f `yes `no if-else `no = check");
  cx_deinit(&cx);
}


static void let_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/math cx/str cx/var;");
  run(&cx, "(let: foo 42; $foo 42 = check)");
  run(&cx, "(let: (x y z) 1 2 3 4 +; $x $y $z + + 10 = check)");
  run(&cx, "(let: (foo Int bar Str) 7 '35'; $foo $bar int + 42 = check)");

  run(&cx, "(`foo var !check "
           " `foo 42 let "
           " `foo var 42 = check)");

  cx_deinit(&cx);
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

static void iter_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/abc cx/cond cx/error cx/iter cx/math;");
  run(&cx, "0 5 &++ map &+ for 15 = check");
  
  cx_deinit(&cx);
}

static void int_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/iter cx/math cx/str;");
  run(&cx, "42 check");
  run(&cx, "0! check");
  run(&cx, "1 2 = !check");
  run(&cx, "42 str<Int> '42' = check");
  run(&cx, "0 5 &+ for 10 = check");
  
  cx_deinit(&cx);
}

static void io_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/io cx/io/buf cx/stack cx/str;");
  run(&cx, "Buf new % 'foo' print % flush str 'foo' = check");
  cx_deinit(&cx);
}

static void char_tests() {
  struct cx cx;
  init_cx(&cx);
  
  run(&cx, "use: cx/cond cx/error cx/iter cx/math cx/str;");
  run(&cx, "@a upper @A = check");
  run(&cx, "@0 int 7 + char @7 = check");
  
  cx_deinit(&cx);
}

static void str_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/stack cx/str;");
  run(&cx, "'foo' check");
  run(&cx, "''! check");
  run(&cx, "'foo' 'foo' = check");
  run(&cx, "'foo' 'bar' = !check");
  run(&cx, "'foo' 'foo' == !check");
  run(&cx, "'foo' % upper 'FOO' = check");
  run(&cx, "'foobar' 3 get @b = check");
  run(&cx, "'42' int 42 = check");
  run(&cx, "'@n' 0 get @@n = check");
  run(&cx, "['foo' 'bar' 'baz'] @/ join 'foo/bar/baz' = check");
  run(&cx, "'foo bar baz' {@@s =} split stack ['foo' 'bar' 'baz'] = check");
  run(&cx, "'foo bar baz' @@s split stack ['foo' 'bar' 'baz'] = check");
  
  cx_deinit(&cx);
}

static void sym_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/stack cx/sym cx/type;");
  run(&cx, "`foo `foo == check");
  run(&cx, "`foo `bar == !check");
  run(&cx, "'foo' sym `foo = check");
  run(&cx, "`foo str 'foo' = check");
  run(&cx, "Sym % new ~ new = !check");
    
  cx_deinit(&cx);
}

static void rat_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/math;");
  run(&cx, "1 2 / 5 2 / * 5 4 / = check");
  run(&cx, "1 2 / 5 2 / + 3 1 / = check");
  
  cx_deinit(&cx);
}

static void time_tests() {
  struct cx cx;
  init_cx(&cx);

  run(&cx, "use: cx/cond cx/error cx/time;");
  run(&cx, "now now <= check");
  run(&cx, "[1 0 0 24] time days 367 = check");
  run(&cx, "2m 120s = check");
  run(&cx, "1 days 1h + 2 * 50h = check");
  
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
  run(&cx, "[1 2 3] % 4 push<Stack A> len 4 = check");
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
  vec_tests();
  set_tests();
  
  comment_tests();
  error_tests();
  lib_tests();
  type_tests();
  group_tests();
  if_tests();
  let_tests();
  func_tests();
  iter_tests();
  int_tests();
  io_tests();
  char_tests();
  str_tests();
  sym_tests();
  rat_tests();
  time_tests();
  ref_tests();
  pair_tests();
  stack_tests();
  table_tests();
  math_tests();
  rec_tests();
  compile_tests();
  return 0;
}

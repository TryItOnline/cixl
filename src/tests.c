#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/cond.h"
#include "cixl/libs/func.h"
#include "cixl/libs/iter.h"
#include "cixl/libs/math.h"
#include "cixl/libs/pair.h"
#include "cixl/libs/rec.h"
#include "cixl/libs/ref.h"
#include "cixl/libs/stack.h"
#include "cixl/libs/str.h"
#include "cixl/libs/table.h"
#include "cixl/libs/time.h"
#include "cixl/libs/type.h"
#include "cixl/libs/var.h"
#include "cixl/libs/vect.h"
#include "cixl/set.h"
#include "cixl/scope.h"
#include "cixl/types/vect.h"
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
  
  if (!cx_eval_str(cx, in)) {
    printf("%s\n", in);
    
    cx_do_vec(&cx->errors, struct cx_error, e) {
      printf("Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
      cx_vect_dump(&e->stack, stdout);
      fputs("\n\n", stdout);
      cx_error_deinit(e);
    }
    
    cx_vec_clear(&cx->errors);
  }
}

static void comment_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  
  run(&cx, "1 //foo bar\n+ 2 = 3 check");
  run(&cx, "1 /*foo \n bar*/+ 2 = 3 check");

  cx_deinit(&cx);
}

static void type_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_type(&cx);

  run(&cx, "42 type Int = check");
  run(&cx, "Int is A check");
  run(&cx, "! (A is Int) check");

  cx_deinit(&cx);
}

static void stack_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_stack(&cx);

  run(&cx, "7 14 % + 28 = check");
  run(&cx, "7 14 % _ + 21 = check");

  cx_deinit(&cx);
}

static void group_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);

  run(&cx, "(7 14 21) 21 = check");

  cx_deinit(&cx);
}

static void if_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_iter(&cx);
  cx_init_str(&cx);
  
  run(&cx, "#t if 42 = 42 check");
  run(&cx, "#f else 42 = 42 check");
  run(&cx, "#t if-else `yes `no = `yes check");
  run(&cx, "#f if-else `yes `no = `no check");
  cx_deinit(&cx);
}

static void let_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_str(&cx);
  cx_init_var(&cx);

  run(&cx, "(let: foo 42; $foo 42 = check)");
  run(&cx, "(let: (x y z) 1 2, 3 + 4; $x + $y + $z 10 = check)");
  run(&cx, "(let: (foo Int bar Str) 7 '35'; $foo, int $bar + = 42 check)");

  run(&cx, "(get-var `foo !check "
           " put-var `foo 42 "
           " get-var `foo = 42 check)");

  cx_deinit(&cx);
}

static void func_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_var(&cx);
  
  run(&cx, "func: foo() (Int) 42; foo = 42 check");
  run(&cx, "(let: x 42; func: foo() (Int) $x;) &foo call = 42 check");
  run(&cx, "func: bar(x A) (Int) $x + 35; bar 7 42 = check");
  run(&cx, "func: baz(x y Int z T0) (Int) $x + $y + $z; baz 1 3 5 9 = check");

  run(&cx,
      "func: maybe-add(x y Num) (T0) $x + $y; "
      "func: maybe-add(x y Int) (Int) $x = 42 if-else 42 {upcall $x $y}; "
      "maybe-add 1 2 3 = check "
      "maybe-add 42 2 42 = check");

  run(&cx,
      "func: answer(0) (Int) 0; "
      "func: answer(x Int) (Int) $x; "
      "func: answer(42) (Sym) `correct; "
      "answer 0 0 = check "
      "answer 1 1 = check "
      "answer 42 `correct = check");

  cx_deinit(&cx);
}

static void iter_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);

  run(&cx, "0, 5 map &++, $ for &+ check");
  
  cx_deinit(&cx);
}

static void int_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_str(&cx);
  
  run(&cx, "42 check");
  run(&cx, "0! check");
  run(&cx, "1 = 2! check");
  run(&cx, "42 str<Int> '42' = check");
  run(&cx, "0, 5 for &+ = 10 check");
  
  cx_deinit(&cx);
}

static void char_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_str(&cx);
  
  run(&cx, "\\a upper \\A = check");
  run(&cx, "\\0 int + 7 char \\7 = check");
  
  cx_deinit(&cx);
}

static void str_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_iter(&cx);
  cx_init_str(&cx);
  
  run(&cx, "'foo' check");
  run(&cx, "''! check");
  run(&cx, "'foo' = 'foo' check");
  run(&cx, "'foo' = 'bar' ! check");
  run(&cx, "'foo' == 'foo' ! check");
  run(&cx, "'foo' upper = 'FOO' check");
  run(&cx, "'foobar' 3 get \\b = check");
  run(&cx, "'42' int 42 = check");
  
  cx_deinit(&cx);
}

static void sym_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);

  run(&cx, "`foo = `foo check");
  run(&cx, "`foo = `bar !check");
  run(&cx, "'foo' sym = `foo check");
  run(&cx, "`foo str = 'foo' check");
  run(&cx, "new Sym, new Sym = !check");
    
  cx_deinit(&cx);
}

static void rat_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);

  run(&cx, "1 / 2, 5 / 2 *, 5 / 4 = check");
  run(&cx, "1 / 2, 5 / 2 +, 3 / 1 = check");
  
  cx_deinit(&cx);
}

static void time_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_time(&cx);

  run(&cx, "now <= now check");
  run(&cx, "[1 0 0 24] time days = 367 check");
  run(&cx, "2m 120s = check");
  run(&cx, "1 days 1h + 2 * 50h = check");
  
  cx_deinit(&cx);
}

static void guid_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_stack(&cx);

  run(&cx, "new Guid, new Guid = !check");
  run(&cx, "new Guid % str guid = check");
  
  cx_deinit(&cx);
}

static void ref_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_ref(&cx);
  cx_init_stack(&cx);

  run(&cx, "#nil ref %, $ put-ref 42 get-ref = 42 check");
  
  cx_deinit(&cx);
}

static void vect_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_stack(&cx);
  cx_init_vect(&cx);

  run(&cx, "1 2 3 ([4 5]) len 2 = check");
  run(&cx, "[1 2 3] pop 3 = check");
  run(&cx, "[1 2 3] % 4 push<Vect A> len 4 = check");
  run(&cx, "[1 2] for {2 *} + 6 = check");
  run(&cx, "[1 2] < ([3 4]) check");
  run(&cx, "[1 2 3] > ([1 2]) check");
  run(&cx, "[3 2 1] %, $ sort #nil for {} + - = -4 check");
  run(&cx, "[1 2 3] %, $ sort {~ <=>} for {} + - = 0 check");
  
  cx_deinit(&cx);
}

static void table_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_iter(&cx);
  cx_init_pair(&cx);
  cx_init_table(&cx);
  cx_init_var(&cx);
  cx_init_vect(&cx);

  run(&cx, "(let: t new Table;"
           " $t put 1 'foo'"
           " $t put 2 'bar'"
           " $t put 1 'baz'"
           " $t get 1 = 'baz' check"
           " $t len = 2 check"
           " $t delete 2"
           " $t len = 1 check)");

  run(&cx, "([(1.'foo') (2.'bar')] table vect len = 2 check");
  
  cx_deinit(&cx);
}

static void math_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);

  run(&cx, "21 +<Int Int> 21 = 42 check");
  run(&cx, "7 + 14, 7 + 14 + = 42 check");
  run(&cx, "fib 50 = 12586269025 check");

  cx_deinit(&cx);
}

static void rec_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_rec(&cx);
  cx_init_stack(&cx);
  cx_init_var(&cx);

  run(&cx,
      "rec: Foo() x Int y Str; "
      "(let: foo new Foo; "
      " $foo !check "
      " $foo put `x 42 "
      " $foo check "
      " $foo get `x = 42 check "
      " $foo get `y = #nil check)");

  run(&cx,
      "let: (bar baz) new Foo %%; "
      "$bar put `x 42 "
      "$baz put `x 42 "
      "$bar = $baz check "
      "$bar put `y 'bar' "
      "$bar = $baz !check "
      "$baz put `y 'baz' "
      "$bar = $baz !check");
  
  run(&cx,
      "func: =(a b Foo) (Bool) $a get `x, $b get `x =; "
      "$bar = $baz check");
  
  cx_deinit(&cx);
}

static void compile_tests() {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_iter(&cx);
  cx_init_math(&cx);
  cx_init_stack(&cx);

  run(&cx, "new Bin %, $ compile '1 + 2' call = 3 check");
  cx_deinit(&cx);
}

int main() {
  vec_tests();
  set_tests();
  
  comment_tests();
  type_tests();
  stack_tests();
  group_tests();
  if_tests();
  let_tests();
  func_tests();
  iter_tests();
  int_tests();
  char_tests();
  str_tests();
  sym_tests();
  rat_tests();
  time_tests();
  guid_tests();
  ref_tests();
  vect_tests();
  table_tests();
  math_tests();
  rec_tests();
  compile_tests();
  return 0;
}

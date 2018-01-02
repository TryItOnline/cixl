#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/math.h"

void cx_init_math(struct cx *cx) {
  cx_test(cx_eval_str(cx,
		      "func: fib-rec(a b n Int) "
		      "$n? if {$b $a $b + $n -- recall} $a;"));

  cx_test(cx_eval_str(cx,
		      "func: fib(n Int) "
		      "0 1 $n fib-rec;"));
}

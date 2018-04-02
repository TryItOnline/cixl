#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/mfile.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/lambda.h"
#include "cixl/repl.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/stack.h"

static bool emit_bmips(struct cx *cx) {
bool _eval(struct cx *cx, ssize_t stop_pc) {
  static bool init = true;

  static struct cx_sym sym_n;
  static struct cx_sym sym_b;
  static struct cx_sym sym_a;

  struct cx_lib *lib_cxEmath() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/math", false)); }
    return l;
  }

  struct cx_type *type_Int() {
    static struct cx_type *t = NULL;
    if (!t) { t = cx_test(cx_get_type(cx, "Int", false)); }
    return t;
  }

  struct cx_func *func_int() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "int", false)); }
    return f;
  }

  struct cx_func *func_M() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "?", false)); }
    return f;
  }

  struct cx_func *func_ifNelse() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "if-else", false)); }
    return f;
  }

  struct cx_func *func_recall() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "recall", false)); }
    return f;
  }

  struct cx_func *func_times() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "times", false)); }
    return f;
  }

  struct cx_func *func__() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "_", false)); }
    return f;
  }

  struct cx_func *func_NN() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "--", false)); }
    return f;
  }

  struct cx_func *func_A() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "+", false)); }
    return f;
  }

  struct cx_func *func_E() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "/", false)); }
    return f;
  }

  struct cx_func *func_fibNrec() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "fib-rec", false)); }
    return f;
  }

  struct cx_func *func_fib() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "fib", false)); }
    return f;
  }

  struct cx_func *func_clock() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "clock", false)); }
    return f;
  }

  struct cx_fimp *func_M_Opt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_M(), "Opt", false)); }
    return f;
  }

  struct cx_fimp *func_ifNelse_OptOptOpt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_ifNelse(), "Opt Opt Opt", false)); }
    return f;
  }

  struct cx_fimp *func_recall_() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_recall(), "", false)); }
    return f;
  }

  struct cx_fimp *func_times_IntA() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_times(), "Int A", false)); }
    return f;
  }

  struct cx_fimp *func___() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func__(), "", false)); }
    return f;
  }

  struct cx_fimp *func_NN_Int() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_NN(), "Int", false)); }
    return f;
  }

  struct cx_fimp *func_A_IntInt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_A(), "Int Int", false)); }
    return f;
  }

  struct cx_fimp *func_E_IntInt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_E(), "Int Int", false)); }
    return f;
  }

  struct cx_fimp *func_fibNrec_IntIntInt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_fibNrec(), "Int Int Int", false)); }
    return f;
  }

  struct cx_fimp *func_fib_Int() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_fib(), "Int", false)); }
    return f;
  }

  struct cx_fimp *func_clock_A() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_clock(), "A", false)); }
    return f;
  }


  if (init) {
    init = false;

    sym_n = cx_sym(cx, "n");
    sym_b = cx_sym(cx, "b");
    sym_a = cx_sym(cx, "a");

struct cx_fimp *imp81 = func_fib_Int();
imp81->bin = cx_bin_ref(cx->bin);
imp81->start_pc = 6;
imp81->nops = 23;
struct cx_fimp *imp82 = func_fibNrec_IntIntInt();
imp82->bin = cx_bin_ref(cx->bin);
imp82->start_pc = 12;
imp82->nops = 15;
  }

  static void *op_labels[36] = {
    &&op0, NULL, &&op2, NULL, &&op4, NULL, &&op6, &&op7, NULL, NULL, NULL, NULL, &&op12, &&op13, NULL, NULL, &&op16, &&op17, NULL, NULL, NULL, &&op21, NULL, &&op23, &&op24, NULL, &&op26, &&op27, &&op28, &&op29, &&op30, &&op31, &&op32, &&op33, &&op34, };

  goto *op_labels[cx->pc];

op0: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 0; cx->row = 1; cx->col = 11;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10000000000;
}

{ /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 1; cx->row = 1; cx->col = 12;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 2, 30);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op32;
}

op2: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 2; cx->row = 1; cx->col = 15;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10;
}

{ /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 3; cx->row = 1; cx->col = 16;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 4, 27);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op31;
}

op4: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 4; cx->row = 1; cx->col = 19;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
}

{ /* CX_TFUNC CX_OFIMP */
cx->pc = 5; cx->row = 1; cx->col = 20;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
goto op29;
}

op6: { /* CX_TFUNC CX_OBEGIN */
cx->pc = 6; cx->row = 1; cx->col = 20;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *parent = func_fib_Int()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
}

op7: { /* CX_TFUNC CX_OPUTARGS */
cx->pc = 7; cx->row = 1; cx->col = 20;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope
  *ds = cx_scope(cx, 0),
  *ss = ds->stack.count ? ds : cx_scope(cx, 1);

*cx_put_var(ds, sym_n) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
}

{ /* CX_TLITERAL CX_OPUSH */
cx->pc = 8; cx->row = 1; cx->col = 1;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
}

{ /* CX_TLITERAL CX_OPUSH */
cx->pc = 9; cx->row = 1; cx->col = 3;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
}

{ /* CX_TID CX_OGETVAR */
cx->pc = 10; cx->row = 1; cx->col = 4;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TFUNC CX_OFIMP */
cx->pc = 11; cx->row = 1; cx->col = 7;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
goto op27;
}

op12: { /* CX_TFUNC CX_OBEGIN */
cx->pc = 12; cx->row = 1; cx->col = 7;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *parent = func_fibNrec_IntIntInt()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
}

op13: { /* CX_TFUNC CX_OPUTARGS */
cx->pc = 13; cx->row = 1; cx->col = 7;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope
  *ds = cx_scope(cx, 0),
  *ss = ds->stack.count ? ds : cx_scope(cx, 1);

*cx_put_var(ds, sym_n) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
*cx_put_var(ds, sym_b) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
*cx_put_var(ds, sym_a) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
}

{ /* CX_TID CX_OGETVAR */
cx->pc = 14; cx->row = 1; cx->col = 0;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TFIMP CX_OFUNCALL */
cx->pc = 15; cx->row = 1; cx->col = 2;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_M();
struct cx_fimp *imp = func_M_Opt();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 16);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op16: { /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 16; cx->row = 1; cx->col = 4;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 17, 7);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op24;
}

op17: { /* CX_TID CX_OGETVAR */
cx->pc = 17; cx->row = 1; cx->col = 5;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OGETVAR */
cx->pc = 18; cx->row = 1; cx->col = 8;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OGETVAR */
cx->pc = 19; cx->row = 1; cx->col = 11;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TFIMP CX_OFUNCALL */
cx->pc = 20; cx->row = 1; cx->col = 14;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_A();
struct cx_fimp *imp = func_A_IntInt();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 21);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op21: { /* CX_TID CX_OGETVAR */
cx->pc = 21; cx->row = 1; cx->col = 16;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TFUNC CX_OFUNCALL */
cx->pc = 22; cx->row = 1; cx->col = 19;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_NN();
struct cx_fimp *imp = func_NN_Int();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 23);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op23: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 23; cx->row = 1; cx->col = 22;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_recall();
struct cx_fimp *imp = func_recall_();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 24);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op24: { /* CX_TID CX_OGETVAR */
cx->pc = 24; cx->row = 1; cx->col = 29;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
}

{ /* CX_TFUNC CX_OFUNCALL */
cx->pc = 25; cx->row = 1; cx->col = 32;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_ifNelse();
struct cx_fimp *imp = func_ifNelse_OptOptOpt();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 26);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op26: { /* CX_TFUNC CX_ORETURN */
cx->pc = 26; cx->row = 1; cx->col = 7;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(func_fibNrec_IntIntInt(), s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }

  call->recalls--;
  goto op13;
} else {
  cx_pop_lib(cx);
  size_t si = 0;
  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      return false;
    }

    struct cx_box *v = cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = type_Int();
      if (!cx_is(v->type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v->type->id);
        return false;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = *v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    return false;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  struct cx_call *call = cx_vec_pop(&cx->calls);

  if (call->return_pc > -1) {
    cx->pc = call->return_pc;
    cx_call_deinit(call);
    goto *op_labels[cx->pc];
  }

  cx_call_deinit(call);
}
}

op27: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 27; cx->row = 1; cx->col = 7;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_fibNrec();
struct cx_fimp *imp = func_fibNrec_IntIntInt();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 28);
  goto op12;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op28: { /* CX_TFUNC CX_ORETURN */
cx->pc = 28; cx->row = 1; cx->col = 20;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(func_fib_Int(), s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }

  call->recalls--;
  goto op7;
} else {
  cx_pop_lib(cx);
  size_t si = 0;
  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      return false;
    }

    struct cx_box *v = cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = type_Int();
      if (!cx_is(v->type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v->type->id);
        return false;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = *v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    return false;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  struct cx_call *call = cx_vec_pop(&cx->calls);

  if (call->return_pc > -1) {
    cx->pc = call->return_pc;
    cx_call_deinit(call);
    goto *op_labels[cx->pc];
  }

  cx_call_deinit(call);
}
}

op29: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 29; cx->row = 1; cx->col = 20;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_fib();
struct cx_fimp *imp = func_fib_Int();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 30);
  goto op6;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op30: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 30; cx->row = 1; cx->col = 24;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func__();
struct cx_fimp *imp = func___();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 31);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op31: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 31; cx->row = 1; cx->col = 26;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_times();
struct cx_fimp *imp = func_times_IntA();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 32);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op32: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 32; cx->row = 1; cx->col = 32;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_clock();
struct cx_fimp *imp = func_clock_A();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 33);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op33: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 33; cx->row = 1; cx->col = 38;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_E();
struct cx_fimp *imp = func_E_IntInt();

if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!imp->ptr && imp->bin == cx->bin) {
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 34);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
}

op34: { /* CX_TFUNC CX_OFUNCALL */
cx->pc = 34; cx->row = 1; cx->col = 40;
if (cx->pc == stop_pc) { return true; }
if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_func *func = func_int();
struct cx_fimp *imp = cx_func_match(func, s);

if (!imp) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
  return false;
}

if (!cx_fimp_call(imp, s)) { return false; }
}

return true;
}

  struct cx_bin *bin = cx_bin_new();
  bin->eval = _eval;
  bool ok = cx_eval(bin, 0, -1, cx);
  cx_bin_deref(bin);
  return ok;
}

void cx_repl(struct cx *cx, FILE *in, FILE *out) {
  cx_use(cx, "cx");
  fprintf(out, "Cixl v%s, ", CX_VERSION);

  cx_eval_str(cx, "10000000000 {10 {50 fib _} times} clock / int");
  emit_bmips(cx);
  cx_eval_str(cx,
	      "let: (bmips emit-bmips);"
	      "[$bmips @/ $emit-bmips ' bmips' @@n] say");

  fputs("Press Return twice to evaluate.\n\n", out);
  if (cx->errors.count) { cx_dump_errors(cx, out); }

  struct cx_mfile body;
  cx_mfile_open(&body);
  char line[CX_REPL_LINE_MAX];

  while (true) {
    fflush(body.stream);
    fputs(body.size ? "..." : "   ", out);
    
    if (fgets(line, sizeof(line), in) == NULL) { break; }

    if (strcmp(line, "\n") == 0) {
      cx_mfile_close(&body);
      cx_eval_str(cx, body.data);
      
      if (cx->errors.count) {
	cx_dump_errors(cx, out);
      } else {
	cx_stack_dump(&cx_scope(cx, 0)->stack, out);
	fputc('\n', out);
      }

      free(body.data);
      cx_mfile_open(&body);
    } else {
      if (strcmp(line, "quit\n") == 0) { break; }
      fputs(line, body.stream);
    }
  }

  cx_mfile_close(&body);
  free(body.data);
}

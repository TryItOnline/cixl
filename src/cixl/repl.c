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
bool _eval(struct cx *cx) {
  static bool init = true;

  static struct cx_sym sym_n;
  static struct cx_sym sym_b;
  static struct cx_sym sym_a;

  struct cx_lib *lib_cxEabc() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/abc", false)); }
    return l;
  }

  struct cx_lib *lib_cxEcond() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/cond", false)); }
    return l;
  }

  struct cx_lib *lib_cxEfunc() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/func", false)); }
    return l;
  }

  struct cx_lib *lib_cxEiter() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/iter", false)); }
    return l;
  }

  struct cx_lib *lib_cxEmath() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/math", false)); }
    return l;
  }

  struct cx_lib *lib_cxEstack() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/stack", false)); }
    return l;
  }

  struct cx_lib *lib_cxEtime() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/time", false)); }
    return l;
  }

  struct cx_type *type_Int() {
    static struct cx_type *t = NULL;
    if (!t) { t = cx_test(cx_get_type(lib_cxEabc(), "Int", false)); }
    return t;
  }

  struct cx_func *func_times() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEiter(), "times", false)); }
    return f;
  }

  struct cx_func *func__() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEstack(), "_", false)); }
    return f;
  }

  struct cx_func *func_int() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEcond(), "int", false)); }
    return f;
  }

  struct cx_func *func_M() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEcond(), "?", false)); }
    return f;
  }

  struct cx_func *func_ifNelse() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEcond(), "if-else", false)); }
    return f;
  }

  struct cx_func *func_recall() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEfunc(), "recall", false)); }
    return f;
  }

  struct cx_func *func_NN() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEmath(), "--", false)); }
    return f;
  }

  struct cx_func *func_A() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEmath(), "+", false)); }
    return f;
  }

  struct cx_func *func_E() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEmath(), "/", false)); }
    return f;
  }

  struct cx_func *func_fibNrec() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEmath(), "fib-rec", false)); }
    return f;
  }

  struct cx_func *func_fib() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEmath(), "fib", false)); }
    return f;
  }

  struct cx_func *func_clock() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(lib_cxEtime(), "clock", false)); }
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


{
  struct cx_fimp *imp = func_fib_Int();
  imp->bin = cx_bin_ref(cx->bin);
  imp->start_pc = 6;
  imp->nops = 24;
}


{
  struct cx_fimp *imp = func_fibNrec_IntIntInt();
  imp->bin = cx_bin_ref(cx->bin);
  imp->start_pc = 12;
  imp->nops = 16;
}

  }

  static void *op_labels[39] = {
    &&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7, &&op8, &&op9, &&op10, 
    &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17, &&op18, &&op19, &&op20, 
    &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27, &&op28, &&op29, &&op30, 
    &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37, &&op38};

  goto *op_labels[cx->pc];

 op0: { /* CX_TLITERAL CX_OPUSH */
    cx->pc = 0; cx->row = 1; cx->col = 11;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10000000000;
 }

 op1: { /* CX_TLAMBDA CX_OLAMBDA */
    cx->pc = 1; cx->row = 1; cx->col = 12;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 2, 33);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op35;
 }

 op2: { /* CX_TLITERAL CX_OPUSH */
    cx->pc = 2; cx->row = 1; cx->col = 14;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10;
 }

 op3: { /* CX_TLAMBDA CX_OLAMBDA */
    cx->pc = 3; cx->row = 1; cx->col = 15;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 4, 29);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op33;
 }

 op4: { /* CX_TLITERAL CX_OPUSH */
    cx->pc = 4; cx->row = 1; cx->col = 17;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
 }

 op5: { /* CX_TFUNC CX_OFIMP */
    cx->pc = 5; cx->row = 1; cx->col = 18;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
goto op30;
 }

 op6: { /* CX_TFUNC CX_OBEGIN */
    cx->pc = 6; cx->row = 1; cx->col = 18;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *parent = func_fib_Int()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
 }

 op7: { /* CX_TFUNC CX_OPUTARGS */
    cx->pc = 7; cx->row = 1; cx->col = 18;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope
  *ds = cx_scope(cx, 0),
  *ss = ds->stack.count ? ds : cx_scope(cx, 1);

*cx_put_var(ds, sym_n, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
 }

 op8: { /* CX_TLITERAL CX_OPUSH */
    cx->pc = 8; cx->row = 1; cx->col = 1;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
 }

 op9: { /* CX_TLITERAL CX_OPUSH */
    cx->pc = 9; cx->row = 1; cx->col = 3;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
 }

 op10: { /* CX_TID CX_OGETVAR */
    cx->pc = 10; cx->row = 1; cx->col = 4;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op11: { /* CX_TFUNC CX_OFIMP */
    cx->pc = 11; cx->row = 1; cx->col = 7;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
goto op28;
 }

 op12: { /* CX_TFUNC CX_OBEGIN */
    cx->pc = 12; cx->row = 1; cx->col = 7;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *parent = func_fibNrec_IntIntInt()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
 }

 op13: { /* CX_TFUNC CX_OPUTARGS */
    cx->pc = 13; cx->row = 1; cx->col = 7;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope
  *ds = cx_scope(cx, 0),
  *ss = ds->stack.count ? ds : cx_scope(cx, 1);

*cx_put_var(ds, sym_n, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
*cx_put_var(ds, sym_b, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
*cx_put_var(ds, sym_a, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
 }

 op14: { /* CX_TID CX_OGETVAR */
    cx->pc = 14; cx->row = 1; cx->col = 0;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op15: { /* CX_TFIMP CX_OFUNCALL */
    cx->pc = 15; cx->row = 1; cx->col = 2;
    if (cx->stop) { return true; }
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
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 17, 8);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op25;
 }

 op17: { /* CX_TID CX_OGETVAR */
    cx->pc = 17; cx->row = 1; cx->col = 4;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op18: { /* CX_TID CX_OGETVAR */
    cx->pc = 18; cx->row = 1; cx->col = 7;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op19: { /* CX_TID CX_OGETVAR */
    cx->pc = 19; cx->row = 1; cx->col = 10;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op20: { /* CX_TFIMP CX_OFUNCALL */
    cx->pc = 20; cx->row = 1; cx->col = 13;
    if (cx->stop) { return true; }
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
    cx->pc = 21; cx->row = 1; cx->col = 15;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op22: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 22; cx->row = 1; cx->col = 18;
    if (cx->stop) { return true; }
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
    cx->pc = 23; cx->row = 1; cx->col = 21;
    if (cx->stop) { return true; }
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

 op24: { /* CX_TLAMBDA CX_OSTOP */
    cx->pc = 24; cx->row = 1; cx->col = 4;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx->stop = true;
 }

 op25: { /* CX_TID CX_OGETVAR */
    cx->pc = 25; cx->row = 1; cx->col = 28;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { return false; }
cx_copy(cx_push(s), v);
 }

 op26: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 26; cx->row = 1; cx->col = 31;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 27);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op27: { /* CX_TFUNC CX_ORETURN */
    cx->pc = 27; cx->row = 1; cx->col = 21;
    if (cx->stop) { return true; }
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
  cx->stop = true;
}
 }

 op28: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 28; cx->row = 1; cx->col = 7;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 29);
  goto op12;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op29: { /* CX_TFUNC CX_ORETURN */
    cx->pc = 29; cx->row = 1; cx->col = 21;
    if (cx->stop) { return true; }
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
  cx->stop = true;
}
 }

 op30: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 30; cx->row = 1; cx->col = 18;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 31);
  goto op6;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op31: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 31; cx->row = 1; cx->col = 22;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 32);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op32: { /* CX_TLAMBDA CX_OSTOP */
    cx->pc = 32; cx->row = 1; cx->col = 15;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx->stop = true;
 }

 op33: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 33; cx->row = 1; cx->col = 24;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 34);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op34: { /* CX_TLAMBDA CX_OSTOP */
    cx->pc = 34; cx->row = 1; cx->col = 12;
    if (cx->stop) { return true; }
    if (cx->errors.count) { return false; }
cx->stop = true;
 }

 op35: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 35; cx->row = 1; cx->col = 30;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 36);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op36: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 36; cx->row = 1; cx->col = 36;
    if (cx->stop) { return true; }
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
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 37);
  goto op0;
} else if (!cx_fimp_call(imp, s)) { return false; }
 }

 op37: { /* CX_TFUNC CX_OFUNCALL */
    cx->pc = 37; cx->row = 1; cx->col = 38;
    if (cx->stop) { return true; }
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

 op38:
  cx->stop = false;
  return true;
}

  struct cx_bin *bin = cx_bin_new();
  bin->eval = _eval;
  bool ok = cx_eval(bin, 0, cx);
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

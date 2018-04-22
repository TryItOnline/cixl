#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
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

static bool init_emit_bmips(struct cx *cx) {
bool _eval(struct cx *cx, ssize_t stop_pc) {
  static bool init = true;

  static struct cx_sym sym_b;
  static struct cx_sym sym_n;
  static struct cx_sym sym_a;

  struct cx_lib *lib_cxEbin() {
    static struct cx_lib *l = NULL;
    if (!l) { l = cx_test(cx_get_lib(cx, "cx/bin", false)); }
    return l;
  }

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

  struct cx_func *func_A() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "+", false)); }
    return f;
  }

  struct cx_func *func_NN() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "--", false)); }
    return f;
  }

  struct cx_func *func_E() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "/", false)); }
    return f;
  }

  struct cx_func *func_M() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "?", false)); }
    return f;
  }

  struct cx_func *func__() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "_", false)); }
    return f;
  }

  struct cx_func *func_clock() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "clock", false)); }
    return f;
  }

  struct cx_func *func_emitNbmips() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "emit-bmips", false)); }
    return f;
  }

  struct cx_func *func_fib() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "fib", false)); }
    return f;
  }

  struct cx_func *func_fibNrec() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "fib-rec", false)); }
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

  struct cx_fimp *func_M_Opt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_M(), "Opt", false)); }
    return f;
  }

  struct cx_fimp *func_fib_Int() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_fib(), "Int", false)); }
    return f;
  }

  struct cx_fimp *func_A_IntInt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_A(), "Int Int", false)); }
    return f;
  }

  struct cx_fimp *func_fibNrec_IntIntInt() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_fibNrec(), "Int Int Int", false)); }
    return f;
  }

  struct cx_fimp *func_emitNbmips_() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_emitNbmips(), "", false)); }
    return f;
  }


  if (init) {
    init = false;

    sym_b = cx_sym(cx, "b");
    sym_n = cx_sym(cx, "n");
    sym_a = cx_sym(cx, "a");

struct cx_arg args97[0] = {
};

struct cx_arg rets98[1] = {
cx_arg(NULL, cx_get_type(cx, "Int", false))};

struct cx_fimp *imp99 = cx_test(cx_add_func(*cx->lib, "emit-bmips", 0, args97, 1, rets98));
struct cx_bin_fimp *bimp100 = cx_test(cx_set_insert(&cx->bin->fimps, &imp99));
imp99->bin = cx_bin_ref(cx->bin);
bimp100->imp = imp99;
bimp100->start_pc = 2;
bimp100->nops = 36;
cx_push_lib(cx, lib_cxEmath());
struct cx_fimp *imp101 = func_fib_Int();
cx_pop_lib(cx);
struct cx_bin_fimp *bimp102 = cx_test(cx_set_insert(&cx->bin->fimps, &imp101));
bimp102->imp = imp101;
bimp102->start_pc = 9;
bimp102->nops = 23;
cx_push_lib(cx, lib_cxEmath());
struct cx_fimp *imp103 = func_fibNrec_IntIntInt();
cx_pop_lib(cx);
struct cx_bin_fimp *bimp104 = cx_test(cx_set_insert(&cx->bin->fimps, &imp103));
bimp104->imp = imp103;
bimp104->start_pc = 15;
bimp104->nops = 15;
  }

  static void *op_labels[39] = {
    &&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7, &&op8, &&op9, &&op10, &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17, &&op18, &&op19, &&op20, &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27, &&op28, &&op29, &&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37, &&op38};

  goto *op_labels[cx->pc];

op0: { /* CX_TMACRO CX_OFUNCDEF */
cx->pc = 0;
if (stop_pc == 0) { goto exit; }
cx->row = 1; cx->col = 5;
struct cx_fimp *i = func_emitNbmips_();
if (!i->scope) { i->scope = cx_scope_ref(cx_scope(cx, 0)); }
}

op1: { /* CX_TMACRO CX_OFIMP */
cx->pc = 1;
if (stop_pc == 1) { goto exit; }
cx->row = 1; cx->col = 5;
goto op38;
}

op2: { /* CX_TMACRO CX_OBEGIN */
cx->pc = 2;
if (stop_pc == 2) { goto exit; }
cx->row = 1; cx->col = 5;
if (cx->errors.count) {
goto op38;
} else {
struct cx_scope *parent = func_emitNbmips_()->scope;
cx_begin(cx, parent);
cx_push_lib(cx, lib_cxEbin());
struct cx_call *call = cx_test(cx_peek_call(cx));
cx_scope_deref(call->scope);
call->scope = cx_scope_ref(cx_scope(cx, 0));
}
}

op3: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 3;
if (stop_pc == 3) { goto exit; }
cx->row = 1; cx->col = 39;
if (!cx->errors.count) {
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10000000000;
}
}

op4: { /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 4;
if (stop_pc == 4) { goto exit; }
cx->row = 1; cx->col = 41;
if (cx->errors.count) {
goto op35;
} else {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 5, 30);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op35;
}
}

op5: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 5;
if (stop_pc == 5) { goto exit; }
cx->row = 1; cx->col = 44;
if (!cx->errors.count) {
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10;
}
}

op6: { /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 6;
if (stop_pc == 6) { goto exit; }
cx->row = 1; cx->col = 46;
if (cx->errors.count) {
goto op34;
} else {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 7, 27);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op34;
}
}

op7: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 7;
if (stop_pc == 7) { goto exit; }
cx->row = 1; cx->col = 49;
if (!cx->errors.count) {
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
}
}

op8: { /* CX_TID CX_OFIMP */
cx->pc = 8;
if (stop_pc == 8) { goto exit; }
cx->row = 1; cx->col = 50;
goto op32;
}

op9: { /* CX_TID CX_OBEGIN */
cx->pc = 9;
if (stop_pc == 9) { goto exit; }
cx->row = 1; cx->col = 50;
if (cx->errors.count) {
goto op32;
} else {
struct cx_scope *parent = func_fib_Int()->scope;
cx_begin(cx, parent);
cx_push_lib(cx, lib_cxEmath());
struct cx_call *call = cx_test(cx_peek_call(cx));
cx_scope_deref(call->scope);
call->scope = cx_scope_ref(cx_scope(cx, 0));
}
}

op10: { /* CX_TID CX_OPUTARGS */
cx->pc = 10;
if (stop_pc == 10) { goto exit; }
cx->row = 1; cx->col = 50;
if (!cx->errors.count) {
struct cx_call *call = cx_test(cx_peek_call(cx));
struct cx_box *a = call->args;
struct cx_scope *s = cx_scope(cx, 0);
cx_copy(cx_put_var(s, sym_n), a);
a++;
}
}

op11: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 11;
if (stop_pc == 11) { goto exit; }
cx->row = 1; cx->col = 1;
if (!cx->errors.count) {
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
}
}

op12: { /* CX_TLITERAL CX_OPUSH */
cx->pc = 12;
if (stop_pc == 12) { goto exit; }
cx->row = 1; cx->col = 3;
if (!cx->errors.count) {
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
}
}

op13: { /* CX_TID CX_OGETVAR */
cx->pc = 13;
if (stop_pc == 13) { goto exit; }
cx->row = 1; cx->col = 4;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto op14; }
cx_copy(cx_push(s), v);
}
}

op14: { /* CX_TID CX_OFIMP */
cx->pc = 14;
if (stop_pc == 14) { goto exit; }
cx->row = 1; cx->col = 7;
goto op30;
}

op15: { /* CX_TID CX_OBEGIN */
cx->pc = 15;
if (stop_pc == 15) { goto exit; }
cx->row = 1; cx->col = 7;
if (cx->errors.count) {
goto op30;
} else {
struct cx_scope *parent = func_fibNrec_IntIntInt()->scope;
cx_begin(cx, parent);
cx_push_lib(cx, lib_cxEmath());
struct cx_call *call = cx_test(cx_peek_call(cx));
cx_scope_deref(call->scope);
call->scope = cx_scope_ref(cx_scope(cx, 0));
}
}

op16: { /* CX_TID CX_OPUTARGS */
cx->pc = 16;
if (stop_pc == 16) { goto exit; }
cx->row = 1; cx->col = 7;
if (!cx->errors.count) {
struct cx_call *call = cx_test(cx_peek_call(cx));
struct cx_box *a = call->args;
struct cx_scope *s = cx_scope(cx, 0);
cx_copy(cx_put_var(s, sym_a), a);
a++;
cx_copy(cx_put_var(s, sym_b), a);
a++;
cx_copy(cx_put_var(s, sym_n), a);
a++;
}
}

op17: { /* CX_TID CX_OGETVAR */
cx->pc = 17;
if (stop_pc == 17) { goto exit; }
cx->row = 1; cx->col = 0;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto op18; }
cx_copy(cx_push(s), v);
}
}

op18: { /* CX_TID CX_OFUNCALL */
cx->pc = 18;
if (stop_pc == 18) { goto exit; }
cx->row = 1; cx->col = 2;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp105 = NULL;
if (!imp105) { imp105 = func_M_Opt(); }
if (imp105 && s->safe && !cx_fimp_match(imp105, s)) { imp105 = NULL; }
if (!imp105) { imp105 = cx_func_match(func_M(), s); }

if (!imp105) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: ?");
  goto op19;
}

if (!cx_fimp_call(imp105, s)) { goto op19; }
}
}

op19: { /* CX_TLAMBDA CX_OLAMBDA */
cx->pc = 19;
if (stop_pc == 19) { goto exit; }
cx->row = 1; cx->col = 10;
if (cx->errors.count) {
goto op27;
} else {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 20, 7);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op27;
}
}

op20: { /* CX_TID CX_OGETVAR */
cx->pc = 20;
if (stop_pc == 20) { goto exit; }
cx->row = 1; cx->col = 11;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { goto op21; }
cx_copy(cx_push(s), v);
}
}

op21: { /* CX_TID CX_OGETVAR */
cx->pc = 21;
if (stop_pc == 21) { goto exit; }
cx->row = 1; cx->col = 14;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { goto op22; }
cx_copy(cx_push(s), v);
}
}

op22: { /* CX_TID CX_OGETVAR */
cx->pc = 22;
if (stop_pc == 22) { goto exit; }
cx->row = 1; cx->col = 17;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { goto op23; }
cx_copy(cx_push(s), v);
}
}

op23: { /* CX_TID CX_OFUNCALL */
cx->pc = 23;
if (stop_pc == 23) { goto exit; }
cx->row = 1; cx->col = 20;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp106 = NULL;
if (!imp106) { imp106 = func_A_IntInt(); }
if (imp106 && s->safe && !cx_fimp_match(imp106, s)) { imp106 = NULL; }
if (!imp106) { imp106 = cx_func_match(func_A(), s); }

if (!imp106) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: +");
  goto op24;
}

if (!cx_fimp_call(imp106, s)) { goto op24; }
}
}

op24: { /* CX_TID CX_OGETVAR */
cx->pc = 24;
if (stop_pc == 24) { goto exit; }
cx->row = 1; cx->col = 31;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto op25; }
cx_copy(cx_push(s), v);
}
}

op25: { /* CX_TID CX_OFUNCALL */
cx->pc = 25;
if (stop_pc == 25) { goto exit; }
cx->row = 1; cx->col = 34;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp107 = NULL;
if (imp107 && s->safe && !cx_fimp_match(imp107, s)) { imp107 = NULL; }
if (!imp107) { imp107 = cx_func_match(func_NN(), s); }

if (!imp107) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: --");
  goto op26;
}

if (!cx_fimp_call(imp107, s)) { goto op26; }
}
}

op26: { /* CX_TID CX_OFUNCALL */
cx->pc = 26;
if (stop_pc == 26) { goto exit; }
cx->row = 1; cx->col = 37;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp108 = NULL;
if (imp108 && s->safe && !cx_fimp_match(imp108, s)) { imp108 = NULL; }
if (!imp108) { imp108 = cx_func_match(func_recall(), s); }

if (!imp108) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: recall");
  goto op27;
}

if (!cx_fimp_call(imp108, s)) { goto op27; }
}
}

op27: { /* CX_TID CX_OGETVAR */
cx->pc = 27;
if (stop_pc == 27) { goto exit; }
cx->row = 1; cx->col = 45;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { goto op28; }
cx_copy(cx_push(s), v);
}
}

op28: { /* CX_TID CX_OFUNCALL */
cx->pc = 28;
if (stop_pc == 28) { goto exit; }
cx->row = 1; cx->col = 48;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp109 = NULL;
if (imp109 && s->safe && !cx_fimp_match(imp109, s)) { imp109 = NULL; }
if (!imp109) { imp109 = cx_func_match(func_ifNelse(), s); }

if (!imp109) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: if-else");
  goto op29;
}

if (!cx_fimp_call(imp109, s)) { goto op29; }
}
}

op29: { /* CX_TID CX_ORETURN */
cx->pc = 29;
if (stop_pc == 29) { goto exit; }
cx->row = 1; cx->col = 7;
if (cx->errors.count) {
cx_end(cx);
cx_pop_lib(cx);
if (!cx_pop_call(cx)) { goto op30; }
} else {
struct cx_fimp *imp = func_fibNrec_IntIntInt();
struct cx_call *call = cx_test(cx_peek_call(cx));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto op30;
  }

  call->recalls--;
  cx_call_deinit_args(call);
  cx_call_pop_args(call);
  goto op16;
} else {
  size_t si = 0;
struct cx_type *get_imp_arg(int i) {
  return (i < imp->args.count)
    ? ((struct cx_arg *)cx_vec_get(&imp->args, i))->type
    : NULL;
}

struct cx_type *get_arg(int i) {
  struct cx_box *v = cx_call_arg(call, i);
  return v ? v->type : NULL;
}

  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto op30;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = cx_resolve_arg_refs(type_Int(), get_imp_arg, get_arg);
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto op30;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto op30;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  if (!cx_pop_call(cx)) { goto op30; }
}
}
}

op30: { /* CX_TID CX_OFUNCALL */
cx->pc = 30;
if (stop_pc == 30) { goto exit; }
cx->row = 1; cx->col = 7;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp110 = NULL;
if (imp110 && s->safe && !cx_fimp_match(imp110, s)) { imp110 = NULL; }
if (!imp110) { imp110 = cx_func_match(func_fibNrec(), s); }

if (!imp110) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: fib-rec");
  goto op31;
}

if (!cx_fimp_call(imp110, s)) { goto op31; }
}
}

op31: { /* CX_TID CX_ORETURN */
cx->pc = 31;
if (stop_pc == 31) { goto exit; }
cx->row = 1; cx->col = 50;
if (cx->errors.count) {
cx_end(cx);
cx_pop_lib(cx);
if (!cx_pop_call(cx)) { goto op32; }
} else {
struct cx_fimp *imp = func_fib_Int();
struct cx_call *call = cx_test(cx_peek_call(cx));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto op32;
  }

  call->recalls--;
  cx_call_deinit_args(call);
  cx_call_pop_args(call);
  goto op10;
} else {
  size_t si = 0;
struct cx_type *get_imp_arg(int i) {
  return (i < imp->args.count)
    ? ((struct cx_arg *)cx_vec_get(&imp->args, i))->type
    : NULL;
}

struct cx_type *get_arg(int i) {
  struct cx_box *v = cx_call_arg(call, i);
  return v ? v->type : NULL;
}

  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto op32;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = cx_resolve_arg_refs(type_Int(), get_imp_arg, get_arg);
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto op32;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto op32;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  if (!cx_pop_call(cx)) { goto op32; }
}
}
}

op32: { /* CX_TID CX_OFUNCALL */
cx->pc = 32;
if (stop_pc == 32) { goto exit; }
cx->row = 1; cx->col = 50;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp111 = NULL;
if (imp111 && s->safe && !cx_fimp_match(imp111, s)) { imp111 = NULL; }
if (!imp111) { imp111 = cx_func_match(func_fib(), s); }

if (!imp111) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: fib");
  goto op33;
}

if (!cx_fimp_call(imp111, s)) { goto op33; }
}
}

op33: { /* CX_TID CX_OFUNCALL */
cx->pc = 33;
if (stop_pc == 33) { goto exit; }
cx->row = 1; cx->col = 54;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp112 = NULL;
if (imp112 && s->safe && !cx_fimp_match(imp112, s)) { imp112 = NULL; }
if (!imp112) { imp112 = cx_func_match(func__(), s); }

if (!imp112) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: _");
  goto op34;
}

if (!cx_fimp_call(imp112, s)) { goto op34; }
}
}

op34: { /* CX_TID CX_OFUNCALL */
cx->pc = 34;
if (stop_pc == 34) { goto exit; }
cx->row = 1; cx->col = 57;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp113 = NULL;
if (imp113 && s->safe && !cx_fimp_match(imp113, s)) { imp113 = NULL; }
if (!imp113) { imp113 = cx_func_match(func_times(), s); }

if (!imp113) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: times");
  goto op35;
}

if (!cx_fimp_call(imp113, s)) { goto op35; }
}
}

op35: { /* CX_TID CX_OFUNCALL */
cx->pc = 35;
if (stop_pc == 35) { goto exit; }
cx->row = 1; cx->col = 64;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp114 = NULL;
if (imp114 && s->safe && !cx_fimp_match(imp114, s)) { imp114 = NULL; }
if (!imp114) { imp114 = cx_func_match(func_clock(), s); }

if (!imp114) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: clock");
  goto op36;
}

if (!cx_fimp_call(imp114, s)) { goto op36; }
}
}

op36: { /* CX_TID CX_OFUNCALL */
cx->pc = 36;
if (stop_pc == 36) { goto exit; }
cx->row = 1; cx->col = 70;
if (!cx->errors.count) {
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp115 = NULL;
if (imp115 && s->safe && !cx_fimp_match(imp115, s)) { imp115 = NULL; }
if (!imp115) { imp115 = cx_func_match(func_E(), s); }

if (!imp115) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: /");
  goto op37;
}

if (!cx_fimp_call(imp115, s)) { goto op37; }
}
}

op37: { /* CX_TMACRO CX_ORETURN */
cx->pc = 37;
if (stop_pc == 37) { goto exit; }
cx->row = 1; cx->col = 5;
if (cx->errors.count) {
cx_end(cx);
cx_pop_lib(cx);
if (!cx_pop_call(cx)) { goto op38; }
} else {
struct cx_fimp *imp = func_emitNbmips_();
struct cx_call *call = cx_test(cx_peek_call(cx));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto op38;
  }

  call->recalls--;
  cx_call_deinit_args(call);
  cx_call_pop_args(call);
  goto op3;
} else {
  size_t si = 0;
struct cx_type *get_imp_arg(int i) {
  return (i < imp->args.count)
    ? ((struct cx_arg *)cx_vec_get(&imp->args, i))->type
    : NULL;
}

struct cx_type *get_arg(int i) {
  struct cx_box *v = cx_call_arg(call, i);
  return v ? v->type : NULL;
}

  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto op38;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = cx_resolve_arg_refs(type_Int(), get_imp_arg, get_arg);
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto op38;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto op38;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  if (!cx_pop_call(cx)) { goto op38; }
}
}
}

 op38:
exit:
  return !cx->errors.count;
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
  init_emit_bmips(cx);

  cx_eval_str(cx,
	      "func: bmips()(_ Int)\n"
	      "  10000000000 {10 {50 fib _} times} clock /;\n"
	      "[bmips @/ emit-bmips ' bmips' @@n] say");

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

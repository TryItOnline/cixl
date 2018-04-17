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
  bool ok = false;

  static struct cx_sym sym_n;
  static struct cx_sym sym_b;
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

  struct cx_func *func_int() {
    static struct cx_func *f = NULL;
    if (!f) { f = cx_test(cx_get_func(cx, "int", false)); }
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

  struct cx_fimp *func_fib_Int() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_fib(), "Int", false)); }
    return f;
  }

  struct cx_fimp *func_emitNbmips_() {
    static struct cx_fimp *f = NULL;
    if (!f) { f = cx_test(cx_get_fimp(func_emitNbmips(), "", false)); }
    return f;
  }


  if (init) {
    init = false;

    sym_n = cx_sym(cx, "n");
    sym_b = cx_sym(cx, "b");
    sym_a = cx_sym(cx, "a");

struct cx_arg args86[0] = {
};

struct cx_arg rets87[1] = {
cx_arg(NULL, cx_get_type(cx, "Int", false))};

struct cx_fimp *imp88 = cx_add_func(*cx->lib, "emit-bmips", 0, args86, 1, rets87);
struct cx_bin_fimp *bimp89 = cx_test(cx_set_insert(&cx->bin->fimps, &imp88));
imp88->bin = cx_bin_ref(cx->bin);
bimp89->imp = imp88;
bimp89->start_pc = 2;
bimp89->nops = 37;
cx_push_lib(cx, lib_cxEmath());
struct cx_fimp *imp90 = func_fib_Int();
cx_pop_lib(cx);
struct cx_bin_fimp *bimp91 = cx_test(cx_set_insert(&cx->bin->fimps, &imp90));
bimp91->imp = imp90;
bimp91->start_pc = 9;
bimp91->nops = 23;
cx_push_lib(cx, lib_cxEmath());
struct cx_fimp *imp92 = func_fibNrec_IntIntInt();
cx_pop_lib(cx);
struct cx_bin_fimp *bimp93 = cx_test(cx_set_insert(&cx->bin->fimps, &imp92));
bimp93->imp = imp92;
bimp93->start_pc = 15;
bimp93->nops = 15;
  }

  static void *op_labels[40] = {
    &&op0, NULL, &&op2, &&op3, NULL, &&op5, NULL, &&op7, NULL, &&op9, &&op10, NULL, NULL, NULL, NULL, &&op15, &&op16, NULL, NULL, &&op19, &&op20, NULL, NULL, NULL, &&op24, NULL, &&op26, &&op27, NULL, &&op29, &&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37, &&op38, &&op39};

  goto *op_labels[cx->pc];

op0: { /* CX_TMACRO CX_OFUNCDEF */
if (cx->errors.count) { goto exit; }
cx->pc = 0;

if (stop_pc == 0) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 5;
struct cx_fimp *i = func_emitNbmips_();
if (!i->scope) { i->scope = cx_scope_ref(cx_scope(cx, 0)); }
}

{ /* CX_TMACRO CX_OFIMP */
if (cx->errors.count) { goto exit; }
cx->pc = 1;

if (stop_pc == 1) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 5;
goto op39;
}

op2: { /* CX_TMACRO CX_OBEGIN */
if (cx->errors.count) { goto exit; }
cx->pc = 2;

if (stop_pc == 2) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 5;
struct cx_scope *parent = func_emitNbmips_()->scope;
cx_push_lib(cx, lib_cxEbin());
cx_begin(cx, parent);
}

op3: { /* CX_TLITERAL CX_OPUSH */
if (cx->errors.count) { goto exit; }
cx->pc = 3;

if (stop_pc == 3) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 35;
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10000000000;
}

{ /* CX_TLAMBDA CX_OLAMBDA */
if (cx->errors.count) { goto exit; }
cx->pc = 4;

if (stop_pc == 4) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 36;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 5, 30);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op35;
}

op5: { /* CX_TLITERAL CX_OPUSH */
if (cx->errors.count) { goto exit; }
cx->pc = 5;

if (stop_pc == 5) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 39;
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10;
}

{ /* CX_TLAMBDA CX_OLAMBDA */
if (cx->errors.count) { goto exit; }
cx->pc = 6;

if (stop_pc == 6) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 40;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 7, 27);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op34;
}

op7: { /* CX_TLITERAL CX_OPUSH */
if (cx->errors.count) { goto exit; }
cx->pc = 7;

if (stop_pc == 7) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 43;
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
}

{ /* CX_TID CX_OFIMP */
if (cx->errors.count) { goto exit; }
cx->pc = 8;

if (stop_pc == 8) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 44;
goto op32;
}

op9: { /* CX_TID CX_OBEGIN */
if (cx->errors.count) { goto exit; }
cx->pc = 9;

if (stop_pc == 9) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 44;
struct cx_scope *parent = func_fib_Int()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
}

op10: { /* CX_TID CX_OPUTARGS */
if (cx->errors.count) { goto exit; }
cx->pc = 10;

if (stop_pc == 10) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 44;
struct cx_scope
  *ds = cx_scope(cx, 0),
  *ss = ds->stack.count ? ds : cx_scope(cx, 1);

*cx_put_var(ds, sym_n) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
cx_vec_delete(&ss->stack, ss->stack.count-1);
}

{ /* CX_TLITERAL CX_OPUSH */
if (cx->errors.count) { goto exit; }
cx->pc = 11;

if (stop_pc == 11) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 1;
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
}

{ /* CX_TLITERAL CX_OPUSH */
if (cx->errors.count) { goto exit; }
cx->pc = 12;

if (stop_pc == 12) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 3;
cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
}

{ /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 13;

if (stop_pc == 13) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 4;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OFIMP */
if (cx->errors.count) { goto exit; }
cx->pc = 14;

if (stop_pc == 14) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 7;
goto op30;
}

op15: { /* CX_TID CX_OBEGIN */
if (cx->errors.count) { goto exit; }
cx->pc = 15;

if (stop_pc == 15) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 7;
struct cx_scope *parent = func_fibNrec_IntIntInt()->scope;
cx_push_lib(cx, lib_cxEmath());
cx_begin(cx, parent);
}

op16: { /* CX_TID CX_OPUTARGS */
if (cx->errors.count) { goto exit; }
cx->pc = 16;

if (stop_pc == 16) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 7;
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
if (cx->errors.count) { goto exit; }
cx->pc = 17;

if (stop_pc == 17) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 0;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 18;

if (stop_pc == 18) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 2;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp94 = NULL;if (!imp94) { imp94 = func_M_Opt(); }
if (imp94 && s->safe && !cx_fimp_match(imp94, s)) { imp94 = NULL; }
if (!imp94) { imp94 = cx_func_match(func_M(), s); }
if (!imp94) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: ?");
  goto exit;
}

if (!cx_fimp_call(imp94, s)) { goto exit; }
}

op19: { /* CX_TLAMBDA CX_OLAMBDA */
if (cx->errors.count) { goto exit; }
cx->pc = 19;

if (stop_pc == 19) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 4;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_lambda *l = cx_lambda_new(s, 20, 7);
cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
goto op27;
}

op20: { /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 20;

if (stop_pc == 20) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 5;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 21;

if (stop_pc == 21) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 8;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 22;

if (stop_pc == 22) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 11;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_b, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 23;

if (stop_pc == 23) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 14;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp95 = NULL;if (!imp95) { imp95 = func_A_IntInt(); }
if (imp95 && s->safe && !cx_fimp_match(imp95, s)) { imp95 = NULL; }
if (!imp95) { imp95 = cx_func_match(func_A(), s); }
if (!imp95) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: +");
  goto exit;
}

if (!cx_fimp_call(imp95, s)) { goto exit; }
}

op24: { /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 24;

if (stop_pc == 24) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 16;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_n, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 25;

if (stop_pc == 25) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 19;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp96 = NULL;if (imp96 && s->safe && !cx_fimp_match(imp96, s)) { imp96 = NULL; }
if (!imp96) { imp96 = cx_func_match(func_NN(), s); }
if (!imp96) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: --");
  goto exit;
}

if (!cx_fimp_call(imp96, s)) { goto exit; }
}

op26: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 26;

if (stop_pc == 26) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 22;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp97 = NULL;if (imp97 && s->safe && !cx_fimp_match(imp97, s)) { imp97 = NULL; }
if (!imp97) { imp97 = cx_func_match(func_recall(), s); }
if (!imp97) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: recall");
  goto exit;
}

if (!cx_fimp_call(imp97, s)) { goto exit; }
}

op27: { /* CX_TID CX_OGETVAR */
if (cx->errors.count) { goto exit; }
cx->pc = 27;

if (stop_pc == 27) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 29;
struct cx_scope *s = cx_scope(cx, 0);
struct cx_box *v = cx_get_var(s, sym_a, false);
if (!v) { goto exit; }
cx_copy(cx_push(s), v);
}

{ /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 28;

if (stop_pc == 28) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 32;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp98 = NULL;if (imp98 && s->safe && !cx_fimp_match(imp98, s)) { imp98 = NULL; }
if (!imp98) { imp98 = cx_func_match(func_ifNelse(), s); }
if (!imp98) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: if-else");
  goto exit;
}

if (!cx_fimp_call(imp98, s)) { goto exit; }
}

op29: { /* CX_TID CX_ORETURN */
if (cx->errors.count) { goto exit; }
cx->pc = 29;

if (stop_pc == 29) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 7;
struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(func_fibNrec_IntIntInt(), s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto exit;
  }

  call->recalls--;
  goto op16;
} else {
  size_t si = 0;
  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto exit;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = type_Int();
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto exit;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto exit;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  cx_call_deinit(cx_vec_pop(&cx->calls));
}
}

op30: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 30;

if (stop_pc == 30) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 7;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp99 = NULL;if (imp99 && s->safe && !cx_fimp_match(imp99, s)) { imp99 = NULL; }
if (!imp99) { imp99 = cx_func_match(func_fibNrec(), s); }
if (!imp99) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: fib-rec");
  goto exit;
}

if (!cx_fimp_call(imp99, s)) { goto exit; }
}

op31: { /* CX_TID CX_ORETURN */
if (cx->errors.count) { goto exit; }
cx->pc = 31;

if (stop_pc == 31) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 44;
struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(func_fib_Int(), s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto exit;
  }

  call->recalls--;
  goto op10;
} else {
  size_t si = 0;
  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto exit;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = type_Int();
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto exit;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto exit;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  cx_call_deinit(cx_vec_pop(&cx->calls));
}
}

op32: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 32;

if (stop_pc == 32) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 44;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp100 = NULL;if (imp100 && s->safe && !cx_fimp_match(imp100, s)) { imp100 = NULL; }
if (!imp100) { imp100 = cx_func_match(func_fib(), s); }
if (!imp100) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: fib");
  goto exit;
}

if (!cx_fimp_call(imp100, s)) { goto exit; }
}

op33: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 33;

if (stop_pc == 33) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 48;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp101 = NULL;if (imp101 && s->safe && !cx_fimp_match(imp101, s)) { imp101 = NULL; }
if (!imp101) { imp101 = cx_func_match(func__(), s); }
if (!imp101) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: _");
  goto exit;
}

if (!cx_fimp_call(imp101, s)) { goto exit; }
}

op34: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 34;

if (stop_pc == 34) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 50;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp102 = NULL;if (imp102 && s->safe && !cx_fimp_match(imp102, s)) { imp102 = NULL; }
if (!imp102) { imp102 = cx_func_match(func_times(), s); }
if (!imp102) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: times");
  goto exit;
}

if (!cx_fimp_call(imp102, s)) { goto exit; }
}

op35: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 35;

if (stop_pc == 35) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 56;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp103 = NULL;if (imp103 && s->safe && !cx_fimp_match(imp103, s)) { imp103 = NULL; }
if (!imp103) { imp103 = cx_func_match(func_clock(), s); }
if (!imp103) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: clock");
  goto exit;
}

if (!cx_fimp_call(imp103, s)) { goto exit; }
}

op36: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 36;

if (stop_pc == 36) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 62;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp104 = NULL;if (imp104 && s->safe && !cx_fimp_match(imp104, s)) { imp104 = NULL; }
if (!imp104) { imp104 = cx_func_match(func_E(), s); }
if (!imp104) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: /");
  goto exit;
}

if (!cx_fimp_call(imp104, s)) { goto exit; }
}

op37: { /* CX_TID CX_OFUNCALL */
if (cx->errors.count) { goto exit; }
cx->pc = 37;

if (stop_pc == 37) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 64;
struct cx_scope *s = cx_scope(cx, 0);
static struct cx_fimp *imp105 = NULL;if (imp105 && s->safe && !cx_fimp_match(imp105, s)) { imp105 = NULL; }
if (!imp105) { imp105 = cx_func_match(func_int(), s); }
if (!imp105) {
  cx_error(cx, cx->row, cx->col, "Func not applicable: int");
  goto exit;
}

if (!cx_fimp_call(imp105, s)) { goto exit; }
}

op38: { /* CX_TMACRO CX_ORETURN */
if (cx->errors.count) { goto exit; }
cx->pc = 38;

if (stop_pc == 38) {
  ok = true;
  goto exit;
}

cx->row = 1; cx->col = 5;
struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
struct cx_scope *s = cx_scope(cx, 0);

if (call->recalls) {
  if (s->safe && !cx_fimp_match(func_emitNbmips_(), s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    goto exit;
  }

  call->recalls--;
  goto op3;
} else {
  size_t si = 0;
  struct cx_scope *ds = cx_scope(cx, 1);
  cx_vec_grow(&ds->stack, ds->stack.count+1);

  {
    struct cx_box v;
    if (si == s->stack.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      goto exit;
    }

    v = *(struct cx_box *)cx_vec_get(&s->stack, si++);

    if (s->safe) {
     struct cx_type *t = type_Int();
      if (!cx_is(v.type, t)) {
        cx_error(cx, cx->row, cx->col,
                 "Invalid return type.\n"
                 "Expected %s, actual: %s",
                 t->id, v.type->id);
        goto exit;
      }
    }

    *(struct cx_box *)cx_vec_push(&ds->stack) = v;
  }

  if (si < s->stack.count) {
    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
    goto exit;
  }

  cx_vec_clear(&s->stack);
  cx_end(cx);
  cx_pop_lib(cx);
  cx_call_deinit(cx_vec_pop(&cx->calls));
}
}

 op39:
  ok = true;
exit:
  return ok;
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
	      "  10000000000 {10 {50 fib _} times} clock / int;\n"
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

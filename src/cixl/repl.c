#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/buf.h"
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

    static struct cx_func *func_SUSU;
    static struct cx_func *func_clock;
    static struct cx_func *func_int;
    static struct cx_func *func_QU;
    static struct cx_func *func_ifSUelse;
    static struct cx_func *func_recall;
    static struct cx_func *func_AD;
    static struct cx_func *func_fib;
    static struct cx_func *func_DI;
    static struct cx_func *func_fibSUrec;
    static struct cx_func *func__;
    static struct cx_fimp *func_SUSU_Int;
    static struct cx_fimp *func_clock_A;
    static struct cx_fimp *func_QU_Opt;
    static struct cx_fimp *func_ifSUelse_OptAA;
    static struct cx_fimp *func_recall_;
    static struct cx_fimp *func_AD_IntInt;
    static struct cx_fimp *func_DI_IntInt;
    static struct cx_fimp *func_int_Rat;
    static struct cx_fimp *func_fibSUrec_IntIntInt;
    static struct cx_fimp *func_fib_Int;
    static struct cx_fimp *func___;
    static struct cx_sym sym_a;
    static struct cx_sym sym_b;
    static struct cx_sym sym_n;
    static struct cx_type *type_Int;

    if (init) {
      init = false;

      {
	struct cx_func *func = cx_get_func(cx, "fib", false);
	struct cx_fimp *imp = cx_get_fimp(func, "Int", false);
	imp->bin = cx_bin_ref(cx->bin);
	imp->start_pc = 4;
	imp->nops = 24;
      }

      {
	struct cx_func *func = cx_get_func(cx, "fib-rec", false);
	struct cx_fimp *imp = cx_get_fimp(func, "Int Int Int", false);
	imp->bin = cx_bin_ref(cx->bin);
	imp->start_pc = 10;
	imp->nops = 16;
      }

      func_SUSU = cx_test(cx_get_func(cx, "--", false));
      func_clock = cx_test(cx_get_func(cx, "clock", false));
      func_int = cx_test(cx_get_func(cx, "int", false));
      func_QU = cx_test(cx_get_func(cx, "?", false));
      func_ifSUelse = cx_test(cx_get_func(cx, "if-else", false));
      func_recall = cx_test(cx_get_func(cx, "recall", false));
      func_AD = cx_test(cx_get_func(cx, "+", false));
      func_fib = cx_test(cx_get_func(cx, "fib", false));
      func_DI = cx_test(cx_get_func(cx, "/", false));
      func_fibSUrec = cx_test(cx_get_func(cx, "fib-rec", false));
      func__ = cx_test(cx_get_func(cx, "_", false));
      func_SUSU_Int = cx_test(cx_get_fimp(func_SUSU, "Int", false));
      func_clock_A = cx_test(cx_get_fimp(func_clock, "A", false));
      func_QU_Opt = cx_test(cx_get_fimp(func_QU, "Opt", false));
      func_ifSUelse_OptAA = cx_test(cx_get_fimp(func_ifSUelse, "Opt A A", false));
      func_recall_ = cx_test(cx_get_fimp(func_recall, "", false));
      func_AD_IntInt = cx_test(cx_get_fimp(func_AD, "Int Int", false));
      func_DI_IntInt = cx_test(cx_get_fimp(func_DI, "Int Int", false));
      func_int_Rat = cx_test(cx_get_fimp(func_int, "Rat", false));
      func_fibSUrec_IntIntInt = cx_test(cx_get_fimp(func_fibSUrec, "Int Int Int", false));
      func_fib_Int = cx_test(cx_get_fimp(func_fib, "Int", false));
      func___ = cx_test(cx_get_fimp(func__, "", false));
      sym_a = cx_sym(cx, "a");
      sym_b = cx_sym(cx, "b");
      sym_n = cx_sym(cx, "n");
      type_Int = cx_test(cx_get_type(cx, "Int", false));
    }

    static void *op_labels[35] = {
      &&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7, &&op8, &&op9, &&op10, 
      &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17, &&op18, &&op19, &&op20, 
      &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27, &&op28, &&op29, &&op30, 
      &&op31, &&op32, &&op33, &&op34};

    goto *op_labels[cx->pc];

  op0: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 0; cx->row = 1; cx->col = 10;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1000000000;
    }

  op1: { /* CX_TLAMBDA CX_OLAMBDA */
      cx->pc = 1; cx->row = 1; cx->col = 11;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_lambda *l = cx_lambda_new(s, 2, 29);
      cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
      goto op31;
    }

  op2: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 2; cx->row = 1; cx->col = 13;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
    }

  op3: { /* CX_TFUNC CX_OFIMP */
      cx->pc = 3; cx->row = 1; cx->col = 14;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      goto op28;
    }

  op4: { /* CX_TFUNC CX_OBEGIN */
      cx->pc = 4; cx->row = 1; cx->col = 14;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *parent = func_fib_Int->scope;
      cx_begin(cx, parent);
    }

  op5: { /* CX_TFUNC CX_OPUTARGS */
      cx->pc = 5; cx->row = 1; cx->col = 14;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope
	*ds = cx_scope(cx, 0),
	*ss = ds->stack.count ? ds : cx_scope(cx, 1);

      *cx_put_var(ds, sym_n, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
      cx_vec_delete(&ss->stack, ss->stack.count-1);
    }

  op6: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 6; cx->row = 1; cx->col = 1;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
    }

  op7: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 7; cx->row = 1; cx->col = 3;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
    }

  op8: { /* CX_TID CX_OGETVAR */
      cx->pc = 8; cx->row = 1; cx->col = 4;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_n, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op9: { /* CX_TFUNC CX_OFIMP */
      cx->pc = 9; cx->row = 1; cx->col = 7;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      goto op26;
    }

  op10: { /* CX_TFUNC CX_OBEGIN */
      cx->pc = 10; cx->row = 1; cx->col = 7;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *parent = func_fibSUrec_IntIntInt->scope;
      cx_begin(cx, parent);
    }

  op11: { /* CX_TFUNC CX_OPUTARGS */
      cx->pc = 11; cx->row = 1; cx->col = 7;
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

  op12: { /* CX_TID CX_OGETVAR */
      cx->pc = 12; cx->row = 1; cx->col = 0;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_n, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op13: { /* CX_TFIMP CX_OFUNCALL */
      cx->pc = 13; cx->row = 1; cx->col = 2;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_QU;
      struct cx_fimp *imp = func_QU_Opt;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 14);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op14: { /* CX_TLAMBDA CX_OLAMBDA */
      cx->pc = 14; cx->row = 1; cx->col = 4;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_lambda *l = cx_lambda_new(s, 15, 8);
      cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
      goto op23;
    }

  op15: { /* CX_TID CX_OGETVAR */
      cx->pc = 15; cx->row = 1; cx->col = 4;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_b, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op16: { /* CX_TID CX_OGETVAR */
      cx->pc = 16; cx->row = 1; cx->col = 7;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_a, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op17: { /* CX_TID CX_OGETVAR */
      cx->pc = 17; cx->row = 1; cx->col = 10;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_b, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op18: { /* CX_TFIMP CX_OFUNCALL */
      cx->pc = 18; cx->row = 1; cx->col = 13;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_AD;
      struct cx_fimp *imp = func_AD_IntInt;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 19);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op19: { /* CX_TID CX_OGETVAR */
      cx->pc = 19; cx->row = 1; cx->col = 15;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_n, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op20: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 20; cx->row = 1; cx->col = 18;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_SUSU;
      struct cx_fimp *imp = func_SUSU_Int;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 21);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op21: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 21; cx->row = 1; cx->col = 21;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_recall;
      struct cx_fimp *imp = func_recall_;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 22);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op22: { /* CX_TLAMBDA CX_OSTOP */
      cx->pc = 22; cx->row = 1; cx->col = 4;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx->stop = true;
    }

  op23: { /* CX_TID CX_OGETVAR */
      cx->pc = 23; cx->row = 1; cx->col = 28;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_a, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op24: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 24; cx->row = 1; cx->col = 31;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_ifSUelse;
      struct cx_fimp *imp = func_ifSUelse_OptAA;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 25);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op25: { /* CX_TFUNC CX_ORETURN */
      cx->pc = 25; cx->row = 1; cx->col = 21;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
      struct cx_scope *s = cx_scope(cx, 0);

      if (call->recalls) {
	if (s->safe && !cx_fimp_match(func_fibSUrec_IntIntInt, s)) {
	  cx_error(cx, cx->row, cx->col, "Recall not applicable");
	  return false;
	}

	call->recalls--;
	goto op11;
      } else {
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
	    struct cx_type *t = type_Int;
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

  op26: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 26; cx->row = 1; cx->col = 7;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_fibSUrec;
      struct cx_fimp *imp = func_fibSUrec_IntIntInt;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 27);
	goto op10;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op27: { /* CX_TFUNC CX_ORETURN */
      cx->pc = 27; cx->row = 1; cx->col = 21;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
      struct cx_scope *s = cx_scope(cx, 0);

      if (call->recalls) {
	if (s->safe && !cx_fimp_match(func_fib_Int, s)) {
	  cx_error(cx, cx->row, cx->col, "Recall not applicable");
	  return false;
	}

	call->recalls--;
	goto op5;
      } else {
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
	    struct cx_type *t = type_Int;
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
      cx->pc = 28; cx->row = 1; cx->col = 14;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_fib;
      struct cx_fimp *imp = func_fib_Int;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 29);
	goto op4;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op29: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 29; cx->row = 1; cx->col = 18;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func__;
      struct cx_fimp *imp = func___;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 30);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op30: { /* CX_TLAMBDA CX_OSTOP */
      cx->pc = 30; cx->row = 1; cx->col = 11;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      cx->stop = true;
    }

  op31: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 31; cx->row = 1; cx->col = 20;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_clock;
      struct cx_fimp *imp = func_clock_A;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 32);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op32: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 32; cx->row = 1; cx->col = 26;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_DI;
      struct cx_fimp *imp = func_DI_IntInt;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 33);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op33: { /* CX_TFIMP CX_OFUNCALL */
      cx->pc = 33; cx->row = 1; cx->col = 28;
      if (cx->stop) { return true; }
      if (cx->errors.count) { return false; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_int;
      struct cx_fimp *imp = func_int_Rat;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 34);
	goto op0;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op34:
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
  cx_use(cx, "cx/io", false);
  cx_use(cx, "cx/math", false);
  cx_use(cx, "cx/meta", false);
  cx_use(cx, "cx/stack/ops", false);
  cx_use(cx, "cx/stack/types", false);
  cx_use(cx, "cx/var", false);
  cx_use(cx, "cx/time", false);

  fprintf(out, "Cixl v%s, ", CX_VERSION);

  cx_eval_str(cx, "1000000000 {50 fib _} clock / int<Rat>");
  emit_bmips(cx);
  cx_eval_str(cx,
	      "let: (bmips emit-bmips);"
	      "[$bmips @/ $emit-bmips ' bmips' @@n] say");

  fputs("Press Return twice to evaluate.\n\n", out);
  if (cx->errors.count) { cx_dump_errors(cx, out); }

  struct cx_buf body;
  cx_buf_open(&body);
  char line[CX_REPL_LINE_MAX];

  while (true) {
    fflush(body.stream);
    fputs(body.size ? "..." : "   ", out);
    
    if (fgets(line, sizeof(line), in) == NULL) { break; }

    if (strcmp(line, "\n") == 0) {
      cx_buf_close(&body);

      if (cx_eval_str(cx, body.data)) {
	cx_stack_dump(&cx_scope(cx, 0)->stack, out);
	fputc('\n', out);
      } else {
	cx_dump_errors(cx, out);
      }

      free(body.data);
      cx_buf_open(&body);
    } else {
      if (strcmp(line, "quit\n") == 0) { break; }
      fputs(line, body.stream);
    }
  }

  cx_buf_close(&body);
  free(body.data);
}

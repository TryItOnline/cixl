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
#include "cixl/eval.h"
#include "cixl/repl.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/types/func.h"
#include "cixl/types/lambda.h"
#include "cixl/types/vect.h"

static bool emit_bmips(struct cx *cx) {
  bool _eval(struct cx *cx) {
    static bool init = true;

    static void *op_labels[32] = {
      &&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7, &&op8, &&op9, &&op10, 
      &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17, &&op18, &&op19, &&op20, 
      &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27, &&op28, &&op29, &&op30, 
      &&op31};

    static struct cx_func *func64;
    static struct cx_fimp *fimp64_0;
    static struct cx_sym sym3;
    static struct cx_func *func63;
    static struct cx_fimp *fimp63_0;
    static struct cx_sym sym26;
    static struct cx_sym sym25;
    static struct cx_func *func22;
    static struct cx_fimp *fimp22_0;
    static struct cx_func *func57;
    static struct cx_fimp *fimp57_0;
    static struct cx_func *func2;
    static struct cx_fimp *fimp2_0;
    static struct cx_func *func30;
    static struct cx_fimp *fimp30_0;
    static struct cx_func *func28;
    static struct cx_fimp *fimp28_0;
    static struct cx_type *type11;
    static struct cx_func *func48;
    static struct cx_fimp *fimp48_0;
    static struct cx_func *func9;
    static struct cx_fimp *fimp9_0;
    static struct cx_func *func60;
    static struct cx_fimp *fimp60_0;
    static struct cx_func *func14;
    static struct cx_fimp *fimp14_1;

    if (init) {
      init = false;
      func64 = cx_get_func(cx, "fib", false);
      fimp64_0 = cx_get_fimp(func64, "Int", false);
      sym3 = cx_sym(cx, "n");
      func63 = cx_get_func(cx, "fib-rec", false);
      fimp63_0 = cx_get_fimp(func63, "Int Int Int", false);
      sym26 = cx_sym(cx, "b");
      sym25 = cx_sym(cx, "a");
      func22 = cx_get_func(cx, "?", false);
      fimp22_0 = cx_get_fimp(func22, "Opt", false);
      func57 = cx_get_func(cx, "+", false);
      fimp57_0 = cx_get_fimp(func57, "Int Int", false);
      func2 = cx_get_func(cx, "--", false);
      fimp2_0 = cx_get_fimp(func2, "Int", false);
      func30 = cx_get_func(cx, "recall", false);
      fimp30_0 = cx_get_fimp(func30, "", false);
      func28 = cx_get_func(cx, "if-else", false);
      fimp28_0 = cx_get_fimp(func28, "Opt A A", false);
      type11 = cx_get_type(cx, "Int", false);
      func48 = cx_get_func(cx, "_", false);
      fimp48_0 = cx_get_fimp(func48, "", false);
      func9 = cx_get_func(cx, "clock", false);
      fimp9_0 = cx_get_fimp(func9, "A", false);
      func60 = cx_get_func(cx, "/", false);
      fimp60_0 = cx_get_fimp(func60, "Int Int", false);
      func14 = cx_get_func(cx, "int", false);
      fimp14_1 = cx_get_fimp(func14, "Rat", false);
    }

    goto *op_labels[cx->pc];

  op0: { /* CX_TLITERAL CX_OPUSH */
      if (cx->stop) { return true; }
      cx->pc = 0; cx->row = 1; cx->col = 10;
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1000000000;
    }

  op1: { /* CX_TLAMBDA CX_OLAMBDA */
      if (cx->stop) { return true; }
      cx->pc = 1; cx->row = 1; cx->col = 11;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_lambda *l = cx_lambda_new(s, 2, 27);
      cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
      cx->pc = 29;
      goto op29;
    }

  op2: { /* CX_TLITERAL CX_OPUSH */
      if (cx->stop) { return true; }
      cx->pc = 2; cx->row = 1; cx->col = 13;
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
    }

  op3: { /* CX_TFUNC CX_OFIMP */
      if (cx->stop) { return true; }
      cx->pc = 3; cx->row = 1; cx->col = 14;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_fimp *imp = fimp64_0;
      if (s->safe && !cx_fimp_match(imp, s)) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s",
		 imp->func->id);
	return false;
      }
      cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 27);
    }

  op4: { /* CX_TFUNC CX_OBEGIN */
      if (cx->stop) { return true; }
      cx->pc = 4; cx->row = 1; cx->col = 14;
      struct cx_scope *parent = fimp64_0->scope;
      cx_begin(cx, parent);
    }

  op5: { /* CX_TFUNC CX_OPUTARGS */
      if (cx->stop) { return true; }
      cx->pc = 5; cx->row = 1; cx->col = 14;
      struct cx_scope
	*ds = cx_scope(cx, 0),
	*ss = ds->stack.count ? ds : cx_scope(cx, 1);
      *cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
    }

  op6: { /* CX_TLITERAL CX_OPUSH */
      if (cx->stop) { return true; }
      cx->pc = 6; cx->row = 1; cx->col = 1;
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
    }

  op7: { /* CX_TLITERAL CX_OPUSH */
      if (cx->stop) { return true; }
      cx->pc = 7; cx->row = 1; cx->col = 3;
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
    }

  op8: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 8; cx->row = 1; cx->col = 4;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym3, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op9: { /* CX_TFUNC CX_OFIMP */
      if (cx->stop) { return true; }
      cx->pc = 9; cx->row = 1; cx->col = 7;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_fimp *imp = fimp63_0;
      if (s->safe && !cx_fimp_match(imp, s)) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s",
		 imp->func->id);
	return false;
      }
      cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 26);
    }

  op10: { /* CX_TFUNC CX_OBEGIN */
      if (cx->stop) { return true; }
      cx->pc = 10; cx->row = 1; cx->col = 7;
      struct cx_scope *parent = fimp63_0->scope;
      cx_begin(cx, parent);
    }

  op11: { /* CX_TFUNC CX_OPUTARGS */
      if (cx->stop) { return true; }
      cx->pc = 11; cx->row = 1; cx->col = 7;
      struct cx_scope
	*ds = cx_scope(cx, 0),
	*ss = ds->stack.count ? ds : cx_scope(cx, 1);
      *cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
      *cx_put_var(ds, sym26, true) = *cx_test(cx_pop(ss, false));
      *cx_put_var(ds, sym25, true) = *cx_test(cx_pop(ss, false));
    }

  op12: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 12; cx->row = 1; cx->col = 0;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym3, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op13: { /* CX_TFIMP CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 13; cx->row = 1; cx->col = 2;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func22;
      struct cx_fimp *imp = fimp22_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op14: { /* CX_TLAMBDA CX_OLAMBDA */
      if (cx->stop) { return true; }
      cx->pc = 14; cx->row = 1; cx->col = 4;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_lambda *l = cx_lambda_new(s, 15, 8);
      cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
      cx->pc = 23;
      goto op23;
    }

  op15: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 15; cx->row = 1; cx->col = 4;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym26, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op16: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 16; cx->row = 1; cx->col = 7;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym25, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op17: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 17; cx->row = 1; cx->col = 10;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym26, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op18: { /* CX_TFIMP CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 18; cx->row = 1; cx->col = 13;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func57;
      struct cx_fimp *imp = fimp57_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op19: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 19; cx->row = 1; cx->col = 15;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym3, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op20: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 20; cx->row = 1; cx->col = 18;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func2;
      struct cx_fimp *imp = fimp2_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op21: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 21; cx->row = 1; cx->col = 21;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func30;
      struct cx_fimp *imp = fimp30_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op22: { /* CX_TLAMBDA CX_OSTOP */
      if (cx->stop) { return true; }
      cx->pc = 22; cx->row = 1; cx->col = 4;
      cx->stop = true;
    }

  op23: { /* CX_TID CX_OGETVAR */
      if (cx->stop) { return true; }
      cx->pc = 23; cx->row = 1; cx->col = 28;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym25, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op24: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 24; cx->row = 1; cx->col = 31;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func28;
      struct cx_fimp *imp = fimp28_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op25: { /* CX_TFUNC CX_ORETURN */
      if (cx->stop) { return true; }
      cx->pc = 25; cx->row = 1; cx->col = 21;
      struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
      struct cx_scope *s = cx_scope(cx, 0);

      if (call->recalls) {
	if (s->safe && !cx_fimp_match(fimp63_0, s)) {
	  cx_error(cx, cx->row, cx->col, "Recall not applicable");
	  return false;
	}

	call->recalls--;
	cx->pc = 11;
	goto op11;
      } else {
	if (s->stack.count > 1) {
	  cx_error(cx, cx->row, cx->col, "Stack not empty on return");
	  return false;
	}

	if (s->stack.count < 1) {
	  cx_error(cx, cx->row, cx->col,
		   "Not enough return values on stack");
	  return false;
	}

	struct cx_scope *ds = cx_scope(cx, 1);
	cx_vec_grow(&ds->stack, ds->stack.count+1);
	struct cx_box *v = cx_vec_start(&s->stack);

	if (s->safe) {
	  struct cx_type *t = NULL;
	  t = type11;
	  if (!cx_is(v->type, t)) {
	    cx_error(cx, cx->row, cx->col,
		     "Invalid return type.\n"
		     "Expected %s, actual: %s",
		     t->id, v->type->id);
	    return false;
	  }
	}

	*(struct cx_box *)cx_vec_push(&ds->stack) = *v;
	v++;

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

  op26: { /* CX_TFUNC CX_ORETURN */
      if (cx->stop) { return true; }
      cx->pc = 26; cx->row = 1; cx->col = 21;
      struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
      struct cx_scope *s = cx_scope(cx, 0);

      if (call->recalls) {
	if (s->safe && !cx_fimp_match(fimp64_0, s)) {
	  cx_error(cx, cx->row, cx->col, "Recall not applicable");
	  return false;
	}

	call->recalls--;
	cx->pc = 5;
	goto op5;
      } else {
	if (s->stack.count > 1) {
	  cx_error(cx, cx->row, cx->col, "Stack not empty on return");
	  return false;
	}

	if (s->stack.count < 1) {
	  cx_error(cx, cx->row, cx->col,
		   "Not enough return values on stack");
	  return false;
	}

	struct cx_scope *ds = cx_scope(cx, 1);
	cx_vec_grow(&ds->stack, ds->stack.count+1);
	struct cx_box *v = cx_vec_start(&s->stack);

	if (s->safe) {
	  struct cx_type *t = NULL;
	  t = type11;
	  if (!cx_is(v->type, t)) {
	    cx_error(cx, cx->row, cx->col,
		     "Invalid return type.\n"
		     "Expected %s, actual: %s",
		     t->id, v->type->id);
	    return false;
	  }
	}

	*(struct cx_box *)cx_vec_push(&ds->stack) = *v;
	v++;

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

  op27: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 27; cx->row = 1; cx->col = 18;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func48;
      struct cx_fimp *imp = fimp48_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op28: { /* CX_TLAMBDA CX_OSTOP */
      if (cx->stop) { return true; }
      cx->pc = 28; cx->row = 1; cx->col = 11;
      cx->stop = true;
    }

  op29: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 29; cx->row = 1; cx->col = 20;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func9;
      struct cx_fimp *imp = fimp9_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op30: { /* CX_TFUNC CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 30; cx->row = 1; cx->col = 26;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func60;
      struct cx_fimp *imp = fimp60_0;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

  op31: { /* CX_TFIMP CX_OFUNCALL */
      if (cx->stop) { return true; }
      cx->pc = 31; cx->row = 1; cx->col = 28;
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func14;
      struct cx_fimp *imp = fimp14_1;
      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %%s", func->id);
	return false;
      }
      if (!cx_fimp_call(imp, s)) { return false; }
    }

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
  fprintf(out, "Cixl v%s, ", CX_VERSION);

  cx_eval_str(cx, "1000000000 {50 fib _} clock / int<Rat>");
  emit_bmips(cx);
  cx_eval_str(cx, "[@/ ~ ' bmips' @@n] say");

  fputs("Press Return twice to evaluate.\n\n", out);
    
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
	cx_stackdump(cx_scope(cx, 0), out);
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

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
    static struct cx_func *func64;
    static struct cx_fimp *fimp64_0;
    static struct cx_sym sym3;
    static struct cx_func *func63;
    static struct cx_fimp *fimp63_0;
    static struct cx_sym sym26;
    static struct cx_sym sym25;
    static struct cx_func *func22;
    static struct cx_func *func28;
    static struct cx_fimp *fimp28_0;
    static struct cx_func *func57;
    static struct cx_fimp *fimp57_0;
    static struct cx_func *func2;
    static struct cx_fimp *fimp2_0;
    static struct cx_func *func30;
    static struct cx_fimp *fimp30_0;
    static struct cx_type *type11;
    static struct cx_func *func48;
    static struct cx_fimp *fimp48_0;
    static struct cx_func *func9;
    static struct cx_fimp *fimp9_0;
    static struct cx_func *func60;
    static struct cx_fimp *fimp60_0;
    static struct cx_func *func14;

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
      func28 = cx_get_func(cx, "if-else", false);
      fimp28_0 = cx_get_fimp(func28, "Opt A A", false);
      func57 = cx_get_func(cx, "+", false);
      fimp57_0 = cx_get_fimp(func57, "Int Int", false);
      func2 = cx_get_func(cx, "--", false);
      fimp2_0 = cx_get_fimp(func2, "Int", false);
      func30 = cx_get_func(cx, "recall", false);
      fimp30_0 = cx_get_fimp(func30, "", false);
      type11 = cx_get_type(cx, "Int", false);
      func48 = cx_get_func(cx, "_", false);
      fimp48_0 = cx_get_fimp(func48, "", false);
      func9 = cx_get_func(cx, "clock", false);
      fimp9_0 = cx_get_fimp(func9, "A", false);
      func60 = cx_get_func(cx, "/", false);
      fimp60_0 = cx_get_fimp(func60, "Int Int", false);
      func14 = cx_get_func(cx, "int", false);
    }

    while (!cx->stop) {
      switch (cx->pc) {
      case 0: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 10;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1000000000;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 1: { /* CX_TLAMBDA CX_OLAMBDA */
        cx->row = 1; cx->col = 11;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_lambda *l = cx_lambda_new(s, 2, 31);
	cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
	cx->pc += l->nops+1;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 2: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 11;
	cx->scan_level += 1;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 3: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 13;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 50;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 4: { /* CX_TFUNC CX_OFIMP */
        cx->row = 1; cx->col = 14;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func64,
				       cx_fimp_scan);
	scan->as_fimp.imp = fimp64_0;
	scan->as_fimp.pc = 5;
	cx->pc += 26;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 5: { /* CX_TFUNC CX_OBEGIN */
        cx->row = 1; cx->col = 14;
	struct cx_scope *parent = fimp64_0->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 6: { /* CX_TFUNC CX_OPUTARGS */
        cx->row = 1; cx->col = 14;
	struct cx_scope
	  *ds = cx_scope(cx, 0),
	  *ss = ds->stack.count ? ds : cx_scope(cx, 1);
	*cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
	cx->pc++;
      }
      case 7: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 1;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 8: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 3;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 9: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 4;
	cx_ogetvar1(sym3, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 10: { /* CX_TFUNC CX_OFIMP */
        cx->row = 1; cx->col = 7;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func63,
				       cx_fimp_scan);
	scan->as_fimp.imp = fimp63_0;
	scan->as_fimp.pc = 11;
	cx->pc += 19;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 11: { /* CX_TFUNC CX_OBEGIN */
        cx->row = 1; cx->col = 7;
	struct cx_scope *parent = fimp63_0->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 12: { /* CX_TFUNC CX_OPUTARGS */
        cx->row = 1; cx->col = 7;
	struct cx_scope
	  *ds = cx_scope(cx, 0),
	  *ss = ds->stack.count ? ds : cx_scope(cx, 1);
	*cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
	*cx_put_var(ds, sym26, true) = *cx_test(cx_pop(ss, false));
	*cx_put_var(ds, sym25, true) = *cx_test(cx_pop(ss, false));
	cx->pc++;
      }
      case 13: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 0;
	cx_ogetvar1(sym3, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 14: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 2;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func22,
				       cx_funcall_scan);
	scan->as_funcall.imp = NULL;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 15: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 4;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func28,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp28_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 16: { /* CX_TLAMBDA CX_OLAMBDA */
        cx->row = 1; cx->col = 12;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_lambda *l = cx_lambda_new(s, 17, 10);
	cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;
	cx->pc += l->nops+1;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 17: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 12;
	cx->scan_level += 1;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 18: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 12;
	cx_ogetvar1(sym26, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 19: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 15;
	cx_ogetvar1(sym25, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 20: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 18;
	cx_ogetvar1(sym26, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 21: { /* CX_TFIMP CX_OFUNCALL */
        cx->row = 1; cx->col = 21;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func57,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp57_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 22: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 23;
	cx_ogetvar1(sym3, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 23: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 26;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func2,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp2_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 24: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 29;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func30,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp30_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 25: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 35;
	cx->scan_level += -1;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
	if (c && c->scan_level == cx->scan_level) {
	  cx_cut_deinit(cx_vec_pop(&s->cuts));
	}
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 26: { /* CX_TLAMBDA CX_OSTOP */
        cx->row = 1; cx->col = 12;
	cx->stop = true;
	cx->pc++;
        break;
      }
      case 27: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 36;
	cx_ogetvar1(sym25, cx_scope(cx, 0));
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 28: { /* CX_TFENCE CX_ORETURN */
        cx->row = 1; cx->col = 35;
	struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

	if (call->recalls) {
	  cx_oreturn_recall(call, 11, cx);
	} else {
	  struct cx_scope *ss = cx_scope(cx, 0);

	  if (ss->stack.count > 1) {
	    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
	    return false;
	  }

	  if (ss->stack.count < 1) {
	    cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	    return false;
	  }

	  struct cx_scope *ds = cx_scope(cx, 1);
	  cx_vec_grow(&ds->stack, ds->stack.count+1);
	  struct cx_box *v = cx_vec_start(&ss->stack);

	  if (ss->safe) {
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

	  cx_oreturn_end(ss);
	}
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 29: { /* CX_TFENCE CX_ORETURN */
        cx->row = 1; cx->col = 35;
	struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

	if (call->recalls) {
	  cx_oreturn_recall(call, 5, cx);
	} else {
	  struct cx_scope *ss = cx_scope(cx, 0);

	  if (ss->stack.count > 1) {
	    cx_error(cx, cx->row, cx->col, "Stack not empty on return");
	    return false;
	  }

	  if (ss->stack.count < 1) {
	    cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	    return false;
	  }

	  struct cx_scope *ds = cx_scope(cx, 1);
	  cx_vec_grow(&ds->stack, ds->stack.count+1);
	  struct cx_box *v = cx_vec_start(&ss->stack);

	  if (ss->safe) {
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

	  cx_oreturn_end(ss);
	}
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 30: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 18;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func48,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp48_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 31: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 19;
	cx->scan_level += -1;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
	if (c && c->scan_level == cx->scan_level) {
	  cx_cut_deinit(cx_vec_pop(&s->cuts));
	}
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 32: { /* CX_TLAMBDA CX_OSTOP */
        cx->row = 1; cx->col = 11;
	cx->stop = true;
	cx->pc++;
        break;
      }
      case 33: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 20;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func9,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp9_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 34: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 26;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func60,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp60_0;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      case 35: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 28;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func14,
				       cx_funcall_scan);
	scan->as_funcall.imp = NULL;
	cx->pc++;
        
        size_t noks = 0;
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
          noks++;
        }
        if (noks) { break; }
      }
      default:
        return true;
      }
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

  cx_eval_str(cx, "1000000000 {50 fib _} clock / int");
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

#include "cixl/bin.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/lambda.h"
#include "cixl/types/vect.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

bool cx_emit_tests(struct cx *cx) {  
  bool eval(struct cx *cx) {
    static bool init = true;
    static struct cx_func *func40 = NULL;
    static struct cx_fimp *fimp40_0 = NULL;
    static struct cx_sym sym2;
    static struct cx_sym sym16;
    static struct cx_func *func32 = NULL;
    static struct cx_func *func9 = NULL;
    static struct cx_fimp *fimp9_0 = NULL;
    static struct cx_func *func3 = NULL;
    static struct cx_fimp *fimp3_0 = NULL;
    static struct cx_func *func64 = NULL;
    static struct cx_fimp *fimp64_0 = NULL;
    static struct cx_sym sym3;
    static struct cx_func *func63 = NULL;
    static struct cx_fimp *fimp63_0 = NULL;
    static struct cx_sym sym26;
    static struct cx_sym sym25;
    static struct cx_func *func22 = NULL;
    static struct cx_func *func28 = NULL;
    static struct cx_fimp *fimp28_0 = NULL;
    static struct cx_func *func57 = NULL;
    static struct cx_fimp *fimp57_0 = NULL;
    static struct cx_func *func2 = NULL;
    static struct cx_fimp *fimp2_0 = NULL;
    static struct cx_func *func30 = NULL;
    static struct cx_fimp *fimp30_0 = NULL;
    static struct cx_type *type11;
    static struct cx_func *func48 = NULL;
    static struct cx_fimp *fimp48_0 = NULL;
    static struct cx_func *func60 = NULL;
    static struct cx_fimp *fimp60_0 = NULL;
    static struct cx_func *func14 = NULL;

    if (init) {
      init = false;
      func40 = cx_get_func(cx, "say", false);
      fimp40_0 = cx_func_get_imp(func40, "A", false);
      sym2 = cx_sym(cx, "v");
      sym16 = cx_sym(cx, "out");
      func32 = cx_get_func(cx, "print", false);
      func9 = cx_get_func(cx, "clock", false);
      fimp9_0 = cx_func_get_imp(func9, "A", false);
      func3 = cx_get_func(cx, "times", false);
      fimp3_0 = cx_func_get_imp(func3, "Int A", false);
      func64 = cx_get_func(cx, "fib", false);
      fimp64_0 = cx_func_get_imp(func64, "Int", false);
      sym3 = cx_sym(cx, "n");
      func63 = cx_get_func(cx, "fib-rec", false);
      fimp63_0 = cx_func_get_imp(func63, "Int Int Int", false);
      sym26 = cx_sym(cx, "b");
      sym25 = cx_sym(cx, "a");
      func22 = cx_get_func(cx, "?", false);
      func28 = cx_get_func(cx, "if-else", false);
      fimp28_0 = cx_func_get_imp(func28, "Opt A A", false);
      func57 = cx_get_func(cx, "+", false);
      fimp57_0 = cx_func_get_imp(func57, "Int Int", false);
      func2 = cx_get_func(cx, "--", false);
      fimp2_0 = cx_func_get_imp(func2, "Int", false);
      func30 = cx_get_func(cx, "recall", false);
      fimp30_0 = cx_func_get_imp(func30, "", false);
      type11 = cx_get_type(cx, "Int", false);
      func48 = cx_get_func(cx, "_", false);
      fimp48_0 = cx_func_get_imp(func48, "", false);
      func60 = cx_get_func(cx, "/", false);
      fimp60_0 = cx_func_get_imp(func60, "Int Int", false);
      func14 = cx_get_func(cx, "int", false);
    }

    while (!cx->stop) {
      switch (cx->pc) {
      case 0: { /* CX_TFUNC CX_OFIMP */
        cx->row = 1; cx->col = 0;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func40,
				       cx_fimp_scan);
	scan->as_fimp.imp = fimp40_0;
	scan->as_fimp.pc = 1;
	cx->pc += 10;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 1: { /* CX_TFUNC CX_OBEGIN */
        cx->row = 1; cx->col = 0;
	struct cx_scope *parent = fimp40_0->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 2: { /* CX_TFUNC CX_OPUTARGS */
        cx->row = 1; cx->col = 0;
	struct cx_scope
	  *ds = cx_scope(cx, 0),
	  *ss = ds->stack.count ? ds : cx_scope(cx, 1);
	*cx_put_var(ds, sym2, true) = *cx_test(cx_pop(ss, false));
	cx->pc++;
      }
      case 3: { /* CX_TID CX_OGETCONST */
        cx->row = 1; cx->col = 0;
	struct cx_box *v = cx_get_const(cx, sym16, false);
	if (!v) { return false; }
	cx_copy(cx_push(cx_scope(cx, 0)), v);
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
      case 4: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 5;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func32,
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
      case 5: { /* CX_TID CX_OGETVAR */
        cx->row = 1; cx->col = 11;
	cx_ogetvar1(sym2, cx_scope(cx, 0));
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
      case 6: { /* CX_TID CX_OGETCONST */
        cx->row = 1; cx->col = 14;
	struct cx_box *v = cx_get_const(cx, sym16, false);
	if (!v) { return false; }
	cx_copy(cx_push(cx_scope(cx, 0)), v);
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
      case 7: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 19;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func32,
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
      case 8: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 25;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->char_type)->as_char = 10;
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
      case 9: { /* CX_TLITERAL CX_ORETURN */
        cx->row = 1; cx->col = 25;
	struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

	if (call->recalls) {
	  cx_oreturn_recall(call, 1, cx);
	} else {
	  struct cx_scope *ss = cx_scope(cx, 0);

	  if (ss->stack.count > 0) {
	    return false;
	  }

	  if (ss->stack.count < 0) {
	    cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	    return false;
	  }

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
      case 10: { /* CX_TGROUP CX_OBEGIN */
        cx->row = 1; cx->col = 4;
	struct cx_scope *parent = cx_scope(cx, 0);
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 11: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 4;
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
      case 12: { /* CX_TLAMBDA CX_OLAMBDA */
        cx->row = 1; cx->col = 10;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_lambda *l = cx_lambda_new(s, 13, 37);
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
      case 13: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 10;
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
      case 14: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 15;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 10000;
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
        cx->row = 1; cx->col = 16;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func3,
				       cx_funcall_scan);
	scan->as_funcall.imp = fimp3_0;
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
        cx->row = 1; cx->col = 22;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_lambda *l = cx_lambda_new(s, 17, 31);
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
        cx->row = 1; cx->col = 22;
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
      case 18: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 24;
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
      case 19: { /* CX_TFUNC CX_OFIMP */
        cx->row = 1; cx->col = 25;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func64,
				       cx_fimp_scan);
	scan->as_fimp.imp = fimp64_0;
	scan->as_fimp.pc = 20;
	cx->pc += 26;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 20: { /* CX_TFUNC CX_OBEGIN */
        cx->row = 1; cx->col = 25;
	struct cx_scope *parent = fimp64_0->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 21: { /* CX_TFUNC CX_OPUTARGS */
        cx->row = 1; cx->col = 25;
	struct cx_scope
	  *ds = cx_scope(cx, 0),
	  *ss = ds->stack.count ? ds : cx_scope(cx, 1);
	*cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
	cx->pc++;
      }
      case 22: { /* CX_TLITERAL CX_OPUSH */
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
      case 23: { /* CX_TLITERAL CX_OPUSH */
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
      case 24: { /* CX_TID CX_OGETVAR */
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
      case 25: { /* CX_TFUNC CX_OFIMP */
        cx->row = 1; cx->col = 7;
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0),
				       func63,
				       cx_fimp_scan);
	scan->as_fimp.imp = fimp63_0;
	scan->as_fimp.pc = 26;
	cx->pc += 19;
        
        while (cx->scans.count) {
          struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
          if (!cx_scan_ok(s)) { break; }
          cx_vec_pop(&cx->scans);
          if (!cx_scan_call(s)) { return false; }
        }
        break;
      }
      case 26: { /* CX_TFUNC CX_OBEGIN */
        cx->row = 1; cx->col = 7;
	struct cx_scope *parent = fimp63_0->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 27: { /* CX_TFUNC CX_OPUTARGS */
        cx->row = 1; cx->col = 7;
	struct cx_scope
	  *ds = cx_scope(cx, 0),
	  *ss = ds->stack.count ? ds : cx_scope(cx, 1);
	*cx_put_var(ds, sym3, true) = *cx_test(cx_pop(ss, false));
	*cx_put_var(ds, sym26, true) = *cx_test(cx_pop(ss, false));
	*cx_put_var(ds, sym25, true) = *cx_test(cx_pop(ss, false));
	cx->pc++;
      }
      case 28: { /* CX_TID CX_OGETVAR */
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
      case 29: { /* CX_TFUNC CX_OFUNCALL */
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
      case 30: { /* CX_TFUNC CX_OFUNCALL */
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
      case 31: { /* CX_TLAMBDA CX_OLAMBDA */
        cx->row = 1; cx->col = 12;
	struct cx_scope *s = cx_scope(cx, 0);
	struct cx_lambda *l = cx_lambda_new(s, 32, 10);
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
      case 32: { /* CX_TFENCE CX_OFENCE */
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
      case 33: { /* CX_TID CX_OGETVAR */
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
      case 34: { /* CX_TID CX_OGETVAR */
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
      case 35: { /* CX_TID CX_OGETVAR */
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
      case 36: { /* CX_TFIMP CX_OFUNCALL */
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
      case 37: { /* CX_TID CX_OGETVAR */
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
      case 38: { /* CX_TFUNC CX_OFUNCALL */
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
      case 39: { /* CX_TFUNC CX_OFUNCALL */
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
      case 40: { /* CX_TFENCE CX_OFENCE */
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
      case 41: { /* CX_TLAMBDA CX_OSTOP */
        cx->row = 1; cx->col = 12;
	cx->stop = true;
	cx->pc++;
        break;
      }
      case 42: { /* CX_TID CX_OGETVAR */
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
      case 43: { /* CX_TFENCE CX_ORETURN */
        cx->row = 1; cx->col = 35;
	struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

	if (call->recalls) {
	  cx_oreturn_recall(call, 26, cx);
	} else {
	  struct cx_scope *ss = cx_scope(cx, 0);

	  if (ss->stack.count > 1) {
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
      case 44: { /* CX_TFENCE CX_ORETURN */
        cx->row = 1; cx->col = 35;
	struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

	if (call->recalls) {
	  cx_oreturn_recall(call, 20, cx);
	} else {
	  struct cx_scope *ss = cx_scope(cx, 0);

	  if (ss->stack.count > 1) {
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
      case 45: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 29;
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
      case 46: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 30;
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
      case 47: { /* CX_TLAMBDA CX_OSTOP */
        cx->row = 1; cx->col = 22;
	cx->stop = true;
	cx->pc++;
        break;
      }
      case 48: { /* CX_TFENCE CX_OFENCE */
        cx->row = 1; cx->col = 30;
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
      case 49: { /* CX_TLAMBDA CX_OSTOP */
        cx->row = 1; cx->col = 10;
	cx->stop = true;
	cx->pc++;
        break;
      }
      case 50: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 31;
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
      case 51: { /* CX_TLITERAL CX_OPUSH */
        cx->row = 1; cx->col = 40;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1000000;
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
      case 52: { /* CX_TFUNC CX_OFUNCALL */
        cx->row = 1; cx->col = 41;
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
      case 53: { /* CX_TGROUP CX_OEND */
        cx->row = 1; cx->col = 4;
	cx_oend(cx);
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
  bin->eval = eval;
  bool ok = cx_eval(bin, 0, cx);
  cx_bin_deref(bin);
  return ok;
}

static bool emit(struct cx_op *op, struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_error(cx, op->row, op->col, "Emit not implemented: %s", op->type->id);
  return false;
}

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id) {
  type->id = id;
  type->scan = false;
  type->init = NULL;
  type->deinit = NULL;
  type->eval = NULL;
  type->emit = emit;
  type->emit_func = NULL;
  type->emit_fimp = NULL;
  type->emit_syms = NULL;
  type->emit_types = NULL;
  type->emit_break = false;
  return type;
}

struct cx_op *cx_op_init(struct cx_bin *bin,
			 struct cx_op_type *type,
			 size_t tok_idx) {
  struct cx_op *op = cx_vec_push(&bin->ops);
  op->type = type;
  op->tok_idx = tok_idx;
  op->pc = bin->ops.count-1;
  op->row = -1; op->col = -1;
  return op;
}

static bool begin_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *parent = op->as_begin.child
    ? cx_scope(cx, 0)
    : op->as_begin.fimp->scope;
  
  cx_begin(cx, parent);
  cx->scan_level++;
  return true;
}

static bool begin_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fputs("struct cx_scope *parent = ", out);
  
  if (op->as_begin.child) {
    fputs("cx_scope(cx, 0);\n", out);
  } else {
    struct cx_fimp *imp = op->as_begin.fimp;
    fprintf(out, "fimp%zd_%zd->scope;\n", imp->func->tag, imp->idx);
  }

  fputs("cx_begin(cx, parent);\n"
	"cx->scan_level++;\n"
	"cx->pc++;\n",
	out);
  
  return true;
}

static struct cx_func *begin_emit_func(struct cx_op *op) {
  return op->as_begin.fimp ? op->as_begin.fimp->func : NULL;
}

static struct cx_fimp *begin_emit_fimp(struct cx_op *op) {
  return op->as_begin.fimp;
}

cx_op_type(CX_OBEGIN, {
    type.eval = begin_eval;
    type.emit = begin_emit;
    type.emit_func = begin_emit_func;
    type.emit_fimp = begin_emit_fimp;
  });

static bool cut_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  cx_cut_init(cx_vec_push(&s->cuts), s);
  return true;
}

cx_op_type(CX_OCUT, {
    type.eval = cut_eval;
  });

static bool else_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_box *v = cx_pop(cx_scope(cx, 0), false);
  if (!v) { return false; }
  if (!cx_ok(v)) { cx->pc += op->as_else.nops; }
  cx_box_deinit(v);
  return true;
}

cx_op_type(CX_OELSE, {
    type.eval = else_eval;
  });

void cx_oend(struct cx *cx) {
  cx_end(cx);
  cx->scan_level--;
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
  if (c && c->scan_level == cx->scan_level) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
}

static bool end_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_oend(cx);
  return true;
}

static bool end_emit(struct cx_op *op, struct cx_bin *bin, FILE *out, struct cx *cx) {
  fputs("cx_oend(cx);\n"
	"cx->pc++;\n",
	out);

  return true;
}

cx_op_type(CX_OEND, {
    type.scan = true;
    type.eval = end_eval;
    type.emit = end_emit;
  });

static bool fence_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->scan_level += op->as_fence.delta_level;

  if (op->as_fence.delta_level < 0) {
    struct cx_scope *s = cx_scope(cx, 0);
    struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
    if (c && c->scan_level == cx->scan_level) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  }
  return true;
}

static bool fence_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fprintf(out, "cx->scan_level += %d;\n", op->as_fence.delta_level);

  if (op->as_fence.delta_level < 0) {
    fputs("struct cx_scope *s = cx_scope(cx, 0);\n"
	  "struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;\n"
	  "if (c && c->scan_level == cx->scan_level) {\n"
	  "  cx_cut_deinit(cx_vec_pop(&s->cuts));\n"
	  "}\n", out);
  }

  fputs("cx->pc++;\n", out);
  return true;
}

cx_op_type(CX_OFENCE, {
    type.scan = true;
    type.eval = fence_eval;
    type.emit = fence_emit;
  });

bool cx_fimp_scan(struct cx_scan *scan) {
  struct cx_fimp *imp = scan->as_fimp.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", imp->func->id);
    return false;
  }
  
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
  cx->pc = scan->as_fimp.pc;
  return true;
}

static bool fimp_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;
  
  if (op->as_fimp.inline1) {
    struct cx_scan *scan = cx_scan(cx_scope(cx, 0), imp->func, cx_fimp_scan);
    scan->as_fimp.imp = imp;
    scan->as_fimp.pc = op->pc+1;
  }
  
  cx->pc += op->as_fimp.nops;
  return true;
}

static bool fimp_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  if (op->as_fimp.inline1) {
    struct cx_fimp *imp = op->as_fimp.imp;
    fprintf(out,
	    "struct cx_scan *scan = cx_scan(cx_scope(cx, 0),\n"
	    "                               func%zd,\n"
	    "                               cx_fimp_scan);\n",
	    imp->func->tag);
    fprintf(out, "scan->as_fimp.imp = fimp%zd_%zd;\n", imp->func->tag, imp->idx);
    fprintf(out, "scan->as_fimp.pc = %zd;\n", op->pc+1);
  }

  fprintf(out, "cx->pc += %zd;\n", op->as_fimp.nops+1);
  return true;
}

static struct cx_func *fimp_emit_func(struct cx_op *op) {
  return op->as_fimp.imp->func;
}

static struct cx_fimp *fimp_emit_fimp(struct cx_op *op) {
  return op->as_fimp.imp;
}

cx_op_type(CX_OFIMP, {
    type.scan = true;
    type.eval = fimp_eval;
    type.emit = fimp_emit;
    type.emit_func = fimp_emit_func;
    type.emit_fimp = fimp_emit_fimp;
    type.emit_break = true;
  });

static bool fimpdef_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimpdef.imp;
  imp->scope = cx_scope_ref(cx_scope(cx, 0));
  return true;
}

cx_op_type(CX_OFIMPDEF, {
    type.eval = fimpdef_eval;
  });

bool cx_funcall_scan(struct cx_scan *scan) {
  struct cx_func *func = scan->func;
  struct cx_fimp *imp = scan->as_funcall.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;

  if (imp) {
    if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
  } else {
    imp = cx_func_match_imp(func, s, 0);
  }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
    
  if (!imp->ptr) {
    struct cx_bin_func *f = cx_bin_get_func(cx->bin, imp);

    if (f) {
      cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
      cx->pc = f->start_pc;
      return true;
    }
  }
  
  return cx_fimp_call(imp, s);
}

static bool funcall_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_funcall_scan);
  scan->as_funcall.imp = op->as_funcall.imp;
  return true;
}

static bool funcall_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  
  fprintf(out,
	  "struct cx_scan *scan = cx_scan(cx_scope(cx, 0),\n"
	  "                               func%zd,\n"
	  "                               cx_funcall_scan);\n",
	  func->tag);

  struct cx_fimp *imp = op->as_funcall.imp;

  if (imp) {
    fprintf(out, "scan->as_funcall.imp = fimp%zd_%zd;\n", imp->func->tag, imp->idx);
  } else {
    fputs("scan->as_funcall.imp = NULL;\n", out);
  }
  
  fputs("cx->pc++;\n", out);
  return true;
}

static struct cx_func *funcall_emit_func(struct cx_op *op) {
  return op->as_funcall.func;
}

static struct cx_fimp *funcall_emit_fimp(struct cx_op *op) {
  return op->as_funcall.imp;
}

cx_op_type(CX_OFUNCALL, {
    type.scan = true;
    type.eval = funcall_eval;
    type.emit = funcall_emit;
    type.emit_func = funcall_emit_func;
    type.emit_fimp = funcall_emit_fimp;
  });

static bool getconst_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_box *v = cx_get_const(cx, op->as_getconst.id, false);
  if (!v) { return false; }
  cx_copy(cx_push(cx_scope(cx, 0)), v);
  return true;
}

static bool getconst_emit(struct cx_op *op,
			  struct cx_bin *bin,
			  FILE *out,
			  struct cx *cx) {
  fprintf(out, "struct cx_box *v = cx_get_const(cx, sym%zd, false);\n",
	  op->as_getconst.id.tag);

  fputs("if (!v) { return false; }\n"
	"cx_copy(cx_push(cx_scope(cx, 0)), v);\n"
	"cx->pc++;\n",
	out);
  
  return true;
}

static void getconst_emit_syms(struct cx_op *op, struct cx_vec *out) {
  *(struct cx_sym *)cx_vec_push(out) = op->as_getconst.id;
}

cx_op_type(CX_OGETCONST, {
    type.scan = true;
    type.eval = getconst_eval;
    type.emit = getconst_emit;
    type.emit_syms = getconst_emit_syms;
  });


bool cx_ogetvar1(struct cx_sym id, struct cx_scope *scope) {
  struct cx_box *v = cx_get_var(scope, id, false);
  if (!v) { return false; }
  cx_copy(cx_push(scope), v);
  return true;
}

bool cx_ogetvar2(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (!scope->cuts.count) {
    cx_error(cx, cx->row, cx->col, "Nothing to uncut");
    return false;
  }
  
  struct cx_cut *c = cx_vec_peek(&scope->cuts, 0);
  
  if (!c->offs) {
    cx_error(cx, cx->row, cx->col, "Nothing to uncut");
    return false;
  }
  
  c->offs--;
  
  if (c->offs < scope->stack.count-1) {
    struct cx_box v = *(struct cx_box *)cx_vec_get(&scope->stack, c->offs);
    cx_vec_delete(&scope->stack, c->offs);
    *(struct cx_box *)cx_vec_push(&scope->stack) = v;
  }
  
  if (!c->offs) { cx_cut_deinit(cx_vec_pop(&scope->cuts)); }
  return true;
}

static bool getvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_sym id = op->as_getvar.id;
  return id.id[0] ? cx_ogetvar1(id, s) : cx_ogetvar2(s);
}

static bool getvar_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_sym id = op->as_getvar.id;

  if (id.id[0]) {
    fprintf(out, "cx_ogetvar1(sym%zd, cx_scope(cx, 0));\n", id.tag);
  } else {
    fputs("cx_ogetvar2(cx_scope(cx, 0));\n", out);
  }

  fputs("cx->pc++;\n", out);
  return true;
}

static void getvar_emit_syms(struct cx_op *op, struct cx_vec *out) {
  struct cx_sym id = op->as_getvar.id;
  if (id.id[0]) { *(struct cx_sym *)cx_vec_push(out) = id; }
}

cx_op_type(CX_OGETVAR, {
    type.scan = true;
    type.eval = getvar_eval;
    type.emit = getvar_emit;
    type.emit_syms = getvar_emit_syms;
  });

static bool jump_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->pc += op->as_jump.nops;
  return true;
}

cx_op_type(CX_OJUMP, {
    type.eval = jump_eval;
  });

static bool lambda_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *l = cx_lambda_new(scope,
				      op->as_lambda.start_op,
				      op->as_lambda.nops);
  cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
  cx->pc += l->nops;
  return true;
}

static bool lambda_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  fputs("struct cx_scope *s = cx_scope(cx, 0);\n", out);
  fprintf(out, "struct cx_lambda *l = cx_lambda_new(s, %zd, %zd);\n",
	  op->as_lambda.start_op, op->as_lambda.nops);

  fputs("cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;\n"
	"cx->pc += l->nops+1;\n",
	out);  

  return true;
}

cx_op_type(CX_OLAMBDA, {
    type.scan = true;
    type.eval = lambda_eval;
    type.emit = lambda_emit;
    type.emit_break = true;
  });

static void push_init(struct cx_op *op, struct cx_tok *tok) {
  cx_copy(&op->as_push.value, &tok->as_box);
}

static void push_deinit(struct cx_op *op) {
  cx_box_deinit(&op->as_push.value);
}

static bool push_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)),  &op->as_push.value);
  return true;
}

static bool push_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  cx_box_emit(&op->as_push.value, out);
  fputs("cx->pc++;\n", out);
  return true;
}

cx_op_type(CX_OPUSH, {
    type.scan = true;
    type.init = push_init;
    type.deinit = push_deinit;
    type.eval = push_eval;
    type.emit = push_emit;
  });

static bool putargs_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;
  struct cx_scope *ds = cx_scope(cx, 0), *ss = ds->stack.count ? ds : cx_scope(cx, 1);

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    struct cx_box *src = cx_test(cx_pop(ss, false));

    if (a->id) {
      *cx_put_var(ds, a->sym_id, true) = *src;
    } else {
      cx_box_deinit(src);
    }
  }

  return true;
}

static bool putargs_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;

  fputs("struct cx_scope\n"
	"*ds = cx_scope(cx, 0),\n"
	"*ss = ds->stack.count ? ds : cx_scope(cx, 1);\n",
	out);

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    if (a->id) {
      fprintf(out,
	      "  *cx_put_var(ds, sym%zd, true) = *cx_test(cx_pop(ss, false));\n",
	      a->sym_id.tag);
    } else {
      fputs("  cx_box_deinit(cx_test(cx_pop(ss, false)));\n", out);
    }
  }
  
  fputs("cx->pc++;\n", out);
  return true;
}

static struct cx_func *putargs_emit_func(struct cx_op *op) {
  return op->as_putargs.imp->func;
}

static struct cx_fimp *putargs_emit_fimp(struct cx_op *op) {
  return op->as_putargs.imp;
}

static void putargs_emit_syms(struct cx_op *op, struct cx_vec *out) {
  struct cx_fimp *imp = op->as_putargs.imp;

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    *(struct cx_sym *)cx_vec_push(out) = a->sym_id;
  }
}

cx_op_type(CX_OPUTARGS, {
    type.eval = putargs_eval;
    type.emit = putargs_emit;
    type.emit_func = putargs_emit_func;
    type.emit_fimp = putargs_emit_fimp;
    type.emit_syms = putargs_emit_syms;
  });

static bool putvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *src = cx_pop(s, false);
  
  if (!src) { return false; }

  if (op->as_putvar.type && !cx_is(src->type, op->as_putvar.type)) {
    cx_error(cx, op->row, op->col,
	     "Expected type %s, actual: %s",
	     op->as_putvar.type->id, src->type->id);

    return false;
  }
  
  struct cx_box *dst = cx_put_var(s, op->as_putvar.id, true);

  if (!dst) { return false; }
  *dst = *src;
  return true;
}

cx_op_type(CX_OPUTVAR, {
    type.eval = putvar_eval;
  });

bool cx_recall_scan(struct cx_scan *scan) {
  struct cx_fimp *imp = scan->as_recall.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }
  
  cx->pc = scan->as_recall.pc+1;
  return true;
}

void cx_oreturn_recall(struct cx_call *call, size_t pc, struct cx *cx) {
  struct cx_fimp *imp = call->target;
  call->recalls--;
  struct cx_scan *scan = cx_scan(cx_scope(cx, 0), imp->func, cx_recall_scan);
  scan->as_recall.imp = imp;
  scan->as_recall.pc = pc;
}

void cx_oreturn_end(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  cx_vec_clear(&scope->stack);

  struct cx_call *call = cx_vec_pop(&cx->calls);
  
  if (call->return_pc > -1) {
    cx->pc = call->return_pc;
  } else {
    cx->stop = true;
  }

  cx_call_deinit(call);
  cx_end(cx);
  cx->scan_level--;
}

static bool return_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;
  struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

  if (call->recalls) {
    cx_oreturn_recall(call, op->as_return.pc, cx);
  } else {
    struct cx_scope *ss = cx_scope(cx, 0);

    if (ss->stack.count > imp->rets.count) {
      cx_error(cx, cx->row, cx->col, "Stack not empty on return");
      return false;
    }
    
    if (ss->stack.count < imp->rets.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      return false;
    }

    if (imp->rets.count) {
      struct cx_scope *ds = cx_scope(cx, 1);
      cx_vec_grow(&ds->stack, ds->stack.count+imp->rets.count);
      size_t i = 0;
      struct cx_func_ret *r = cx_vec_start(&imp->rets);
      
      for (struct cx_box *v = cx_vec_start(&ss->stack);
	   i < ss->stack.count;
	   i++, v++, r++) {
	if (ss->safe) {
	  struct cx_type *t = r->type;
	  
	  if (!r->type) {
	    struct cx_func_arg *a = cx_vec_get(&imp->args, r->narg);
	    struct cx_box *av = cx_test(cx_get_var(ss, a->sym_id, false));
	    t = av->type;
	  }
	  
	  if (!cx_is(v->type, t)) {
	    cx_error(cx, cx->row, cx->col,
		     "Invalid return type.\nExpected %s, actual: %s",
		     t->id, v->type->id);
	    
	    return false;
	  }
	}
	
	*(struct cx_box *)cx_vec_push(&ds->stack) = *v;
      }    
    }

    cx_oreturn_end(ss);
  }
  
  return true;
}

static bool return_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;

  fprintf(out,
	  "struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));\n\n"
	  "if (call->recalls) {\n"
	  "  cx_oreturn_recall(call, %zd, cx);\n"
	  "} else {\n"
	  "  struct cx_scope *ss = cx_scope(cx, 0);\n\n"
	  "if (ss->stack.count > %zd) {\n"
	  "  cx_error(cx, cx->row, cx->col, \"Stack not empty on return\");\n"
	  "  return false;\n"
	  "}\n\n"
	  "if (ss->stack.count < %zd) {\n"
	  "  cx_error(cx, cx->row, cx->col, \"Not enough return values on stack\");\n"
	  "  return false;\n"
	  "}\n\n",
	  op->as_return.pc, imp->rets.count, imp->rets.count);
  
  if (imp->rets.count) {
    fputs("struct cx_scope *ds = cx_scope(cx, 1);\n", out);
    fprintf(out, "cx_vec_grow(&ds->stack, ds->stack.count+%zd);\n", imp->rets.count);
    fputs("struct cx_box *v = cx_vec_start(&ss->stack);\n\n", out);
    
    for (struct cx_func_ret *r = cx_vec_start(&imp->rets);
	 r != cx_vec_end(&imp->rets);
	 r++) {
      fputs("  if (ss->safe) {\n"
	    "    struct cx_type *t = NULL;\n",
	    out);
      
      if (r->type) {
	fprintf(out, "    t = type%zd;\n", r->type->tag);
      } else {
	fprintf(out,
		"    struct cx_func_arg *a = cx_vec_get(&fimp%zd_%zd->args, %d);\n"
		"    struct cx_box *av = cx_test(cx_get_var(ss, a->sym_id, false));\n"
		"    t = av->type;\n",
		imp->func->tag, imp->idx, r->narg);
      }
      
      fputs("    if (!cx_is(v->type, t)) {\n"
	    "      cx_error(cx, cx->row, cx->col,\n"
	    "               \"Invalid return type.\\n\"\n"
            "               \"Expected %s, actual: %s\",\n"
	    "               t->id, v->type->id);\n"
	    "      return false;\n"
	    "    }\n"
	    "  }\n\n"
	    "  *(struct cx_box *)cx_vec_push(&ds->stack) = *v;\n"
	    "  v++;\n\n",
	    out);
    }
  } 
  
  fputs("  cx_oreturn_end(ss);\n"
	"}\n",
	out);
  
  return true;  
}

static struct cx_func *return_emit_func(struct cx_op *op) {
  return op->as_return.imp->func;
}

static struct cx_fimp *return_emit_fimp(struct cx_op *op) {
  return op->as_return.imp;
}

static void return_emit_types(struct cx_op *op, struct cx_vec *out) {
  struct cx_fimp *imp = op->as_return.imp;
  
  for (struct cx_func_ret *r = cx_vec_start(&imp->rets);
       r != cx_vec_end(&imp->rets);
       r++) {
    if (r->type) { *(struct cx_type **)cx_vec_push(out) = r->type; }
  }
}

cx_op_type(CX_ORETURN, {
    type.scan = true;
    type.eval = return_eval;
    type.emit = return_emit;
    type.emit_func = return_emit_func;
    type.emit_fimp = return_emit_fimp;
    type.emit_types = return_emit_types;
    type.emit_break = true;
  });

static bool stash_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_vect *out = cx_vect_new();

  struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;

  if (c && c->offs) {
    for (struct cx_box *v = cx_vec_get(&s->stack, c->offs);
	 v != cx_vec_end(&s->stack);
	 v++) {
      *(struct cx_box *)cx_vec_push(&out->imp) = *v;
    }

    s->stack.count = c->offs;
  } else {
    out->imp = s->stack;
    cx_vec_init(&s->stack, sizeof(struct cx_box));
  }
  
  cx_box_init(cx_push(s), s->cx->vect_type)->as_ptr = out;
  return true;
}

cx_op_type(CX_OSTASH, {
    type.scan = true;
    type.eval = stash_eval;
  });

static bool stop_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->stop = true;
  return true;
}

static bool stop_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  fputs("cx->stop = true;\n"
	"cx->pc++;\n",
	out);
  
  return true;
}

cx_op_type(CX_OSTOP, {
    type.eval = stop_eval;
    type.emit = stop_emit;
    type.emit_break = true;
  });

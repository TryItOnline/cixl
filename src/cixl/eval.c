#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/eval.h"
#include "cixl/error.h"
#include "cixl/op.h"
#include "cixl/parse.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/types/func.h"
#include "cixl/types/lambda.h"
#include "cixl/util.h"
#include "cixl/vec.h"

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_str(cx, in, &toks)) { goto exit1; }

  if (!toks.count) {
    ok = true;
    goto exit1;
  }

  struct cx_bin *bin = cx_bin_new();
  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin)) { goto exit2; }
  if (!cx_eval(bin, 0, cx)) { goto exit2; }
  ok = true;
 exit2:
  cx_bin_deref(bin);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *args) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));
  bool ok = false;
  
  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *id = t->as_ptr;

      if (id[0] == 'T' && isdigit(id[1])) {
	int i = strtoimax(id+1, NULL, 10);

	if (i >= args->count || (!i && id[1] != '0')) {
	  cx_error(cx, t->row, t->col, "Invalid arg type: %s", t->as_ptr);
	  goto exit;
	}

	struct cx_func_arg *a = cx_vec_get(args, i);

	if (!a->type && a->narg == -1) {
	  cx_error(cx, t->row, t->col, "Value arg referred by index: %d", i);
	  goto exit;
	}
	
	if (!tmp_ids.count) {
	  cx_error(cx, t->row, t->col, "Missing args for type: %s", t->as_ptr);
	  goto exit;
	}
	
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  *(struct cx_func_arg *)cx_vec_push(args) = cx_narg(id->as_ptr, i);      
	}

	cx_vec_clear(&tmp_ids);
      } else {
	*(struct cx_tok *)cx_vec_push(&tmp_ids) = *t;
      }
    } else if (t->type == CX_TLITERAL()) {
      *(struct cx_func_arg *)cx_vec_push(args) = cx_varg(&t->as_box);        
    } else if (t->type == CX_TTYPE()) {
      struct cx_type *type = t->as_ptr;

      if (tmp_ids.count) {
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  *(struct cx_func_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, type);      
	}
      
	cx_vec_clear(&tmp_ids);
      } else {
	struct cx_box box;
	cx_box_init(&box, cx->meta_type)->as_ptr = type;
	*(struct cx_func_arg *)cx_vec_push(args) = cx_varg(&box);
      }
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      goto exit;
    }
  }

  if (tmp_ids.count) {
    struct cx_tok *t = cx_vec_get(&tmp_ids, 0);
    cx_error(cx, t->row, t->col, "Missing type for id: %s", t->as_ptr);
  }
  
  ok = true;
 exit:
  cx_vec_deinit(&tmp_ids);
  return ok;
}

bool cx_eval_rets(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *args,
		  struct cx_vec *rets) {
  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *id = t->as_ptr;

      if (id[0] == 'T' && isdigit(id[1])) {
	int i = strtoimax(id+1, NULL, 10);

	if (i >= args->count || (!i && id[1] != '0')) {
	  cx_error(cx, t->row, t->col, "Invalid ret: %s", t->as_ptr);
	  return false;
	}

	struct cx_func_arg *a = cx_vec_get(args, i);

	if (!a->type && a->narg == -1) {
	  cx_error(cx, t->row, t->col, "Value arg referred by index: %d", i);
	  return false;
	}
	
	*(struct cx_func_ret *)cx_vec_push(rets) = cx_nret(i);      
      } else {
	  cx_error(cx, t->row, t->col, "Invalid ret: %s", t->as_ptr);
	  return false;	
      }
    } else if (t->type == CX_TTYPE()) {
      *(struct cx_func_ret *)cx_vec_push(rets) = cx_ret(t->as_ptr);      
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      return false;
    }
  }

  return true;
}

#include <stdlib.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/macro.h"
#include "cixl/tok.h"
#include "cixl/vec.h"

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   enum cx_tok_type type,
			   void *data,
			   int row, int col) {
  tok->type = type;
  tok->data = data;
  tok->row = row;
  tok->col = col;
  return tok;
}

struct cx_tok *cx_tok_deinit(struct cx_tok *tok) {
  switch (tok->type) {
  case CX_TGROUP:
  case CX_TLAMBDA: {
    struct cx_vec *body = tok->data;
    cx_do_vec(body, struct cx_tok, t) { cx_tok_deinit(t); }
    free(cx_vec_deinit(body));
    break;
  }
  case CX_TID:
    free(tok->data);
    break;
  case CX_TLITERAL:
    free(cx_box_deinit(tok->data));
    break;
  case CX_TMACRO:
    cx_macro_eval_unref(tok->data);
    break;
  default:
    break;
  }
   
  return tok;
}

void cx_tok_copy(struct cx_tok *dst, struct cx_tok *src) {
  dst->row = src->row;
  dst->col = src->col;
  dst->type = src->type;
  
  switch (src->type) {
  case CX_TGROUP:
  case CX_TLAMBDA: {
    struct cx_vec *body = cx_vec_new(sizeof(struct cx_tok));
    
    cx_do_vec((struct cx_vec *)src->data, struct cx_tok, t) {
      cx_tok_copy(cx_vec_push(body), t);
    }

    dst->data = body;
    break;
  }
  case CX_TID:
    dst->data = strdup(src->data);
    break;
  case CX_TLITERAL:
    dst->data = cx_copy(malloc(sizeof(struct cx_box)), src->data);
    break;
  case CX_TMACRO:
    dst->data = cx_macro_eval_ref(src->data);
    break;
  default:
    dst->data = src->data;
    break;
  }
}

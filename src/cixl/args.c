#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/args.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/tok.h"
#include "cixl/type.h"

struct cx_arg *cx_arg_deinit(struct cx_arg *arg) {
  if (arg->id) { free(arg->id); }
  if (!arg->type && arg->narg == -1) { cx_box_deinit(&arg->value); }
  return arg;
}

struct cx_arg cx_arg(const char *id, struct cx_type *type) {
  return (struct cx_arg) { .id = id ? strdup(id) : NULL, .type = type };
}

struct cx_arg cx_varg(struct cx_box *value) {
  struct cx_arg arg = { .id = NULL, .type = NULL, .narg = -1};
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_arg cx_narg(const char *id, int n) {
  return (struct cx_arg) { .id = id ? strdup(id) : NULL, .type = NULL, .narg = n };
}

void cx_arg_print(struct cx_arg *a, FILE *out) {
  if (a->type) {
    fputs(a->type->id, out);
  } else if (a->narg != -1) {
    fprintf(out, "%d", a->narg);
  } else {
    cx_dump(&a->value, out);
  }
}

bool cx_parse_args(struct cx *cx,
		   struct cx_vec *toks,
		   struct cx_vec *args,
		   bool ret) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));
  bool ok = false;
  
  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *id = t->as_ptr;

      if (strncmp(id, "Arg", 3) == 0 && isdigit(id[3])) {
	int i = strtoimax(id+3, NULL, 10);

	if (tmp_ids.count) {
	  cx_do_vec(&tmp_ids, struct cx_tok, id) {
	    *(struct cx_arg *)cx_vec_push(args) = cx_narg(id->as_ptr, i);      
	  }
	  
	  cx_vec_clear(&tmp_ids);
	} else {
	  *(struct cx_arg *)cx_vec_push(args) = cx_narg(NULL, i);      
	}	
      } else {
	*(struct cx_tok *)cx_vec_push(&tmp_ids) = *t;
      }
    } else if (t->type == CX_TLITERAL()) {
      *(struct cx_arg *)cx_vec_push(args) = cx_varg(&t->as_box);        
    } else if (t->type == CX_TTYPE()) {
      struct cx_type *type = t->as_ptr;

      if (tmp_ids.count) {
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  *(struct cx_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, type);      
	}
      
	cx_vec_clear(&tmp_ids);
      } else if (ret) {
	*(struct cx_arg *)cx_vec_push(args) = cx_arg(NULL, type);
      } else {
	struct cx_box box;
	cx_box_init(&box, cx->meta_type)->as_ptr = type;
	*(struct cx_arg *)cx_vec_push(args) = cx_varg(&box);
      }
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      goto exit;
    }
  }

  cx_do_vec(&tmp_ids, struct cx_tok, id) {
    *(struct cx_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, cx->any_type);      
  }
  
  ok = true;
 exit:
  cx_vec_deinit(&tmp_ids);
  return ok;
}

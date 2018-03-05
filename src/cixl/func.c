#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/buf.h"
#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/scope.h"

static const void *get_imp_id(const void *value) {
  struct cx_fimp *const *imp = value;
  return &(*imp)->id;
}

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs) {
  func->cx = cx;
  func->id = strdup(id);
  func->emit_id = cx_emit_id("func", id);
  cx_set_init(&func->imps, sizeof(struct cx_fimp *), cx_cmp_cstr);
  func->imps.key = get_imp_id;
  func->nargs = nargs;
  func->nrefs = 1;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  free(func->emit_id);
  cx_do_set(&func->imps, struct cx_fimp *, i) { cx_fimp_deref(*i); }
  cx_set_deinit(&func->imps);
  return func; 
}

struct cx_func *cx_func_ref(struct cx_func *func) {
  func->nrefs++;
  return func;
}

void cx_func_deref(struct cx_func *func) {
  cx_test(func->nrefs);
  func->nrefs--;
  if (!func->nrefs) { free(cx_func_deinit(func)); }
}

bool cx_ensure_fimp(struct cx_func *func, struct cx_fimp *imp) {
  struct cx_fimp **ok = cx_set_get(&func->imps, &imp->id);

  if (ok) {
    if (*ok != imp) {
      cx_fimp_deref(*ok);
      *ok = cx_fimp_ref(imp);
      return true;
    }

    return false;
  }

  ok = cx_set_insert(&func->imps, &imp->id);
  *ok = cx_fimp_ref(imp);
  return true;
}

struct cx_fimp *cx_add_fimp(struct cx_func *func,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_arg));
  struct cx *cx = func->cx;
  
  struct cx_buf id;
  cx_buf_open(&id);
  
  if (nargs) {
    cx_vec_grow(&imp_args, nargs);

    for (int i=0; i < nargs; i++) {
      struct cx_arg *a = args+i;

      if (a->arg_type == CX_ARG && a->type == NULL) {
	cx_error(cx, cx->row, cx->col, "Unknown type for arg %d", i);
	return NULL;
      }
      
      if (a->arg_type == CX_NARG) {
	if (a->narg >= nargs) {
	  cx_error(cx, cx->row, cx->col, "Arg index out of bounds: %d", a->narg);
	  return NULL;
	}
	
	struct cx_arg *aa = args+a->narg;

	if (aa->arg_type == CX_VARG) {
	  cx_error(cx, cx->row, cx->col, "Value arg ref by index: %d",
		   a->narg);
	  return NULL;
	}
      }


      if (a->id) { a->sym_id = cx_sym(func->cx, a->id); }
      *(struct cx_arg *)cx_vec_push(&imp_args) = *a;
      if (i) { fputc(' ', id.stream); }
      cx_arg_print(a, id.stream);
    }
  }

  cx_buf_close(&id);
  struct cx_fimp **found = cx_set_get(&func->imps, &id.data);
  struct cx_fimp *imp = NULL;
  
  if (found) {
    imp = *found;
    cx_set_delete(&func->imps, &imp->id);
    cx_fimp_deref(imp);
  }

  imp = cx_fimp_init(malloc(sizeof(struct cx_fimp)),
		     func,
		     id.data);
  *(struct cx_fimp **)cx_set_insert(&func->imps, &id.data) = imp;
  imp->args = imp_args;

  if (nrets) {
    cx_vec_grow(&imp->rets, nrets);

    for (int i=0; i < nrets; i++) {
      struct cx_arg *r = rets+i;
      
      if (r->arg_type == CX_ARG && r->type == NULL) {
	cx_error(cx, cx->row, cx->col, "Unknown type for result %d", i);
	return NULL;
      }

      if (r->arg_type == CX_NARG) {
	if (r->narg >= nargs) {
	  cx_error(cx, cx->row, cx->col, "Arg index out of bounds: %d", r->narg);
	  return NULL;
	}
	
	struct cx_arg *a = args+r->narg;

	if (!a->id) {
	  cx_error(cx, cx->row, cx->col, "Anonymous arg ref by index: %d",
		   r->narg);
	  return NULL;
	}
      }
      
      if (r->id) { r->sym_id = cx_sym(func->cx, r->id); }
      *(struct cx_arg *)cx_vec_push(&imp->rets) = *r;
    }
  }

  return imp;
}

struct cx_fimp *cx_get_fimp(struct cx_func *func,
			    const char *id,
			    bool silent) {
  struct cx_fimp **imp = cx_set_get(&func->imps, &id);
  
  if (!imp && silent) {
    struct cx *cx = func->cx;
    cx_error(cx, cx->row, cx->col, "Fimp not found: %s<%s>", func->id, id);
    return NULL;
  }

  return *imp;
}

struct cx_fimp *cx_func_match(struct cx_func *func, struct cx_scope *scope) {
  struct cx_fimp *best_match = NULL;
  ssize_t best_score = -1;
  
  cx_do_set(&func->imps, struct cx_fimp *, i) {
    ssize_t s = cx_fimp_score(*i, scope);

    switch (s) {
    case -1:
      continue;
    case 0:
      return *i;
    }
    
    if (best_score == -1 || best_score > s) {
      best_match = *i;
      best_score = s;
    }
  }

  return best_match;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *func = value->as_ptr;
  struct cx_fimp *imp = cx_func_match(func, scope);

  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", func->id);
    return -1;
  }

  return cx_fimp_call(imp, scope);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return cx_call_iter_new(v);
}

static void write_imp(struct cx_box *value, FILE *out) {
  struct cx_func *func = value->as_ptr;
  fprintf(out, "&%s", func->id);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_func *func = value->as_ptr;
  fprintf(out, "Func(%s)", func->id);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_func *func = v->as_ptr;
  
  fprintf(out,
	  "cx_box_init(%s, cx->func_type)->as_ptr = %s;\n",
	  exp, func->emit_id);

  return true;
}

struct cx_type *cx_init_func_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Func", cx->any_type, cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;
  return t;
}

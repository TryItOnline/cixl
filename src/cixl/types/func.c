#include <stdlib.h>
#include <string.h>

#include "cixl/buf.h"
#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

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
  cx_set_init(&func->imp_lookup, sizeof(struct cx_fimp *), cx_cmp_cstr);
  func->imp_lookup.key = get_imp_id;
  cx_vec_init(&func->imps, sizeof(struct cx_fimp *));
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_do_set(&func->imp_lookup, struct cx_fimp *, i) { free(cx_fimp_deinit(*i)); }
  cx_set_deinit(&func->imp_lookup);
  cx_vec_deinit(&func->imps);
  return func; 
}

struct cx_func_arg *cx_func_arg_deinit(struct cx_func_arg *arg) {
  if (arg->id) { free(arg->id); }
  if (!arg->type && arg->narg == -1) { cx_box_deinit(&arg->value); }
  return arg;
}

struct cx_func_arg cx_arg(const char *id, struct cx_type *type) {
  return (struct cx_func_arg) { .id = strdup(id), .type = type };
}

struct cx_func_arg cx_varg(struct cx_box *value) {
  struct cx_func_arg arg = { .id = NULL, .type = NULL, .narg = -1};
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_func_arg cx_narg(const char *id, int n) {
  return (struct cx_func_arg) { .id = strdup(id), .type = NULL, .narg = n };
}

struct cx_func_ret cx_ret(struct cx_type *type) {
  return (struct cx_func_ret) { .type = type, .narg = -1 };
}

struct cx_func_ret cx_nret(int n) {
  return (struct cx_func_ret) { .type = NULL, .narg = n };
}

static void print_arg_id(struct cx_func_arg *a,
			  struct cx_vec *args,
			  FILE *out) {
  if (a->type) {
    fputs(a->type->id, out);
  } else if (a->narg != -1) {
    fprintf(out, "%d", a->narg);
  } else {
    cx_dump(&a->value, out);
  }
}

struct cx_fimp *cx_func_add_imp(struct cx_func *func,
				int nargs, struct cx_func_arg *args,
				int nrets, struct cx_func_ret *rets) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_func_arg));

  struct cx_buf id;
  cx_buf_open(&id);
  
  if (nargs) {
    cx_vec_grow(&imp_args, nargs);

    for (int i=0; i < nargs; i++) {
      struct cx_func_arg a = args[i];
      if (a.id) { a.sym_id = cx_sym(func->cx, a.id); }
      *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
      if (i) { fputc(' ', id.stream); }
      print_arg_id(&a, &imp_args, id.stream);
    }
  }

  cx_buf_close(&id);
  struct cx_fimp **found = cx_set_get(&func->imp_lookup, &id.data);
  struct cx_fimp *imp = NULL;
  
  if (found) {
    imp = *found;
    size_t i = imp->i;
    cx_fimp_deinit(imp);
    cx_fimp_init(imp, func, id.data, i);
  } else {
    imp = cx_fimp_init(malloc(sizeof(struct cx_fimp)),
		       func,
		       id.data,
		       func->imps.count);
    *(struct cx_fimp **)cx_set_insert(&func->imp_lookup, &id.data) = imp;
    *(struct cx_fimp **)cx_vec_push(&func->imps) = imp;
  }
  
  imp->args = imp_args;

  if (nrets) {
    cx_vec_grow(&imp->rets, nrets);

    for (int i=0; i < nrets; i++) {
      struct cx_func_ret r = rets[i];
      *(struct cx_func_ret *)cx_vec_push(&imp->rets) = r;
    }
  }

  return imp;
}

struct cx_fimp *cx_func_get_imp(struct cx_func *func,
				const char *id,
				bool silent) {
  struct cx_fimp **imp = cx_set_get(&func->imp_lookup, &id);
  
  if (!imp && silent) {
    struct cx *cx = func->cx;
    cx_error(cx, cx->row, cx->col, "Fimp not found: %s<%s>", func->id, id);
    return NULL;
  }

  return *imp;
}

struct cx_fimp *cx_func_match_imp(struct cx_func *func,
				  struct cx_scope *scope,
				  size_t offs) {
  if (offs >= func->imps.count) { return NULL; }

  for (struct cx_fimp **i = cx_vec_peek(&func->imps, offs);
       i >= (struct cx_fimp **)func->imps.items;
       i--) {
    if ((!scope->safe && i == cx_vec_start(&func->imps)) ||
	cx_fimp_match(*i, scope)) {
      return *i;
    }
  }

  return NULL;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *func = value->as_ptr;
  struct cx_fimp *imp = cx_func_match_imp(func, scope, 0);

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

struct cx_type *cx_init_func_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Func", cx->any_type, cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;

  return t;
}

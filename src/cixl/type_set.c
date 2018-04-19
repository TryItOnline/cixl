#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/parse.h"
#include "cixl/type_set.h"

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%s(", v->type->raw->id);
  struct cx_type *at = *(struct cx_type **)cx_test(cx_vec_start(&v->type->args));
  at->dump(v, out);
  fputc(')', out);
}

static struct cx_type *type_new_imp(struct cx_type *t,
				    const char *id,
				    int nargs, struct cx_type *args[]) {
  struct cx_type_set
    *ts = cx_baseof(t, struct cx_type_set, imp),
    *nts = cx_type_set_new(t->lib, id, false);

  nts->type_init = ts->type_init;

  cx_do_set(&ts->set, struct cx_type *, mt) {
    *(struct cx_type **)cx_vec_push(&nts->set.members) = *mt;
  }

  return &nts->imp;
}

static bool type_init_imp(struct cx_type *t,
			  int nargs, struct cx_type *args[]) {  
  cx_test(nargs);
  struct cx_type *at = args[0];
    
  t->eqval = at->eqval;
  t->equid = at->equid;
  t->cmp = at->cmp;
  t->ok = at->ok;
  t->call = at->call;
  t->copy = at->copy;
  t->clone = at->clone;
  t->iter = at->iter;
  t->write = at->write;
  t->print = at->print;
  t->emit = at->emit;
  t->deinit = at->deinit;
    
  t->dump = dump_imp;

  struct cx_type_set *ts = cx_baseof(t, struct cx_type_set, imp);
  if (ts->type_init) { return ts->type_init(t, nargs, args); }
  return true;
}
  
static void *type_deinit_imp(struct cx_type *t) {
  struct cx_type_set *ts = cx_baseof(t, struct cx_type_set, imp);
  cx_set_deinit(&ts->set);
  return ts;
}

struct cx_type_set *cx_type_set_new(struct cx_lib *lib,
				    const char *id,
				    bool raw) {
  struct cx_type_set *ts = malloc(sizeof(struct cx_type_set));
  
  const char *i = strchr(id, '<');
  
  if (i && raw) {
    char tid[i-id+1];
    strncpy(tid, id, i-id);
    tid[i-id] = 0;
    cx_type_init(&ts->imp, lib, tid);
  } else {
    cx_type_init(&ts->imp, lib, id);
  }
  
  cx_set_init(&ts->set, sizeof(struct cx_type *), cx_cmp_ptr);
  
  ts->type_init = NULL;
  ts->imp.type_new = type_new_imp;
  ts->imp.type_init = type_init_imp;
  ts->imp.type_deinit = type_deinit_imp;

  if (i) {
    i++;
    
    char args[strlen(i)+1];
    strcpy(args, i);
    char *j = args;

    while (true) {
      struct cx_type *tt = cx_parse_type_arg(lib->cx, &j);
      if (!tt) { break; }
      *(struct cx_type **)cx_vec_push(&ts->imp.args) = tt;
    }
  }
  
  return ts;
}

bool cx_type_id_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]) {
  struct cx_type *get_raw(int i) {
    return (i < t->raw->args.count)
      ? *(struct cx_type **)cx_vec_get(&t->raw->args, i)
      : NULL;
  }

  struct cx_type *get_arg(int i) {
    return (i < nargs) ? args[i] : NULL;
  }
  
  struct cx_type_set *ts = cx_baseof(t->raw, struct cx_type_set, imp);

  for (struct cx_type **m = cx_vec_start(&ts->set.members);
       m != cx_vec_end(&ts->set.members);
       m++) {
    if (*m != t->raw && *m != t) {
      struct cx_type *mt = cx_resolve_arg_refs(*m, get_raw, get_arg);
      if (mt && mt != *m) { cx_derive(mt, t); }
    }
  }

  return true;
}

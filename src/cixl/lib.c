#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/rec.h"
#include "cixl/parse.h"
#include "cixl/set.h"

struct cx_lib_init cx_lib_ptr(cx_lib_init_ptr_t ptr) {
  return (struct cx_lib_init){
    .ptr = ptr,
      .bin = NULL,
      .start_pc = 0,
      .nops = 0,
      .done = false };
}

struct cx_lib_init cx_lib_ops(struct cx_bin *bin, size_t start_pc, size_t nops) {
  return (struct cx_lib_init){
    .ptr = NULL,
      .bin = cx_bin_ref(bin),
      .start_pc = start_pc,
      .nops = nops,
      .done = false };
}

static const void *get_type_id(const void *value) {
  struct cx_type *const *type = value;
  return &(*type)->id;
}

static const void *get_pmacro_id(const void *value) {
  struct cx_pmacro *const *pmacro = value;
  return &(*pmacro)->id;
}

static const void *get_rmacro_id(const void *value) {
  struct cx_rmacro *const *rmacro = value;
  return &(*rmacro)->id;
}

static const void *get_func_id(const void *value) {
  struct cx_func *const *func = value;
  return &(*func)->id;
}

struct cx_lib *cx_lib_init(struct cx_lib *lib, struct cx *cx, struct cx_sym id) {
  lib->cx = cx;
  lib->id = id;
  lib->emit_id = cx_emit_id("lib", id.id);

  cx_vec_init(&lib->inits, sizeof(struct cx_lib_init));
  
  cx_set_init(&lib->types, sizeof(struct cx_type *), cx_cmp_cstr);
  lib->types.key = get_type_id;

  cx_set_init(&lib->pmacros, sizeof(struct cx_pmacro *), cx_cmp_cstr);
  lib->pmacros.key = get_pmacro_id;

  cx_set_init(&lib->rmacros, sizeof(struct cx_rmacro *), cx_cmp_cstr);
  lib->rmacros.key = get_rmacro_id;

  cx_set_init(&lib->funcs, sizeof(struct cx_func *), cx_cmp_cstr);
  lib->funcs.key = get_func_id;

  cx_env_init(&lib->consts, &cx->var_alloc);

  return lib;
}

static void deinit(struct cx_lib_init *i) {
  if (i->bin) { cx_bin_deref(i->bin); }
}

struct cx_lib *cx_lib_deinit(struct cx_lib *lib) {
  cx_env_deinit(&lib->consts);
  cx_set_deinit(&lib->funcs);
  cx_set_deinit(&lib->pmacros);
  cx_set_deinit(&lib->rmacros);
  cx_set_deinit(&lib->types);

  cx_do_vec(&lib->inits, struct cx_lib_init, i) { deinit(i); }
  cx_vec_deinit(&lib->inits);

  free(lib->emit_id);
  return lib;
}

void cx_lib_push_init(struct cx_lib *lib, struct cx_lib_init init) {
  *(struct cx_lib_init *)cx_vec_push(&lib->inits) = init;
}

struct cx_type *_cx_add_type(struct cx_lib *lib, const char *id, ...) {
  va_list parents;
  va_start(parents, id);				
  struct cx_type *t = cx_vadd_type(lib, id, parents);
  va_end(parents);					
  return t;
}

bool cx_lib_push_type(struct cx_lib *l, struct cx_type *t) {
  struct cx_type **p = cx_set_insert(&l->types, &t->id);

  if (!p) {
    cx_error(l->cx, l->cx->row, l->cx->col, "Duplicate type: '%s'", t->id);
    return false;
  }

  *p = t;

  *(struct cx_type **)cx_vec_push(&l->cx->types) = t;
  return true;
}

struct cx_type *cx_vadd_type(struct cx_lib *l, const char *id, va_list parents) {
  struct cx_type *t = NULL;
  t = cx_type_new(l, id);

  if (!cx_lib_push_type(l, t)) {
    free(t);
    return NULL;
  }
  
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_derive(t, pt); }
  return t;
}

struct cx_rec_type *cx_add_rec_type(struct cx_lib *lib, const char *id) {
  int id_len = strlen(id);
  char _id[id_len+1];
  strcpy(_id, id);
  
  char *ie = strchr(_id, '<');

  if (ie) {
    *ie = 0;
    ie++;
  } else {
    ie = _id+id_len;
  }
  
  char *ip = _id;
  
  struct cx *cx = lib->cx;
  struct cx_type **found = cx_set_get(&lib->types, &ip);
  if (found) { return NULL; }
  
  if (found) {
    struct cx_rec_type *t = cx_baseof(*found, struct cx_rec_type, imp);
    cx_rec_type_reinit(t);
    return t;
  }

  struct cx_rec_type *t = cx_rec_type_new(lib, ip);
  *(struct cx_type **)cx_vec_push(&cx->types) = &t->imp;
  *(struct cx_type **)cx_test(cx_set_insert(&lib->types, &ip)) = &t->imp;

  while (true) {
    struct cx_type *tt = cx_parse_type_arg(cx, &ie);
    if (!tt) { break; }
    cx_type_push_args(&t->imp, tt);
  }

  return t;
}

static struct cx_type *lib_get_type(struct cx_lib **lib, const char *id) {
  struct cx *cx = (*lib)->cx;
  struct cx_type **t = cx_set_get(&(*lib)->types, &id);

  if (!t && lib > (struct cx_lib **)cx->libs.items) {
    return lib_get_type(lib-1, id);
  }

  return t ? *t : NULL;
}

struct cx_type *cx_lib_get_type(struct cx_lib *lib, const char *id, bool silent) {
  struct cx *cx = lib->cx;
  struct cx_type **t = cx_set_get(&lib->types, &id);
  if (!t && !silent) { cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id); }
  return t ? *t : NULL;
}

struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent) {
  struct cx_type *t = NULL;
  const char *i = strchr(id, '<');

  if (i) {
    const char *j = i;
    i++;
    
    char args[strlen(i)+1];
    strcpy(args, i);
    char *k = args;

    struct cx_vec types;
    cx_vec_init(&types, sizeof(struct cx_type *));

    while (true) {
      struct cx_type *tt = cx_parse_type_arg(cx, &k);
      if (!tt) { break; }
      *(struct cx_type **)cx_vec_push(&types) = tt;
    }

    char tid[j-id+1];
    strncpy(tid, id, j-id);
    tid[j-id] = 0;

    t = cx_get_type(cx, tid, true);
    if (!t && strncmp(tid, "Arg", 3) == 0) { t = &cx_add_arg_ref(cx, tid)->imp; }
    if (t) { t = cx_type_vget(t, types.count, (struct cx_type **)types.items); }
    cx_vec_deinit(&types);
  } else {
    t = lib_get_type(cx->lib, id);
    if (!t && strncmp(id, "Arg", 3) == 0) { t = &cx_add_arg_ref(cx, id)->imp; }
  }
  
  if (!t && !silent) { cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id); }
  return t;
}

struct cx_pmacro *cx_add_pmacro(struct cx_lib *lib,
				const char *id,
				cx_pmacro_eval_t eval) {
  struct cx *cx= lib->cx;
  struct cx_pmacro **m = cx_test(cx_set_insert(&lib->pmacros, &id));

  if (!m) {
    cx_error(cx, cx->row, cx->col, "Duplicate pmacro: '%s'", id);
    return NULL;
  }

  *m = cx_pmacro_init(malloc(sizeof(struct cx_pmacro)), id, eval); 
  *(struct cx_pmacro **)cx_vec_push(&cx->pmacros) = *m;
  return *m;
}

static struct cx_pmacro *lib_get_pmacro(struct cx_lib **lib,
					const char *id,
					bool silent) {
  struct cx *cx = (*lib)->cx;
  struct cx_pmacro **m = cx_set_get(&(*lib)->pmacros, &id);

  if (!m && lib > (struct cx_lib **)cx->libs.items) {
    return lib_get_pmacro(lib-1, id, silent);
  }

  if (!m && !silent) { cx_error(cx, cx->row, cx->col, "Unknown pmacro: '%s'", id); }
  return m ? *m : NULL;
}

struct cx_pmacro *cx_get_pmacro(struct cx *cx, const char *id, bool silent) {
  return lib_get_pmacro(cx->lib, id, silent);
}

struct cx_rmacro *cx_add_rmacro(struct cx_lib *lib,
				const char *id,
				cx_rmacro_parse_t imp) {
  struct cx *cx= lib->cx;
  struct cx_rmacro **m = cx_test(cx_set_insert(&lib->rmacros, &id));

  if (!m) {
    cx_error(cx, cx->row, cx->col, "Duplicate rmacro: '%s'", id);
    return NULL;
  }

  *m = cx_rmacro_init(malloc(sizeof(struct cx_rmacro)), id, imp); 
  *(struct cx_rmacro **)cx_vec_push(&cx->rmacros) = *m;
  return *m;
}

static struct cx_rmacro *lib_get_rmacro(struct cx_lib **lib,
					const char *id,
					bool silent) {
  struct cx *cx = (*lib)->cx;
  struct cx_rmacro **m = cx_set_get(&(*lib)->rmacros, &id);

  if (!m && lib > (struct cx_lib **)cx->libs.items) {
    return lib_get_rmacro(lib-1, id, silent);
  }

  if (!m && !silent) { cx_error(cx, cx->row, cx->col, "Unknown rmacro: '%s'", id); }
  return m ? *m : NULL;
}

struct cx_rmacro *cx_get_rmacro(struct cx *cx, const char *id, bool silent) {
  return lib_get_rmacro(cx->lib, id, silent);
}

struct cx_fimp *cx_add_func(struct cx_lib *lib,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets) {
  struct cx *cx = lib->cx;
  struct cx_func **f = cx_set_get(&lib->funcs, &id);

  if (f) {
    if ((*f)->nargs != nargs) {
      cx_error(cx,
	       cx->row, cx->col,
	       "Wrong number of args for func '%s': %d/%d",
	       id, nargs, (*f)->nargs);
      return NULL;
    }
  } else {
    f = cx_set_insert(&lib->funcs, &id);
    *f = cx_func_init(malloc(sizeof(struct cx_func)), lib, id, nargs);
    *(struct cx_func **)cx_vec_push(&cx->funcs) = *f;
  }
  
  return cx_add_fimp(*f, nargs, args, nrets, rets);
}

struct cx_fimp *cx_add_cfunc(struct cx_lib *lib,
			     const char *id,
			     int nargs, struct cx_arg *args,
			     int nrets, struct cx_arg *rets,
			     cx_fimp_ptr_t ptr) {
  struct cx_fimp *imp = cx_add_func(lib, id, nargs, args, nrets, rets);
  if (imp) { imp->ptr = ptr; }
  return imp;
}

struct cx_fimp *cx_add_cxfunc(struct cx_lib *lib,
			      const char *id,
			      int nargs, struct cx_arg *args,
			      int nrets, struct cx_arg *rets,
			      const char *body) {
  struct cx_fimp *imp = cx_add_func(lib, id, nargs, args, nrets, rets);
  if (imp && strlen(body)) { cx_parse_str(lib->cx, body, &imp->toks, true); }
  return imp;
}

static struct cx_func *lib_get_func(struct cx_lib **lib,
				    const char *id,
				    bool silent) {
  struct cx *cx = (*lib)->cx;
  struct cx_func **f = cx_set_get(&(*lib)->funcs, &id);

  if (!f && lib > (struct cx_lib **)cx->libs.items) {
    return lib_get_func(lib-1, id, silent);
  }
  
  if (!f && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown func: '%s'", id);
  }

  return f ? *f : NULL;
}

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent) {
  return lib_get_func(cx->lib, id, silent);
}

static struct cx_box *lib_get_const(struct cx_lib **lib,
				    struct cx_sym id,
				    bool silent) {
  struct cx *cx = (*lib)->cx;
  struct cx_var *v = cx_env_get(&(*lib)->consts, id);

  if (!v) {
    if (lib > (struct cx_lib **)cx->libs.items) {
      return lib_get_const(lib-1, id, silent);
    }

    if (!silent) {
      cx_error(cx, cx->row, cx->col, "Unknown const: %s/%s", (*lib)->id.id, id.id);
    }
    
    return NULL;
  }

  return &v->value;
}

struct cx_box *cx_get_const(struct cx *cx, struct cx_sym id, bool silent) {
  return lib_get_const(cx->lib, id, silent);
}

struct cx_box *cx_lib_get_const(struct cx_lib *lib, struct cx_sym id, bool silent) {
  struct cx *cx = lib->cx;
  struct cx_var *v = cx_env_get(&lib->consts, id);

  if (!v) {
    if (!silent) {
      cx_error(cx, cx->row, cx->col, "Unknown const: %s/%s", lib->id.id, id.id);
    }
    
    return NULL;
  }

  return &v->value;
}

struct cx_box *cx_put_const(struct cx_lib *lib, struct cx_sym id, bool force) {
  struct cx_var *v = cx_env_get(&lib->consts, id);

  if (v) {
    if (!force) { return NULL; }
    cx_box_deinit(&v->value);
    return &v->value;
  }

  return cx_env_put(&lib->consts, id);
}

static void use_type(struct cx_type *t, struct cx_lib *dst) {
  struct cx_type **ok = cx_set_insert(&dst->types, &t->id);
  if (ok) { *ok = t; }
 }

static void use_pmacro(struct cx_pmacro *m, struct cx_lib *dst) {
  struct cx_pmacro **ok = cx_set_insert(&dst->pmacros, &m->id);
  if (ok) { *ok = m; }
}

static void use_rmacro(struct cx_rmacro *m, struct cx_lib *dst) {
  struct cx_rmacro **ok = cx_set_insert(&dst->rmacros, &m->id);
  if (ok) { *ok = m; }
}

static bool use_func(struct cx_func *f, struct cx_lib *dst) {
  struct cx_func **ok = cx_set_get(&dst->funcs, &f->id);

  if (ok) {
    if ((*ok)->nargs != f->nargs) {
      struct cx *cx = f->lib->cx;
      cx_error(cx, cx->row, cx->col, "Wrong arity (%d/%d) for func: %s/%s",
	       f->nargs, (*ok)->nargs, dst->id.id, f->id);
      return false;
    }
    
    cx_do_set(&f->imps, struct cx_fimp *, i) {
      cx_ensure_fimp(*ok, *i);
    }
  } else {
    ok = cx_set_insert(&dst->funcs, &f->id);
    *ok = f;
  }
  
  return true;
}

static void use_const(struct cx_var *v, struct cx_lib *dst) {
  struct cx_var *prev = cx_env_get(&dst->consts, v->id);
  if (!prev) { cx_copy(cx_env_put(&dst->consts, v->id), &v->value); }
}

static bool use_all(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  
  cx_do_set(&lib->types, struct cx_type *, t) { use_type(*t, *cx->lib); }
  cx_do_set(&lib->pmacros, struct cx_pmacro *, m) { use_pmacro(*m, *cx->lib); }
  cx_do_set(&lib->rmacros, struct cx_rmacro *, m) { use_rmacro(*m, *cx->lib); }
  
  cx_do_set(&lib->funcs, struct cx_func *, f) {
    if (!use_func(*f, *cx->lib)) { return false; }
  }

  cx_do_env(&lib->consts, v) { use_const(v, *cx->lib); }
  return true;
}

static bool use_id(struct cx_lib *lib, const char *id) {
  struct cx *cx = lib->cx;

  if (id[0] == '#') {
    struct cx_var *v = cx_env_get(&lib->consts, cx_sym(cx, id+1));

    if (v) {
      use_const(v, *cx->lib);
      return true;
    }
  } else if (isupper(id[0])) {
    struct cx_type **t = cx_set_get(&lib->types, &id);
    
    if (t) {
      use_type(*t, *cx->lib);
      return true;
    }
  } else {
    struct cx_pmacro **pm = cx_set_get(&lib->pmacros, &id);

    if (pm) {
      use_pmacro(*pm, *cx->lib);
      return true;
    }

    struct cx_rmacro **rm = cx_set_get(&lib->rmacros, &id);

    if (rm) {
      use_rmacro(*rm, *cx->lib);
      return true;
    }
    
    struct cx_func **f = cx_set_get(&lib->funcs, &id);
    if (f) { return use_func(*f, *cx->lib); }
  }

  cx_error(cx, cx->row, cx->col, "%s/%s not found", lib->id.id, id);
  return false;
}

bool cx_lib_vuse(struct cx_lib *lib, unsigned int nids, const char **ids) {
  struct cx *cx = lib->cx;
  bool ok = true;
  
  if (lib->inits.count) {
    cx_push_lib(cx, lib);
    
    cx_do_vec(&lib->inits, struct cx_lib_init, i) {
      if (i->done) { continue; }
      
      if (i->ptr) {
	ok = i->ptr(lib) && ok;
      } else {
	ok = cx_eval(i->bin, i->start_pc, i->start_pc+i->nops, cx) && ok;
      }
      
      i->done = true;
    }
    
    cx_pop_lib(cx);
  }

  if (nids) {
    for (unsigned int i = 0; i < nids; i++) {
      ok = use_id(lib, ids[i]) && ok;
    }

    return true;
  }
  
  return use_all(lib) && ok;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_lib == y->as_lib;
}

static void write_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "`%s get-lib", v->as_lib->id.id);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Lib(%s)", v->as_lib->id.id);
}

static void print_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_lib->id.id, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->lib_type)->as_lib = %s();\n",
	  exp, v->as_lib->emit_id);
  
  return true;
}

bool cx_vuse(struct cx *cx,
	     const char *lib_id,
	     unsigned int nids, const char *ids[]) {
  struct cx_sym lid = cx_sym(cx, lib_id);
  struct cx_lib **ok = cx_set_get(&cx->lib_lookup, &lid);

  if (!ok) {
    cx_error(cx, cx->row, cx->col, "Lib not found: %s", lib_id);
    return false;
  }

  return cx_lib_vuse(*ok, nids, ids);
}

struct cx_type *cx_init_lib_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Lib", lib->cx->any_type);
  t->equid = equid_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->emit = emit_imp;
  return t;
}

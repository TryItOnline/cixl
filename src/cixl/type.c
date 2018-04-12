#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/type.h"

struct cx_type *cx_type_new(struct cx_lib *lib, const char *id) {
  return cx_type_init(malloc(sizeof(struct cx_type)), lib, id);
}

struct cx_type *cx_type_init(struct cx_type *type,
			     struct cx_lib *lib,
			     const char *id) {
  type->lib = lib;
  type->id = strdup(id);
  type->emit_id = cx_emit_id("type", id);
  type->tag = lib->cx->next_type_tag++;
  type->level = 0;
  type->trait = false;
  type->root = type;

  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  cx_set_init(&type->children, sizeof(struct cx_type *), cx_cmp_ptr);

  cx_vec_init(&type->is, sizeof(bool));
  *(bool *)cx_vec_put(&type->is, type->tag) = true;

  cx_vec_init(&type->args, sizeof(struct cx_type *));
  
  type->new = NULL;
  type->eqval = NULL;
  type->equid = NULL;
  type->cmp = NULL;
  type->ok = NULL;
  type->call = NULL;
  type->copy = NULL;
  type->clone = NULL;
  type->iter = NULL;
  type->write = NULL;
  type->dump = NULL;
  type->print = NULL;
  type->emit = NULL;
  type->deinit = NULL;

  type->type_clone = NULL;
  type->type_deinit = NULL;
  return type;
}

struct cx_type *cx_type_reinit(struct cx_type *type) {
  type->level = 0;
  
  for (size_t i=0; i < type->is.count; i++) {
    if (i != type->tag) { *(bool *)cx_vec_get(&type->is, i) = false; }
  }

  cx_vec_clear(&type->args);
  
  cx_do_set(&type->parents, struct cx_type *, t) {
    cx_set_delete(&(*t)->children, t);
  }
  
  cx_set_clear(&type->parents);

  cx_do_set(&type->children, struct cx_type *, t) {
    cx_set_delete(&(*t)->parents, &type);
    (*t)->level = 0;
    
    cx_do_set(&(*t)->parents, struct cx_type *, pt) {
      (*t)->level = cx_max((*t)->level, (*pt)->level+1);
    }
    
    *(bool *)cx_vec_put(&(*t)->is, type->tag) = false;
  }
  
  cx_set_clear(&type->children);
  return type;
}

void *cx_type_deinit(struct cx_type *type) {
  void *ptr = type;
  if (type->type_deinit) { ptr = type->type_deinit(type); }  
  cx_set_deinit(&type->parents);
  cx_set_deinit(&type->children);
  cx_vec_deinit(&type->is);
  cx_vec_deinit(&type->args);
  free(type->id);
  free(type->emit_id);
  return ptr;  
}

void cx_type_vpush_args(struct cx_type *t, int nargs, struct cx_type *args[]) {
  for (int i=0; i < nargs; i++) {
    *(struct cx_type **)cx_vec_push(&t->args) = args[i];
  }
}

struct cx_type *cx_type_vclone(struct cx_type *t,
			       const char *id,
			       int nargs, struct cx_type *args[]) {  
  struct cx_type *tt = t->type_clone
    ? t->type_clone(t, id)
    : cx_type_new(t->lib, id);
  
  tt->root = t->root;
  tt->new = t->new;
  tt->eqval = t->eqval;
  tt->equid = t->equid;
  tt->cmp = t->cmp;
  tt->ok = t->ok;
  tt->call = t->call;
  tt->copy = t->copy;
  tt->clone = t->clone;
  tt->iter = t->iter;
  tt->write = t->write;
  tt->dump = t->dump;
  tt->print = t->print;
  tt->emit = t->emit;
  tt->deinit = t->deinit;
  tt->type_clone = t->type_clone;
  tt->type_deinit = t->type_deinit;

  cx_derive(tt, t);
  cx_type_vpush_args(tt, nargs, args);
  cx_lib_push_type(t->lib, tt);
  return tt;
}

struct cx_type *cx_type_vget(struct cx_type *t, int nargs, struct cx_type *args[]) {
  struct cx *cx = t->lib->cx;

  if (nargs != t->args.count) {
    cx_error(cx, cx->row, cx->col,
	     "Wrong number of args for type %s: %d", t->id, nargs);
    
    return NULL;
  }
  
  for (struct cx_type
	 **i = cx_vec_start(&t->args),
	 **j = args;
       i != cx_vec_end(&t->args);
       i++, j++) {
    if (!cx_is(*j, *i)) {
      cx_error(cx, cx->row, cx->col,
	       "Expected type arg %s, actual: %s", (*i)->id, (*j)->id);
      
      return NULL;
    }
  }

  struct cx_mfile id;
  cx_mfile_open(&id);
  fputs(t->root->id, id.stream);
  fputc('<', id.stream);
  char sep = 0;
  
  for (int i=0; i < nargs; i++) {
    if (sep) { fputc(sep, id.stream); }
    fputs(args[i]->id, id.stream);
    sep = ' ';
  }
  
  fputc('>', id.stream);
  cx_mfile_close(&id);

  struct cx_type *tt = cx_lib_get_type(t->lib, id.data, true);

  if (tt) {
    free(id.data);
    return tt;
  }
    
  tt = cx_type_vclone(t, id.data, nargs, args);
  free(id.data);
  return tt;
}

static void derive(struct cx_type *child, struct cx_type *parent) {
  *(bool *)cx_vec_put(&child->is, parent->tag) = true;
  child->level = cx_max(child->level, parent->level+1);
  
  cx_do_set(&parent->parents, struct cx_type *, t) { derive(child, *t); }
  cx_do_set(&child->children, struct cx_type *, t) { derive(*t, parent); }
}

void cx_derive(struct cx_type *child, struct cx_type *parent) {
  struct cx_type **tp = cx_set_insert(&child->parents, &parent);
  if (tp) { *tp = parent; }
  
  tp = cx_set_insert(&parent->children, &child);
  if (tp) { *tp = child; }

  derive(child, parent);
}

bool cx_is(struct cx_type *child, struct cx_type *parent) {  
  bool ok = (parent->tag < child->is.count)
    ? *(bool *)cx_vec_get(&child->is, parent->tag)
    : false;

  if (!ok) { return false; }
  
  for (struct cx_type
	 **i = cx_vec_start(&child->args),
	 **j = cx_vec_start(&parent->args);
       i != cx_vec_end(&child->args) &&
	 j != cx_vec_end(&parent->args);
       i++, j++) {
    if (!cx_is(*i, *j)) { return false; }
  }

  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_type *type = value->as_ptr;
  fputs(type->id, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_type *t = v->as_ptr;
  
  fprintf(out,
	  "cx_box_init(%s, cx->meta_type)->as_ptr = %s();\n",
	  exp, t->emit_id);
  
  return true;
}

struct cx_type *cx_init_meta_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Type", lib->cx->any_type);
  t->equid = equid_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;
  return t;
}

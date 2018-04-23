#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/scope.h"
#include "cixl/str.h"

struct char_iter {
  struct cx_iter iter;
  struct cx_str *str;
  char *ptr;
};

static bool char_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  
  if (it->ptr == it->str->data+it->str->len) {
    iter->done = true;
    return false;
  }
  
  unsigned char c = *it->ptr;
  
  cx_box_init(out, scope->cx->char_type)->as_char = c;
  it->ptr++;
  return true;
}

static void *char_deinit(struct cx_iter *iter) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  cx_str_deref(it->str);
  return it;
}

static cx_iter_type(char_iter, {
    type.next = char_next;
    type.deinit = char_deinit;
  });

static struct cx_iter *char_iter_new(struct cx_str *str) {
  struct char_iter *it = malloc(sizeof(struct char_iter));
  cx_iter_init(&it->iter, char_iter());
  it->str = cx_str_ref(str);
  it->ptr = str->data;
  return &it->iter;
}

struct cx_str *cx_str_new(const char *data, ssize_t len) {
  if (len == -1) { len = strlen(data); }
  struct cx_str *str = malloc(sizeof(struct cx_str)+len+1);
  if (data) { memcpy(str->data, data, len); }
  str->data[len] = 0;
  str->len = len;
  str->nrefs = 1;
  return str;
}

struct cx_str *cx_str_ref(struct cx_str *str) {
  str->nrefs++;
  return str;
}

void cx_str_deref(struct cx_str *str) {
  cx_test(str->nrefs);
  str->nrefs--;
  if (!str->nrefs) { free(str); }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_str == y->as_str;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_str *xs = x->as_str, *ys = y->as_str;
  if (xs->len != ys->len) { return false; }
  return strncmp(xs->data, ys->data, xs->len) == 0;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  struct cx_str *xs = x->as_str, *ys = y->as_str;
  int cmp = strncmp(xs->data, ys->data, cx_min(xs->len, ys->len));
  if (!cmp) { return cx_cmp_size(&xs->len, &ys->len); }
  if (cmp < 0) { return CX_CMP_LT; }
  return (cmp > 0) ? CX_CMP_GT : CX_CMP_EQ;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_str->len;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_str = cx_str_ref(src->as_str);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_str = cx_str_new(src->as_str->data, src->as_str->len);
}

static void iter_imp(struct cx_box *in, struct cx_box *out) {
  struct cx *cx = in->type->lib->cx;
  cx_box_init(out, cx_type_get(cx->iter_type, cx->char_type))->as_iter =
    char_iter_new(in->as_str);
}

void cx_cstr_encode(const char *in, size_t len, FILE *out) {
  for (const char *c = in; c < in+len; c++) {
    switch (*c) {
    case ' ':
      fputc(' ', out);
      break;
    case '\n':
      fputs("@n", out);
      break;
    case '\r':
      fputs("@r", out);
      break;      
    case '\t':
      fputs("@t", out);
      break;
    default:
      if (isgraph(*c) && *c != '\'') {
	fputc(*c, out);
      } else {
	fprintf(out, "@%03d", *(unsigned char *)c);
      }
    }
  }
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputc('\'', out);
  cx_cstr_encode(v->as_str->data, v->as_str->len, out);
  fputc('\'', out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_str *s = v->as_str;
  fprintf(out, "'%s'", s->data);
}

static void print_imp(struct cx_box *v, FILE *out) {
  struct cx_str *s = v->as_str;
  
  for (char *c = s->data; c < s->data+s->len; c++) {
    fputc(*c, out);
  }
}

void cx_cstr_cencode(const char *in, size_t len, FILE *out) {
  for (const char *c = in; c < in+len; c++) {
    switch (*c) {
    case ' ':
      fputc(' ', out);
      break;
    case '"':
      fputs("\\\"", out);
      break;
    case '\n':
      fputs("\\n", out);
      break;
    case '\r':
      fputs("\\r", out);
      break;      
    case '\t':
      fputs("\\t", out);
      break;
    default:
      if (isgraph(*c)) {
	fputc(*c, out);
      } else {
	fprintf(out, "\" \"\\x%c%c\" \"", cx_bin_hex(*c / 16), cx_bin_hex(*c % 16));
      }
    }
  }
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->str_type)->as_str = cx_str_new(\"",
	  exp);
  
  cx_cstr_cencode(v->as_str->data, v->as_str->len, out);

  fprintf(out,
	  "\", %zd);\n",
	  v->as_str->len);

  return true;
}

static void deinit_imp(struct cx_box *v) {
  cx_str_deref(v->as_str);
}

struct cx_type *cx_init_str_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Str",
				  cx->cmp_type,
				  cx_type_get(cx->seq_type, cx->char_type));
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->clone = clone_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->emit = emit_imp;
  t->deinit = deinit_imp;
  return t;
}

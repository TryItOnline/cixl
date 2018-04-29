#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/char.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/str.h"
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/str.h"

typedef bool (*cx_split_t)(unsigned char c);

struct cx_split_iter {
  struct cx_iter iter;
  struct cx_iter *in;
  cx_split_t split_fn;
  struct cx_box split;
  struct cx_mfile out;
};

bool split_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_split_iter *it = cx_baseof(iter, struct cx_split_iter, iter);
  struct cx *cx = scope->cx;
  struct cx_box c;
  bool ok = false;
  
  while (true) {
    if (!cx_iter_next(it->in, &c, scope)) {
      iter->done = true;
      fflush(it->out.stream);
      if (it->out.data[0]) { break; }
      goto exit;
    }

    bool split = false;
    
    if (it->split_fn) {
      split = it->split_fn(c.as_char);
    } else {
      if (it->split.type == cx->char_type) {
	split = c.as_char == it->split.as_char;
      } else {
	*cx_push(scope) = c;
	if (!cx_call(&it->split, scope)) { goto exit; }
	struct cx_box *res = cx_pop(scope, true);
	
	if (!res) {
	  cx_error(cx, cx->row, cx->col, "Missing split result");
	  goto exit;
	}
	
	split = res->as_bool;
      }
    } 
    
    if (split) {
      fflush(it->out.stream);
      if (it->out.data[0]) { break; }
    } else {
      fputc(c.as_char, it->out.stream);
    }
  }

  cx_box_init(out, cx->str_type)->as_str = cx_str_new(it->out.data, it->out.size);
  ok = true;
 exit:
  cx_mfile_close(&it->out);
  free(it->out.data);
  if (!iter->done) { cx_mfile_open(&it->out); }
  return ok;
}

void *split_deinit(struct cx_iter *iter) {
  struct cx_split_iter *it = cx_baseof(iter, struct cx_split_iter, iter);
  cx_iter_deref(it->in);

  if (it->out.stream) {
    cx_mfile_close(&it->out);
    free(it->out.data);
  }
  
  if (!it->split_fn) { cx_box_deinit(&it->split); }
  return it;
}

cx_iter_type(split_iter, {
    type.next = split_next;
    type.deinit = split_deinit;
  });

struct cx_split_iter *cx_split_iter_new(struct cx_iter *in) {
  struct cx_split_iter *it = malloc(sizeof(struct cx_split_iter));
  cx_iter_init(&it->iter, split_iter());
  it->in = in;
  cx_mfile_open(&it->out);
  it->split_fn = NULL;
  return it;
}

struct hex_coder {
  struct cx_iter iter;
  struct cx_iter *in;
  int next;
};

static bool hex_coder_next(struct cx_iter *iter,
			   struct cx_box *out,
			   struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct hex_coder *it = cx_baseof(iter, struct hex_coder, iter);
  int c = it->next;

  if (c == -1) {
    struct cx_box v;

    if (!cx_iter_next(it->in, &v, scope)) {
      iter->done = it->in->done;
      return false;
    }

    c = cx_bin_hex(v.as_char / 16);
    it->next = cx_bin_hex(v.as_char % 16);
  } else {
    it->next = -1;
  }
    
  cx_box_init(out, cx->char_type)->as_char = c;
  return true;
}

static void *hex_coder_deinit(struct cx_iter *iter) {
  struct hex_coder *it = cx_baseof(iter, struct hex_coder, iter);
  cx_iter_deref(it->in);
  return it;
}

static cx_iter_type(hex_coder, {
    type.next = hex_coder_next;
    type.deinit = hex_coder_deinit;
  });

static struct cx_iter *hex_coder_new(struct cx_iter *in) {
  struct hex_coder *it = malloc(sizeof(struct hex_coder));
  cx_iter_init(&it->iter, hex_coder());
  it->in = cx_iter_ref(in);
  it->next = -1;
  return &it->iter;
}

struct hex_decoder {
  struct cx_iter iter;
  struct cx_iter *in;
};

static bool hex_decoder_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct hex_decoder *it = cx_baseof(iter, struct hex_decoder, iter);
  struct cx_box v;
  
  if (!cx_iter_next(it->in, &v, scope)) {
    iter->done = it->in->done;
    return false;
  }

  int c = cx_hex_bin(v.as_char) * 16;
  
  if (!cx_iter_next(it->in, &v, scope)) {
    iter->done = it->in->done;
    return false;
  }

  c += cx_hex_bin(v.as_char);    
  cx_box_init(out, cx->char_type)->as_char = c;
  return true;
}

static void *hex_decoder_deinit(struct cx_iter *iter) {
  struct hex_decoder *it = cx_baseof(iter, struct hex_decoder, iter);
  cx_iter_deref(it->in);
  return it;
}

static cx_iter_type(hex_decoder, {
    type.next = hex_decoder_next;
    type.deinit = hex_decoder_deinit;
  });

static struct cx_iter *hex_decoder_new(struct cx_iter *in) {
  struct hex_decoder *it = malloc(sizeof(struct hex_decoder));
  cx_iter_init(&it->iter, hex_decoder());
  it->in = cx_iter_ref(in);
  return &it->iter;
}

bool split_lines(unsigned char c) { return c == '\r' || c == '\n'; }

static bool lines_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), in_it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &in_it);
  struct cx_split_iter *it = cx_split_iter_new(in_it.as_iter);
  it->split_fn = split_lines;
  cx_box_init(cx_push(s), s->cx->iter_type)->as_iter = &it->iter;
  return true;
}

bool split_words(unsigned char c) {
  return !isalpha(c);
}

static bool words_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), in_it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &in_it);
  struct cx_split_iter *it = cx_split_iter_new(in_it.as_iter);
  it->split_fn = split_words;
  cx_box_init(cx_push(s), s->cx->iter_type)->as_iter = &it->iter;
  return true;
}

static bool split_imp(struct cx_call *call) {
  struct cx_box
    *split = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0)),
    in_it;

  struct cx_scope *s = call->scope;
  cx_iter(in, &in_it);
  struct cx_split_iter *it = cx_split_iter_new(in_it.as_iter);
  cx_copy(&it->split, split);
  cx_box_init(cx_push(s), s->cx->iter_type)->as_iter = &it->iter;
  return true;
}

static bool is_graph_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = isgraph(v->as_char);;
  return true;
}

static bool char_upper_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->char_type)->as_char = toupper(v->as_char);
  return true;
}

static bool char_lower_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->char_type)->as_char = tolower(v->as_char);
  return true;
}

static bool char_int_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_char;
  return true;
}

static bool int_char_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  if (v->as_int < 0 || v->as_int > 255) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Invalid char: %" PRId64, v->as_int);
    return false;
  }
  
  cx_box_init(cx_push(s), s->cx->char_type)->as_char = v->as_int;
  return true;
}

static bool int_str_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  char *sv = cx_fmt("%" PRId64, v->as_int);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(sv, strlen(sv));
  free(sv);
  return true;
}

static bool len_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_str->len;
  return true;
}

static bool get_imp(struct cx_call *call) {
  struct cx_box
    *i = cx_test(cx_call_arg(call, 1)),
    *v = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  
  if (i->as_int < 0 || i->as_int >= v->as_str->len) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Index out of bounds: %" PRId64, i->as_int);
    
    return false;
  }
  
  cx_box_init(cx_push(s), s->cx->char_type)->as_char = *(v->as_str->data+i->as_int);
  return true;
}

static bool last_imp(struct cx_call *call) {
  struct cx_str *v = cx_test(cx_call_arg(call, 0))->as_str;
  struct cx_scope *s = call->scope;
  
  if (v->len) {
    cx_box_init(cx_push(s), s->cx->char_type)->as_char = v->data[v->len-1];
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool fill_imp(struct cx_call *call) {
  struct cx_box
    *v = cx_test(cx_call_arg(call, 0)),
    *n = cx_test(cx_call_arg(call, 1)),
    *a = cx_test(cx_call_arg(call, 2));
  
  struct cx_scope *s = call->scope;
  
  if (!n->as_int) {
    cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_ref(v->as_str);
    return true;
  }
  
  struct cx_mfile out;
  cx_mfile_open(&out);
  fputs(v->as_str->data, out.stream);
  bool ok = false;
  
  for (int64_t i=0; i<n->as_int; i++) {
    if (a->type == s->cx->char_type) {
      fputc(a->as_char, out.stream);
    } else {
      if (!cx_call(a, s)) { goto exit; }
      struct cx_box *v = cx_pop(s, false);
      if (!v) { goto exit; }
      cx_print(v, out.stream);
    }
  }

  fflush(out.stream);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str =
    cx_str_new(out.data, ftell(out.stream));
  ok = true;
 exit:
  cx_mfile_close(&out);
  free(out.data);
  return ok;
}

static bool pop_imp(struct cx_call *call) {
  struct cx_str *v = cx_test(cx_call_arg(call, 0))->as_str;
  struct cx_scope *s = call->scope;
  
  if (v->len) {
    cx_box_init(cx_push(s), s->cx->char_type)->as_char = v->data[v->len-1];
    v->len--;
    v->data[v->len] = 0;
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool seq_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_mfile out;
  cx_mfile_open(&out);
  struct cx_box c;
  
  while (cx_iter_next(it.as_iter, &c, s)) {
    if (c.type == s->cx->nil_type) { continue; }
    fputc(c.as_char, out.stream);
  }
  
  fflush(out.stream);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str =
    cx_str_new(out.data, ftell(out.stream));
  cx_mfile_close(&out);
  free(out.data);
  cx_box_deinit(&it);
  return true;
}

static bool str_int_imp(struct cx_call *call) {
  struct cx_str *v = cx_test(cx_call_arg(call, 0))->as_str;
  struct cx_scope *s = call->scope;
  int64_t iv = strtoimax(v->data, NULL, 10);
  
  if (!iv && v->data[0] != '0') {
    cx_box_init(cx_push(s), s->cx->nil_type);
  } else {
    cx_box_init(cx_push(s), s->cx->int_type)->as_int = iv;
  }
  
  return true;
}

static bool str_sub_imp(struct cx_call *call) {
  struct cx_box
    *x = cx_test(cx_call_arg(call, 0)),
    *y = cx_test(cx_call_arg(call, 1));

  struct cx_scope *s = call->scope;
  
  cx_box_init(cx_push(s),
	      s->cx->int_type)->as_int = cx_str_dist(x->as_str->data,
						     y->as_str->data);
  
  return true;
}

static bool str_upper_imp(struct cx_call *call) {
  struct cx_str *v = cx_test(cx_call_arg(call, 0))->as_str;
  for (char *c = v->data; c < v->data+v->len; c++) { *c = toupper(*c); }
  return true;
}

static bool str_lower_imp(struct cx_call *call) {
  struct cx_str *v = cx_test(cx_call_arg(call, 0))->as_str;
  for (char *c = v->data; c < v->data+v->len; c++) { *c = tolower(*c); }
  return true;
}

static bool str_reverse_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  cx_reverse(v->as_str->data, v->as_str->len);
  return true;
}

static bool join_imp(struct cx_call *call) {
  struct cx_box
    *sep = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0)),
    it;

  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_mfile out;
  cx_mfile_open(&out);
  struct cx_box v;
  bool print_sep = false;
  
  while (cx_iter_next(it.as_iter, &v, s)) {
    if (print_sep) { cx_print(sep, out.stream); }
    cx_print(&v, out.stream);
    cx_box_deinit(&v);
    print_sep = sep->type != s->cx->nil_type;
  }

  cx_box_deinit(&it);
  cx_mfile_close(&out);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(out.data, out.size);
  free(out.data);
  return true;
}

static bool hex_coder_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);

  cx_box_init(cx_push(s), cx_type_get(s->cx->iter_type, s->cx->char_type))->as_iter =
    hex_coder_new(it.as_iter);

  return true;
}

static bool hex_decoder_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  
  cx_box_init(cx_push(s), cx_type_get(s->cx->iter_type, s->cx->char_type))->as_iter =
    hex_decoder_new(it.as_iter);

  return true;
}

cx_lib(cx_init_str, "cx/str") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc",
	      "A", "Bool", "Char", "Cmp", "Int", "Iter", "Opt", "Seq", "Str")) {
    return false;
  }

  cx_add_cfunc(lib, "lines",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->char_type))),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       lines_imp);

  cx_add_cfunc(lib, "words",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->char_type))),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       words_imp);

  cx_add_cfunc(lib, "split",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->char_type)),
		       cx_arg("s", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       split_imp);

  cx_add_cfunc(lib, "is-graph",
	       cx_args(cx_arg("c", cx->char_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       is_graph_imp);

  cx_add_cfunc(lib, "upper",
	       cx_args(cx_arg("c", cx->char_type)),
	       cx_args(cx_arg(NULL, cx->char_type)),
	       char_upper_imp);

  cx_add_cfunc(lib, "lower",
	       cx_args(cx_arg("c", cx->char_type)),
	       cx_args(cx_arg(NULL, cx->char_type)),
	       char_lower_imp);

  cx_add_cfunc(lib, "int",
	       cx_args(cx_arg("c", cx->char_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       char_int_imp);

  cx_add_cfunc(lib, "char",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->char_type)),
	       int_char_imp);
  
  cx_add_cfunc(lib, "str",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       int_str_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  cx_add_cfunc(lib, "get",
	       cx_args(cx_arg("s", cx->str_type), cx_arg("i", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->char_type)),
	       get_imp);

  cx_add_cfunc(lib, "last",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->char_type))),
	       last_imp);

  cx_add_cfunc(lib, "fill",
	       cx_args(cx_arg("s", cx->str_type),
		       cx_arg("n", cx->int_type),
		       cx_arg("a", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       fill_imp);
  
  cx_add_cfunc(lib, "pop",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->char_type))),
	       pop_imp);

  cx_add_cfunc(lib, "str",
	       cx_args(cx_arg("s", cx_type_get(cx->seq_type, cx->char_type))),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       seq_imp);

  cx_add_cfunc(lib, "int",
	       cx_args(cx_arg("s", cx->str_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->int_type))),
	       str_int_imp);

  cx_add_cfunc(lib, "-",
	       cx_args(cx_arg("x", cx->str_type), cx_arg("y", cx->str_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       str_sub_imp);

  cx_add_cfunc(lib, "upper",
	       cx_args(cx_arg("s", cx->str_type)), cx_args(),
	       str_upper_imp);

  cx_add_cfunc(lib, "lower",
	       cx_args(cx_arg("s", cx->str_type)), cx_args(),
	       str_lower_imp);

  cx_add_cfunc(lib, "reverse",
	       cx_args(cx_arg("s", cx->str_type)), cx_args(),
	       str_reverse_imp);

  cx_add_cfunc(lib, "join",
	       cx_args(cx_arg("in", cx->seq_type),
		       cx_arg("sep", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	      join_imp);

  cx_add_cfunc(lib, "hex-coder",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->char_type))),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       hex_coder_imp);

  cx_add_cfunc(lib, "hex-decoder",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->char_type))),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       hex_decoder_imp);

  return true;
}

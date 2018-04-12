#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
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

static bool lines_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_split_iter *it = cx_split_iter_new(cx_iter(&in));
  it->split_fn = split_lines;
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = &it->iter;
  cx_box_deinit(&in);
  return true;
}

bool split_words(unsigned char c) {
  return !isalpha(c);
}

static bool words_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_split_iter *it = cx_split_iter_new(cx_iter(&in));
  it->split_fn = split_words;
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = &it->iter;
  cx_box_deinit(&in);
  return true;
}

static bool split_imp(struct cx_scope *scope) {
  struct cx_box
    s = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));
  struct cx_split_iter *it = cx_split_iter_new(cx_iter(&in));
  it->split = s;
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = &it->iter;
  cx_box_deinit(&in);
  return true;
}

static bool is_graph_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  bool ig = isgraph(v->as_char);
  cx_box_init(v, scope->cx->bool_type)->as_bool = ig;
  return true;
}

static bool char_upper_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_char = toupper(v->as_char);
  return true;
}

static bool char_lower_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_char = tolower(v->as_char);
  return true;
}

static bool char_int_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  cx_box_init(v, scope->cx->int_type)->as_int = v->as_char;
  return true;
}

static bool int_char_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box *v = cx_test(cx_peek(scope, false));
  
  if (v->as_int < 0 || v->as_int > 255) {
    cx_error(cx, cx->row, cx->col, "Invalid char: %" PRId64, v->as_int);
    return false;
  }
  
  cx_box_init(v, cx->char_type)->as_char = v->as_int;
  return true;
}

static bool int_str_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  char *s = cx_fmt("%" PRId64, v->as_int);
  cx_box_init(v, scope->cx->str_type)->as_str = cx_str_new(s, strlen(s));
  free(s);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  size_t len = v->as_str->len;
  cx_box_deinit(v);
  cx_box_init(v, scope->cx->int_type)->as_int = len;
  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    i = *cx_test(cx_pop(scope, false)),
    s = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  if (i.as_int < 0 || i.as_int >= s.as_str->len) {
    cx_error(cx, cx->row, cx->col, "Index out of bounds: %" PRId64, i.as_int);
    goto exit;
  }
  
  cx_box_init(cx_push(scope), cx->char_type)->as_char = *(s.as_str->data+i.as_int);
  ok = true;
 exit:
  cx_box_deinit(&s);
  return ok;
}

static bool pop_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_str *s = in.as_str;
  
  if (s->len) {
    cx_box_init(cx_push(scope), cx->char_type)->as_char = s->data[s->len-1];
    s->len--;
    s->data[s->len] = 0;
  } else {
    cx_box_init(cx_push(scope), cx->nil_type);
  }

  cx_box_deinit(&in);
  return true;
}

static bool seq_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_iter(&in);
  struct cx_mfile out;
  cx_mfile_open(&out);
  struct cx_box c;
  
  while (cx_iter_next(it, &c, scope)) {
    if (c.type == cx->nil_type) { continue; }
    fputc(c.as_char, out.stream);
  }
  
  fflush(out.stream);
  cx_box_init(cx_push(scope), cx->str_type)->as_str =
    cx_str_new(out.data, ftell(out.stream));
  cx_mfile_close(&out);
  free(out.data);
  cx_box_deinit(&in);
  cx_iter_deref(it);
  return true;
}

static bool str_int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  int64_t iv = strtoimax(s->data, NULL, 10);
  
  if (!iv && s->data[0] != '0') {
    cx_box_init(cx_push(scope), cx->nil_type);
  } else {
    cx_box_init(cx_push(scope), cx->int_type)->as_int = iv;
  }
  
  cx_box_deinit(&v);
  return true;
}

static bool str_sub_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    y = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope),
	      scope->cx->int_type)->as_int = cx_str_dist(x.as_str->data,
							 y.as_str->data);
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool str_upper_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  for (char *c = s->data; c < s->data+s->len; c++) { *c = toupper(*c); }
  cx_box_deinit(&v);
  return true;
}

static bool str_lower_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  for (char *c = s->data; c < s->data+s->len; c++) { *c = tolower(*c); }
  cx_box_deinit(&v);
  return true;
}

static bool str_reverse_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  cx_reverse(s.as_str->data, s.as_str->len);
  cx_box_deinit(&s);
  return true;
}

static bool join_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    sep = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_iter *it = cx_iter(&in);
  struct cx_mfile out;
  cx_mfile_open(&out);
  struct cx_box v;
  bool print_sep = false;
  
  while (cx_iter_next(it, &v, scope)) {
    if (print_sep) { cx_print(&sep, out.stream); }
    cx_print(&v, out.stream);
    cx_box_deinit(&v);
    print_sep = sep.type != cx->nil_type;
  }

  cx_iter_deref(it);
  cx_mfile_close(&out);
  cx_box_init(cx_push(scope), cx->str_type)->as_str = cx_str_new(out.data, out.size);
  free(out.data);
  cx_box_deinit(&sep);
  cx_box_deinit(&in);
  return true;
}

static bool hex_coder_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = hex_coder_new(cx_iter(&in));
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;
  cx_box_deinit(&in);
  return true;
}

static bool hex_decoder_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = hex_decoder_new(cx_iter(&in));
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;
  cx_box_deinit(&in);
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

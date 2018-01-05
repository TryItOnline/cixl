#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/parse.h"
#include "cixl/types/func.h"
#include "cixl/types/int.h"
#include "cixl/types/lambda.h"
#include "cixl/vec.h"

static bool parse_type(struct cx *cx,
		       const char *id,
		       struct cx_vec *out,
		       bool lookup) {
  struct cx_type *t = cx_get_type(cx, id, !lookup);
  
  if (t) {
    cx_tok_init(cx_vec_push(out),
		CX_TTYPE(),
		cx->row, cx->col)->as_ptr = t;
  } else if (!lookup) {
    cx_tok_init(cx_vec_push(out),
		CX_TID(),
		cx->row, cx->col)->as_ptr = strdup(id);
  } else {
    return false;
  }

  return true;
}

static struct cx_vec *parse_fimp(struct cx *cx,
				 struct cx_func *func,
				 FILE *in,
				 struct cx_vec *out) {
  char c = fgetc(in);

  if (c != '<') {
    ungetc(c, in);
    return 0;
  }
  
  int row = cx->row, col = cx->col;
  struct cx_vec *types = cx_vec_new(sizeof(struct cx_box));
  
  while (true) {
    if (!cx_parse_tok(cx, in, out, false)) { return false; }
    struct cx_tok *tok = cx_vec_pop(out);
    if (tok->type == CX_TUNTYPE()) { break; }

    if (tok->type == CX_TTYPE()) {
      struct cx_box *v = cx_box_init(cx_vec_push(types), tok->as_ptr);
      v->undef = true;
    } else if (tok->type == CX_TLITERAL()) {
      cx_copy(cx_vec_push(types), &tok->as_box);      
    } else {
      cx_error(cx, row, col, "Invalid func type");
      free(cx_vec_deinit(types));
      return NULL;
    }
  }

  return types;
}

static bool parse_func(struct cx *cx, const char *id, FILE *in, struct cx_vec *out) {
  bool ref = id[0] == '&';
  struct cx_func *f = cx_get_func(cx, ref ? id+1 : id, false);
  if (!f) { return false; }

  struct cx_fimp *imp = NULL;
  struct cx_vec *types = parse_fimp(cx, f, in, out);
	  
  if (types) {
    imp = cx_func_get_imp(f, types, 0);
    if (!imp) { cx_error(cx, cx->row, cx->col, "Func imp not found"); }
    free(cx_vec_deinit(types));
  }
  
  if (ref) {
    struct cx_box *box = &cx_tok_init(cx_vec_push(out),
				      CX_TLITERAL(),
				      cx->row, cx->col)->as_box;
    if (imp) {
      cx_box_init(box, cx->fimp_type)->as_ptr = imp;
    } else {
      cx_box_init(box, cx->func_type)->as_ptr = f;
    }
  } else {
    if (imp) {
      cx_tok_init(cx_vec_push(out),
		  CX_TFIMP(),
		  cx->row, cx->col)->as_ptr = imp;
    } else {
      cx_tok_init(cx_vec_push(out),
		  CX_TFUNC(),
		  cx->row, cx->col)->as_ptr = f;
    }
  }

  return true;
}

static bool parse_id(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  struct cx_buf id;
  cx_buf_open(&id);
  bool ok = true;
  int col = cx->col;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto exit; }
    bool sep = cx_is_separator(cx, c);
    
    if (col != cx->col && sep) {
      ok = ungetc(c, in) != EOF;
      goto exit;
    }

    fputc(c, id.stream);
    col++;
    if (sep) { break; }
  }
 exit: {
    cx_buf_close(&id);

    if (ok) {
      struct cx_macro *m = cx_get_macro(cx, id.data, true);
      
      if (m) {
	cx->col = col;
	ok = m->imp(cx, in, out);
      } else {
	if (isupper(id.data[0])) {
	  ok = parse_type(cx, id.data, out, lookup);
	} else if (!lookup || id.data[0] == '#' || id.data[0] == '$') {
	  cx_tok_init(cx_vec_push(out),
		      CX_TID(),
		      cx->row, cx->col)->as_ptr = strdup(id.data);
	} else {
	  ok = parse_func(cx, id.data, in, out);
	}

	cx->col = col;
      }
    } else {
      cx_error(cx, cx->row, cx->col, "Failed parsing id");
    }
    
    free(id.data);
    return ok;
  }
}

static bool parse_int(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_buf value;
  cx_buf_open(&value);
  int col = cx->col;
  bool ok = true;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto exit; }
      
    if (col > cx->col && !isdigit(c)) {
      ok = (ungetc(c, in) != EOF);
      goto exit;
    }
    
    fputc(c, value.stream);
    col++;
  }
  
 exit: {
    cx_buf_close(&value);
    
    if (ok) {
      cx_int_t int_value = strtoimax(value.data, NULL, 10);
      free(value.data);
      
      if (int_value || !errno) {
	struct cx_box *box = &cx_tok_init(cx_vec_push(out),
					  CX_TLITERAL(),
					  cx->row, cx->col)->as_box;
	cx_box_init(box, cx->int_type)->as_int = int_value;
	cx->col = col;
      }
    } else {
      cx_error(cx, cx->row, cx->col, "Failed parsing int");
      free(value.data);
    }
    
    return ok;
  }
}

static bool parse_char(struct cx *cx, FILE *in, struct cx_vec *out) {
  char c = fgetc(in);
  int row = cx->row, col = cx->col;
  
  if (c == EOF) {
    cx_error(cx, row, col, "Invalid char literal");
    return false;
  }

  if (c == '\\') {
    c = fgetc(in);
    
    switch(c) {
    case 'n':
      c = '\n';
      break;
    case 't':
      c = '\t';
      break;
    default:
      ungetc(c, in);
      
      if (!parse_int(cx, in, out)) {
	cx_error(cx, row, col, "Invalid char literal");
	return false;
      }

      struct cx_tok *t = cx_vec_pop(out);
      c = t->as_box.as_int;
    }
  }
  
  struct cx_box *box = &cx_tok_init(cx_vec_push(out),
				    CX_TLITERAL(),
				    cx->row, cx->col)->as_box;
  cx_box_init(box, cx->char_type)->as_char = c;
  
  cx->col++;
  return true;
}

static bool parse_str(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_buf value;
  cx_buf_open(&value);
  int row = cx->row, col = cx->col;
  bool ok = false;
  char pc = 0;
  
  while (true) {
    char c = fgetc(in);

    if (c == EOF) {
      cx_error(cx, row, col, "Unterminated str literal");
      goto exit;
    }
    
    if (c == '\'' && pc != '\\') { break; }

    if (pc == '\\' && c != '\\') {
      switch (c) {
      case 'n':
	c = '\n';
	break;
      case 't':
	c = '\t';
	break;
      }
    }

    if (c != '\\' || pc == '\\') { fputc(c, value.stream); }
    pc = c;
    cx->col++;
  }

  ok = true;
 exit: {
    cx_buf_close(&value);
    
    if (ok) {
      struct cx_box *box = &cx_tok_init(cx_vec_push(out),
					CX_TLITERAL(),
					row, col)->as_box;
      cx_box_init(box, cx->str_type)->as_ptr = value.data;
    } else {
      free(value.data);
    }
    
    return ok;
  }
}

static bool parse_group(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TGROUP(),
				     cx->row, cx->col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body, lookup)) { return false; }
    struct cx_tok *tok = cx_vec_peek(body, 0);
    
    if (tok->type == CX_TUNGROUP()) {
      cx_tok_deinit(cx_vec_pop(body));
      break;
    }
  }

  return true;
}

static bool parse_vect(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TGROUP(),
				     cx->row, cx->col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body, lookup)) { return false; }
    struct cx_tok *tok = cx_vec_peek(body, 0);
    
    if (tok->type == CX_TUNVECT()) {
      cx_tok_deinit(cx_vec_pop(body));
      break;
    }
  }

  cx_tok_init(cx_vec_push(body), CX_TSTASH(), cx->row, cx->col);
  return true;
}

static bool parse_lambda(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  int row = cx->row, col = cx->col;

  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TLAMBDA(),
				     row, col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body, lookup)) { return false; }
    struct cx_tok *tok = cx_vec_peek(body, 0);
    
    if (tok->type == CX_TUNLAMBDA()) {
      cx_tok_deinit(cx_vec_pop(body));
      break;
    }
  }

  return true;
}

bool cx_parse_tok(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  int row = cx->row, col = cx->col;
  bool done = false;

  while (!done) {
    char c = fgetc(in);
      
    switch (c) {
    case EOF:
      done = true;
      break;
    case ' ':
      cx->col++;
      break;
    case '\n':
      cx->row++;
      break;
    case ',':
      cx_tok_init(cx_vec_push(out), CX_TCUT(), row, col);
      return true;
    case ';':
      cx_tok_init(cx_vec_push(out), CX_TEND(), row, col);
      return true;
    case '_':
      cx_tok_init(cx_vec_push(out), CX_TZAP(), row, col);
      return true;
    case '(':
      return parse_group(cx, in, out, lookup);
    case ')':
      cx_tok_init(cx_vec_push(out), CX_TUNGROUP(), row, col);
      return true;	
    case '[':
      return parse_vect(cx, in, out, lookup);
    case ']':
      cx_tok_init(cx_vec_push(out), CX_TUNVECT(), row, col);
      return true;	
    case '{':
      return parse_lambda(cx, in, out, lookup);
    case '}':
      cx_tok_init(cx_vec_push(out), CX_TUNLAMBDA(), row, col);
      return true;
    case '>':
      cx_tok_init(cx_vec_push(out), CX_TUNTYPE(), row, col);
      return true;	
    case '\\':
      return parse_char(cx, in, out);
    case '\'':
      return parse_str(cx, in, out);
    case '-': {
      char c1 = fgetc(in);
      if (isdigit(c1)) {
	ungetc(c1, in);
	ungetc(c, in);
	return parse_int(cx, in, out);
      } else {
	ungetc(c1, in);
	ungetc(c, in);
	return parse_id(cx, in, out, lookup);
      }
	
      break;
    }
    default:
      if (isdigit(c)) {
	ungetc(c, in);
	return parse_int(cx, in, out);
      }
	
      ungetc(c, in);
      return parse_id(cx, in, out, lookup);
    }
  }

  return false;
}

bool cx_parse_end(struct cx *cx, FILE *in, struct cx_vec *out) {
  int depth = 1;
  
  while (depth) {
    if (!cx_parse_tok(cx, in, out, true)) { return false; }
    struct cx_tok *tok = cx_vec_peek(out, 0);

    if (tok->type == CX_TID()) {
      char *id = tok->as_ptr;
      if (id[strlen(id)-1] == ':') { depth++; }
    } else if (tok->type == CX_TEND()) {
      depth--;
    }
  }

  cx_vec_pop(out);
  return true;
}

bool cx_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  cx->row = cx->col = 1;
  
  while (true) {
    if (!cx_parse_tok(cx, in, out, true)) { break; }
  }

  return cx->errors.count == 0;
}

bool cx_parse_str(struct cx *cx, const char *in, struct cx_vec *out) {
  FILE *is = fmemopen ((void *)in, strlen(in), "r");
  bool ok = false;
  if (!cx_parse(cx, is, out)) { goto exit; }
  ok = true;
 exit:
  fclose(is);
  return ok;
}

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/int.h"
#include "cixl/lambda.h"
#include "cixl/parse.h"
#include "cixl/vec.h"

static bool parse_id(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  struct cx_buf id;
  cx_buf_open(&id);
  bool ok = true;
  int col = cx->col;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto exit; }

    if (col != cx->col && cx_is_separator(cx, c)) {
      ok = ungetc(c, in) != EOF;
      goto exit;
    }

    fputc(c, id.stream);
    col++;
  }

 exit: {
    cx_buf_close(&id);

    if (ok) {
      struct cx_macro *m = cx_get_macro(cx, id.data, true);
      
      if (m) {
	cx->col = col;
	ok = m->imp(cx, in, out);
	free(id.data);
      } else {
	if (isupper(id.data[0])) {
	  struct cx_type *t = cx_get_type(cx, id.data, false);
	  free(id.data);
	  if (!t) { return false; }
	  cx_tok_init(cx_vec_push(out), CX_TTYPE, t, cx->row, cx->col);
	} else if (strcmp(id.data, "t") == 0 || strcmp(id.data, "f") == 0) {
	  cx_tok_init(cx_vec_push(out),
		      id.data[0] == 't' ? CX_TTRUE : CX_TFALSE,
		      NULL,
		      cx->row, cx->col);
	} else if (strcmp(id.data, "_") == 0) {
	  cx_tok_init(cx_vec_push(out), CX_TNIL, NULL, cx->row, cx->col);
	} else if (!lookup || id.data[0] == '$') {
	  cx_tok_init(cx_vec_push(out), CX_TID, id.data, cx->row, cx->col);
	} else {
	  bool ref = id.data[0] == '&';
	  struct cx_func *f = cx_get_func(cx, ref ? id.data+1 : id.data, false);

	  if (!f) {
	    free(id.data);
	    return false;
	  }

	  if (ref) {
	    struct cx_box *box = cx_box_new(cx->func_type);
	    box->as_ptr = f;
	    cx_tok_init(cx_vec_push(out), CX_TLITERAL, box, cx->row, cx->col);
	  } else {
	    cx_tok_init(cx_vec_push(out), CX_TFUNC, f, cx->row, cx->col);
	  }
	  
	  free(id.data);
	}
	
	cx->col = col;
      }
    } else {
      cx_error(cx, cx->row, cx->col, "Failed parsing id");
      free(id.data);
    }
    
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
      
    if (!isdigit(c)) {
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
	struct cx_box *box = cx_box_new(cx->int_type);
	box->as_int = int_value;
	cx_tok_init(cx_vec_push(out), CX_TLITERAL, box, cx->row, cx->col);
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

  if (c == EOF) {
    cx_error(cx, cx->row, cx->col, "Invalid char literal");
    return false;
  }
  
  struct cx_box *box = cx_box_new(cx->char_type);
  box->as_char = c;
  cx_tok_init(cx_vec_push(out), CX_TLITERAL, box, cx->row, cx->col);
  cx->col++;
  return true;
}

static bool parse_str(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_buf value;
  cx_buf_open(&value);
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  while (true) {
    char c = fgetc(in);

    if (c == EOF) {
      cx_error(cx, row, col, "Unterminated str literal");
      goto exit;
    }
      
    if (c == '\'') { break; }
    fputc(c, value.stream);
    cx->col++;
  }

  ok = true;
 exit: {
    cx_buf_close(&value);
    
    if (ok) {
      struct cx_box *box = cx_box_new(cx->str_type);
      box->as_ptr = value.data;
      cx_tok_init(cx_vec_push(out), CX_TLITERAL, box, row, col);
    } else {
      free(value.data);
    }
    
    return ok;
  }
}

static bool parse_group(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  int row = cx->row, col = cx->col;
  struct cx_vec *body = cx_vec_init(malloc(sizeof(struct cx_vec)),
				    sizeof(struct cx_tok));
  
  while (true) {
    if (!cx_parse_tok(cx, in, body, lookup)) {
      free(body);
      return false;
    }

    struct cx_tok *tok = cx_vec_peek(body, 0);
    if (tok->type == CX_TUNGROUP) {
      cx_tok_deinit(cx_vec_pop(body));
      break;
    }
  }

  cx_tok_init(cx_vec_push(out), CX_TGROUP, body, row, col);
  return true;
}

static bool parse_lambda(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  int row = cx->row, col = cx->col;
  struct cx_vec *body = cx_vec_new(sizeof(struct cx_tok));
  
  while (true) {
    if (!cx_parse_tok(cx, in, body, lookup)) {
      cx_do_vec(body, struct cx_tok, t) { cx_tok_deinit(t); }
      free(cx_vec_deinit(body));
      return false;
    }

    struct cx_tok *tok = cx_vec_peek(body, 0);
    
    if (tok->type == CX_TUNLAMBDA) {
      cx_tok_deinit(cx_vec_pop(body));
      break;
    }
  }

  cx_tok_init(cx_vec_push(out), CX_TLAMBDA, body, row, col);
  return true;
}

bool cx_parse_tok(struct cx *cx, FILE *in, struct cx_vec *out, bool lookup) {
  int row = cx->row, col = cx->col;
  bool done = false;
  
  void push_tok(enum cx_tok_type type, void *data) {
    cx_tok_init(cx_vec_push(out), type, data, row, col);
  }
  
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
	cx_tok_init(cx_vec_push(out), CX_TCUT, NULL, row, col);
	return true;
      case ';':
	cx_tok_init(cx_vec_push(out), CX_TEND, NULL, row, col);
	return true;
      case '(':
	return parse_group(cx, in, out, lookup);
      case ')':
	cx_tok_init(cx_vec_push(out), CX_TUNGROUP, NULL, row, col);
	return true;	
      case '{':
	return parse_lambda(cx, in, out, lookup);
      case '}':
	cx_tok_init(cx_vec_push(out), CX_TUNLAMBDA, NULL, row, col);
	return true;
      case '\\':
	return parse_char(cx, in, out);
      case '\'':
	return parse_str(cx, in, out);
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

    switch(tok->type) {
    case CX_TID: {
      char *id = tok->data;
      if (id[strlen(id)-1] == ':') { depth++; }
      break;
    }
    case CX_TEND:
      depth--;
      break;
    default:
      break;
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

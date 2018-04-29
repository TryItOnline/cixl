#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/mfile.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/lambda.h"
#include "cixl/parse.h"
#include "cixl/str.h"
#include "cixl/vec.h"

static void update_pos(struct cx *cx, char c) {
  if (c == '\n') {
    cx->row++;
    cx->col = 0;
  } else if (c != EOF) {
    cx->col++;
  }
}

struct cx_type *cx_parse_type_arg(struct cx *cx, char **in) {
  if (!**in) { return NULL; }

  char *i = *in;
  while (*i == ' ') { i++; }
  char *j = i+1;
  int depth = 1;
  
  while (*j && (depth > 1 || (*j != ' ' && *j != '>'))) {
    switch (*j) {
    case '<':
      depth++;
      break;
    case '>':
      depth--;
      break;
    default:
      break;
    }
    
    j++;
  }

  if (j > i) {
    char tmp = *j;
    *j = 0;
    struct cx_type *t = cx_get_type(cx, i, false);
    *j = tmp;
    if (*j == '>') { j++; }
    *in = j;
    return t;
  }

  return NULL;
}

static char *parse_type_args(struct cx *cx, const char *id, FILE *in) {
  int row = cx->row, col = cx->col;
  char c = fgetc(in);

  if (c != '<') {
    ungetc(c, in);
    return NULL;
  }

  cx->col++;
  struct cx_mfile aid;
  cx_mfile_open(&aid);
  fputs(id, aid.stream);
  fputc('<', aid.stream);
  int depth = 1;

  while (depth) {
    c = fgetc(in);
    update_pos(cx, c);
    
    switch (c) {
    case EOF:
      cx_error(cx, row, col, "Open type arg list");
      return NULL;
    case '<':
      depth++;
      break;
    case '>':
      depth--;
      break;
    default:
      break;
    }

    if (depth) { fputc(c, aid.stream); }
  }

  fputc('>', aid.stream);  
  cx_mfile_close(&aid);
  return aid.data;
}

static bool parse_line_comment(struct cx *cx, FILE *in) {
  bool done = false;
  
  while (!done) {
    char c = fgetc(in);

    switch(c) {
    case '\n':
    case EOF:
      done = true;
      break;
    }

    update_pos(cx, c);
  }

  return true;
}

static bool parse_block_comment(struct cx *cx, FILE *in) {
  int row = cx->row, col = cx->col;
  char pc = 0;
  
  while (true) {
    char c = fgetc(in);

    if (c == EOF) {
      cx_error(cx, row, col, "Unterminated comment");
      return false;
    }

    update_pos(cx, c);
    if (c == '/' && pc == '*') { break; }
    pc = c;
  }

  return true;
}

static bool parse_id(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_mfile id;
  cx_mfile_open(&id);
  bool ok = true;
  int row = cx->row, col = cx->col;
  char pc = 0;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto done; }
    bool sep = cx_is_separator(cx, c);

    if (cx->col != col &&
	(cx->col-col > 2 || pc != '&') &&
	(sep || c == '<')) {
      ok = ungetc(c, in) != EOF;
      goto done;
    }

    update_pos(cx, c);
    fputc(c, id.stream);
    if (sep) { break; }
    pc = c;
  }
 done: {
    cx_mfile_close(&id);
    
    if (ok) {
      char *s = id.data;

      if (s[0] != '#' && s[0] != '$' && !isupper(s[0])) {
	struct cx_macro *m = cx_get_macro(cx, s, true);
	
	if (m) {
	  ok = m->imp(cx, in, out);
	  goto exit;
	}
      }

      char *args = parse_type_args(cx, s, in);
      
      cx_tok_init(cx_vec_push(out),
		  CX_TID(),
		  row, col)->as_ptr = args ? args : strdup(s);
    } else {
      cx_error(cx, row, col, "Failed parsing id");
    }

  exit:
    free(id.data);
    return ok;
  }
}

static bool parse_int(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_mfile s;
  cx_mfile_open(&s);
  int row = cx->row, col = cx->col;
  bool ok = true, is_float = false;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto exit; }
    
    if (cx->col > col && !isdigit(c) && c != '.') {
      ok = (ungetc(c, in) != EOF);
      goto exit;
    }
    
    update_pos(cx, c);
    if (c == '.') { is_float = true; }
    fputc(c, s.stream);
  }
  
 exit: {
    cx_mfile_close(&s);
    
    if (ok) {
      if (is_float) {
	errno = 0;
	cx_float_t v = strtold(s.data, NULL);
	free(s.data);

	if (errno) {
	  cx_error(cx, row, col, "Failed parsing float: %d", errno);
	  return false;
	}
	
	struct cx_box *box = &cx_tok_init(cx_vec_push(out),
					  CX_TLITERAL(),
					  row, col)->as_box;
	
	cx_box_init(box, cx->float_type)->as_float = v;
      } else {
	errno = 0;
	int64_t v = strtoimax(s.data, NULL, 10);
	free(s.data);

	if (errno) {
	  cx_error(cx, row, col, "Failed parsing int: %d", errno);
	  return false;
	}
	
	struct cx_box *box = &cx_tok_init(cx_vec_push(out),
					  CX_TLITERAL(),
					  row, col)->as_box;
	
	cx_box_init(box, cx->int_type)->as_int = v;
      }
    } else {
      cx_error(cx, row, col, "Failed parsing token");
      free(s.data);
    }
    
    return ok;
  }
}

static bool parse_char(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  cx->col++;
  
  char c = fgetc(in);
  
  if (c == EOF) {
    cx_error(cx, row, col, "Invalid char literal");
    return false;
  }

  cx->col++;

  if (c == '@') {
    c = fgetc(in);
    cx->col++;
    
    switch(c) {
    case 'n':
      c = '\n';
      break;
    case 'r':
      c = '\r';
      break;
    case 's':
      c = ' ';
      break;
    case 't':
      c = '\t';
      break;
    default:      
      ungetc(c, in);
      
      if (isdigit(c)) {
	char n[4];
	
	if (!fgets(n, 4, in)) {
	  cx_error(cx, row, col, "Invalid char literal");
	  return false;
	}

	cx->col += 3;
	c = 0;
	
	for (int i=0, m = 100; i<3; i++, m /= 10) {
	  if (!isdigit(n[i])) {
	    cx_error(cx, row, col, "Expected digit: %c", n[i]);
	    return false;
	  }

	  c += (n[i] - '0') * m;
	}
      } else {
	cx->col--;
	c = '@';
      }
    }
  }
  
  struct cx_box *box = &cx_tok_init(cx_vec_push(out),
				    CX_TLITERAL(),
				    cx->row, cx->col)->as_box;
  cx_box_init(box, cx->char_type)->as_char = c;
  return true;
}

static bool parse_str(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_mfile value;
  cx_mfile_open(&value);
  int row = cx->row, col = cx->col;
  cx->col++;
  bool ok = false;
  char pc = 0;
  
  while (true) {
    char c = fgetc(in);

    if (c == EOF) {
      cx_error(cx, row, col, "Unterminated str literal");
      goto exit;
    }

    if (c == '\'' && pc != '\\') {
      cx->col++;
      break;
    }

    if (c == '@') {
      ungetc(c, in);
      if (!parse_char(cx, in, out)) { goto exit; }
      struct cx_tok *t = cx_vec_pop(out);
      c = t->as_box.as_char;
    } else {
      update_pos(cx, c);
    }

    fputc(c, value.stream);
    pc = c;
  }

  ok = true;
 exit: {    
    if (ok) {
      struct cx_box *box = &cx_tok_init(cx_vec_push(out),
					CX_TLITERAL(),
					row, col)->as_box;

      fflush(value.stream);
      
      cx_box_init(box, cx->str_type)->as_str =
	cx_str_new(value.data, ftell(value.stream));
    }

    cx_mfile_close(&value);
    free(value.data);
    return ok;
  }
}

static bool parse_sym(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_mfile id;
  cx_mfile_open(&id);
  int col = cx->col;
  cx->col++;
  bool ok = true;
  
  while (true) {
    char c = fgetc(in);
    if (c == EOF) { goto exit; }
    
    if (cx_is_separator(cx, c)) {
      ok = ungetc(c, in) != EOF;
      goto exit;
    }

    fputc(c, id.stream);
    col++;
  }

  ok = true;
 exit:
  cx_mfile_close(&id);

  if (ok) {
    struct cx_box *box = &cx_tok_init(cx_vec_push(out),
				      CX_TLITERAL(),
				      cx->row, col)->as_box;
    cx_box_init(box, cx->sym_type)->as_sym = cx_sym(cx, id.data);
  }
  
  free(id.data);
  return ok;
}


static bool parse_group(struct cx *cx, FILE *in, struct cx_vec *out) {
  cx->col++;
  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TGROUP(),
				     cx->row, cx->col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body)) { return false; }

    if (body->count) {
      struct cx_tok *tok = cx_vec_peek(body, 0);
    
      if (tok->type == CX_TUNGROUP()) {
	cx_tok_deinit(cx_vec_pop(body));
	break;
      }
    }
  }

  return true;
}

static bool parse_stack(struct cx *cx, FILE *in, struct cx_vec *out) {
  cx->col++;
  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TSTACK(),
				     cx->row, cx->col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body)) { return false; }

    if (body->count) {
      struct cx_tok *tok = cx_vec_peek(body, 0);
    
      if (tok->type == CX_TUNSTACK()) {
	cx_tok_deinit(cx_vec_pop(body));
	break;
      }
    }
  }

  return true;
}

static bool parse_lambda(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  cx->col++;
  
  struct cx_vec *body = &cx_tok_init(cx_vec_push(out),
				     CX_TLAMBDA(),
				     row, col)->as_vec;
  cx_vec_init(body, sizeof(struct cx_tok));

  while (true) {
    if (!cx_parse_tok(cx, in, body)) { return false; }

    if (body->count) {
      struct cx_tok *tok = cx_vec_peek(body, 0);
    
      if (tok->type == CX_TUNLAMBDA()) {
	cx_tok_deinit(cx_vec_pop(body));
	break;
      }
    }
  }

  return true;
}

bool cx_parse_tok(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool done = false;

  while (!done) {
    char c = fgetc(in);
    update_pos(cx, c);
    
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
      break;
    case EOF:
      done = true;
      break;
    case ';':
      cx_tok_init(cx_vec_push(out), CX_TEND(), row, col);
      return true;
    case '(':
      return parse_group(cx, in, out);
    case ')':
      cx_tok_init(cx_vec_push(out), CX_TUNGROUP(), row, col);
      return true;	
    case '[':
      return parse_stack(cx, in, out);
    case ']':
      cx_tok_init(cx_vec_push(out), CX_TUNSTACK(), row, col);
      return true;	
    case '{':
      return parse_lambda(cx, in, out);
    case '}':
      cx_tok_init(cx_vec_push(out), CX_TUNLAMBDA(), row, col);
      return true;
    case '@':
      return parse_char(cx, in, out);
    case '\'':
      return parse_str(cx, in, out);
    case '`':
      return parse_sym(cx, in, out);
    case '-': {
      cx->col--;
      char c1 = fgetc(in);
      ungetc(c1, in);
      ungetc(c, in);
      
      if (isdigit(c1)) {
	return parse_int(cx, in, out);
      } else {
	return parse_id(cx, in, out);
      }
	
      break;
    }
    default:
      if (isdigit(c)) {
	ungetc(c, in);
	cx->col--;
	return parse_int(cx, in, out);
      }

      if (c == '/') {
	char cc = fgetc(in);
	cx->col++;
	
	if (cc == '/') {
	  return parse_line_comment(cx, in);
	} else if (cc == '*') {
	  return parse_block_comment(cx, in);
	}

	ungetc(cc, in);
	cx->col--;
      }
      
      ungetc(c, in);
      cx->col--;
      return parse_id(cx, in, out);
    }
  }

  return false;
}

bool cx_parse_end(struct cx *cx, FILE *in, struct cx_vec *out) {
  int depth = 1;
  
  while (depth) {
    if (!cx_parse_tok(cx, in, out) || !out->count) { return false; }
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
  while (!feof(in)) { cx_parse_tok(cx, in, out); }
  return !cx->errors.count;
}

bool cx_parse_str(struct cx *cx, const char *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  cx->row = 1; cx->col = 0;
  FILE *is = fmemopen ((void *)in, strlen(in), "r");
  bool ok = false;
  if (!cx_parse(cx, is, out)) { goto exit; }
  ok = true;
 exit:
  cx->row = row; cx->col = col;
  fclose(is);
  return ok;
}

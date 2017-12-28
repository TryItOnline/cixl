#ifndef CX_TOK_H
#define CX_TOK_H

enum cx_tok_type { CX_TCUT, CX_TEND, CX_TFALSE, CX_TFUNC, CX_TFUNC_IMP, CX_TGROUP,
		   CX_TID, CX_TLAMBDA, CX_TLITERAL, CX_TMACRO, CX_TNIL, CX_TTRUE,
		   CX_TTYPE, CX_TUNGROUP, CX_TUNLAMBDA };

struct cx_tok {
  enum cx_tok_type type;
  void *data;
  int row, col;
};

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   enum cx_tok_type type,
			   void *data,
			   int row, int col);

struct cx_tok *cx_tok_deinit(struct cx_tok *tok);
void cx_tok_copy(struct cx_tok *dst, struct cx_tok *src);

#endif

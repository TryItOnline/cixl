#ifndef CX_LS_H
#define CX_LS_H

#define _cx_do_ls(_next, ls, var)			 \
  for (struct cx_ls *var = ls->next, *_next = var->next; \
       var != ls;					 \
       var = _next, _next = var->next)			 \

#define cx_do_ls(ls, var)			 \
  _cx_do_ls(cx_gencid(next), (ls), var)		 \
		      
struct cx_ls {
  struct cx_ls *prev, *next;
};

struct cx_ls *cx_ls_init(struct cx_ls *ls);
void cx_ls_append(struct cx_ls *ls, struct cx_ls *next);
void cx_ls_prepend(struct cx_ls *ls, struct cx_ls *prev);
void cx_ls_delete(struct cx_ls *ls);

#endif

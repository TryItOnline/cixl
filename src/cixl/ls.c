#include "cixl/ls.h"

struct cx_ls *cx_ls_init(struct cx_ls *ls) {
  ls->prev = ls->next = ls;
  return ls;
}

void cx_ls_append(struct cx_ls *ls, struct cx_ls *next) {
  next->prev = ls;
  next->next = ls->next;
  ls->next->prev = next;
  ls->next = next;
}

void cx_ls_prepend(struct cx_ls *ls, struct cx_ls *prev) {
  prev->prev = ls->prev;
  prev->next = ls;
  ls->prev->next = prev;
  ls->prev = prev;
}

void cx_ls_delete(struct cx_ls *ls) {
  ls->prev->next = ls->next;
  ls->next->prev = ls->prev;
}

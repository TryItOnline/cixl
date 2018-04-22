#ifndef CX_LIB_TERM_H
#define CX_LIB_TERM_H

#define CX_CSI_ESC "\x1b["

struct cx;
struct cx_lib;

struct cx_lib *cx_init_term(struct cx *cx);

#endif

#ifndef CX_REPL_H
#define CX_REPL_H

#include <stdio.h>

#define CX_REPL_LINE_MAX 512

struct cx;

void cx_repl(struct cx *cx, FILE *in, FILE *out);

#endif

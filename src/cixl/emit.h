#ifndef CX_EMIT_H
#define CX_EMIT_H

#include <stdbool.h>
#include <stdio.h>

struct cx;

char *cx_emit_id(const char *prefix, const char *in);
bool cx_emit_file(struct cx *cx, const char *fname, FILE *out);
  
#endif

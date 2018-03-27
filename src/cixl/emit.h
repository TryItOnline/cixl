#ifndef CX_EMIT_H
#define CX_EMIT_H

#include <stdbool.h>
#include <stdio.h>

struct cx;
struct cx_bin;

char *cx_emit_id(const char *prefix, const char *in);
void cx_push_args(struct cx *cx, int argc, char *argv[]);
bool cx_emit_file(struct cx *cx, struct cx_bin *bin, FILE *out);
  
#endif

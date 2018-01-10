#ifndef CX_TYPE_GUID_H
#define CX_TYPE_GUID_H

#include <stdio.h>

struct cx;
struct cx_type;

typedef uint32_t cx_guid_t[4];

void cx_guid_init(cx_guid_t id);
void cx_guid_fprint(cx_guid_t id, FILE *out);

struct cx_type *cx_init_guid_type(struct cx *cx);

#endif

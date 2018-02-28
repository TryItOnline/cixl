#ifndef CX_GUID_H
#define CX_GUID_H

#include <stdio.h>

#define CX_GUID_LEN 37

struct cx;
struct cx_type;

typedef uint32_t cx_guid_t[4];

void cx_guid_init(cx_guid_t id);
char *cx_guid_str(cx_guid_t id, char *out);
bool cx_guid_parse(const char *in, cx_guid_t out);

struct cx_type *cx_init_guid_type(struct cx *cx);

#endif

#ifndef CX_SCAN_H
#define CX_SCAN_H

#include <stdbool.h>

struct cx;
struct cx_func;

typedef bool (*cx_scan_callback_t)(void *);

struct cx_scan {
  struct cx_scope *scope;
  struct cx_func *func;
  cx_scan_callback_t callback;
  void *data;
};

struct cx_scan *cx_scan_init(struct cx_scan *scan,
			     struct cx_scope *scope,
			     struct cx_func *func,
			     cx_scan_callback_t callback,
			     void *data);

void cx_scan(struct cx *cx,
	     struct cx_func *func,
	     cx_scan_callback_t callback,
	     void *data);

bool cx_scan_ok(struct cx_scan *scan, struct cx_scope *scope);
bool cx_scan_call(struct cx_scan *scan);

#endif

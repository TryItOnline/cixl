#ifndef CX_SCAN_H
#define CX_SCAN_H

#include <stdbool.h>

struct cx;
struct cx_func;
struct cx_scan;

typedef bool (*cx_scan_callback_t)(struct cx_scan *);

struct cx_fimp_scan {
  struct cx_fimp *imp;
  size_t pc;
};

struct cx_funcall_scan {
  struct cx_fimp *imp;
};

struct cx_recall_scan {
  struct cx_fimp *imp;
  size_t pc;  
};

struct cx_upcall_scan {
  struct cx_fimp *imp;
};

struct cx_scan {
  struct cx_scope *scope;
  int level;
  struct cx_func *func;
  cx_scan_callback_t callback;

  union {
    struct cx_fimp_scan as_fimp;
    struct cx_funcall_scan as_funcall;
    struct cx_recall_scan as_recall;
    struct cx_upcall_scan as_upcall;
  };
};

struct cx_scan *cx_scan_init(struct cx_scan *scan,
			     struct cx_scope *scope,
			     struct cx_func *func,
			     cx_scan_callback_t callback);

struct cx_scan *cx_scan(struct cx_scope *scope,
			struct cx_func *func,
			cx_scan_callback_t callback);

bool cx_scan_ok(struct cx_scan *scan);
bool cx_scan_call(struct cx_scan *scan);

#endif

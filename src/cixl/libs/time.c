#include <errno.h>
#include <time.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/libs/io.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/time.h"
#include "cixl/types/vect.h"
#include "cixl/util.h"

static bool is_leap_year(int year) {
  return !(year%4) && ((year%100) || !(year%400));
}

static int days_in_month(int year, int month) {
  cx_test(month >= 0 && month < 12);
  
  switch (month) {
  case 0: case 2: case 4: case 6: case 7: case 9: case 11:
    return 31;
  case 1:
    return is_leap_year(year) ? 29 : 28;
  }

  return 30;
}

static bool years_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       n.as_int*12, 0);

  return true;
}

static bool months_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       n.as_int, 0);

  return true;
}

static bool days_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_DAY);

  return true;
}

static bool h_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_HOUR);

  return true;
}

static bool m_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_MIN);

  return true;
}

static bool s_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_SEC);

  return true;
}

static bool ms_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_MSEC);

  return true;
}

static bool us_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_USEC);

  return true;
}

static bool ns_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int);
  
  return true;
}

static bool vect_time_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_vect *vs = in.as_ptr;

  struct cx_time t;
  cx_time_init(&t, 0, 0);
  
  for (int i = 0; i < cx_min(vs->imp.count, 7); i++) {
    struct cx_box *v = cx_vec_get(&vs->imp, i);

    if (v->type != cx->int_type) {
      cx_box_init(cx_push(scope), cx->nil_type);
      goto exit;
    }
    
    switch (i) {
    case 0:
      t.months += v->as_int*12;
      break;
    case 1:
      t.months += v->as_int;
      break;
    case 2:
      t.ns += v->as_int*CX_DAY;
      break;
    case 3:
      t.ns += v->as_int*CX_HOUR;
      break;
    case 4:
      t.ns += v->as_int*CX_MIN;
      break;
    case 5:
      t.ns += v->as_int*CX_SEC;
      break;
    case 6:
      t.ns += v->as_int;
      break;
    }
  }

  cx_box_init(cx_push(scope), cx->time_type)->as_time = t;
 exit:
  cx_box_deinit(&in);  
  return true;
}

static bool now_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm tm;
  
  if (!localtime_r(&ts.tv_sec, &tm)) {
    cx_error(cx, cx->row, cx->col, "Failed destructuring time: %d", errno);
  }
  
  struct cx_time *t = &cx_box_init(cx_push(scope), cx->time_type)->as_time;

  cx_time_init(t,
	       (tm.tm_year+1900)*12 + tm.tm_mon,
	       (tm.tm_mday-1) * CX_DAY +
	       (tm.tm_hour) * CX_HOUR +
	       (tm.tm_min) * CX_MIN +
	       (tm.tm_sec) * CX_SEC +
	       ts.tv_nsec);
  
  return true;
}

static bool time_date_imp(struct cx_scope *scope) {
  struct cx_time *t = &cx_test(cx_peek(scope, false))->as_time;
  t->ns = t->ns / CX_DAY * CX_DAY;
  return true;
}

static bool time_time_imp(struct cx_scope *scope) {
  struct cx_time *t = &cx_test(cx_peek(scope, false))->as_time;
  t->months = 0;
  t->ns %= CX_DAY;
  return true;
}

static bool time_years_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months / 12;
  return true;
}

static bool month_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months % 12;
  return true;
}

static bool time_months_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months;
  return true;
}

static bool time_day_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_DAY;
  return true;
}

static bool time_days_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_time t = cx_test(cx_pop(scope, false))->as_time;

  int y_max = cx_abs(t.months/12), m_max = cx_abs(t.months%12);
  int64_t days = 0;

  for (int y = 0, m = 0; y < y_max || m < m_max;) {
    days += days_in_month(y, m);

    if (m == 11) {
      y++;
      m = 0;
    } else {
      m++;
    }
  }

  if (t.months < 0) { days *= -1; }
  days += t.ns / CX_DAY;
  cx_box_init(cx_push(scope), cx->int_type)->as_int = days;
  return true;
}

static bool hour_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_DAY) / CX_HOUR;
  return true;
}

static bool minute_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_HOUR) / CX_MIN;
  return true;
}

static bool second_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_MIN) / CX_SEC;
  return true;
}

static bool nsecond_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = (t.as_time.ns % CX_SEC);
  return true;
}

static bool time_h_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_HOUR;
  return true;
}

static bool time_m_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_MIN;
  return true;
}

static bool time_s_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_SEC;
  return true;
}

static bool time_ms_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_MSEC;
  return true;
}

static bool time_us_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_USEC;
  return true;
}

static bool time_ns_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns;
  return true;
}

static bool add_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));
  
  x->as_time.months += y.as_time.months;
  x->as_time.ns += y.as_time.ns;
  return true;
}

static bool sub_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));
  
  x->as_time.months -= y.as_time.months;
  x->as_time.ns -= y.as_time.ns;
  return true;
}

static bool mul_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));
  
  x->as_time.months *= y.as_int;
  x->as_time.ns *= y.as_int;
  return true;
}

void cx_init_time(struct cx *cx) {
  cx_time_init(&cx_box_init(cx_set_const(cx, cx_sym(cx, "min-time"), false),
			    cx->time_type)->as_time,
	       INT32_MIN, INT64_MIN);
  
  cx_time_init(&cx_box_init(cx_set_const(cx, cx_sym(cx, "max-time"), false),
			    cx->time_type)->as_time,
	       INT32_MAX, INT64_MAX);
  
  cx_add_cfunc(cx, "years",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       years_imp);

  cx_add_cfunc(cx, "months",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       months_imp);
  
  cx_add_cfunc(cx, "days",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       days_imp);
  
  cx_add_cfunc(cx, "h",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       h_imp);
  
  cx_add_cfunc(cx, "m",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       m_imp);

  cx_add_cfunc(cx, "s",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       s_imp);
  
  cx_add_cfunc(cx, "ms",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       ms_imp);
  
  cx_add_cfunc(cx, "us",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       us_imp);
  
  cx_add_cfunc(cx, "ns",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       ns_imp);  

  cx_add_cfunc(cx, "time",
	       cx_args(cx_arg("in", cx->vect_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       vect_time_imp);
  
  cx_add_cfunc(cx, "now", cx_args(), cx_args(cx_arg(NULL, cx->time_type)), now_imp);

  cx_add_cfunc(cx, "date",
	       cx_args(cx_arg("in", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       time_date_imp);
  
  cx_add_cfunc(cx, "time",
	       cx_args(cx_arg("in", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       time_time_imp);

  cx_add_cfunc(cx, "year",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_years_imp);
  
  cx_add_cfunc(cx, "years",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_years_imp);
  
  cx_add_cfunc(cx, "month",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       month_imp);
  
  cx_add_cfunc(cx, "months",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_months_imp);
  
  cx_add_cfunc(cx, "day",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_day_imp);
  
  cx_add_cfunc(cx, "days",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_days_imp);

  cx_add_cfunc(cx, "hour",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),	       
	       hour_imp);
  
  cx_add_cfunc(cx, "minute",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       minute_imp);
  
  cx_add_cfunc(cx, "second",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       second_imp);
  
  cx_add_cfunc(cx, "nsecond",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       nsecond_imp);
  
  cx_add_cfunc(cx, "h",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),	       
	       time_h_imp);
  
  cx_add_cfunc(cx, "m",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_m_imp);
  
  cx_add_cfunc(cx, "s",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_s_imp);

  cx_add_cfunc(cx, "ms",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),	       
	       time_ms_imp);
  
  cx_add_cfunc(cx, "us",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_us_imp);
  
  cx_add_cfunc(cx, "ns",
	       cx_args(cx_arg("t", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       time_ns_imp);
  
  cx_add_cfunc(cx, "+", 
	       cx_args(cx_arg("x", cx->time_type), cx_arg("y", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       add_imp);
  
  cx_add_cfunc(cx, "-", 
	       cx_args(cx_arg("x", cx->time_type), cx_arg("y", cx->time_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       sub_imp);
  
  cx_add_cfunc(cx, "*", 
	       cx_args(cx_arg("x", cx->time_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->time_type)),
	       mul_imp);

  cx_add_cxfunc(cx, "today",
		cx_args(), cx_args(cx_arg(NULL, cx->time_type)),
		"now date");
}

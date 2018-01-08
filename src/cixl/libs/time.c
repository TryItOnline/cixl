#include <errno.h>
#include <time.h>

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

  if (vs->imp.count > 7) {
    cx_error(cx, cx->row, cx->col, "Too many time values");
    return false;
  }

  struct cx_time t;
  cx_time_init(&t, 0, 0);
  
  for (int i = 0; i < cx_min(vs->imp.count, 7); i++) {
    struct cx_box *v = cx_vec_get(&vs->imp, i);

    if (v->type != cx->int_type) {
      cx_error(cx, cx->row, cx->col, "Invalid time value");
      return false;
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
  cx_int_t days = 0;

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

static bool lt_imp(struct cx_scope *scope) {
  struct cx_time
    yt = cx_test(cx_pop(scope, false))->as_time,
    xt = cx_test(cx_pop(scope, false))->as_time;
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool =
    xt.months < yt.months ||
    (xt.months == yt.months && xt.ns < yt.ns);
  
  return true;
}

static bool gt_imp(struct cx_scope *scope) {
  struct cx_time
    yt = cx_test(cx_pop(scope, false))->as_time,
    xt = cx_test(cx_pop(scope, false))->as_time;
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool =
    xt.months > yt.months ||
    (xt.months == yt.months && xt.ns > yt.ns);
  
  return true;
}

void cx_init_time(struct cx *cx) {
  cx_add_func(cx, "years", cx_arg(cx->int_type))->ptr = years_imp;
  cx_add_func(cx, "months", cx_arg(cx->int_type))->ptr = months_imp;
  cx_add_func(cx, "days", cx_arg(cx->int_type))->ptr = days_imp;
  cx_add_func(cx, "h", cx_arg(cx->int_type))->ptr = h_imp;
  cx_add_func(cx, "m", cx_arg(cx->int_type))->ptr = m_imp;
  cx_add_func(cx, "s", cx_arg(cx->int_type))->ptr = s_imp;
  cx_add_func(cx, "ms", cx_arg(cx->int_type))->ptr = ms_imp;
  cx_add_func(cx, "us", cx_arg(cx->int_type))->ptr = us_imp;
  cx_add_func(cx, "ns", cx_arg(cx->int_type))->ptr = ns_imp;  

  
  cx_add_func(cx, "time", cx_arg(cx->vect_type))->ptr = vect_time_imp;
  cx_add_func(cx, "now")->ptr = now_imp;

  cx_add_func(cx, "date", cx_arg(cx->time_type))->ptr = time_date_imp;
  cx_add_func(cx, "time", cx_arg(cx->time_type))->ptr = time_time_imp;

  cx_add_func(cx, "year", cx_arg(cx->time_type))->ptr = time_years_imp;
  cx_add_func(cx, "years", cx_arg(cx->time_type))->ptr = time_years_imp;
  cx_add_func(cx, "month", cx_arg(cx->time_type))->ptr = month_imp;
  cx_add_func(cx, "months", cx_arg(cx->time_type))->ptr = time_months_imp;
  cx_add_func(cx, "day", cx_arg(cx->time_type))->ptr = time_day_imp;
  cx_add_func(cx, "days", cx_arg(cx->time_type))->ptr = time_days_imp;

  cx_add_func(cx, "hour", cx_arg(cx->time_type))->ptr = hour_imp;
  cx_add_func(cx, "minute", cx_arg(cx->time_type))->ptr = minute_imp;
  cx_add_func(cx, "second", cx_arg(cx->time_type))->ptr = second_imp;
  cx_add_func(cx, "nsecond", cx_arg(cx->time_type))->ptr = nsecond_imp;
  
  cx_add_func(cx, "h", cx_arg(cx->time_type))->ptr = time_h_imp;
  cx_add_func(cx, "m", cx_arg(cx->time_type))->ptr = time_m_imp;
  cx_add_func(cx, "s", cx_arg(cx->time_type))->ptr = time_s_imp;

  cx_add_func(cx, "ms", cx_arg(cx->time_type))->ptr = time_ms_imp;
  cx_add_func(cx, "us", cx_arg(cx->time_type))->ptr = time_us_imp;
  cx_add_func(cx, "ns", cx_arg(cx->time_type))->ptr = time_ns_imp;

  
  cx_add_func(cx, "+",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = add_imp;
  cx_add_func(cx, "-",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = sub_imp;
  cx_add_func(cx, "*",
	      cx_arg(cx->time_type), cx_arg(cx->int_type))->ptr = mul_imp;

  cx_add_func(cx, "<",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = lt_imp;
  cx_add_func(cx, ">",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = gt_imp;

  cx_test(cx_eval_str(cx, "func: today() now date;"));
}

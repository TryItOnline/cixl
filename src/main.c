#include <string.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/libs/cond.h"
#include "cixl/libs/func.h"
#include "cixl/libs/io.h"
#include "cixl/libs/iter.h"
#include "cixl/libs/math.h"
#include "cixl/libs/meta.h"
#include "cixl/libs/pair.h"
#include "cixl/libs/rec.h"
#include "cixl/libs/ref.h"
#include "cixl/libs/stack.h"
#include "cixl/libs/str.h"
#include "cixl/libs/table.h"
#include "cixl/libs/time.h"
#include "cixl/libs/type.h"
#include "cixl/libs/var.h"
#include "cixl/libs/vect.h"
#include "cixl/op.h"
#include "cixl/repl.h"
#include "cixl/scope.h"
#include "cixl/scope.h"
#include "cixl/types/str.h"
#include "cixl/types/vect.h"

int main(int argc, char *argv[]) {
  struct cx cx;
  cx_init(&cx);
  cx_init_cond(&cx);
  cx_init_func(&cx);
  cx_init_io(&cx);
  cx_init_iter(&cx);
  cx_init_stack(&cx);
  cx_init_pair(&cx);
  cx_init_math(&cx);
  cx_init_type(&cx);
  cx_init_vect(&cx);
  cx_init_rec(&cx);
  cx_init_ref(&cx);
  cx_init_str(&cx);
  cx_init_table(&cx);
  cx_init_time(&cx);
  cx_init_var(&cx);
  cx_init_meta(&cx);

  bool emit = false;
  
  for (int i=1; i < argc && *argv[i] == '-'; i++, argc--) {
    if (strcmp(argv[i], "-e") == 0) {
      emit = true;
    } else {
      printf("Invalid option %s\n", argv[i]);
      return -1;
    }
  }
  
  if (argc == 1 && !emit) {
    cx_repl(&cx, stdin, stdout);
  } else {
    if (emit) {
      cx_emit_tests(&cx);
    } else {
      for (int i = 2; i < argc; i++) {
	cx_box_init(cx_push(cx.main), cx.str_type)->as_str = cx_str_new(argv[i]);
      }
      
      cx_load(&cx, argv[1]);
      
      cx_do_vec(&cx.errors, struct cx_error, e) {
	fprintf(stderr, "Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
	cx_vect_dump(&e->stack, stderr);
	fputs("\n\n", stderr);
	cx_error_deinit(e);
      }
	
      cx_vec_clear(&cx.errors);
    }
  }

  cx_deinit(&cx);
  return 0;
}

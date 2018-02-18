#include <errno.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/buf.h"
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
  int argi = 1;
  
  for (; argi < argc && *argv[argi] == '-'; argi++) {
    if (strcmp(argv[argi], "-e") == 0) {
      emit = true;
    } else {
      fprintf(stderr, "Invalid option %s\n", argv[argi]);
      return -1;
    }
  }
  
  if (argi == argc && !emit) {
    cx_repl(&cx, stdin, stdout);
  } else {
    if (emit) {
      struct cx_bin *bin = cx_bin_new();

      if (argi == argc) {
	fputs("Missing filename\n", stderr);
	return -1;
      }

      if (!cx_load(&cx, argv[argi++], bin)) {
	cx_dump_errors(&cx, stderr);
	return -1;
      }

      struct cx_buf cmd;
      cx_buf_open(&cmd);
      fputs("gcc -x c -std=gnu1x -Wall -Werror -O2 -g - -lcixl", cmd.stream);

      for (; argi < argc; argi++) {
	fprintf(cmd.stream, " %s", argv[argi]);
      }

      cx_buf_close(&cmd);
      FILE *out = popen(cmd.data, "w");

      if (!out) {
	fprintf(stderr, "Failed executing compiler: %d\n%s\n", errno, cmd.data);
	free(cmd.data);
	return -1;
      }

      free(cmd.data);

      fputs("#include \"cixl/bin.h\"\n"
	    "#include \"cixl/call.h\"\n"
	    "#include \"cixl/cx.h\"\n"
            "#include \"cixl/error.h\"\n"
            "#include \"cixl/libs/cond.h\"\n"
            "#include \"cixl/libs/func.h\"\n"
            "#include \"cixl/libs/io.h\"\n"
            "#include \"cixl/libs/iter.h\"\n"
            "#include \"cixl/libs/math.h\"\n"
            "#include \"cixl/libs/meta.h\"\n"
            "#include \"cixl/libs/pair.h\"\n"
            "#include \"cixl/libs/rec.h\"\n"
            "#include \"cixl/libs/ref.h\"\n"
            "#include \"cixl/libs/stack.h\"\n"
            "#include \"cixl/libs/str.h\"\n"
            "#include \"cixl/libs/table.h\"\n"
            "#include \"cixl/libs/time.h\"\n"
            "#include \"cixl/libs/type.h\"\n"
            "#include \"cixl/libs/var.h\"\n"
            "#include \"cixl/libs/vect.h\"\n"
            "#include \"cixl/op.h\"\n"
            "#include \"cixl/scan.h\"\n"
            "#include \"cixl/scope.h\"\n"
            "#include \"cixl/types/func.h\"\n"
            "#include \"cixl/types/lambda.h\"\n"
            "#include \"cixl/types/str.h\"\n"
            "#include \"cixl/types/vect.h\"\n\n",
	    out);

      if (!cx_emit(bin, out, &cx)) {
	cx_dump_errors(&cx, stderr);
	cx_bin_deref(bin);
	pclose(out);
	return -1;
      }
      
      cx_bin_deref(bin);
      
      fputs("int main() {\n"
	    "  struct cx cx;\n"
	    "  cx_init(&cx);\n"
	    "  cx_init_cond(&cx);\n"
	    "  cx_init_func(&cx);\n"
	    "  cx_init_io(&cx);\n"
	    "  cx_init_iter(&cx);\n"
	    "  cx_init_stack(&cx);\n"
	    "  cx_init_pair(&cx);\n"
	    "  cx_init_math(&cx);\n"
	    "  cx_init_type(&cx);\n"
	    "  cx_init_vect(&cx);\n"
	    "  cx_init_rec(&cx);\n"
	    "  cx_init_ref(&cx);\n"
	    "  cx_init_str(&cx);\n"
	    "  cx_init_table(&cx);\n"
	    "  cx_init_time(&cx);\n"
	    "  cx_init_var(&cx);\n"
	    "  cx_init_meta(&cx);\n\n"
	    "  if (!eval(&cx)) {\n"
	    "    cx_dump_errors(&cx, stderr);\n"
	    "    return -1;\n"
	    "  }\n\n"
	    "  cx_deinit(&cx);\n"
	    "  return 0;\n"
	    "}",
	    out);
      
      pclose(out);
    } else {
      if (argi == argc) {
	fputs("Error: Missing file\n", stderr);
	return -1;
      }
      
      char *fn = argv[argi++];
      
      for (; argi < argc; argi++) {
	cx_box_init(cx_push(cx.main), cx.str_type)->as_str = cx_str_new(argv[argi]);
      }

      struct cx_bin *bin = cx_bin_new();
      
      if (!cx_load(&cx, fn, bin) || !cx_eval(bin, 0, &cx)) {
	cx_dump_errors(&cx, stderr);
	cx_bin_deref(bin);
	return -1;
      }

      cx_bin_deref(bin);
    }
  }

  cx_deinit(&cx);
  return 0;
}

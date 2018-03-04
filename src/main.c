#include <errno.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/op.h"
#include "cixl/repl.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/scope.h"

int main(int argc, char *argv[]) {
  struct cx cx;
  cx_init(&cx);
  cx_use(&cx, "cx/meta");
  
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

      fputs("#include \"cixl/arg.h\"\n"
	    "#include \"cixl/bin.h\"\n"
	    "#include \"cixl/call.h\"\n"
	    "#include \"cixl/cx.h\"\n"
            "#include \"cixl/error.h\"\n"
            "#include \"cixl/func.h\"\n"
            "#include \"cixl/lambda.h\"\n"
            "#include \"cixl/rec.h\"\n"
            "#include \"cixl/op.h\"\n"
            "#include \"cixl/scope.h\"\n"
            "#include \"cixl/stack.h\"\n"
            "#include \"cixl/str.h\"\n\n",
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
	    "  cx_init(&cx);\n\n"
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
      struct cx_scope *s = cx_scope(&cx, 0);

      for (; argi < argc; argi++) {
	cx_box_init(cx_push(s), cx.str_type)->as_str = cx_str_new(argv[argi]);
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

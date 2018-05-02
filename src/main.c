#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>

#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/link.h"
#include "cixl/mfile.h"
#include "cixl/op.h"
#include "cixl/repl.h"
#include "cixl/scope.h"

struct cx cx;

static void deinit_cx() {
  cx_dump_errors(&cx, stderr);
  cx_deinit(&cx);
}

static void on_abort(int _) {
  cx_dump_errors(&cx, stderr);
  raise(SIGTERM);
}

struct cx_bin *bin;
static void deinit_bin() { cx_bin_deref(bin); }

int main(int argc, char *argv[]) {
  srand((ptrdiff_t)argv + clock());

  cx_init(&cx);
  cx_test(atexit(deinit_cx) == 0);
  
  if (signal(SIGABRT, on_abort) == SIG_ERR) {
    cx_error(&cx, cx.row, cx.col, "Failed installing ABRT handler: %d", errno);
    return -1;
  }

  if (signal(SIGSEGV, on_abort) == SIG_ERR) {
    cx_error(&cx, cx.row, cx.col, "Failed installing SEGV handler: %d", errno);
    return -1;
  }

  cx_init_libs(&cx);
  cx_use(&cx, "cx/io", "include:");
  cx_use(&cx, "cx/meta", "lib:", "use:");
  cx_use(&cx, "cx/sys", "#args", "init:", "link:");
  
  bool emit = false;
  bool compile = false;
  int argi = 1;
  
  for (; argi < argc && *argv[argi] == '-'; argi++) {
    if (strcmp(argv[argi], "-e") == 0) {
      emit = true;
    } else if (strcmp(argv[argi], "-c") == 0) {
      compile = true;
    } else {
      fprintf(stderr, "Invalid option %s\n", argv[argi]);
      return -1;
    }
  }
  
  if (argi == argc && !emit) {    
    cx_repl(&cx, stdin, stdout);
  } else {
    if (emit) {
      if (argi == argc) {
	fputs("Missing filename\n", stderr);
	return -1;
      }

      const char *fname = argv[argi++];
      bin = cx_bin_new();
      cx_test(atexit(deinit_bin) == 0);
      
      if (!cx_load(&cx, fname, bin)) {
	cx_dump_errors(&cx, stderr);
	return -1;
      }

      if (compile) {	
	if (!cx_emit_file(&cx, bin, stdout)) {
	  cx_dump_errors(&cx, stderr);
	  return -1;
	}	
      } else {	
	struct cx_mfile cmd;
	cx_mfile_open(&cmd);
      
	fputs("gcc -x c -std=gnu1x "
	      "-Wall -Werror "
	      "-Wno-unused-label -Wno-unused-function -Wno-unused-variable "
	      "-Wno-unused-but-set-variable "
	      "- -Bstatic",
	      cmd.stream);

	if (cx.links.count) {
	  for (struct cx_link *l = cx_vec_peek(&cx.links, 0);
	       l >= (struct cx_link *)cx.links.items;
	       l--) {
	    fprintf(cmd.stream, " -l%s", l->id+3);
	  }
	}
	
	fputs(" -lcixl -Bdynamic -ldl -lm", cmd.stream);
	for (; argi < argc; argi++) { fprintf(cmd.stream, " %s", argv[argi]); }
	cx_mfile_close(&cmd);

	printf("%s\n", cmd.data);
	
	FILE *out = popen(cmd.data, "w");
	
	if (!out) {
	  fprintf(stderr, "Failed executing compiler: %d\n%s\n", errno, cmd.data);
	  free(cmd.data);
	  return -1;
	}

	free(cmd.data);

	if (!cx_emit_file(&cx, bin, out)) {
	  cx_dump_errors(&cx, stderr);
	  return -1;
	}

	pclose(out);
      }
    } else {
      if (argi == argc) {
	fputs("Error: Missing file\n", stderr);
	return -1;
      }
      
      char *fn = argv[argi++];
      cx_push_args(&cx, argc-argi, argv+argi);
      bin = cx_bin_new();
      cx_test(atexit(deinit_bin) == 0);
      
      if (!cx_load(&cx, fn, bin) || !cx_eval(bin, 0, -1, &cx)) {
	cx_dump_errors(&cx, stderr);
	return -1;
      }
    }
  }

  return 0;
}

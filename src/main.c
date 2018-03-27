#include <errno.h>
#include <string.h>
#include <time.h>
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

int main(int argc, char *argv[]) {
  srand((ptrdiff_t)argv + clock());

  struct cx cx;
  cx_init(&cx);
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
      cx_deinit(&cx);
      return -1;
    }
  }
  
  if (argi == argc && !emit) {    
    cx_repl(&cx, stdin, stdout);
  } else {
    if (emit) {
      if (argi == argc) {
	fputs("Missing filename\n", stderr);
	cx_deinit(&cx);
	return -1;
      }

      const char *fname = argv[argi++];
      struct cx_bin *bin = cx_bin_new();
	
      if (!cx_load(&cx, fname, bin)) {
	cx_dump_errors(&cx, stderr);
	cx_bin_deref(bin);
	cx_deinit(&cx);
	return -1;
      }

      if (compile) {	
	if (!cx_emit_file(&cx, bin, stdout)) {
	  cx_dump_errors(&cx, stderr);
	  cx_bin_deref(bin);
	  cx_deinit(&cx);
	  return -1;
	}	
      } else {	
	struct cx_mfile cmd;
	cx_mfile_open(&cmd);
      
	fputs("gcc -x c -std=gnu1x -O2 -g "
	      "-Wall -Werror -Wno-unused-function -Wno-unused-but-set-variable "
	      "- -lcixl -ldl",
	      cmd.stream);

	for (struct cx_link *l = cx_vec_peek(&cx.links, 0);
	     l >= (struct cx_link *)cx.links.items;
	     l--) {
	  fprintf(cmd.stream, " -l%s", l->id+3);
	}
	
	for (; argi < argc; argi++) { fprintf(cmd.stream, " %s", argv[argi]); }
	cx_mfile_close(&cmd);

	printf("%s\n", cmd.data);
	
	FILE *out = popen(cmd.data, "w");
	
	if (!out) {
	  fprintf(stderr, "Failed executing compiler: %d\n%s\n", errno, cmd.data);
	  free(cmd.data);
	  cx_bin_deref(bin);
	  cx_deinit(&cx);
	  return -1;
	}

	free(cmd.data);

	if (!cx_emit_file(&cx, bin, out)) {
	  cx_dump_errors(&cx, stderr);
	  cx_bin_deref(bin);
	  cx_deinit(&cx);
	  return -1;
	}

	cx_bin_deref(bin);
	pclose(out);
      }
    } else {
      if (argi == argc) {
	fputs("Error: Missing file\n", stderr);
	cx_deinit(&cx);
	return -1;
      }
      
      char *fn = argv[argi++];
      cx_push_args(&cx, argc-argi, argv+argi);
      struct cx_bin *bin = cx_bin_new();
      
      if (!cx_load(&cx, fn, bin) || !cx_eval(bin, 0, -1, &cx)) {
	cx_dump_errors(&cx, stderr);
	cx_bin_deref(bin);
	cx_deinit(&cx);
	return -1;
      }

      cx_bin_deref(bin);
    }
  }

  cx_deinit(&cx);
  return 0;
}

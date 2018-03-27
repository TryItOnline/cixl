#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/link.h"
#include "cixl/stack.h"
#include "cixl/str.h"

char *cx_emit_id(const char *prefix, const char *in) {
  char *out = malloc(strlen(prefix)+strlen(in)*2+2), *p = out;
  for (const char *c=prefix; *c; c++) { *p++ = *c; }
  *p++ = '_';
  
  for (const char *c=in; *c; c++) {
    switch (*c) {
    case ' ':
      break;
    case '+':
      *p++ = 'A';
      break;
    case '`':
      *p++ = 'B';
    case '&':
      *p++ = 'C';
      break;
    case '|':
      *p++ = 'D';
      break;
    case '/':
      *p++ = 'E';
      break;
    case '.':
      *p++ = 'F';
      break;      
    case '=':
      *p++ = 'G';
      break;
    case '!':
      *p++ = 'H';
      break;
    case '<':
      *p++ = 'I';
      break;
    case '>':
      *p++ = 'J';
      break;
    case '*':
      *p++ = 'K';
      break;
    case '%':
      *p++ = 'L';
      break;
    case '?':
      *p++ = 'M';
      break;
    case '-':
      *p++ = 'N';
      break;
    case '~':
      *p++ = 'S';
      break;
    default:
      *(p++) = *c;
    }
  }

  *p = 0;
  return out;
}

void cx_push_args(struct cx *cx, int argc, char *argv[]) {
  struct cx_stack *args =
    cx_test(cx_get_const(*cx->lib, cx_sym(cx, "args"), false))->as_ptr;
  
  for (int i=0; i < argc; i++) {
    const char *a = argv[i];
    cx_box_init(cx_vec_push(&args->imp), cx->str_type)->as_str =
      cx_str_new(a, strlen(a));
  }
}

bool cx_emit_file(struct cx *cx, struct cx_bin *bin, FILE *out) {
  bool ok = false;

  fputs("#include <stdlib.h>\n"
	"#include <string.h>\n"
	"#include <time.h>\n"
	"#include \"cixl/arg.h\"\n"
	"#include \"cixl/bin.h\"\n"
	"#include \"cixl/call.h\"\n"
	"#include \"cixl/catch.h\"\n"
	"#include \"cixl/cx.h\"\n"
	"#include \"cixl/emit.h\"\n"
	"#include \"cixl/error.h\"\n"
	"#include \"cixl/func.h\"\n"
	"#include \"cixl/lambda.h\"\n"
	"#include \"cixl/rec.h\"\n"
	"#include \"cixl/op.h\"\n"
	"#include \"cixl/scope.h\"\n"
	"#include \"cixl/stack.h\"\n"
	"#include \"cixl/str.h\"\n\n",
	out);

  cx_do_vec(&cx->inits, struct cx_str *, i) {
    fprintf(out, "extern bool cx_init_%s(struct cx *cx);\n\n", (*i)->data);
  }
  
  if (!cx_emit(bin, out, cx)) { goto exit; }
      
  fputs("int main(int argc, char *argv[]) {\n"
	"  srand((ptrdiff_t)argv + clock());\n"
	"  struct cx cx;\n"
	"  cx_init(&cx);\n"
	"  cx_init_libs(&cx);\n"
	"  cx_use(&cx, \"cx/abc\", \"Str\");\n"
        "  cx_use(&cx, \"cx/io\", \"include:\");\n"
        "  cx_use(&cx, \"cx/meta\", \"lib:\", \"use:\");\n"
        "  cx_use(&cx, \"cx/sys\", \"#args\");\n"
        "  cx_push_args(&cx, argc, argv);\n\n"

	"  for (int i=1; i < argc; i++) {\n"
	"    const char *a = argv[i];\n"
	"    cx_box_init(cx_push(cx.root_scope), cx.str_type)->as_str = "
	      "cx_str_new(a, strlen(a));\n"
	"  }\n\n"

	"  if (!eval(&cx)) {\n"
	"    cx_dump_errors(&cx, stderr);\n"
	"    return -1;\n"
	"  }\n\n"

	"  cx_deinit(&cx);\n"
	"  return 0;\n"
	"}",
	out);

  ok = true;
 exit:
  return ok;
}

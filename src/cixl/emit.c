#include <stdlib.h>
#include <string.h>

#include "cixl/emit.h"

char *cx_emit_id(const char *prefix, const char *in) {
  char *out = malloc(strlen(prefix)+strlen(in)*2+2), *p = out;
  for (const char *c=prefix; *c; c++) { *p++ = *c; }
  *p++ = '_';
  
  for (const char *c=in; *c; c++) {
    switch (*c) {
    case ' ':
      break;      
    case '?':
      *p++ = 'q';
      *p++ = 'U';
      break;
    case '!':
      *p++ = 'e';
      *p++ = 'X';
      break;
    case '=':
      *p++ = 'e';
      *p++ = 'Q';
      break;
    case '<':
      *p++ = 'l';
      *p++ = 'T';
      break;
    case '>':
      *p++ = 'g';
      *p++ = 'T';
      break;
    case '+':
      *p++ = 'a';
      *p++ = 'D';
      break;
    case '-':
      *p++ = 's';
      *p++ = 'U';
      break;
    case '*':
      *p++ = 'm';
      *p++ = 'U';
      break;
    case '/':
      *p++ = 'd';
      *p++ = 'I';
      break;
    case '%':
      *p++ = 'p';
      *p++ = 'E';
      break;
    case '~':
      *p++ = 't';
      *p++ = 'I';
      break;
    case '|':
      *p++ = 'b';
      *p++ = 'A';
      break;
    default:
      *(p++) = *c;
    }
  }

  *p = 0;
  return out;
}

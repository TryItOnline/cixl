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
    case '+':
      *p++ = 'A';
      *p++ = 'D';
      break;
    case '&':
      *p++ = 'A';
      *p++ = 'M';
      break;
    case '|':
      *p++ = 'B';
      *p++ = 'A';
      break;
    case '/':
      *p++ = 'D';
      *p++ = 'I';
      break;
    case '=':
      *p++ = 'E';
      *p++ = 'Q';
      break;
    case '!':
      *p++ = 'E';
      *p++ = 'X';
      break;
    case '<':
      *p++ = 'L';
      *p++ = 'T';
      break;
    case '>':
      *p++ = 'G';
      *p++ = 'T';
      break;
    case '*':
      *p++ = 'M';
      *p++ = 'U';
      break;
    case '%':
      *p++ = 'P';
      *p++ = 'E';
      break;
    case '?':
      *p++ = 'Q';
      *p++ = 'U';
      break;
    case '-':
      *p++ = 'S';
      *p++ = 'U';
      break;
    case '~':
      *p++ = 'T';
      *p++ = 'I';
      break;
    default:
      *(p++) = *c;
    }
  }

  *p = 0;
  return out;
}

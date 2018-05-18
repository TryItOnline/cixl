#ifndef CX_COMPAT_UCONTEXT_H
#define CX_COMPAT_UCONTEXT_H

#if __APPLE__ && __MACH__
  #include <sys/ucontext.h>
#else 
  #include <ucontext.h>
#endif

#endif

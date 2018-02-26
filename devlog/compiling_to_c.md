## Compiling to C
#### 2018-02-17

### Intro
While [Cixl](https://github.com/basic-gongfu/cixl)'s primary use case is scripting, in some situations it's just not very practical to drag a script around and require that clients have everything installed. Since the language is implemented in, and designed to be embedded into C; and since ease of maintenance is more important than raw speed in this case; compiling to C seemed like a good compromise. The short story is that Cixl now supports compiling blocks of code into GNU-C functions which execute the corresponding operations when called; compiled code reuses much of the interpreter and runs 50% faster.

Calling ```cixl -e``` compiles the specified file to a native, statically linked executable. Flags following the filename are passed straight to ```gcc```.

```
$ git clone https://github.com/basic-gongfu/cixl.git
$ cd cixl
$ mkdir build
$ cd build
$ cmake ..
$ sudo make install
$ cixl -e ../examples/guess.cx -o guess
$ ls -all guess
-rwxrwxr-x 1 a a 941856 Feb 17 18:53 guess
$ ./guess
Your guess: 50
Too high!
Your guess: 25
Too low!
Your guess: 37
Too low!
Your guess: 43
Correct!
$ 
```

The REPL has been updated to display compiled as well as interpreted bmips on startup, which gives a hint at what kind of relative performance to expect. And the ```Bin``` type has been extended with support for emitting C from within Cixl.

```
Cixl v0.9.1, 18571/29582 bmips

Press Return twice to evaluate.

   | Bin new % '1 2 +' compile emit
...
['bool eval(struct cx *cx) {
  bool _eval(struct cx *cx) {
    static bool init = true;

    static struct cx_func *func_AD;

    if (init) {
      init = false;

      func_AD = cx_get_func(cx, "+", false);
    }

    static void *op_labels[4] = {
      &&op0, &&op1, &&op2, &&op3};

    goto *op_labels[cx->pc];

  op0: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 0; cx->row = 1; cx->col = 1;
      if (cx->stop) { return true; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 1;
    }

  op1: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 1; cx->row = 1; cx->col = 3;
      if (cx->stop) { return true; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 2;
    }

  op2: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 2; cx->row = 1; cx->col = 4;
      if (cx->stop) { return true; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_AD;
      struct cx_fimp *imp = cx_func_match(func, s, 0);

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op3:
    cx->stop = false;
    return true;
  }

  struct cx_bin *bin = cx_bin_new();
  bin->eval = _eval;
  bool ok = cx_eval(bin, 0, cx);
  cx_bin_deref(bin);
  return ok;
}
'r1]
```

### Implementation
It was clear from the start that the compiler had to support embedded use, I also wanted to share as much of the implementation as possible with the interpreter to reduce the maintenance burden. The resulting code implements a threaded state machine based on computed gotos with unrolled loops that calls into the supplied interpreter. Types, functions, constants and symbols are cached inside the C function on first call and reused.

Included below is a slightly more elaborate example with a function definition.

```
| Bin new % 'func: foo(x y Int) (Int) $x $y +; 35 7 foo' compile emit
...

['bool eval(struct cx *cx) {
  bool _eval(struct cx *cx) {
    static bool init = true;

    static struct cx_func *func_AD;
    static struct cx_func *func_foo;
    static struct cx_fimp *func_foo_IntInt;
    static struct cx_sym sym_x;
    static struct cx_sym sym_y;
    static struct cx_type *type_Int;

    if (init) {
      init = false;

      {
	struct cx_arg args[2] = {
	  cx_arg("x", cx_get_type(cx, "Int", false)),
	  cx_arg("y", cx_get_type(cx, "Int", false))};

	struct cx_arg rets[1] = {
	  cx_arg(NULL, cx_get_type(cx, "Int", false))};

	cx_add_func(cx, "foo", 2, args, 1, rets);
      }

      {
	struct cx_func *func = cx_get_func(cx, "foo", false);
	struct cx_fimp *imp = cx_get_fimp(func, "Int Int", false);
	imp->bin = cx_bin_ref(cx->bin);
	imp->start_pc = 2;
	imp->nops = 6;
      }

      func_AD = cx_get_func(cx, "+", false);
      func_foo = cx_get_func(cx, "foo", false);
      func_foo_IntInt = cx_get_fimp(func_foo, "Int Int", false);
      sym_x = cx_sym(cx, "x");
      sym_y = cx_sym(cx, "y");
      type_Int = cx_get_type(cx, "Int", false);
    }

    static void *op_labels[12] = {
      &&op0, &&op1, &&op2, &&op3, &&op4, &&op5, &&op6, &&op7, &&op8, &&op9, &&op10, 
      &&op11};

    goto *op_labels[cx->pc];

  op0: { /* CX_TMACRO CX_OFIMPDEF */
      cx->pc = 0; cx->row = 1; cx->col = 5;
      if (cx->stop) { return true; }
      func_foo_IntInt->scope = cx_scope_ref(cx_scope(cx, 0));
    }

  op1: { /* CX_TMACRO CX_OFIMP */
      cx->pc = 1; cx->row = 1; cx->col = 5;
      if (cx->stop) { return true; }
      goto op8;
    }

  op2: { /* CX_TMACRO CX_OBEGIN */
      cx->pc = 2; cx->row = 1; cx->col = 5;
      if (cx->stop) { return true; }
      struct cx_scope *parent = func_foo_IntInt->scope;
      cx_begin(cx, parent);
    }

  op3: { /* CX_TMACRO CX_OPUTARGS */
      cx->pc = 3; cx->row = 1; cx->col = 5;
      if (cx->stop) { return true; }
      struct cx_scope
	*ds = cx_scope(cx, 0),
	*ss = ds->stack.count ? ds : cx_scope(cx, 1);

      *cx_put_var(ds, sym_y, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
      cx_vec_delete(&ss->stack, ss->stack.count-1);
      *cx_put_var(ds, sym_x, true) = *(struct cx_box *)cx_vec_peek(&ss->stack, 0);
      cx_vec_delete(&ss->stack, ss->stack.count-1);
    }

  op4: { /* CX_TID CX_OGETVAR */
      cx->pc = 4; cx->row = 1; cx->col = 21;
      if (cx->stop) { return true; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_x, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op5: { /* CX_TID CX_OGETVAR */
      cx->pc = 5; cx->row = 1; cx->col = 24;
      if (cx->stop) { return true; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_box *v = cx_get_var(s, sym_y, false);
      if (!v) { return false; }
      cx_copy(cx_push(s), v);
    }

  op6: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 6; cx->row = 1; cx->col = 27;
      if (cx->stop) { return true; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_AD;
      struct cx_fimp *imp = cx_func_match(func, s, 0);

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op7: { /* CX_TFUNC CX_ORETURN */
      cx->pc = 7; cx->row = 1; cx->col = 27;
      if (cx->stop) { return true; }
      struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
      struct cx_scope *s = cx_scope(cx, 0);

      if (call->recalls) {
	if (s->safe && !cx_fimp_match(func_foo_IntInt, s)) {
	  cx_error(cx, cx->row, cx->col, "Recall not applicable");
	  return false;
	}

	call->recalls--;
	goto op3;
      } else {
	size_t si = 0;
	struct cx_scope *ds = cx_scope(cx, 1);
	cx_vec_grow(&ds->stack, ds->stack.count+1);

	{
	  if (si == s->stack.count) {
	    cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	    return false;
	  }

	  struct cx_box *v = sv++;
	  si++;

	  if (s->safe) {
	    struct cx_type *t = type_Int;
	    if (!cx_is(v->type, t)) {
	      cx_error(cx, cx->row, cx->col,
		       "Invalid return type.\n"
		       "Expected %s, actual: %s",
		       t->id, v->type->id);
	      return false;
	    }
	  }

	  *(struct cx_box *)cx_vec_push(&ds->stack) = *v;
	}

	if (si < s->stack.count) {
	  cx_error(cx, cx->row, cx->col, "Stack not empty on return");
	  return false;
	}

	cx_vec_clear(&s->stack);
	cx_end(cx);
	struct cx_call *call = cx_vec_pop(&cx->calls);

	if (call->return_pc > -1) {
	  cx->pc = call->return_pc;
	  cx_call_deinit(call);
	  goto *op_labels[cx->pc];
	}

	cx_call_deinit(call);
	cx->stop = true;
      }
    }

  op8: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 8; cx->row = 1; cx->col = 31;
      if (cx->stop) { return true; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 35;
    }

  op9: { /* CX_TLITERAL CX_OPUSH */
      cx->pc = 9; cx->row = 1; cx->col = 33;
      if (cx->stop) { return true; }
      cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 7;
    }

  op10: { /* CX_TFUNC CX_OFUNCALL */
      cx->pc = 10; cx->row = 1; cx->col = 34;
      if (cx->stop) { return true; }
      struct cx_scope *s = cx_scope(cx, 0);
      struct cx_func *func = func_foo;
      struct cx_fimp *imp = func_foo_IntInt;

      if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }

      if (!imp) {
	cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
	return false;
      }

      if (!imp->ptr && imp->bin == cx->bin) {
	cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, 11);
	goto op2;
      } else if (!cx_fimp_call(imp, s)) {
	return false;
      }
    }

  op11:
    cx->stop = false;
    return true;
  }

  struct cx_bin *bin = cx_bin_new();
  bin->eval = _eval;
  bool ok = cx_eval(bin, 0, cx);
  cx_bin_deref(bin);
  return ok;
}
'r1]
```

### Performance
The current implementation compiles code that is roughly 50% faster than interpreted. While that may not sound very impressive; the interpreter was already closing in on Python3, which is pretty fast as far as interpreters go. The profile is mostly dominated by the shared implementation at this point, which means that any major improvements will benefit both modes of operation.

### Mind the Gap
The worst thing that could happen is that you get an error about emit not being implemented, followed by a failed attempt to compile the code; if that happens, I would appreciate if you took the time to register an issue in the [repo](https://github.com/basic-gongfu/cixl).

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
## Gradual Generics
#### 2018-04-12

### Intro
One of the games I played growing up is called Plockepinn in Swedish, which literally means picking sticks; both the name and the game are as Swedish as they get. The setting is a pile of matches or sticks with a group of people seated around it, and the only rule that you may pick as many sticks as you can as long as no other stick moves; the winner has the most sticks once the pile is gone. Recently, finally in posession of enough round tuits to tackle bolting generics onto [Cixl](https://github.com/basic-gongfu/cixl); it occured to me that the way I write code these days has a lot in common with the game. I mostly optimize for maximum momentum and minimal disruption, and prefer adding features gradually to big bang integrations. I've found that the pain of repeatedly rewriting and refactoring in small steps is a fair price to pay for avoiding epic failure modes. And as an added bonus; it enables pausing at any local maximum to gather more experience before deciding where to go from there.

### Generics
I have plenty of scars to show from a previous implementation of generic types in a precursor to [Cixl](https://github.com/basic-gongfu/cixl), which is why I decided to wait longer before diving in this time around. The first time around, I designed the entire language around generic types from the start; which ended up complicating the design and slowing me down to the point where it didn't make any sense to push it further. With Cixl I'm doing the opposite, adding as much support as needed to provide one feature at a time. One notable difference from other generic type systems I've run into is that Cixl doesn't treat type arguments as an orthogonal dimension to raw types; ```Iter<Int>``` is considered to be compatible with ```Seq<Num>```, for example. I would love to get some more details on why this approach is so unpopular, especially given that it seems to be what most people expect when they first encounter generic types.

### Gradual
I was neck deeep into C++ by the time Java added support for generic types, which is why I was never very excited about Java's implementation. One thing they got right was wildcards, peppering your class hierarchy with abstract types for any combination of type arguments that need to coexist in the same collection gets old fast. Cixl takes an even more gradual approach where you mostly get to choose exactly how specific you want to be. Raw types work more or less as ordinary types; generic types may specify upper bounds for arguments, which are used in place of actual arguments when dealing with raw types. Generic types are currently used for references, optionals, sequences and iterators; which greatly increases the value of these features since it's now possible to use them without loosing type information.

```
   | 42 ref
...
[Ref<Int>(42)r1]

   'foo' set
...
Error in row 1, col 6:
Func not applicable: set
[Ref<Int>(42)r2 'foo'r2]

   _ % 7 set  
...
[Ref<Int>(7)r1]

   | 'foo' iter
...
[Iter<Char>(0x21410a0)r1]

   | 42 Opt<Num> is
...
[#t]

   | 42 Opt<Str> is
...
[#f]
```

### Implementation
The type struct has grown a reference to its raw type, which defaults to the type itself; and a dynamic array of type arguments.

```
struct cx_type {
  ...
  struct cx_type *raw;
  struct cx_vec args;
};
```

```cx_type_get(struct cx_type *, ...)``` may now be used to parameterize the specified type with provided arguments. This means that functions such as ```fork```:

```
cx_add_cfunc(lib, "fork",
	     cx_args(cx_arg("in",    cx->opt_type),
		     cx_arg("out",   cx->opt_type),
		     cx_arg("error", cx->opt_type)),
	     cx_args(cx_arg(NULL, cx->opt_type)),
	     fork_imp);
```

now have the means to express their intent more clearly:

```
cx_add_cfunc(lib, "fork",
	     cx_args(cx_arg("in",    cx_type_get(cx->opt_type, cx->rfile_type)),
		     cx_arg("out",   cx_type_get(cx->opt_type, cx->wfile_type)),
		     cx_arg("error", cx_type_get(cx->opt_type, cx->wfile_type))),
	     cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->proc_type))),
	     fork_imp);
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
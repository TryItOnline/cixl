## Gradual Generics
#### 2018-04-12

### Intro
One of the games I played growing up is called Plockepinn in Swedish, which literally means picking sticks; both name and game are as Swedish as they get. The setting is a pile of matches or sticks with a group of people seated around it, and the only rule that you may pick as many sticks as you can as long as no other stick moves; the winner has the most sticks once the pile is gone. Recently, finally in posession of enough round tuits to tackle bolting generics onto [Cixl](https://github.com/basic-gongfu/cixl); it occured to me that the way I write code these days has a lot in common with the game; I'll mostly optimize for maximum momentum and minimal disruption, preferring to add features gradually and backing off as soon as it starts to look hairy. I've found that the pain of repeatedly rewriting and refactoring in small steps, and ripping out failed attempts; is a fair price to pay for avoiding epic failure modes. And as an added bonus; it enables pausing at any local maximum to gather more experience before deciding where to go from there.

### Generics
I have plenty of scars to show from a previous implementation of generic types in a precursor to [Cixl](https://github.com/basic-gongfu/cixl), which is why I decided to wait longer before diving in this time around. Last time, I designed the entire language around generic types from the start; which ended up complicating the design and slowing me down to the point where it didn't make any sense to push it further. With Cixl I'm doing the opposite, adding as much support as needed to provide one feature at a time. One notable difference from other generic type systems I've run into is that Cixl doesn't treat type arguments as an orthogonal dimension to raw types; ```Iter<Int>``` is considered to be compatible with ```Seq<Num>```, for example. I would love to get some more details on why this approach is so uncommon, especially given that it seems to be what most people expect when they first encounter generic types.

### Gradual
I was already neck deeep into C++ by the time Java added support for generic types, which is why I was never very excited about Java's implementation. One thing they got right is wildcards, peppering your class hierarchy with abstract types for any combination of type arguments that need to coexist in the same collection gets old fast. [Cixl](https://github.com/basic-gongfu/cixl) takes an even more gradual approach where you mostly get to choose exactly how specific you want to be. Raw types work more or less as ordinary types; generic types may specify upper bounds for arguments, which are used in place of actual arguments. Generic types are currently used for references, pairs, optionals, sequences and iterators; which greatly increases the value of these features since it's now possible to use them without loosing type information.

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

   | 42 'foo' . type
...
[Pair<Int Str>]

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

Functions gained support for referring to type arguments by extending the existing syntax for referring to raw types.

```
   /*
     Takes and a start value and returns a result compatible with $in's item type.
   */
   
   func: my-sum(start Arg1:0 in Seq<Num>)(_ Arg1:0)
     $start $in &+ for;
   | 0 10 iter my-sum
...
[45]
```

### Implementation
The type struct has grown a reference to its raw type, which defaults to the type itself; and a dynamic array of type arguments.

```C
struct cx_type {
  ...
  struct cx_type *raw;
  struct cx_vec args;
};
```

```cx_type_get``` may be used to parameterize the specified type with provided arguments, which means that functions such as this:

```C
cx_add_cfunc(lib, "fork",
	     cx_args(cx_arg("in",    cx->opt_type),
		     cx_arg("out",   cx->opt_type),
		     cx_arg("error", cx->opt_type)),
	     cx_args(cx_arg(NULL, cx->opt_type)),
	     fork_imp);
```

now have the means to express their intent more clearly:

```C
cx_add_cfunc(lib, "fork",
	     cx_args(cx_arg("in",    cx_type_get(cx->opt_type, cx->rfile_type)),
		     cx_arg("out",   cx_type_get(cx->opt_type, cx->wfile_type)),
		     cx_arg("error", cx_type_get(cx->opt_type, cx->wfile_type))),
	     cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->proc_type))),
	     fork_imp);
```

Most of the magic happens inside ```cx_is```, the implementation is still somewhat in flux but handles all cases supported so far without slowing things down to a crawl. The new part is the bottom loop, which runs when the parent type takes arguments; it's purpose is to find the least specific subtype of ```parent```'s raw type among ```child```'s supertypes and then compare its arguments to ```parent```'s. Deriving the same type several times with different arguments is supported.

```C
bool cx_is(struct cx_type *child, struct cx_type *parent) {
  if (!parent->args.count) {
    return (parent->tag < child->is.count)
      ? *(struct cx_type **)cx_vec_get(&child->is, parent->tag)
      : false;
  }
  
  if (parent->raw->tag >= child->is.count) { return false; }
  struct cx_type **ce = cx_vec_end(&child->is);

  for (struct cx_type **c = cx_vec_get(&child->is, parent->raw->tag);
       c != ce;
       c++) {
    if (*c && (*c)->raw == parent->raw) {
      if (*c == parent) { return true; }

      struct cx_type
	**ie = cx_vec_end(&(*c)->args),
	**je = cx_vec_end(&parent->args);

      bool ok = true;

      for (struct cx_type
	     **i = cx_vec_start(&(*c)->args),
	     **j = cx_vec_start(&parent->args);
	   i != ie && j != je;
	   i++, j++) {
	if (!cx_is(*i, *j)) {
	  ok = false;
	  break;
	}
      }

      if (ok) { return true; }
    }
  }

  return false;
}
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
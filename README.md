## ![](cixl.png?raw=true)<br/>
<a href="https://paypal.me/basicgongfu"><img alt="Donate using PayPal" src="paypal.png?raw=true"></a><a href="https://liberapay.com/basic-gongfu/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>
#### Cixl - a Lispy Forth in C

Cixl shares many ideas with C, Forth and Common Lisp; as well as the hacker mindset that unites them. The language is implemented as a straight forward 3-stage (parse/compile/eval) interpreter that is designed to be as fast as possible without compromising on simplicity, transparency and flexibility; combined with a C code generator for compiling native executables. The codebase has no external dependencies and is currently hovering around 13 kloc including tests and standard library.

### Status
Examples should work in the most recent version and run clean in ```valgrind```, outside of that I can't really promise much at the moment. Current work is focused on profiling and filling obvious gaps in functionality.

### Getting Started
You may try Cixl online [here](https://tio.run/#cixl). To build it yourself, you'll need a reasonably modern GCC and CMake installed. Building on macOS unfortunately doesn't work yet, as it's missing support for POSIX timers. A basic REPL is included, it's highly recommended to run it through ```rlwrap``` for a less nerve wrecking experience.

```
$ git clone https://github.com/basic-gongfu/cixl.git
$ cd cixl
$ mkdir build
$ cd build
$ cmake ..
$ sudo make install
$ rlwrap cixl

Cixl v0.9.5, 18571/29582 bmips

Press Return twice to evaluate.

   1 2 +
...
[3]

   quit
$
```

### Scripting
When launched with parameters, Cixl interprets the first parameter as filename to load code from, and pushes remaining parameters on the stack.

test.cx
```
#!/usr/local/bin/cixl

use:
  (cx/stack %)
  (cx/str   upper)
  (cx/io    say);

% upper say
```

```
$ ./cixl test.cx foo
FOO

$ sudo cp ./cixl /usr/local/bin
$ chmod +x test.cx
$ ./test.cx foo
FOO
```

### Compiling
Executing ```cixl -e``` compiles the specified file to a native, statically linked executable. Flags following the filename are passed straight to ```gcc```.

```
$ cixl -e cixl/examples/guess.cx -o guess
$ ls -all guess
-rwxrwxr-x 1 a a 941856 Feb 17 18:53 guess
$ ./guess
Your guess: 50
Too high!
Your guess: 25
Too low!
Your guess:
$
```

### Libraries
The entire language is split into libraries to enable building custom languages on top of sub sets of existing functionality. ```use: cx;``` may be used as a short cut to import everything. The REPL starts with everything imported while the interpreter and compiler starts with nothing but ```include:```, ```lib:``` and ```use:```. The following standard libraries are available:

* cx/abc
* cx/bin
* cx/cond
* cx/const
* cx/error
* cx/func
* cx/io
* cx/io/buf
* cx/io/poll
* cx/io/term
* cx/iter
* cx/math
* cx/net
* cx/pair
* cx/rec
* cx/ref
* cx/stack
* cx/str
* cx/sym
* cx/table
* cx/time
* cx/type
* cx/var

The default library is called the ```lobby```.

```
   | cx-lib
...
[Lib(lobby)]

   id
...
[`lobby]

   get-lib
...
[Lib(lobby)]
```

All types and functions belong to a library, ```lib``` may be used to find out which.

```
   | Int lib
...
[Lib(cx/abc)]

   | &=
...
[Func(=)]

   lib
...
[Lib(cx/cond)]

   | &=<Rec Rec>
...
[Fimp(= Rec Rec)]

   lib
...
[Lib(cx/rec)]
```

### Stack Basics
Cixl expects arguments before operations and provides direct access to the stack. Basic stack operations have dedicated operators; ```%``` for copying the last value, ```_``` for dropping it, ```~``` for flipping the last two values and ```|``` for clearing the stack.

```
   | 1 2 3 %
...
[1 2 3 3]

   _
...
[1 2 3]

   ~
...
[1 3 2]

   |
...
[]
```

### Variables
Variables may be bound once per scope using the ```let:``` macro.

```
   | let: foo 'bar';
...$foo
...
['bar'r2]

   | let: foo 'baz';
...
Error in row 1, col 10:
Attempt to rebind variable: 'foo'
[]
```

Multiple names may be bound at the same time by enclosing them in parens.

```
   | let: (x y z) 1 2 3 4 +;
...$x $y $z
...
[1 2 7]
```

Types may be specified for documentation and type checking.

```
   | let: (x y Int z Str) 1 2 3;
...
Error in row 1, col 5:
Expected type Str, actual: Int
[1 2]
```

Since ```let:``` doesn't introduce its own scope, values already on the stack may be bound using the same construct.

```
   | 1 2 3
...let: (x y z);
...$z $y $x
[3 2 1]
```

The same functionality may be accessed symbolically.

```
   | `foo var
...
[#nil]

   | `foo 42 let
...`foo var
...
[42]
```

### Constants
Constants may be bound using the ```define:``` macro. They behave much like variables; but live in a separate, library global namespace prefixed by ```#``` rather than ```$```; and are bound at compile time rather than evaluation.

```
   func: launch-rockets()(_ Int)
...  'Launching rockets!' say
...  42;
...| define: (nrockets Int) launch-rockets;
...
Launching rockets!
[]

   #nrockets
...
[42]
```

### Equality
Two flavors of equality are provided.

Value equality:
```
   | [1 2 3] [1 2 3] =
...
[#t]
```

And identity:
```
   | 'foo' 'foo' ==
...
[#f]

   | 42 42 ==
...
[#t]
```

### Symbols
Symbols are immutable singleton strings that support fast equality checks.

```
   | `foo
...
[`foo]

   = `foo
...
[#t]

   | `foo `bar =
...
[#f]

   | 'baz' sym
...
[`baz]

   str
...
['baz'r1]
```

Unique symbols may be generated by calling ```new```.

```
   | Sym % new ~ new
...
[`S7 `S8]
```

### References
Some values are reference counted; strings, stacks, lambdas etc. Reference counted values display the number of references following ```r``` when printed. Doubling the copy operator results in a deep copy where applicable and defaults to regular copy where not.

```
   | [1 2 3] %
...
[[1 2 3]r2 [1 2 3]r2]

   | [1 2 3] %%
...
[[1 2 3]r1 [1 2 3]r1]
```

References may be created manually, which enables using reference semantics for value types.

```
   | let: r #nil ref;
...$r
...
[Ref(#nil)r2]

   42 set
...$r
...
[Ref(42)r2]

   deref
...
[42]
```

### Scopes
Code enclosed in parens is evaluated in a separate scope, the last value on the stack is automatically returned on scope exit.

```
   | (1 2 3)
...
[3]
```

Variables in the parent scope may be referenced from within, but variables defined inside are not visible from the outside.

```
   | let: foo 1;
...(let: foo 2; $foo)
...$foo
...
[2 1]
```

### Strings

Strings are null terminated, reference counted sequences of characters.

```
   | 'foo' stack
...
[[@f @o @o]r1]
```

Strings may alternatively be iterated by line,

```
   | 'foo@nbar@r@nbaz' lines stack
...
[['foo'r1 'bar'r1 'baz'r1]r1]
```

or by word.

```
   | 'foo,bar-baz!?' words stack
...
[['foo'r1 'bar-baz'r1]r1]
```

Subtracting strings returns the [edit distance](https://en.wikipedia.org/wiki/Levenshtein_distance).

```
   | 'fooxxxbar' 'foobar' -
...
[3]
```

### Characters
Characters are single bytes, a separate unicode type might be added eventually. Literals are preceded by ```@```, or ```@@``` for non-printable characters outside of strings. 

```
   | 'foo@10bar@nbaz'
...
['foo
bar
baz'r1]

   3 get
...
[@@n]
```

### Terminal
```say``` and ```ask``` may be used to perform basic terminal IO.

```
   | 'Hello' say  
...'What@'s your name? ' ask
...
Hello
What's your name? Sifoo
['Sifoo'r1]
```

Code may be loaded from external files using ```load```. The loaded code is evaluated in the current scope by default.

test.cx:
```
+ 2
```

```
   | 1 'test.cx' load
...
[3]
```

### Serialization
Most values support being written to files and read back in. Calling ```write``` on a value will write it's serialized representation to the specified stream.

```
   | now
...
[Time(2018/0/12 1:25:12.123436182)]

   #out ~ write
...
[2018 0 12 1 25 12 123436182] time
[]
```

While calling ```read``` will parse and evaluate one value at a time from the specified stream.

```
   | #in read
...
[Iter(0x5475950)r1]

   next
...
[2018 0 12 1 25 12 123436182] time
[Time(2018/0/12 1:25:12.123436182)]
```

### Files

Files may be opened for reading/writing by calling ```fopen```, the type of the returned file depends on the specified mode. Valid modes are the same as in C, r/w/a(+). Files are closed automatically when the last reference is dropped.

```
   let: f 'test.out' `w fopen;
...$f
...
[RWFile(0x5361130)r1]

   now write
...
[]
```

Any value may be printed to a ```WFile``` using ```print```.

```
   $f ['foo' 42 @@n] print
...
[]
```

Files iterate characters by default, which means that string sequence functions may be used directly.

test.txt
```
foo, bar
baz
```

```
   let: f 'test.txt' `r fopen;
...| $f str
...
['foo, bar
baz
'r1]

let: f 'test.txt' `r fopen;
...| $f words stack
...
[['foo'r1 'bar'r1 'baz'r1]r1]
```

### Comments
Two kinds of code comments are supported, line comments and block comments.

```
   | 1 // Line comments terminate on line breaks
...+ 2
[3]

   | 1 /* While block comments may span
...multiple lines */
...+ 2
...
[3]
```

### Errors
Besides [optionals](https://github.com/basic-gongfu/cixl#optionals), Cixl provides basic exceptions. Two functions are provided for signalling errors. ```throw``` may be used to throw any value as an error.

```
   | 'Going down!' throw
...
Error in row 1, col 6:
Going down!
[]
```

While ```check``` may be used to throw an error when the specified condition doesn't hold.

```
   | 1 2 = check
...
Error in row 1, col 7:
Check failed
[]
```

Thrown values may be caught using ```catch:```, the first matching clause is evaluated with the error pushed on stack.

```
   | catch: (
...      (Int `int ~.)
...      (A   `a ~.)
...    )
...
...    42 throw
...    `foo;
...`bar
...
[(`int 42)r1 `bar]
```

```dump``` may be used to print information about the current error within a catch clause.

```
   | catch: (A _ dump) 42 throw;
...
Error in row 1, col 21:
42
[]
```

### Lambdas
Putting braces around a block of code defines a lambda that is pushed on the stack.

```
   | {1 2 3}
...
[Lambda(0x52d97d0)r1]

   call
...
[1 2 3]
```

Lambdas inherit the defining scope.

```
   | (let: x 42; {$x}) call
...
[42]
```

### Conditions
Any value may be treated as a boolean; some are always true; integers test true for anything but zero; empty strings test false etc. The ```?``` operator may be used to transform any value to its boolean representation.

```
   | 0?
...
[#f]
```

While the ```!``` operator negates any value.

```
   | 42!
...
[#f]
```

```if```, ```else``` and ```if-else``` may be used to branch on a condition, they call '?' implicitly so you can throw any value at them.

```
  | 'foo' %% &upper if
...
['FOO'r1]

  | #nil { say 'not true' } else
...
not true
[]

  | 42 `not-zero `zero if-else
...
[`not zero]
```

Values may be chained using ```and``` / ```or```.

```
   | #t 42 and
...
[42]

   | 0 42 and
...
[0]

   | 42 #t or
...
[42]

   | 0 42 or
...
[42]
```

Lambdas may be used to to prevent evaluating unused arguments when chaining.

```
   | 42 {say 'Bummer!' #t} or
...
[42]
```

The ```switch:``` macro may be used to untangle chains of if/-else calls. The first clause which condition returns a value that's conditionally ```#t``` is executed.

[examples/guess.cx](https://github.com/basic-gongfu/cixl/blob/master/examples/guess.cx)
```
((let: n 100 rand ++; {
  'Your guess: ' ask
  
  % {
    let: c int $n <=>;

    switch:
      (($c `< =) 'Too low!'  say #t)
      (($c `> =) 'Too high!' say #t)
      (#t        'Correct!'  say #nil);
  } {
    _ #nil
  } if-else
}) &_ for)
```

```
   |'examples/guess.cx' load
...
Your guess: 50
Too high!
Your guess: 25
Too low!
Your guess: 37
Too low!
Your guess: 43
Correct!
```

### Functions
The ```func:``` macro may be used to define named functions. Several implementations may be defined with the same name as long as they also have the same arity and different argument types. Functions capture their defining environment and open an implicit child scope on evaluation. Function definitions are allowed anywhere, but are processed in order of appearance during compilation.

Functions are required to specify their arguments and results.

```
   func: say-hi(n)() ['Hi ' $n @!] say;
...| 'stranger' say-hi
...
Hi stranger!
[]
```

Function arguments and results may optionally be anonymous and/or typed. ```_``` may be used in place of a name for anonymous arguments and results. ```A``` may be used to match any type and is used as default when no type is specified.

```
   func: any-add(x y)(_ A) $x $y +;
...| 7 35 any-add
...
[42]
```

Previous argument types may be referenced by index, it is substituted for the actual type on evaluation.

```
   func: same-add(x Num y Arg0)(_ Arg0) $x $y +;
...| 7 34 same-add
...
[42]

   | 7 'foo' same-add
...
Error in row 1, col 7:
Func not applicable: same-add
[7 'foo'r1]
```

Literal values may used instead of types. Anonymous arguments are pushed on the function stack before evaluation.

```
   func: is-fortytwo(_ Int)(#t) _;
...func: is-fortytwo(42)(#f);
...| 41 is-fortytwo
...
[#f]

   | 42 is-fortytwo
...
[#t]
```

Functions may return multiple results.

```
   func: flip(x y Opt)(_ Arg1 _ Arg0)
...  $y $x;
...1 2 flip
...
[2 1]
```

Overriding existing implementations is as easy as defining a function with the same arguments.

```

   func: +(x y Int)(_ Int) 42;
...| 1 2 +
...
[42]
```

```recall``` may be used to call the current function recursively in the same scope. The call may be placed anywhere, but the actual calling doesn't take place until the current call exits.

```  
   func: fib-rec(a b n Int)(_ Int)
...  $n? {$b $a $b + $n -- recall} $a if-else;
...func: fib(n Int)(_ Int)
...  0 1 $n fib-rec;
...| 50 fib
...
[12586269025]
```

Argument types may be specified in angle brackets to select a specific function implementation. Besides documentation and type checking, this allows disambiguating calls and helps the compiler inline the definition in cases where more than one implementation share the same name.

```
   | &+<Int Int>
...
[Fimp(+ Int Int)]

   | &+<Str Str>
...
Error in row 1, col 4:
Fimp not found: +<Str Str>
[]

   | 7 35 +<Int Int>
...
[42]
```

A stack containing all implementations for a specific function may be retrieved by calling the ```imps``` function.

```
   | &+ imps
...
[[Fimp(+ Rat Rat) Fimp(+ Int Int)]r1]
```

Prefixing a function name with ```&``` pushes a reference on the stack.

```
   | 35 7 &+
...
[35 7 Func(+)]

   call
...
[42]
```

### Conversions
Where conversions to other types make sense, a function named after the target type is provided.

```
   | '42' int
...
[42]

   str<Int>
...
['42'r1]

   1 get
...
[@2]

   int
...
[50]

   5 + char
...
[@7]
```

### Rationals
Basic rational arithmetics is supported out of the box.

```
   | 1 2 / -42 2 / *
...
[-21/2]

   int
...
-10
```

### Optionals
The ```#nil``` value may be used to represent missing values. Since ```Nil``` isn't derived from ```A```, stray ```#nil``` values never get far before being trapped in a function call; ```Opt``` may be used instead where ```#nil``` is allowed.

```
...func: foo(x A)();
...func: bar(x Opt)(_ Int) 42;
...| #nil foo
...
Error in row 1, col 1:
Func not applicable: foo
[#nil]

   | #nil bar
...
[42]
```

### Stacks
Stacks are one dimensional dynamic arrays that supports efficient pushing, popping and random access.

```
   | [1 2 3 4 +]
...
[[1 2 7]r1]

   % 5 push
...
[[1 2 7 5]r1]

   % pop
...
[[1 2 7]r1 5]

   _ {2 *} for 
...
[2 4 14]
```

Stacks may be sorted in place by calling ```sort```.

```
   | [3 2 1] % #nil sort
...
[[1 2 3]r1]

   | [1 2 3] % {~ <=>} sort
...
[[3 2 1]r1]
```

### Pairs
Values may be paired by calling ```.```, the result provides reference semantics and access to parts using ```x``` and ```y```.

```
   | 1 2 .
...
[(1 2)r1]

   % x ~ y
...
[1 2]
```

### Tables
Tables may be used to map ```Cmp``` keys to values, entries are ordered by key.

```
   let: t new Table;
...$t 2 'bar' put
...$t 1 'foo' put
...$t
...
[Table((1 'foo'r1) (2 'bar'r1))r2]

   stack
...
[[(1 'foo'r2)r1 (2 'bar'r2)r1]r1]

...| $t 1 'baz' put
...$t 2 delete
...$t
[Table((1 'baz'r1))r2]
```

### Iteration
The ```times``` function may be used to repeat an action N times.

```
   | 10 42 times
...
[42 42 42 42 42 42 42 42 42 42]
```

```
   | 0 42 &++ times
...
[42]
```

While ```for``` loop repeats an action once for each value in any sequence.

```
   | 10 {+ 42,} for
...
[42 43 44 45 46 47 48 49 50 51]
```

```
   | 'foo' &upper for
...
[@F @O @O]
```

Sequences support mapping actions over their values, ```map``` returns an iterator that may be chained further or consumed.

```
   | 'foo' {int ++ char} map
...
[Iter(0x545db40)r1]

   str
...
['gpp'r1]
```

Sequences may be filtered, which also results in a new iterator.

```
   | 10 {, $ > 5} filter
...
[Iter(0x54dfd80)r1]

   {} for
...
[6 7 8 9]
```

Iterators may be created manually by calling ```iter``` on any sequence and consumed manually using ```next``` and ```drop```.

```
   | [1 2 3] iter
...
[Iter(0x53ec8c0)r1]

   % % 2 drop next ~ next
...
[3 #nil]
```

Functions and lambdas are sequences, calling ```iter``` creates an iterator that keeps returning values until the target returns ```#nil```.

```
   func: forever(n Int)(_ Lambda) {$n};
...| 42 forever iter
...% next ~ next
...
[42 42]
```

### Time
Cixl provides a single concept to represent points in time and intervals. Internally; time is represented as an absolute, zero-based number of months and nanoseconds. The representation is key to providing dual semantics, since it allows remembering enough information to give sensible answers.

Times may be queried for absolute and relative field values;

```
   | let: t now; $t
...
[Time(2018-00-03 20:14:48.105655092)]

   % date ~ time
...
[Time(2018-00-03) Time(20:14:48.105655092)]

   | $t year $t month $t day
...
[2018 0 3]

   | $t months
...
[24216]

   12 / int
...
[2018]

   | $t hour $t minute $t second $t nsecond
...
[20 14 48 105655092]

   | $t h $t m $t s $, ms $t us $t ns
...
[93 5591 335485 335485094 335485094756 335485094756404]

   | $t h 24 / int
...
[3]

   | $t m 60 / int
...
[93]
```

manually constructed;

```
   | [2018 0 3 20 14] time
...
[Time(2018-00-03 20:14:0.0)]

   | 3 days
...
[Time(72:0:0.0)]

   days
...
[3]
```

compared, added and subtracted;

```
   | 2m 120s =
...
[#t]

   | 1 years 2 months + 3 days + 12h -
...
[Time(0001-02-02) 12:0:0.0]

   now <=
...
[#t]

   | 10 days 1 years -
...
[Time(-1/0/10)]

   days
...
[-356]
```

and scaled.

```
   | 1 months 1 days + 3 *
...
[Time(0/3/3)]
```

### Types
Cixl is statically and strongly typed; but since it's approach to typing is gradual, it allows you to be exactly as precise as you feel like. All types have capitalized names, the following are defined out of the box:

| Type      | Parents     | Lib         |
| --------- | ----------- |-------------|
| A         | Opt         | cx/abc      |
| Bin       | A           | cx/bin      |
| Buf       | A           | cx/io/buf   |
| Bool      | A           | cx/abc      |
| Cmp       | A           | cx/abc      |
| File      | Cmp         | cx/io       |  
| Fimp      | Seq         | cx/abc      |
| Func      | Seq         | cx/abc      |
| Int       | Num Seq     | cx/abc      |
| Iter      | Seq         | cx/abc      |
| Lambda    | Seq         | cx/abc      |
| Nil       | Opt         | cx/abc      |
| Num       | Cmp         | cx/abc      |
| Opt       |             | cx/abc      |
| Pair      | Cmp         | cx/pair     |
| Poll      | A           | cx/io/poll  |
| Rat       | Num         | cx/math     |
| Rec       | Cmp         | cx/rec      |
| Ref       | A           | cx/ref      |
| RFile     | File        | cx/io       |
| RWFile    | RFile WFile | cx/io       |
| Seq       | A           | cx/abc      |
| Stack     | Cmp Seq     | cx/abc      |
| Str       | Cmp Seq     | cx/abc      |
| Sym       | A           | cx/abc      |  
| Table     | Seq         | cx/table    |
| TCPClient | RWFile      | cx/net      |
| TCPServer | RFile       | cx/net      |
| Time      | Cmp         | cx/time     |
| Type      | A           | cx/abc      |
| WFile     | File        | cx/io       |

```
   | 42 type
...
[Int]

   A is
...
[#t]

   | 42 Str is
...
[#f]
```

### Records
Records map finite sets of typed fields to values. Record types are required to specify an (optionally empty) list of parent types and traits; and will inherit any fields that don't clash with its own. Record definitions are allowed anywhere, but are processed in order of appearance during compilation. ```new``` may be used to create new record instances. Getting and putting field values is accomplished using symbols, uninitialized fields return ```#nil```.


```
   rec: Node()
...  left right Node
...  value A;
...| let: n Node new;
...$n `value 42 put
...$n
...
[Node((value 42))r2]

   `value get
...
[42]

   | $n `left get
...
[#nil]
```

Records support full deep equality by default, but ```=``` may be implemented to customize the behavior.

```
   rec: Foo() x Int y Str;
...| let: (bar baz) Foo new %%;
...$bar `x 42 put
...$bar `y 'bar' put
...$baz `x 42 put
...$baz `y 'baz' put
...$bar $baz =
...
[#f]

   func: =(a b Foo)(_ Bool) $a `x get $b `x get =;
...| $bar $baz =
...
[#t]
```

### Traits
Traits are abstract types that may be used to simplify type checking and/or function dispatch. Besides the standard offering; ```A```, ```Cmp```, ```Num```, ```Opt```, ```Rec``` and ```Seq```; new traits may be defined using the ```trait:``` macro. Trait definitions are allowed anywhere, but are processed in order of appearance during compilation.

```
   trait: StrInt Str Int;
...| Str StrInt is Int StrInt is Sym StrInt is
...
[#t #t #f]
```

### Binaries
A ```Bin``` represents a block of compiled code. The compiler may be invoked from within the language through the ```compile``` function. Binaries may be passed around and called, which simply executes the compiled operations in the current scope.

```
   | Bin new % '1 2 +' compile call
...
[3]
```

### Embedding & Extending
Everything about Cixl has been designed from the ground up to support embedding in, and extending from C. The makefile contains a target named ```libcixl``` that builds a static library containing everything you need to get started. Adding a type and associated function goes something like this:

```C
#include <cixl/box.h>
#include <cixl/cx.h>
#include <cixl/error.h>
#include <cixl/func.h>
#include <cixl/scope.h>

static bool len_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = strlen(v.as_ptr);
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return strcmp(x->as_ptr, y->as_ptr) == 0;
}

static bool ok_imp(struct cx_box *v) {
  char *s = v->as_ptr;
  return s[0];
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'", (char *)v->as_ptr);
}

static void deinit_imp(struct cx_box *v) {
  free(v->as_ptr);
}

int main() {
  struct cx cx;
  cx_init(&cx);
  
  struct cx_type *t = cx_add_type(&cx, "Str", cx.any_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  
  cx_add_cfunc(&cx, "len",
               cx_args(cx_arg("s", t)), cx_args(cx_arg(NULL, cx.int_type)),
	       len_imp);

  cx_add_cxfunc(cx, "upper",
	   	cx_args(cx_arg("s", t)), cx_args(cx_arg(NULL, t)),
	        "$s &upper map str");

  ...

  cx_deinit(&cx);
  return 0;
}
```

### Safety
Type checking may be partly disabled for the current scope by calling ```unsafe```, which allows code to run slightly faster. New scopes inherit their safety level from the parent scope. Calling ```safe``` enables all type checks for the current scope.

```
   | {10000 {50 fib _} times} clock 1000000 / int
...
[317]

   | unsafe
...{10000 {50 fib _} times} clock 1000000 / int
...
[282]
```

### Performance
There is still plenty of work remaining in the profiling and benchmarking departments, but preliminary indications puts compiled Cixl at slightly faster to twice as slow as Python3. Measured time is displayed in milliseconds.

Let's start with a tail-recursive fibonacci to exercise the interpreter loop, it's worth mentioning that Cixl uses 64-bit integers while Python settles for 32-bit.

```
use: cx;

func: fib-rec(a b n Int)(_ Int)
  $n?<Opt> {$b $a $b +<Int Int> $n -- recall} $a if-else;

func: fib(n Int)(_ Int)
  0 1 $n fib-rec;

{10000 {50 fib _} times} clock 1000000 / int say

$ cixl -e cixl/perf/bench1.cx -o bench1
$ ./bench1
192
```

```
from timeit import timeit

def _fib(a, b, n):
    return _fib(b, a+b, n-1) if n > 0 else a

def fib(n):
    return _fib(0, 1, n)

def test():
    for i in range(10000):
        fib(50)

print(int(timeit(test, number=1) * 1000))

$ python3 cixl/perf/bench1.py 
118
```

Next up is consing a stack.

```
use: cx;

{let: v []; 10000000 {$v ~ push} for} clock 1000000 / int say

$ cixl -e cixl/perf/bench2.cx -o bench2
$ ./bench2
1184
```

```
from timeit import timeit

def test():
    v = []
    
    for i in range(10000000):
        v.append(i)

print(int(timeit(test, number=1) * 1000))

$ python3 cixl/perf/bench2.py 
1348
```

Moving on to instantiating records.

```
use: cx;
rec: Foo() x Int y Str;

{10000000 {Foo new % `x 42 put<Rec Sym A> `y 'bar' put<Rec Sym A>} times}
clock 1000000 / int say

$ cixl -e cixl/perf/bench3.cx -o bench3
$ ./bench3
3207
```

```
from timeit import timeit

class Foo():
    pass

def test():
    for i in range(10000000):
        foo = Foo()
        foo.x = 42
        foo.y = "bar"

print(int(timeit(test, number=1) * 1000))

$ python3 cixl/perf/bench3.py
3213
```

### Zen

- Orthogonal is better
- Terseness counts
- There is no right way to do it
- Obvious is overrated
- Symmetry beats consistency
- Rules are for machines
- Everything is not X
- Only fools predict the future
- Duality of syntax is right up there

### License
LGPLv3

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
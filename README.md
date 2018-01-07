## ![](cixl.png?raw=true) <a href="https://liberapay.com/basic-gongfu/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>
#### cixl - a minimal, decently typed scripting language

This project aims to produce a minimal, decently typed scripting language for embedding in and extending from C. The language is implemented as a straight forward 3-stage (parse/compile/eval) interpreter that is designed to be as fast as possible without compromising on simplicity, transparency and flexibility. The codebase has no external dependencies and is currently hovering around 4 kloc including tests and standard library.

### Getting Started
To get started, you'll need a reasonably modern C compiler with GNU-extensions and CMake installed. A basic REPL is included, it's highly recommended to run it through ```rlwrap``` for a less nerve-wrecking editing experience.

```
git clone https://github.com/basic-gongfu/cixl.git
cd cixl
mkdir build
cd build
cmake ..
make
rlwrap ./cixl

cixl v0.7, 18765 bmips

Press Return twice to eval input.

   1 2 3
...
[1 2 3]

   quit
```

### Status
Examples should work in the most recent version and run clean in ```valgrind```, outside of that I can't really promise much at the moment. Current work is focused on profiling and filling obvious gaps in functionality.

### Stack
The stack is accessible from user code, just like in Forth. Basic stack operations have dedicated operators; ```%``` for copying the last value, ```_``` for dropping it, ```~``` for flipping the last two values and ```|``` for clearing the stack.

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

### Expressions
But unlike Forth, functions scan forward until enough arguments are on the stack to allow reordering parameters and operations in user code to fit the problem being solved.

```
   | 1 + 2
...
[3]

   1 2 +
...
[3 3]

   + 1 2
...
[6 1 2]

   + +
...
[9]
```

The ```,``` operator may be used to cut the stack into discrete pieces and force functions to scan forward.

```
   |
...1 + 2
...3 + 4
...
[6 4]

   1 + 2,
...3 + 4
...
[3 7]
```

### Variables
Named variables may be defined once per scope using the ```let:``` macro.

```
   | let: foo 'bar';
...$foo
...
['bar']

   let: foo 'baz';
...
Error in row 1, col 10:
Attempt to rebind variable: 'foo'
```

Multiple names may be bound at the same time by enclosing them in parens.

```
   |
...let: (x y z) 1 2, 3 + 4;
...$x $y $z
...
[1 2 7]
```

Types may be specified for documentation and type checking.

```
   |
...let: (x y Int z Str) 1 2 3;
...$x $y $z
...
Error in row 1, col 5:
Expected type Str, actual: Int
```

### Equality
Two flavors of equality are provided.

Value equality:
```
   | [1 2 3] = [1 2 3]
...
[#t]
```

And identity:
```
   | 'foo' == 'foo'
...
[#f]
```

```
   | 42 == 42
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

   | `foo = `bar
...
[#f]

   | 'baz' sym
...
[`baz]

   str
...
['baz']
```

### References
Some values are reference counted; vectors, lambdas etc. Reference counted values display the number of references following ```@``` when printed. Doubling the copy operator results in a deep copy where applicable and defaults to regular copy where not.

```
   | [1 2 3] %
...
[[1 2 3]@2 [1 2 3]@2]

   | [1 2 3] %%
...
[[1 2 3]@1 [1 2 3]@1]
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
   |
...let: foo 1;
...(let: foo 2; $foo)
...$foo
...
[2 1]
```

### IO
```say``` and ```ask``` may be used to perform basic IO.

```
   |
...say 'hello'  
...ask 'what\'s your name? '
...
hello
what's your name? Sifoo
['Sifoo']
```

Code may be loaded from file using ```load```, it is evaluated in the current scope.

test.cx:
```
+ 2
```

```
   | 1, load 'test.cx'
...
[3]
```

### Conditions
All types are useable as conditions; some are always true; integers test true for anything but zero; empty strings test false etc. The ```?``` operator may be used to transform any value to its conditional representation.

```
   | 0?
...
[#f]
```

The ```!``` operator negates any condition.

```
   | 42!
...
[#f]
```

The ```if``` statement may be used to branch on a condition, it calls '?' implicitly so you can throw any value at it.

```
   | 42 if `not-zero `zero
...
['not zero']

   | ''! if `empty `not-empty
...
['empty']
```

### Lambdas
Putting braces around a block of code defines a lambda that is pushed on the stack.

```
   | {1 2 3}
...
[Lambda(0x52d97d0)@1]

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

### Functions
The ```func:``` macro may be used to define named functions. Several implementations may be defined for the same name as long as they have the same arity and different argument types. Each function opens an implicit scope that is closed on exit.

```
   |
...func: foo() 42;
...foo
...
[42]
```

Prefixing a function name with ```&``` pushes a reference on the stack.

```
   |
...func: foo() 42;
...&foo
...
[Func(foo)]

   call
...
[42]
```

Each argument needs a type, ```A``` may be used to accept any type.

```
   |
...func: bar(x A) $x + 35;
...bar 7
...
[42]
```

Several parameters may share the same type.

```
   |
...func: baz(x y Int) $x + $y;
...baz 7 35
...
[42]
```


An index may may be specified instead of type to refer to previous arguments, it is substituted for the actual type on evaluation.

```
   |
...func: baz(x A y T0)
...  $x + $y;
...baz 1 2
...
[3]

   | baz 1 'foo'
...
Error in row 1, col 7:
Func not applicable: baz
```

It's possible to specify literal values for arguments instead of names and types.

```
   |
...func: bar(x Int) #f;
...func: bar(42) #t;
...bar 41, bar 42
...
[#f #t]
```

Overriding existing implementations is as easy as defining a function with identical argument list.

```
   |
...func: +(x y Int) 42;
...1 + 2
...
[42]
```

```recall``` may be used to call the current function recursively in the same scope, it supports scanning for arguments just like a regular function call.

```
   |
...func: fib-rec(a b n Int)
...  $n? if {, recall $b, $a + $b, -- $n} $a;
...func: fib(n Int)
...  fib-rec 0 1 $n;
...fib 50
...
[12586269025]
```

Argument types may be specified in angle brackets to select a specific function implementation. Besides documentation and type checking, this allows the compiler to inline the definition in cases where more than one implementation share the same name.

```
   | &+<Int Int>
...
[Fimp(+ Int Int)]

   &+<Str Str>
...
Error in row 1, col 4:
Func imp not found

   | 7 +<Int Int> 35
...
[42]
```

A vector containing all implementations for a specific function in the order they are considered during dispatch may be retrieved by calling the ```imps``` function.

```
   | &+ imps
...
[[Fimp(+ Rat Rat) Fimp(+ Int Int)]@1]
```

```upcall``` provides an easy way to call the next matching implementation, it also supports scanning for arguments.

```
   |
...func: maybe-add(x y Num) $x + $y;
...func: maybe-add(x y Int) $x = 42 if 42 {upcall $x $y};
...maybe-add 1 2 , maybe-add 42 2
...
[3 42]
```

### Conversions
Where conversions to other types make sense, a function named after the target type is provided.

```
   | '42' int
...
[42]

   str
...
['42']

   1 get
...
[\2]

   int
...
[50]

   + 5 char
...
[\7]
```

### Rationals
Basic rational arithmetics is supported out of the box.

```
   | 1 / 2, -42 / 2 *
...
[-21/2]

   int
...
-10
```

### Optionals
The ```#nil``` value may be used to represent missing values. Since ```Nil``` isn't derived from ```A```, stray ```#nil``` values never get far before being trapped in a function call; ```Opt``` may be used instead where ```#nil``` is allowed.

```
   |
...func: foo(x A);
...func: bar(x Opt) 42;
..
[]

   foo #nil
...
Error in row 1, col 1:
Func not applicable: 'foo'

   | bar #nil
...
[42]
```

### Vectors
A vector is a one dimensional dynamic array that supports efficient pushing / popping and random access.

```
   | [1 2, 3 + 4]
...
[[1 2 7]@1]

   % 5 push
...
[[1 2 7 5]@1]

   % pop
...
[[1 2 7]@1 5]

   _ {2 *} for 
...
[2 4 14]
```

### Loops
The ```times``` loop may be used to repeat an action N times.

```
   | 10 times 42
...
[42 42 42 42 42 42 42 42 42 42]
```

```
   | 0, 42 times &++
...
[42]
```

While the ```for``` loop repeats an action once for each value in a sequence, the current value is pushed on the stack before calling the action.

```
   | 10 for {+ 42,}
...
[42 43 44 45 46 47 48 49 50 51]
```

```
   | 'foo' for &upper
...
[\F \O \O]
```

Some types support mapping actions over their contents using ```map```.

```
   | 'foo' map {int ++ char}
...
['gpp']
```

### Time
Cixl provides a single concept to represent points in time and intervals. Internally; time is represented as an absolute, zero-based number of months and nanoseconds. The representation is key to providing dual semantics, since it allows remembering enough information to give sensible answers.

Times may be queried for absolute and relative field values;

```
   | let: t now; $t
...
[Time(2018/0/3 20:14:48.105655092)]

   % date ~ time
...
[Time(2018/0/3) Time(20:14:48.105655092)]

   | year $t, month $t, day $t
...
[2018 0 3]

   | months $t
...
[24216]

   / 12 int
...
[2018]

   | hour $t, minute $t, second $t, nsecond $t
...
[20 14 48 105655092]

   | h $t, m $t, s $t, ms $t, us $t, ns $t
...
[93 5591 335485 335485094 335485094756 335485094756404]

   | h $t / 24 int
...
[3]

   | m $t / 60 int
...
[93]
```

manually constructed;

```
   | [2018 0 3 20 14] time
...
[Time(2018/0/3 20:14:0.0)]

   | 3 days
...
[Time(72:0:0.0)]

   days
...
[3]
```

compared, added and subtracted;

```
   | 2m =, 120s
...
[#t]

   | 1 years +, 2 months +, 3 days -, 12h
...
[Time(1/2/2) 12:0:0.0]

   < now
...
[#t]

   | 10 days -, 1 years
...
[Time(-1/0/10)]

   days
...
[-356]
```

and scaled.

```
   | 1 months +, 1 days * 3
...
[Time(0/3/3)]
```

### Types
Capitalized names are treated as types, the following list is defined out of the box:

- A (Opt)
- Bin (A)
- Bool (A)
- Fimp (A)
- Func (A)
- Int (Num)
- Lambda (A)
- Nil (Opt)
- Num (A)
- Opt ()
- Rat (Num)
- Rec (A)
- Str (A)
- Sym (A)
- Time (A)
- Type (A)
- Vect (A)

```
   | type 42
...
[Int]

   is A
...
[#t]
```

### Records
Records map finite sets of typed fields to values. Record types are required to specify an (optionally empty) list of parent types and traits; and will inherit any fields that don't clash with its own. 

```
   |
   rec: Node(A)
     left right Node
     value A;
...
[]
```

```new``` may be used to create new record instances. Getting and putting field values is accomplished using symbols, uninitialized fields return ```#nil```.

```
   |
...let: n Node new;
...$n put `value 42
...$n
...
[Node(value.42)@1]

   get `value
...
[42]

   | $n get `left
...
[#nil]
```

### Traits
Traits are abstract types that may be used to simplify type checking and/or function dispatch. Besides the standard offering; 'A', 'Num', 'Opt' and 'Rec'; new traits may be defined using the ```trait:``` macro.

```
   |
   trait: StrInt Str Int;
...Str is StrInt, Int is StrInt, Sym is StrInt
...
[#t #t #f]
```

### Meta
The compiler may be invoked from within the language through the ```compile``` function, the result is a compiled sequence of operations that may be passed around and called.

```
   | '1 + 2' compile
...
[Bin(0x8899a0)@1]

   call
...
[3]
```

### Embedding & Extending
Everything about cixl has been designed from the ground up to support embedding in, and extending from C. The makefile contains a target named ```libcixl``` that builds a static library containing everything you need to get started. Adding a type and associated function goes something like this:

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

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void fprint_imp(struct cx_box *v, FILE *out) {
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
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;
  
  cx_add_func(&cx, "len", cx_arg(t))->ptr = len_imp;

  ...

  cx_deinit(&cx);
  return 0;
}
```

### Performance
There is still plenty of work remaining in the profiling and benchmarking departments, but preliminary indications puts cixl at around 1/2-4 times slower than Python. Measured time is displayed in milliseconds.

Let's start with a tail-recursive fibonacci to exercise the interpreter loop, it's worth mentioning that cixl uses 64-bit integers while Python settles for 32-bit.

```
   |
...func: fib-rec(a b n Int)
...  $n? if {$b $a $b + $n -- recall} $a;
...func: fib(n Int)
...  fib-rec 0 1 $n;
...clock {10000 times {50 fib _}} / 1000000 int
...
[520]
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

$ python3 fib.py 
118
```

Next up is consing a vector.

```
   | clock {(let: v []; 10000000 for {$v ~ push})} / 1000000 int
...
[1886]
```

```
from timeit import timeit

def test():
    v = []
    
    for i in range(10000000):
        v.append(i)

print(int(timeit(test, number=1) * 1000))

$ python3 vect.py 
1348
```

### Zen

- Orthogonal is better
- Terseness counts
- There is no right way to do it
- Obvious is overrated
- Symmetry beats consistency
- Rules are for machines
- Only fools predict "the future"
- Intuition goes with the flow
- Duality of syntax is one honking great idea

### License
GPLv3

Give me a yell if something is unclear, wrong or missing. And please do consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
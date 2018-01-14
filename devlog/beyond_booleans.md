## Beyond Booleans
#### 2018-01-14

### Intro
No programming language gets far without supporting some flavor of boolean logic. But despite the it's conceptual simplicity, there is a wide spectrum of implementation details to explore. This post compares the approaches taken by C, Common Lisp, Python and [Cixl](https://github.com/basic-gongfu/cixl).

### C
For a long, long time; C didn't have a dedicated boolean type; which was mostly fine, since it allows treating ```0``` and ```NULL``` as false and anything else as true. What C gets right is allowing several different kinds of values to be projected onto the boolean domain. And that's pretty much it. It doesn't allow chaining operators without loosing the original types and offers no facilities for user code to hook into the protocol.

```
int a = 42;
int *b = NULL || &a; // Nice try
```

### Common Lisp
Common Lisp treats ```nil``` as false and any other value as true, which prevents dispatching methods on booleans among other issues. It does provide a boolean type, with a single value called ```t```; but no false value. Like C, it offers no facilities for user code to hook into the protocol. It does however allow chaining values, which enables using ```or```to select the first non-```nil``` value among other tricks.

```
* (or nil (list 1 2 3))
(1 2 3)

* (and "foo" "bar")
"bar"
```

### Python
Python took an idea that kind of made sense in C and turned it into something that's just plain wrong by making the ```bool```-type inherit ```int```. 

```
>>> import inspect
>>> inspect.getmro(type(True))
(<type 'bool'>, <type 'int'>, <type 'object'>)
```

It does on the other hand support chaining values using boolean operators just like Common Lisp.

```
>>> False or 42
42
```

And even makes an attempt at providing user hooks in the form of a built-in ```__bool__``` method, but unfortunately misses the mark by not calling it when chaining values using boolean operators.

```
class Foo():
    def __bool__(self):
        print('Called\n')
        return True
    
> print(False or Foo())
<__main__.Foo object at 0x7f0711f37dd8>
```

### Cixl
[Cixl](https://github.com/basic-gongfu/cixl) makes an honest attempt at capturing the essence of what came before and sorting out remaining issues. Any value may be treated as a boolean; some types test true regardless of actual value; like in C, integers are true unless zero; empty strings test false etc. ```?``` may be used to transform any value to it's conditional representation.

```
   | 0?
...
[#f]

   | 'foo'?
...
[#t]
```

The ```?```-operator may be overloaded for user defined types, which allows hooking into the boolean protocol.

```
   |
...rec: Foo() bar Int;
...func: ?(x Foo) $x `bar get = 42;
...let: foo Foo new %, $ `bar put 21;
...$foo?
...
[#f]

   |
...$foo `bar put 42
...$foo?
...
[#t]

```

The rest of the language will call ```?``` whenever a boolean projection is required.

```
   | 42!
...
[#f]

   | 'foo' if {say 'Yes'}
...
Yes
[]

   | #nil or $foo
...
...
[Foo(0x53dd068)@2]
```

And if you ask politely, you may even get the integer projection of a boolean value.

```
   | #t int
...
[1]
```

Give me a yell if something is unclear, wrong or missing. And please do consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
## Iterating towards v1.0
#### 2018-02-04

### Intro
[Cixl](https://github.com/basic-gongfu/cixl) has come a long way in a month and a half, but it's still missing bits and pieces to be considered a minimum viable implementation of what I can see from here. 32 years of searching for the perfect language that would scratch all the itches I accumulated along the way; crystalized into the idea of two languages; integrated in harmony, rather than one in control of the other. Unfortunately, and a bit surprising to me; I turned out to be the person writing one of them; and the only tool I had at my disposal was vanilla C. Forth wasn't even on my radar any more, since I found it too dogmatic and restrictive to fill the role.

### C?
C is taking quite a beating these days. And I get it, it's unsafe; chainsaw unsafe. But it's a feature, not a bug; that's why it's used so much, not because it's users are ignorant or stupid. It takes a lot of practice to learn, the first few years of any C coders output will have undefined behaviour written all over. Sometimes I still mess up in spectacular ways, and I've written plenty of C over 30-ish years. But if you have any kind of common sense, you soon start developing strategies to deal with weak points in any language. And the risks come with power and flexibility, that allow a unique level of control over a computer.

### Postfix??
The instant Forth clicked for me, my mind started skunkworking strategies for sneaking out of postfix prison. Being stuck anywhere is bad enough; I happen to prefer many operations expressed as infix operators, even with more than one parameter on either side; and I suspect many would agree once they start admitting the possibility. It turns out that the order of functions and parameters isn't as critical as we like to pretend; letting go of it actually allows expressing intent more clearly since you're now free to move the pieces around. If you think that sounds crazy, I don't blame you; I didn't make much progress myself until I had enough experience from Forth, Lisp and Haskell. I mostly considered the question answered, infix with exceptions was as good as it was going to get. Someone said that it's fruitful to question what you can't think.

### Iterators
Since their inception, iterators have come to play an increasingly important role in Cixl. Contrary to most languages; they are seemlessly integrated into the language, rather than bolted onto the side as an additional feature. I keep finding more uses for them so this story has at least a couple of chapters remaining to be written. All iterable types derive ```Seq```; functions, lambdas, integers, strings, vectors, tables and readable files are all iterable. Mapping an action over a sequence results in an iterator; which is, drumroll, iterable.

```
   | 'foo' {int ++ char} map
...
[Iter(0x545db40)@1]

   str
...
['gpp'@1]
```
Sequences may be filtered, which also results in a new iterator.

```
   | 10 {, $ > 5} filter
...
[Iter(0x54dfd80)@1]

   {} for
...
[6 7 8 9]
```

```for``` plays the combined role of for, for-each and and reduce by pushing each value in a sequence on the stack and calling the specified action.

```
   | 0 ([1 2 3]) &+ for
...
[6]

```

Iterators may be created manually by calling ```iter``` on any sequence and consumed manually using ```next``` and ```drop```.

```
   | [1 2 3] iter
...
[Iter(0x53ec8c0)@1]

   % % 2 drop next ~ next
...
[3 #nil]
```

Calling ```iter``` on functions and lambdas creates an iterator that keeps returning values until the target returns ```#nil```.

```
   func: forever(n Int) (Lambda) {$n};
...| 42 forever iter
...% next ~ next
...
[42 42]
```

One final example before we leave iterators behind and move on into the future, a number guessing game implemented as a lambda iterator.

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

### Support
Due to health and logistic reasons outside of my control; I'm currently unemployed, have been for quite a while. While this gives me time to think longer thoughts, and write more code; it also means that Cixl depends on your support to get to the next level. If you feel like this is worth pursuing; please consider helping out via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate). Should I by any chance receive more than what I need for essentials, which is around €400/month; I'll happily pass that on to other projects in need of support.

### Modularity
Library facilities need to be put into code; the basic structure is there, but much remains to be done. The idea is that definitions are tagged with the categories they belong to, and only definitions for which all categories have been imported are even parsed. It's libraries in most other languages turned inside out, if that makes any sense. This solves the dependency problem; and in combination with most language features being definitions, and a finely grained division into categories; it provides a convenient and flexible method for customizing the language to fit the needs of each application/use. Imagine a function that takes a string parameter and returns an iterator, and uses; it would need to be tagged with both ```Str``` and ```Iter```; and would only be visible, even parsed; once both categories are imported. Any categories that the function uses internally would also have to be tagged.

### Error Handling
When it comes to error handling, it seems like we've gotten collectively stuck in the mindset of finding the one strategy to rule them all. But not all errors are created equal, some need to be handled immediately; some are more optional. What is often needed regardless is a way to pass information out of band. I don't consider encoding the error in the return value a viable approach, it's too cumbersome to use and doesn't allow mixing strategies. It's fine to return #nil, indicating that something went wrong; but the specifics are better dealt with using other means. The idea is to add two kinds of errors, ```fail``` for errors that unwind the stack until dealt with, and ```throw```/```catch:``` for passing specifics out of band. Any value may be used as an error and both failures and thrown values are trapped in ```catch:```; catching ```Opt``` always executes the specified action, while catching ```Nil``` only runs when there are no errors.

```
   |(
     catch:
       Int `int ~ .
       Opt `opt ~ .
       Nil `nil;
     (42 fail `foo)
     `bar
   )
...
[(`int 42)r1 (`opt 42)r1 `bar]
```

### Emitting C
At the moment; Cixl compiles to an internal instruction set, where each instruction is implemented as a C function. To gain speed, how much remains to be seen; and to allow compiling static executables; facilities for emitting the resulting instructions inline as a single C function that may be manually called or compiled with a harness into an executable in one step will be added. The beauty of this approach is that much of the code is shared, and that it's easy to compile multiple segments of Cixl code into C functions and manually stitch them together from the outside; the opposite approach is already well [supported](https://github.com/basic-gongfu/cixl#embedding--extending).

That's all for now.

Happy [Cixl](https://github.com/basic-gongfu/cixl) pushing!
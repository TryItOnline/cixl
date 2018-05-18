## Minimal Fibers
#### 2018-05-17

### Intro
Fibers, or Green Threads, is one of those features that I knew [Cixl](https://github.com/basic-gongfu/cixl) would have to support eventually; but I wanted to wait until the dust settled before deciding on implementation details. Some languages, like Lua for example; expose raw coroutines and leave scheduling concerns as an excercise. While Cixl may eventually support raw coroutines, I still feel like it makes sense to separate concerns and provide fibers and scheduling as first class concepts. The implementation described here is my best attempt at cutting the concepts down to their core within the framework provided by Cixl; it has much in common with Smalltalk's flavor of cooperative concurrency, but comes with the added twist of first class schedulers.

### Examples
The following example demonstrates how to create a scheduler, push some fibers and run until all are finished:

```
#!/usr/local/bin/cixl

use: cx;

let: s Sched new;
let: out [];

$s {
  $out 1 push
  resched
  $out 2 push
} push

$s {
  $out 3 push
  resched
  $out 4 push
} push

$s run
$out ', ' join say
```

Output:
```
1, 3, 2, 4
```

The scheduler may alternatively be iterated for more granular control, each iteration triggers one fiber and pushes the number of fibers that are still running on the stack:

```
let: s1 Sched new;
let: out [];

$s1 {
  let: s2 Sched new;

  $s2 {
    $out `foo push
    resched
    $out `bar push
  } push

  $s2 {
    $out `baz push
  } push

  $s2 {$out ~ push} for
} push

$s1 run
$out ', ' join say
```

Output:
```
foo, 2, baz, 1, bar, 0
```

### Performance
To get an idea about what kind of raw performance to expect, I wrote a simple benchmark that runs 2M task switches in Ruby and Cixl. I suspect much of the difference is due to Cixl running the entire scheduler loop in C. Measured time is displayed in milliseconds.

[bench5.rb]()
```
n = 1000000

task1 = Fiber.new {n.times {Fiber.yield}}
task2 = Fiber.new {n.times {Fiber.yield}}

t1 = Time.now
n.times { task1.resume task2.resume }
t2 = Time.now
delta = (t2 - t1) * 1000
puts "#{delta.to_i}"

$ ruby bench5.rb
3546
```

[bench5.cx]()
```
define: n 1000000;
let: s Sched new;

2 {$s {#n &resched times} push} times
{$s run} clock 1000000 / say

$ cixl bench5.cx
3169
```

### Implementation
Most heavy lifting is delegated to [ucontext.h], each fiber (or task as Cixl likes to call them) carries its own fixed size C stack. Besides managing the stack, tasks additionally backup and restore Cixl's runtime state when rescheduled and resumed. 

```
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
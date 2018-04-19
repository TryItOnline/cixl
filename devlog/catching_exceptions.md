## Catching Exceptions without Trying
#### 2018-04-17

### Intro
One of the itches I've developed over the years is improving on the status quo surrounding exceptions. None of the alternatives really solve the same problems, they all shuffle the task to user code in one way or other. And one of the issues with exceptions is that they like to manage the call stack, even Common Lisp doesn't support catching exceptions without putting the offending code in a try block. [Cixl](https://github.com/basic-gongfu/cixl) breaks with this tradition by allowing exceptions to be caught anywhere.

test.cx:
```
use: cx;

/* Any value may be thrown */

42 throw

/* Catching _ always succeeds and rethrows */

catch: _
  'Cleaning up...' say;

'What, no exceptions?' say

/* The first matching clause is executed with the exception on stack */

catch:
  (Int
    let: e;
    ['Caught Int: ' $e value] say)
  (Str
    let: e;
    ['Caught Str: ' $e value] say);

'Back to normal!' say
```

```
$ cixl test.cx
Cleaning up...
Caught Int: 42
Back to normal!
```

### Implementation
[Cixl](https://github.com/basic-gongfu/cixl) doesn't jump any more than usual when an exception is thrown; as long as exceptions are in flight, it will fast forward through the same instructions as otherwise without executing any user code except ```catch:```. While this approach may be slightly slower when throwing, I consider that a small price to pay for the simplicity and added flexibility.

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
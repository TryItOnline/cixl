## All work and no Tetris
#### 2018-04-04

### Intro
Tetris will always have a place in my heart; it was one of the first puzzle games I got seriously hooked on, and I still enjoy a game now and then. I find that it works much like breathing exercises in Yoga mentally, simply following the method improves awareness indirectly by making it really inconvenient to lock focus. The game owes much of its charm to simplicity, which makes it an excellent candidate for gently bending [Cixl](https://github.com/basic-gongfu/cixl) in new directions; taking the [ANSI graphics library](https://github.com/basic-gongfu/cixl/blob/master/examples/ansi.cx) for a spin, and blowing off some steam after the recent deep dive into [cryptography](https://github.com/basic-gongfu/cixl/blob/master/devlog/turn_on_plugin_encrypt.md). The code may be found [here](https://github.com/basic-gongfu/cixl/blob/master/examples/cixtris.cx) and a Linux/64 binary over [there](https://github.com/basic-gongfu/cxbin/blob/master/linux64/cixtris).

![Screenshot](https://raw.github.com/basic-gongfu/cixl/master/devlog/cixtris.png)

### Changes
One of the more important features added along the way is support for block moves within stacks, the first feature to really take advantage of the fact that Cixl's stacks are represented as continous blocks of memory. ```move``` takes a stack, a start offset, length and delta; and moves the specified items by the delta in one operation. ```find-if``` is another nice addition which allows searching for items within any iterable sequence, the code below searches the range ```0..max-x - 1``` for free tiles on the current row.

```
/*
   Clears full rows intersecting current shape of block.
*/

func: clear-rows(b Block)()
  let: nrows $b get-shape shape-height;
  let: y $b `y get ref;

  $nrows {
    $y {
      let: yy;

      let: is-full $max-x -- {
        let: o ++ $yy get-offs;
	$tiles $o get is-nil
      } find-if is-nil;

      $is-full {
        $tiles 0 1 $yy get-offs $max-x move
	refresh-tiles
      } if
      
      $yy ++
    } set-call
  } times;
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
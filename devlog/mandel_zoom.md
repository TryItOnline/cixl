## A zooming Mandelbrot using ANSI graphics
#### 2018-04-22

### Intro
As far back as I can remember, I've been fascinated by all kinds of fractals; but it wasn't until now that I finally found a reason to write one. I was browsing Rosettacode when I came across the Mandelbrot [task](https://rosettacode.org/wiki/Mandelbrot_set), which looked doable in [Cixl](https://github.com/basic-gongfu/cixl) given a handful of new features that were already waiting to be added. The implementation described here uses ANSI graphics and supports zooming. You may find the code [here](https://github.com/basic-gongfu/cixl/blob/master/examples/mandelbrot.cx) and a Linux/64 binary [over there](https://github.com/basic-gongfu/cxbin/blob/master/linux64/mandelbrot).

![Screenshot](https://raw.github.com/basic-gongfu/cixl/master/devlog/mandelbrot.png)

### Implementation
It's easy to get the impression that there are as many ways to draw a Mandelbrot as there are implementations, each with its own set of magic numbers and quirks. I shamelessly copied constants and algorithm from Rosettacode's Kotlin [solution](https://rosettacode.org/wiki/Mandelbrot_set#Kotlin), and added an outer poll loop and changed the algorithm to only calculate the upper half of the screen and mirror.

```
use: cx;

define: max 4.0;
define: max-iter 570;

let: (max-x max-y) screen-size;
let: max-cx $max-x 2.0 /;
let: max-cy $max-y 2.0 /;
let: rows Stack<Str> new;
let: buf Buf new;
let: zoom 0 ref;

func: render()()
  $rows clear
  
  $max-y 2 / {
    let: y;
    $buf 0 seek

    $max-x {
      let: x;
      let: (zx zy) 0.0 ref %%;
      let: cx $x $max-cx - $zoom deref /;
      let: cy $y $max-cy - $zoom deref /;
      let: i #max-iter ref;

      {
        let: nzx $zx deref ** $zy deref ** - $cx +;
	$zy $zx deref *2 $zy deref * $cy + set
	$zx $nzx set
        $i &-- set-call	
        $nzx ** $zy deref ** + #max < $i deref and
      } while

      let: c $i deref % -7 bsh bor 256 mod;
      $c {$x 256 mod $y 256 mod} {0 0} if-else $c new-rgb $buf set-bg
      @@s $buf print
    } for

    $rows $buf str push   
  } for

  1 1 #out move-to
  $rows {#out print} for
  $rows riter {#out print} for;

#out hide-cursor
raw-mode

let: poll Poll new;
let: is-done #f ref;

$poll #in {
  #in read-char _
  $is-done #t set
} on-read

{
  $zoom &++ set-call
  render
  $poll 0 wait _
  $is-done deref !
} while

#out reset-style
#out clear-screen
1 1 #out move-to
#out show-cursor
normal-mode
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
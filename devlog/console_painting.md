## Console Painting
#### 2018-05-02

### Intro
As mentioned in the previous [post](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation.md), I've been on the lookout for the perfect pixel pushing library to integrate into [Cixl](https://github.com/basic-gongfu/cixl). I finally settled on splitting the problem in two by [using](https://github.com/basic-gongfu/cximage) [libcairo](https://www.cairographics.org/) for painting combined with a (yet to be materialized) [GTK](https://www.gtk.org/) backend. One of the benefits of choosing this route is that it also enables drawing directly to the console, as well as any other backend that may be represented as an array of pixels; using the full power of libcairo. To give you an idea of what that means, and scratch another long time itch; I put together the following [script](https://github.com/basic-gongfu/cximage/blob/master/examples/pngans.cx) to view PNG images in the console. You may find a Linux/64 binary [here](https://github.com/basic-gongfu/cxbin/blob/master/linux64/pngans);

![Screenshot](https://raw.github.com/basic-gongfu/cixl/master/devlog/monalisa.png)

### Implementation
We start by loading the specified image file into a newly allocated frame. If you've been following along, you might remember frames being mentioned in the previous [post](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation.md); they have now been promoted to plugin status as part of [cximage](https://github.com/basic-gongfu/cximage).

```
#!/usr/local/bin/cixl

link:
  'libcairo'
  'libcximage';
  
init: 'image';

use: cx cx/image;

func: print-usage()()
  ['Usage: cixl pngans.cx foo.png'] say
  -1 exit;

let: fn #args 0 get &print-usage or; 
let: (w h) screen-size --;

let: i $fn load-png;
```

Next we allocate yet another frame to use as destination for scaling and a painter. Once proportional scaling factors are calculated, the loaded image is set as destination brush and painted. Frames support printing ANSI graphics directly to output streams.

```
let: f $w $h new-frame;
let: p $f image new-painter;

$p
1.0 $w $i width float / min
1.0 $h $i height float / min
min % scale

$p $i 0 0 set-source
$p paint

#out clear-screen
1 1 #out move-to
$f #out print
#out flush
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
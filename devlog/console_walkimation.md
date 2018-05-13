## Console Walkimation
#### 2018-04-29

### Intro
When I started out on the C64 and Amiga, putting something on the screen that you owned was definitely less involved than ever after. Owning meaning that the only interaction is between programmer and screen, pixels and code; with no wiggly abstraction towers to balance. These days, it's all about the money; and you can't sell the past. Unfortunately, since anyone who was there will tell you that messing around with the latest JS framework just doesn't compare. One of the goals of [Cixl](https://github.com/basic-gongfu/cixl) is to provide the simplicity and directness of yesterdays tools in a modern package. You may find an archived version of everything mention in this post [here](https://github.com/basic-gongfu/cixl/tree/master/devlog/console_walkimation) and the master repo over [there](https://github.com/basic-gongfu/walker).

### Console
Modern Unix consoles may be treated as a buffer of 24-bit pixels, which allows a quick and convenient way to basic graphic capabilities. [Cixl](https://github.com/basic-gongfu/cixl) prefers powerful tools on top of a small number of universal abstractions to elaborate hierarchies and special cases. Except for a thin layer of features to output ANSI graphics, the general goal is to provide tools that are orthogonal to the chosen output medium. Since we've already checked [Tetris](https://github.com/basic-gongfu/cixl/blob/master/devlog/all_work_no_tetris.md) and Mandelbrot [fractals](https://github.com/basic-gongfu/cixl/blob/master/devlog/mandel_zoom.md) off the list, I figured it was time to have a look at animation.

### Walkimation
One of the many captivating details of [Lemmings](https://en.wikipedia.org/wiki/Lemmings_%28video_game%29) is how lifelike the animations are despite their simplicity, which is why the walker was an easy choice. The entire sequence consists of eight unique frames that are mirrored in code.

![Screenshot](https://raw.github.com/basic-gongfu/cixl/master/devlog/walker_static.png)

### Shapes, Reels, Palettes, Frames and Clips
Frames are composed from shapes and palettes. To keep things separate and allow easy editing of shapes, without requiring external files at runtime; shapes are read from files, processed and stored as clips inside the executable. The following is an example of what a shape file, or reel; looks like. Each letter indicates a color from the palette. By convention, highlights use uppercase letters and shadows lowercase. ```,``` may be used to include empty space into the shape, empty lines in this case.

[walk.dat](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation/walk.dat)

```
,
,
        HHhhHHHH
      hhHHhhhhhhHH
    HHHHhhhhsssshhhh
    HHhhhhhhssssssHH
    hhhhssssssssSSSS
    HHhhSSssssSSSSSS
      hhSSBBbbbb
        SSssBBbb
        SSssBBBBbb
        SSssssBBbb
        BBSSssssbb
        BBBBSSssbb
        BBBBBBBBbbbb
        bbBBBBbbbbbb
SSSSBBBBBBBBbbbbbbbb
SSssBBBBbbbbbbbbbbbb
SSss      SSSSssssss
SSSSss      SSSSssSSss

,
,
  HHHHHHhhHHhhHH
HHhhhhhhhhhhHHhhhh
hhHHhhhhssss
HHhhhhhhssssss
HHhhssssssSSSSSS
HHhhssssSSssSSSS
  hhssSSBBbb
    SSssssBB
    BBSSssbb
    BBSSssBBbb
    SSSSssBBbb
  SSssssBBBBbbbb
    BBBBBBBBbbbb
  SSBBBBBBbbbbbb
SSssBBBBbbbbbbbb
SSssbbbbbbbbbb
    SSssssss
    SSSSssssss
```

Loading and mirroring reels goes something like this:

[walker.cx](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation/walker.cx)
```
type-id: Shape Stack<Str>;
type-id: Reel Stack<Shape>;

func: load-reel(fname Str)(out Reel)
  let: out Reel new;
  let: d Stack<Str> new;
  let: width 0 ref;

  $fname `r fopen lines {
    let: l;
    
    $l len {
      $l last @, = {$l pop _} if
      $d $l push
      $width {$l len max} set-call
    } {
      $out $width deref $d new-shape push
      $d clear
      $width 0 set
    } if-else
  } for;

define: r-reel 'walk.dat' load-reel;

func: h-mirror(s Shape)(ms Shape)
  let: ms $s %% ~ _;
  $ms &reverse for;

define: l-reel #r-reel &h-mirror map stack;
```

The loaded reels are then combined with palettes into frames, and finally clips:

[walker.cx](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation/walker.cx)
```
type-id: Palette Table<Char Color>;

rec: Frame
  height
  width  Int
  data   Stack<Opt<Color>>;

func: new-frame(h w Int)(_ Frame) [
  `height $h,
  `width $w,
  `data Stack<Opt<Color>> new,
] Frame new ->;

func: frame(s Shape p Palette)(f Frame)
  let: f $s len $s 0 get len new-frame;
  let: d $f `data get;
  
  $s {{
    let: c;
    $d $c @@s = #nil {$p $c get} if-else push
  } for} for;

func: clip(r Reel p Palette)(c Clip)
  let: c Clip new;
  $c `height 0 put
  $c `width 0 put
  
  $c `frames $r {
    let: f $p frame;
    $c `height {$f `height get max} put-call
    $c `width {$f `width get max} put-call
    $f
  } map Stack<Frame> new into put;

define: body  36  38 252 rgb;
define: skin 252 186 148 rgb;
define: hair  56 104  56 rgb;

define: hlf 1.1;

define: palette [
  @b #body,
  @B #body #hlf *,
  @s #skin,
  @S #skin #hlf *,
  @h #hair,
  @H #hair #hlf *,
] Palette new ->;

define: r-clip #r-reel #palette clip;
define: l-clip #l-reel #palette clip;
```

### Scripts
Once the clips are in place, we need a representation for individual walkers and a render loop to get things moving. You may find a Linux/64 binary running the code below [here](https://github.com/basic-gongfu/cxbin/blob/master/linux64/walker).

![Screenshot](https://raw.github.com/basic-gongfu/cixl/master/devlog/walker_dynamic.gif)

Each walker keeps track of its position, frame index and clip:

[walker.cx](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation/walker.cx)
```
rec: Walker
  x y i Int
  clip  Clip;

func: new-walker(x y Int c Clip)(_ Walker) [
  `i 0,
  `x $x, `y $y,
  `clip $c,
] Walker new ->;
  
func: next-frame(w Walker)(f Frame)
  let: i $w `i get;
  let: c $w `clip get;
  let: fs $c `frames get;
  let: f $fs $i get;
  $w `i $i ++ $fs len mod put;
```

We'll use lambdas to represent scripts for now, since the only thing they care about is execution:

[dynamic.cx](https://github.com/basic-gongfu/cixl/blob/master/devlog/console_walkimation/dynamic.cx)
```
define: (r-stride l-stride) 4 -4;

let: (screen-x screen-y) screen-size;
let: max-x $screen-x #l-clip `width get -;
let: max-y $screen-y #l-clip `height get -;

let: scripts Stack new;
let: buf Buf new;

func: start-script()()
  let: start-x $max-x rand ++;
  let: start-y $max-y rand ++;
  let: left 2 rand 1 =;
  let: w $start-x $start-y $left #l-clip #r-clip if-else new-walker;
  let: dx $left #l-stride #r-stride if-else ref;

  $scripts {
    let: x $w `x get;
    $x $w `y get $buf move-to
    $w next-frame $buf print
    let: nx $x $dx deref +;
    $w `x $nx put

    $nx 0 > {
      $nx $max-x > {
        $dx #l-stride set
        $w `clip #l-clip put
	$w `x $max-x put
      } if
    } {
      $dx #r-stride set
      $w `clip #r-clip put
      $w `x 1 put
    } if-else
  } push;

func: render()()
  $buf 0 seek
  $scripts &call for
  #out clear-screen
  $buf str #out print
  #out flush;
```

### Future
The goal is to eventually pull basic animation support into the standard library, but there is plenty of experimentation left to be done. I would like to add support for timing to enable running scripts at different speeds, extract a sprite concept with terrain and collision tracking and more. And as retro cool as it is to pretend the console is a frame buffer, a more expressive output medium wouldn't hurt; but I'm still searching for the perfect pixel pushing library.

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
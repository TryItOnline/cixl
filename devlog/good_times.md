## Good Times
#### 2018-01-05

### Intro
Raise hands everyone who have at some point torn their hair out and cursed at a time library API. I know I have, more than I wish to remember. For some reason we've agreed to make time really, really complex; and it's been going on for thousands of years. Epochs, leaps, months; arbitrary multiples of 6, 7 and 10; time zones, daylight savings; the list goes on and on. I was once tasked with the very unpleasant problem of calculating week numbers with varying start days for cabin rentals, something precious few libraries are capable of; ever since then I feel like throwing up whenever someone mentions week numbers. The question that's been lingering in my mind is how much complexity is intrinsic to the problem and how much comes from choosing suboptimal abstractions. I recently had a reason to ponder that question again while adding time support to [cixl](https://github.com/basic-gongfu/cixl).

### Two Times
If you cut the concept of time down to its core, there are really two kinds of time that we care about; points and intervals. Both dates and times have this property in common, the difference is what units they use. On one end of the spectrum, C treats time as number of seconds and leaves most of the rest for user code to deal with; on the other end Python's designers added as many distinct freaking [concepts](https://docs.python.org/3.5/library/datetime.html) as they could possibly think of. The advantage of using fewer types is a more composable and powerful API, one disadvantage is that it may be tricky to overlay several modes of usage on the same type without causing a mess.

### The Middle Way
[Cixl](https://github.com/basic-gongfu/cixl) chooses the middle way and provides a single type with dual semantics. Internally; time is represented as an absolute, zero-based number of months and nanoseconds. The representation is key to providing dual semantics, since it allows remembering enough information to give sensible answers.

Time may be queried for absolute and relative field values;

```
   | let: t now; $t
...
[Time(2018/0/3 20:14:48.105655092)]

   | years $t, month $t, days $t
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

### That's It?
There is more to come, but I figured this would make a good introduction to some of the choices made. Mixing time and zones never made any sense to me, they are clearly better off being implemented as independent conversions in my mind. Consider unicode strings as a comparison; we generally don't tack encodings onto byte strings just in case someone might want to go unicode in the future; what usually happens instead is that user code is asked to specify an encoding at conversion time. We will have to wait and see if I manage to talk myself into taking on week numbers.

Give me a yell if something is unclear, wrong or missing. And please do consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
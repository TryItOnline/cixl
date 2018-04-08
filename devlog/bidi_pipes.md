## Bidirectional Unix Pipes
#### 2018-04-08

### Intro
One gotcha you're bound to run into while finding your way around the Unix shell is the lack of bidirectional pipes. Unix being Unix, the way to get bidirectional streams setup between processes is to reuse what's already there. By creating extra pipes; cloning the process; rebinding the child's standard streams, and finally swapping the child for a process running the target command. This post will run through an implementation of this recipe in C, demonstrate how [Cixl](https://github.com/basic-gongfu/cixl) may help with solving these kinds of problems, and provide a simple tool to quickly perform the same magic from the shell.

The following example demonstrates how to run external processes from [Cixl](https://github.com/basic-gongfu/cixl), by encrypting and decrypting a message using [external](https://github.com/basic-gongfu/cxbin/blob/master/linux64/encrypt) [tools](https://github.com/basic-gongfu/cxbin/blob/master/linux64/decrypt). Be aware that this implementation uses blocking IO to read and write all data in one go. ```in``` and ```out``` are mapped to the child process standard input/output

```
use: cx;

let: p 'secret';
let: e 'encrypt' [$p] popen;
$e in % 'message' print close

let: d 'decrypt' [$p] popen;
$d in % $e out str print close
$d out str say
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
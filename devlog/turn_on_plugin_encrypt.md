## Turn On, Plugin, Encrypt
#### 2018-03-26

### Intro
One of the things I've been itching to add to [Cixl](https://github.com/basic-gongfu/cixl) is basic crypto support; as in password hashing, symmetric and asymmetric encryption. The reason I've been waiting is that I didn't have a good plugin story yet, and I refuse to add any dependencies to the core language. Convenient access to basic encryption using modern algorithms and sane defaults is something that's sadly often missing. [libsodium](https://github.com/jedisct1/libsodium) takes as many steps in the right direction as C allows, which makes it an ideal foundation for a crypto [plugin](https://github.com/basic-gongfu/cxcrypt).

```
/* Link libraries */
link:
  'libsodium'
  'libcxcrypt';

/* Initialize cxcrypt */
init: 'crypt';

/* Plugins are not included in the default cx package and have to be used
   separately */

use: cx cx/crypt;

/* Hash key using random salt */
let: s 'key' Salt new secret;

/* Equality is overloaded to do the right thing */
$s 'key' = check
$s 'wrong' = !check

/* Encryption round trip */
$s 'message' encrypt
$s decrypt 'message' = check
```

### Plugins
A Cixl plugin is any regular C library containing external functions with the signature ```bool cx_init_X(struct cx *)```. ```init:``` looks up the specified functions and calls them with the current context as argument during parsing. Libraries passed to ```link``` are dynamically loaded when interpreting and linked when compiling to C.

### Algorithms
The libsodium APIs used by [cxcrypt](https://github.com/basic-gongfu/cxcrypt) rely on Argon2 for hashing keys and XSalsa20 with Poly1305 MAC authentication for symmetric encryption. I'm still working on integrating support for asymmetric encryption.

### Examples
[encrypt.cx](https://raw.githubusercontent.com/basic-gongfu/cxcrypt/master/examples/encrypt.cx) and [decrypt.cx](https://raw.githubusercontent.com/basic-gongfu/cxcrypt/master/examples/decrypt.cx) both use [cxcrypt](https://github.com/basic-gongfu/cxcrypt) to implement basic tools for encrypting/decrypting arbitrary files from the shell. Linux/64 binaries may be found [here](https://github.com/basic-gongfu/cxbin/tree/master/linux64).

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
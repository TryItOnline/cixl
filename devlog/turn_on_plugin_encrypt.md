## Turn On, Plugin, Encrypt
#### 2018-03-26

### Intro
One of the things I've been waiting to add to [Cixl](https://github.com/basic-gongfu/cixl) is basic crypto support; as in password hashing, symmetric and asymmetric en-/decryption. The reason I've been waiting is that I didn't have a good story for plugins yet, and I refuse to add dependencies to the core language. Convenient access to basic encryption using modern algorithms and sane defaults is something that's sadly often missing. [libsodium](https://github.com/jedisct1/libsodium) takes as many steps in the right direction as C allows, which makes it an ideal foundation for a crypto plugin.

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
## The Seed of a Collab Server
#### 2018-03-17

### Intro
One of the joys of creating a programming language is coming up with problems of just the right size that are interesting enough to warrant solving; just the right size meaning that viable solutions intersect the current feature set of the language by a large margin, while putting a healthy amount of stress on the design. To help drive implementation of error handling and networking in [Cixl](https://github.com/basic-gongfu/cixl) forward, I decided I might as well scratch the collab server itch that I've developed lately.

### Collab Who?
Collab as in collaboration, think Facebook or Slack minus awesome profits. In its current state, [Collabri](https://raw.githubusercontent.com/basic-gongfu/cixl/master/examples/collabri.cx) implements just enough functionality to get networked, persistent, chronological feeds organized in hierarchies. The user interface is text based for now; as well as client agnostic, meaning you may use ```nc``` or any other tool capable of doing line IO over a TCP connection without getting in the way to access the server.

```
$ ./collabri 7707
Collabri v0.2

Server Setup

Intro: Welcome to my Collabri!

User: me
Password: secret

Done!

Listening on port 7707
```

```
$ nc 127.0.0.1 7707
Welcome to my Collabri!

User: me
Password: secret

/ :branch foo
/foo/ :branch bar
/foo/bar/ hello world
/foo/bar/ :join ..
/foo/ :tail
18 00:31 @me /foo/bar/ hello world

/foo/ :join /
/ :list
/foo/bar/

/ :join foo/bar
/foo/bar/
```

### Running
A (hope)fully functioning implementation is available [here](https://raw.githubusercontent.com/basic-gongfu/cixl/master/examples/collabri.cx). A statically linked Linux64 binary is [provided](https://github.com/basic-gongfu/cixl/blob/master/bin/collabri); and instructions on how to compile the code yourself may be found [here](https://github.com/basic-gongfu/cixl#getting-started).

### Async
[Cixl](https://github.com/basic-gongfu/cixl) encourages using async networking by opening sockets in non blocking mode and providing a convenient poll abstraction with callbacks. Collabri takes the idea one step further and integrates database and logging into the same event loop. Included below is the main loop.

```
let: poll Poll new;
let: server #nil $port #backlog listen;

$poll $server {
  let: io $server accept;
  $io {$io new-client % push-prompt} if
} on-read

['Listening on port ' $port] say

{$poll -1 wait} while
```

### Database
Collabri includes a simple log-based database [engine](https://raw.githubusercontent.com/basic-gongfu/cixl/master/examples/qdb.cx), all data is stored in ```~/.collabri```.

```
func: get-path(n Str)(_ Str)
  [home-dir '.collabri' $n] @/ join;

'' get-path make-dir

let: users   'users.db'   get-path [`name] new-db-table;
let: topics  'topics.db'  get-path [`id]   new-db-table;
let: options 'options.db' get-path [`id]   new-db-table;
```

### Clients
Each client is tagged with a user once authenticated. New users are created automagically on login with a new name. Output is buffered in ```buf``` until the client is ready to receive it. New clients start their life in the root topic and register a read callback in the event loop.

```
rec: Client ()
  state   Sym
  user    User
  topic   Topic
  io      TCPClient
  buf     Buf
  is-mute Bool;

func: new-client(io TCPClient)(_ Client)
  let: c Client new;
  
  $c `state   `auth-user put
  $c `io      $io        put
  $c `buf     Buf new    put
  $c `is-mute #f         put
  
  $c $root join-topic
  let: ils $io lines;
  
  $poll $io {
    let: in $ils next;

    $in is-nil {
      $ils is-done {$c disconnect} if
    } {
      catch: (A $c ~ push-error $c push-prompt dump rollback)
	$c % `state get $in handle-in
	poll commit;
    } if-else
  } on-read
  
  $c;
```

### Topics
Collabri supports organizing feeds, or topics, into hierarchies. This enables interacting with a tree of topics on different levels depending on current needs, moving back and forth between observing multiple topics and drilling down to have a closer look or engage in a discussion. ```load-topic``` and ```save-topic``` are called by the database engine to de/serialize topics from/to disk.

```
rec: Topic ()
  id name  Sym
  parent   Topic
  children Table
  clients  Table
  log-path Str
  log-buf  Buf
  log      WFile;

func: add-topic(pt t Topic)()
  let: pc $pt `children get;

  $pc {
    $pc $t `name get $t put
  } {
    let: pc Table new;
    $pt `children $pc put
    $pc $t `name get $t put    
  } if-else;

func: find-topic(id Sym)(_ Opt)
  $topics [$id] find-key;

func: load-topic(in Stack)(_ Topic)
  let: out Topic new;
  let: (id name parent-id log-path) $in {} for;

  $out `id       $id       put
  $out `name     $name     put
  $out `clients  Table new put
  $out `log-path $log-path put

  $parent-id {
    let: pt $parent-id `/ = $root {$parent-id find-topic} if-else;
    $pt {['Topic not found: ' $parent-id] throw} else
    $out `parent $pt put
    $pt $out add-topic
  } if

  $out;
  
$topics &load-topic on-load-rec

func: save-topic(in Topic)(_ Stack) [
  $in `id get
  $in `name get
  $in `parent get `id get
  $in `log-path get
];

$topics &save-topic on-save-rec

func: join-topic(c Client ns Str)()
  $c `topic get `clients get $c delete
  $ns 0 get @/ = {$c `topic $root put} if
  
  $ns @/ split {
    let: n;
    let: t $c `topic get;

    $n '..' = {
      let: pt $t `parent get;
      $pt {'Topic not found: ..' throw} else
      $c `topic $pt put
    } {
      let: tc $t `children get;
      $tc {['Topic not found: ' $n] throw} else
    
      let: ct $tc $n sym get;
      $ct {['Topic not found: ' $n] throw} else

      $c `topic $ct put
    } if-else
  } for

  $c `topic get `clients get $c #t put;
```

### States
Each state has a separate function for handling user input, adding a new state is as easy as defining ```handle-in``` with a unique symbol. ```ok``` is the default command state once authenticated.

```
func: handle-in(c Client `auth-user in Str)()
  $in {
    let: name $in sym;
    let: u $name find-user;
    $c `user $u {$name new-user} or put
    $c `state `auth-password put
    $c push-prompt
  } {
    $c disconnect
  } if-else;

func: handle-in(c Client `auth-password in Str)()
  let: u $c `user get;
  let: p $u `password get;

  $p is-nil {
    $u `password $in put
    $users $u upsert
  } {
    $p $in = {
      $c `user #nil put
      $c `state `auth-user put
      'Authentication failed' throw
    } else
  } if-else
  
  $root `clients get $c #t put  
  $c `state `ok put
  $c @@n push-out
  $c push-prompt;

func: handle-in(c Client `ok in Str)()
  $in {
    let: it $in iter;

    $it next @: = {
      let: cmd $it words next sym;
      $c $cmd $it handle-cmd
    } {
      $c $in new-post
      $c push-prompt
    } if-else
  } {
    $c push-prompt
  } if-else;
```

### Commands
Like states, each command has a separate function; adding a new command is as easy as defining ```handle-cmd``` with a unique symbol.

```
func: handle-cmd(c Client `branch in Iter)()
  let: n $in str;
  $n {'Missing topic name' throw} else
  $c $n sym new-topic
  $c push-prompt;

func: handle-cmd(c Client `join in Iter)()
  $c $in str $root or join-topic
  $c push-prompt;

func: handle-cmd(c Client `list in Iter)()
  $c % `topic get #f list-topics
  $c @@n push-out
  $c push-prompt;

func: handle-cmd(c Client `tail in Iter)()
  $c $in str int #tail-default or topic-tail;
```

### Logging
Posts are logged per topic in regular text files that may be processed using external tools or queried through the server. Logging takes hierarchies into account, child topics are included in the parent topic's log. Topic logs are stored in ```~/.collabri/log```. Log files are registered for write polling if the buffer was previously empty, and unregistered once the buffer is consumed.

```
func: open-log(t Topic m Sym)(_ File)
  let: p $t `log-path get;
  [$p @/ $t `name get '.txt'] #nil join $m fopen;

func: get-log(t Topic)(_ WFile)
  $t `log get {
    let: p $t `log-path get;
    $p make-dir

    let: l $t `a open-log;    
    $t `log $l put
    
    $l
  } or;

func: push-log(t Topic in Str)()
  let: b $t `log-buf get {(
    let: b Buf new;
    $t `log-buf $b put
    $b
  )} or;

  let: l $t get-log;

  $b {    
    $poll $l {
      $l $b write-bytes
      $b {$poll $l no-write} else
    } on-write
  } else
  
  $b $in push;
```

### One Last Thing
Please keep in mind that the implementation described here is insecure over public networks as all data is sent unencrypted. I'm considering eventually supporting optional encrypted traffic. Additionally; password are currently stored unhashed, which means that anyone with access to the database can read all passwords.

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).
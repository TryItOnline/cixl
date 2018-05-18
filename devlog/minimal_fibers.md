## Minimal Fibers
#### 2018-05-17

### Intro
Fibers, or Green Threads, is one of those features that I knew [Cixl](https://github.com/basic-gongfu/cixl) would have to support eventually; but I wanted to wait until the dust settled before deciding on implementation details. Some languages, like Lua for example; expose raw coroutines and leave scheduling as an excercise. While Cixl may eventually support raw coroutines, I still feel like it makes sense to separate concerns and provide fibers and scheduling as first class concepts. The implementation described here is my best attempt at cutting the concepts down to their core within the framework provided by Cixl; it has much in common with Smalltalk's flavor of cooperative concurrency, but comes with the added twist of first class schedulers.

### Examples
The following example demonstrates how to create a scheduler, push some fibers and wait until all are finished:

```
#!/usr/local/bin/cixl

use: cx;

let: s Sched new;
let: out [];

$s {
  $out 1 push
  resched
  $out 2 push
} push

$s {
  $out 3 push
  resched
  $out 4 push
} push

$s run
$out ', ' join say
```

Output:
```
1, 3, 2, 4
```

The scheduler may alternatively be iterated for more granular control, each iteration triggers one fiber and pushes the number of fibers that are still active on the stack:

```
let: s Sched new;
let: out [];

$s {
  $out `foo push
  resched
  $out `bar push
} push

$s {
  $out `baz push
} push

$s {$out ~ push} for
$out ', ' join say
```

Output:
```
foo, 2, baz, 1, bar, 0
```

### Performance
To get an idea about what kind of raw performance to expect, I wrote a simple benchmark that runs 2M task switches in Ruby and Cixl. I suspect most of the difference is due to Cixl running the scheduler loop in C. Measured time is displayed in milliseconds.

[bench5.rb](https://github.com/basic-gongfu/cixl/blob/master/perf/bench5.rb)
```
n = 1000000

task1 = Fiber.new {n.times {Fiber.yield}}
task2 = Fiber.new {n.times {Fiber.yield}}

t1 = Time.now
n.times { task1.resume task2.resume }
t2 = Time.now
delta = (t2 - t1) * 1000
puts "#{delta.to_i}"

$ ruby bench5.rb
3546
```

[bench5.cx](https://github.com/basic-gongfu/cixl/blob/master/perf/bench5.cx)
```
define: n 1000000;
let: s Sched new;

2 {$s {#n &resched times} push} times
{$s run} clock 1000000 / say

$ cixl bench5.cx
3169
```

### Implementation
Most heavy lifting is delegated to [ucontext.h](http://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html), each fiber (or task as Cixl likes to call them) carries its own fixed size C stack. Besides managing the stack, tasks additionally backup and restore Cixl's runtime state when rescheduled and resumed. 

sched.[h](https://github.com/basic-gongfu/cixl/blob/master/src/cixl/sched.h)/[c](https://github.com/basic-gongfu/cixl/blob/master/src/cixl/sched.c)
```
struct cx_sched {
  ucontext_t context;
  
  struct cx_ls tasks;
  unsigned int ntasks, nrefs;
};

bool cx_sched_next(struct cx_sched *s, struct cx_scope *scope) {
  if (s->tasks.next == &s->tasks) { return false; }
  struct cx_task *t = cx_baseof(s->tasks.next, struct cx_task, queue);
  cx_ls_delete(&t->queue);    
  if (!cx_task_run(t, scope)) { return false; }
  
  if (t->state == CX_TASK_DONE) {
    free(cx_task_deinit(t));
    s->ntasks--;
  } else {
    cx_ls_prepend(&s->tasks, &t->queue);
  }

  return true;
}

bool cx_sched_run(struct cx_sched *s, struct cx_scope *scope) {
  while (cx_sched_next(s, scope));
  return true;
}
```

task.[h](https://github.com/basic-gongfu/cixl/blob/master/src/cixl/task.h)/[c](https://github.com/basic-gongfu/cixl/blob/master/src/cixl/task.c)
```
static void on_start(int t_lo, int t_hi,
		     int scope_lo, int scope_hi) {
  uintptr_t t_ptr = (uintptr_t)t_lo | ((uintptr_t)t_hi << 32);
  struct cx_task *t = (struct cx_task *)t_ptr;
  t->state = CX_TASK_RUN;

  uintptr_t scope_ptr = (uintptr_t)scope_lo | ((uintptr_t)scope_hi << 32);
  struct cx_scope *scope = (struct cx_scope *)scope_ptr;
  struct cx *cx = scope->cx;
  cx_call(&t->action, scope);

  cx->task = t->prev_task;
  cx->bin = t->prev_bin;
  cx->pc = t->prev_pc;
  while (cx->libs.count > t->prev_nlibs) { cx_pop_lib(cx); }
  while (cx->scopes.count > t->prev_nscopes) { cx_pop_scope(cx, false); }
  while (cx->ncalls > t->prev_ncalls) { cx_test(cx_pop_call(cx)); }
  t->state = CX_TASK_DONE;
}

bool cx_task_run(struct cx_task *t, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  t->prev_task = cx->task;
  cx->task = t;
  t->prev_bin = cx->bin;
  cx->bin = t->bin;
  t->prev_pc = cx->pc;
  cx->pc = t->pc;
  t->prev_nlibs = cx->libs.count;
  t->prev_nscopes = cx->scopes.count;
  t->prev_ncalls = cx->ncalls;

  switch (t->state) {
  case CX_TASK_NEW: {
    if (getcontext(&t->context) == -1) {
      cx_error(cx, cx->row, cx->col, "Failed getting context: %d", errno);
      return false;
    }
    
    t->context.uc_stack.ss_sp = t->stack;
    t->context.uc_stack.ss_size = CX_TASK_STACK_SIZE;
    t->context.uc_link = &t->sched->context;
    uintptr_t t_ptr = (uintptr_t)t;
    uintptr_t scope_ptr = (uintptr_t)scope;
    
    makecontext(&t->context,
		(void (*)(void))on_start,
		4,
		(int)t_ptr, (int)(t_ptr >> 32),
		(int)scope_ptr, (int)(scope_ptr >> 32));
    
    if (swapcontext(&t->sched->context, &t->context) == -1) {
      cx_error(cx, cx->row, cx->col, "Failed swapping context: %d", errno);
      return false;
    }
    
    return true;
  }
  case CX_TASK_RUN: {
    if (t->libs.count) {
      cx_vec_grow(&cx->libs, cx->libs.count+t->libs.count);
    
      memcpy(cx_vec_end(&cx->libs),
	     t->libs.items,
	     sizeof(struct cx_lib *) * t->libs.count);
    
      cx->libs.count += t->libs.count;
      cx->lib = cx_vec_peek(&cx->libs, 0);
      cx_vec_clear(&t->libs);
    }

    if (t->scopes.count) {
      cx_vec_grow(&cx->scopes, cx->scopes.count+t->scopes.count);
    
      memcpy(cx_vec_end(&cx->scopes),
	     t->scopes.items,
	     sizeof(struct cx_scope *) * t->scopes.count);
    
      cx->scopes.count += t->scopes.count;
      cx->scope = cx_vec_peek(&cx->scopes, 0);
      cx_vec_clear(&t->scopes);
    }

    if (t->calls.count) {
      memcpy(cx->calls+cx->ncalls,
	     t->calls.items,
	     sizeof(struct cx_call) * t->calls.count);
    
      cx->ncalls += t->calls.count;
      cx_vec_clear(&t->calls);
    }
    
    if (swapcontext(&t->sched->context, &t->context) == -1) {
      cx_error(cx, cx->row, cx->col, "Failed swapping context: %d", errno);
      return false;
    }
    
    return true;
  }
  default:
    cx_error(cx, cx->row, cx->col, "Invalid task run state: %d", t->state);
    break;
  }

  return false;
}

bool cx_task_resched(struct cx_task *t, struct cx_scope *scope) {
  cx_test(t->state != CX_TASK_DONE);
  struct cx *cx = scope->cx;
  cx->task = t->prev_task;
  t->bin = cx->bin;
  cx->bin = t->prev_bin;
  t->pc = cx->pc;
  cx->pc = t->prev_pc;

  ssize_t nlibs = cx->libs.count - t->prev_nlibs;
  
  if (nlibs) {
    cx_vec_grow(&t->libs, nlibs);

    memcpy(t->libs.items,
	   cx_vec_get(&cx->libs, t->prev_nlibs),
	   sizeof(struct cx_lib *) * nlibs);

    t->libs.count = nlibs;
    cx->libs.count -= nlibs;
    cx->lib -= nlibs;
  }

  ssize_t nscopes = cx->scopes.count - t->prev_nscopes;
  
  if (nscopes) {
    cx_vec_grow(&t->scopes, nscopes);

    memcpy(t->scopes.items,
	   cx_vec_get(&cx->scopes, t->prev_nscopes),
	   sizeof(struct cx_scope *) * nscopes);

    t->scopes.count = nscopes;
    cx->scopes.count -= nscopes;
    cx->scope -= nscopes;
  }

  ssize_t ncalls = cx->ncalls - t->prev_ncalls;
  
  if (ncalls) {
    cx_vec_grow(&t->calls, ncalls);
    memcpy(t->calls.items, cx->calls+t->prev_ncalls, sizeof(struct cx_call) * ncalls);
    t->calls.count = ncalls;
    cx->ncalls -= ncalls;
  }
  
  if (swapcontext(&t->context, &t->sched->context) == -1) {
    cx_error(cx, cx->row, cx->col, "Failed swapping context: %d", errno);
    return false;
  }
  
  return true;
}
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts.
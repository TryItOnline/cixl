from timeit import timeit

def _fib(a, b, n):
    return _fib(b, a+b, n-1) if n > 0 else a

def fib(n):
    return _fib(0, 1, n)

def test():
    for i in range(10000):
        fib(50)

print(int(timeit(test, number=1) * 1000))

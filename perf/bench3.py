from timeit import timeit

class Foo():
    pass

def test():
    for i in range(10000000):
        foo = Foo()
        foo.x = 42
        foo.y = "bar"

print(int(timeit(test, number=1) * 1000))

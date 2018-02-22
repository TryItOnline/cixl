from timeit import timeit

def test():
    v = []
    
    for i in range(10000000):
        v.append(i)

print(int(timeit(test, number=1) * 1000))

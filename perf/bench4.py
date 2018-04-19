from timeit import timeit

def test():
    for i in range(10000000):
        try:
            raise Exception('error')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
            print('skipped')
        except Exception as e:
            pass

print(int(timeit(test, number=1) * 1000))

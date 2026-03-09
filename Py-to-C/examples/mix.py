def f(n):
    total = 0
    for i in range(n):
        if i > 2:
            total = total + i
    return total

def g(x):
    return f(x) + f(x + 1)

a = g(3)

if a > 5:
    print("big")
else:
    print("small")
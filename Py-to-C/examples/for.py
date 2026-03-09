def square(n):
    return n * n

total = 0

for i in range(5):
    total = total + square(i)

if total >= 30:
    total = total - 10
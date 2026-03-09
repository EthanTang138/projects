#!/usr/bin/env python3
"""
Quick test script to verify the parser handles indentation correctly
"""

from parser import parser, ASTBuilder
from dataclasses import asdict
import json

# Test 1: Simple function
code1 = """
def greet(name):
    return name
"""

# Test 2: If statement with else
code2 = """
x = 5
if x > 3:
    y = 10
else:
    y = 20
"""

# Test 3: For loop
code3 = """
total = 0
for i in range(5):
    total = total + i
"""

# Test 4: Nested structure
code4 = """
def factorial(n):
    if n == 0:
        return 1
    else:
        return n
"""

tests = [
    ("Simple function", code1),
    ("If-else statement", code2),
    ("For loop", code3),
    ("Nested structure", code4)
]

print("Testing Parser with Indentation Support")
print("=" * 60)

for name, code in tests:
    print(f"\nTest: {name}")
    print("-" * 60)
    try:
        tree = parser.parse(code)
        ast = ASTBuilder().transform(tree)
        print("✓ Parsed successfully!")
        print(json.dumps(asdict(ast), indent=2))
    except Exception as e:
        print(f"✗ Parse failed: {e}")

print("\n" + "=" * 60)
print("Testing complete!")

#!/usr/bin/env bash

EXAMPLES_DIR="examples"

if [ ! -d "$EXAMPLES_DIR" ]; then
  echo "Examples directory '$EXAMPLES_DIR' not found."
  exit 1
fi

for example in "$EXAMPLES_DIR"/*.py; do
  echo "Running parser on: $example"
  python3 parser.py "$example"
  echo
done

echo "All examples have been processed."
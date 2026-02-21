#!/bin/bash
echo "Testing 3-char password (abc)..."
HASH=$(echo -n "abc" | sha256sum | cut -d" " -f1)
./sha256_cracker --target $HASH --mode gpu --min 3 --max 3 --alphabet "abc"

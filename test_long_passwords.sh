#!/bin/bash
echo "Testing 5-char password (hello)..."
HASH=$(echo -n "hello" | sha256sum | cut -d" " -f1)
./sha256_cracker --target $HASH --mode gpu --min 5 --max 5 --alphabet "helo"

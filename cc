#!/bin/sh

# main's stack pointer isn't properly aligned to 16 bytes, fix this later
gcc -mno-sse -mno-sse2  -ffreestanding -fno-builtin -Wno-builtin-declaration-mismatch -nostdlib -Wall -Wextra -pedantic -std=c11 -I../mystdlib/include/ $@ -L../mystdlib -lministd

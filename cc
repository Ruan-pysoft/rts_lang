#!/bin/sh

gcc -ffreestanding -fno-builtin -Wno-builtin-declaration-mismatch -nostdlib -Wall -Wextra -pedantic -std=c11 -I../mystdlib/include/ $@ -L../mystdlib -lministd

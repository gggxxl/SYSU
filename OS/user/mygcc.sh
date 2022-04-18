#!/bin/bash

nasm -f elf32 lib/starter.asm -o startfiles.o
gcc -I lib/ -march=i386 -m16 -fno-pic -fno-builtin -mpreferred-stack-boundary=2 -ffreestanding -c $1 -o $1.o
ld -m elf_i386 -N startfiles.o $1.o -Ttext 0x0100 --oformat binary $2 $3
rm startfiles.o
rm $1.o
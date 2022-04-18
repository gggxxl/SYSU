#! /bin/bash
nasm -f elf32 src/gxlOS.asm -o s1.o
nasm -f elf32 src/io.asm -o s2.o
nasm -f elf32 src/int.asm -o s3.o

gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/cmain.c -o c1.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/io.c -o c2.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/dir.c -o c3.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/fat.c -o c4.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/file.c -o c5.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/mysys.c -o c6.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/run.c -o c7.o
gcc -march=i386 -m16 -fno-pic -mpreferred-stack-boundary=2 -ffreestanding -c src/str.c -o c8.o

ld -m elf_i386 -N s1.o s2.o s3.o c1.o c2.o c3.o c4.o c5.o c6.o c7.o c8.o -Ttext 0x0100 --oformat binary -o os.syc

rm *.o

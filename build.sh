#!/bin/sh 

## assemble file of nops
nasm -f elf64 src/asm/nul.asm -o nul.o
nasm -f elf32 src/asm/nul32.asm -o nul32.o
ld -s -n nul.o -o nul
ld -s -n -m elf_i386 nul32.o -o nul32
rm nul.o nul32.o

## compile lasm with keystone and ncurses
gcc -Wall -o lasm src/main.c src/utils/*.c -lncurses -lkeystone

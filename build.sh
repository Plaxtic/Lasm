#!/bin/sh -e

## check dependencies
for dep in $(cat Requirements.txt)
do
    D=`whereis $dep | cut -d ':' -f 2`
    if ! [[ $D ]]
    then
        echo "missing dependency $dep"
        exit 1
    fi
done

## install option
if [ "$1" == "--install" ]
then

    # check root for install
    if (( $UID ))
    then
        echo "Run as root!"
        exit 1
    fi

    PREFIX="/usr/local/bin/"
    DEFINE="INSTALL"

    touch "$HOME/.lasm_history"
else
    PREFIX=""
    DEFINE="NORMAL"

    touch ".lasm_history"
fi

## assemble file of nops
nasm -f elf64 src/asm/nul.asm -o "nul.o"
nasm -f elf32 src/asm/nul32.asm -o "nul32.o"
ld -s -n nul.o -o $PREFIX"nul"
ld -s -n -m elf_i386 nul32.o -o $PREFIX"nul32"
rm nul.o nul32.o

## compile lasm with keystone and ncurses
gcc -Wall -o $PREFIX"lasm" src/main.c src/utils/*.c -D "$DEFINE" -lncurses -lkeystone


# Lasm

![](imgs/example.png)

# Live assembly shell 

Assembles, injects, and executes x86 CPU instuctions. 
Includes view of stack and register state.

Requires nasm and keystone assembler library.

To place a label (save an address to jmp to), type a string with ':' 
at the end.

```
                          [0x400080] label:
```
```
label:
4831c0                    [0x400080]> xor rax, rax
4885c0                    [0x400083]> test rax, rax
74f8                      [0x400086]> jz label
                          [0x400080]>
```

Type s to take a single step, type s [N] to take several steps.  
```
                          [0x400080] s 10
```
```
                          [0x40008a]
```

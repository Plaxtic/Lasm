# Lasm

![](imgs/example.png)

Requires keystone assembler library

To place a label (save an address to jmp to), type a string with ':' at the end 

```
        [0x400080] label:
```
```
label:
        [0x400080] xchg rax, rax 
```


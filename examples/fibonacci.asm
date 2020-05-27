mov a, 1
mov b, 0

fib:
    mov c, 0
    add c, a
    add c, b
    prnt c
    mov a, b
    mov b, c

cmp c 100
jl fib

end
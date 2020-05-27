inc  c
call foo
inc  d
end

foo:
    inc  a
    call bar
    inc  a
    ret

bar:
    inc  b
    ret
mov  a, 0
call recursion
end

recursion:
    cmp  a, 10
    je   done
    inc  a
    prnt a    
    call recursion

done:
    ret
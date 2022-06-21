    lw 0 1 one      $1 = 1
    lw 0 2 two      $2 = 2
    lw 0 3 three    $3 = 3
    lw 0 4 four     $4 = 4
loop beq 1 2 done   
    noop           
    noop            
    noop            
    add 1 4 1       $1 = $1 + $4 = 1 + 4 = 5
    add 2 3 2       $2 = $2 + $3 = 2 + 3 = 5
    beq 0 0 loop    
    noop           
    noop            
    noop            
done halt
one .fill 1
two .fill 2
three .fill 3
four .fill 4
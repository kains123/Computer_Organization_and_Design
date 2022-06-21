    noop
    lw 0 1 one      $1 = 1
    add 1 1 2       $2 = $1 + $1 = 2 
    nor 1 2 2       $2 = $1 nor $2 = 1 nor 2 = -4
    add 2 2 2       $2 = $2 + $2 = -8
    halt
one .fill 1
two .fill 2
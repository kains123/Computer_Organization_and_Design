        lw      0       1       input   
        lw      0       2       pos1    
        lw      2       5       pos1    
        nor     1       0       3       
        nor     1       1       4       
        ad     1       3       3       
        add     2       4       4       
        sw      5       3       0       
        noop
        halt
input   .fill   20
pos1    .fill   1
mempos  .fill   none1
none1   .fill   0
none2   .fill   0

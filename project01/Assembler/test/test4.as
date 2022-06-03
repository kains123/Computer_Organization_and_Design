		lw		0	1	one
		lw		1	2	one
                lw		2	6	one !err! line too long
		lw		1	3	four
		halt
one		.fill	1
four	.fill	4
nine	.fill	9
	lw		1	0	a 	# More complex LW, ADD, and SW test with hazards
	lw		2	0	b
	add		3	1	2
	sw 		3	0	a
	lw		4	0	a
	add		4	4	2
	add		4	4	2
	sw 		4	0	a
	halt	
a	.fill	4	
b	.fill	2
	lw		1	0	a 	# Simple LW, ADD, NAND, and SW test with hazards
	lw		2	0	b
	add		3	1	2
	add		3	3	2
	add		3	3	2
	nand	3	1	2
	nand	3	3	3	
	sw 		3	0	a
	halt	
a	.fill	4	
b	.fill	2
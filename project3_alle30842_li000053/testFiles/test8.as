	lw		1	0	a 	# spicificly test lw, add, lw following each other with hazards
	add		3	1	1
	lw		2	0	b
	add		3	3	2
	sw 		3	0	a
	halt	
a	.fill	4	
b	.fill	2
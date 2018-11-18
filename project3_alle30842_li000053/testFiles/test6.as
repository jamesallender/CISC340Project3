	lw		1	0	a 	# load reg 1 with 1 # Complex LW, SW test with hazards
	add		2	0	0	# set reg 2 to 0
	add		2	2	1	# increment reg 2
	sw 		2	0	a 	# store reg 2
	lw		3	0	a 	# load back into reg 2
	add		3	3	1	# increment reg 2
	sw 		3	0	a 	# store reg 2
	lw		2	0	a 	# load back into reg 2
	add		2	2	1	# increment reg 2
	sw 		2	0	a 	# store reg 2
	lw		3	0	a 	# load back into reg 2
	add		3	3	1	# increment reg 2
	sw 		3	0	a 	# store reg 2
	lw		2	0	a 	# load back into reg 2
	add		2	2	1	# increment reg 2
	sw 		2	0	a 	# store reg 2	-	Should be 5
	halt	
a	.fill	4	
b	.fill	2
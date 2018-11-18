	lw		1	0	a	# Simple test to ensure program operates correctly with no hazards for ADD, NAND, LW, SW, HALT, and NOOP
	noop
	noop
	noop
	lw		2	0	b
	noop
	noop
	noop
	add		3	1	2
	noop
	noop
	noop
	add		3	3	2
	noop
	noop
	noop
	add		3	3	2
	noop
	noop
	noop
	nand	3	1	2
	noop
	noop
	noop
	nand	3	3	3	
	noop
	noop
	noop
	sw		1	0	b	# ovewrite b with contents of reg 1
	noop
	noop
	noop
	halt	
a	.fill	4	
b	.fill	2
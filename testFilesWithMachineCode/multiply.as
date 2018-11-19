	lw	1	0	mask	# load our inital bitmask int reg 1
	lw	2	0	mcand	# load Multiplicand into reg 2
	lw	3	0	mplier	# load Multiplier into reg 3
	lw	5	0	loops	# load 32 into loop register to check what loop were on
	lw	6	0	neg1	# load -1 into neg1R for loop decramenting
loop	nand	4	3	1	# nand the mask with the mplier and put in 4
	nand	4	4	4	# nand 4 with 4 to get and mask and mplier
	add	5	6	5	# decrement loopR by 1
	beq	0	4	skip	# if 4 is 0 skip down to skip
	add	7	2	7	# add mcand to prodR
skip	add	2	2	2	# add multiplicand to istself shifting it left
	add	1	1	1	# add the mask to itself to shift it left
	beq	0	5	done	# of loopR is 0 branch to done
	beq	0	0	loop	# jump to loop label
done	halt
mask	.fill	1	# fill 1 for the inital value for the mask
mcand	.fill	29562	# the value of the multiplicad
mplier	.fill	11834	# the value of the Multiplier
prodR	.fill	7	# the register of the product
loops	.fill	32	# 32 to be initall put in reg 5 (loopR)
loopR	.fill	5	# the register holding the loop num decramenting from 32
neg1	.fill 	-1	# -1 to be put in reg 6 (neg1R)
		lw		1	0	a		# load 1 into reg 1 to be used for counting - # Test simple BEQ loop with no hazards 
		noop
		noop
		noop
		lw		2	0	b		# load 5 into reg 2 as the loop mac
		noop
		noop
		noop
		add		3	0	0		# put 0 in reg 3 to initalize the the counter
		noop
		noop
		noop
loop	add		3	3	1		# i++
		noop
		noop
		noop
		beq 	3	2	end		# end loop if reg 2 = reg 3
		noop
		noop
		noop
		beq 	0	0	loop	# end loop if reg 2 = reg 3
		noop
		noop
		noop
end		halt	
a		.fill	1	
b		.fill	5
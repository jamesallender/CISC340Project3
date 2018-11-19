		lw		1	0	a		# load 1 into reg 1 to be used for counting - # Test BEQ loop with hazards 
		lw		2	0	b		# load 5 into reg 2 as the loop mac
		add		3	0	0		# put 0 in reg 3 to initalize the the counter
loop	add		3	3	1		# i++
		beq 	3	2	end		# end loop if reg 2 = reg 3
		beq 	0	0	loop	# end loop if reg 2 = reg 3
end		halt	
a		.fill	1	
b		.fill	5
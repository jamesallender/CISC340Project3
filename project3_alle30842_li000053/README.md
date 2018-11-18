James Allender
Vincent Li
CISC-340 
Project 03
README

pipelineSimulator.c - This file is the C source code for our pipelined simulator

Makefile - This is a makefile to be used with Make to build our project

Test Files:
	test1.as: Simple test to ensure program operates correctly with no hazards for ADD, NAND, LW, SW, HALT, and NOOP
	test1.mc: machine code for test1.as
	test2.as: Test simple BEQ loop with no hazards 
	test2.mc: machine code for test2.as
	test3.as: Simple load and halt test provided by the Professor
	test3.mc: machine code for test3.as
	test4.as: Simple LW, ADD, and SW test with hazards
	test4.mc: machine code for test4.as
	test5.as: More complex LW, ADD, and SW test with hazards
	test5.mc: machine code for test5.as
	test6.as: load reg 1 with 1 # Complex LW, SW test with hazards
	test6.mc: machine code for test6.as
	test7.as: Test simple BEQ loop with hazards 
	test7.mc: machine code for test7.as
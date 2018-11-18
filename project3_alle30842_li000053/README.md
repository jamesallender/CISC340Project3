James Allender
Vincent Li
CISC-340 
Project 03
README

pipelineSimulator.c - This file is the C source code for our pipelined simulator
	Compile using the comand $make to build our project using the included make file
	Run with syntax $pipelineSimulator -i "input file path"

Makefile - This is a makefile to be used with Make to build our project
	Run comand [$make] to create our project
	Run commad [$make clean] to clean the project for rebuilding

Project3Overview.pdf - overview document describing project 3

README.me - This README file

Test Files (./testFiles/):
	test1.as: Simple test to ensure program operates correctly with no hazards for ADD, NAND, LW, SW, HALT, and NOOP
	test2.as: Test simple BEQ loop with no hazards 
	test3.as: Simple load and halt test provided by the Professor
	test4.as: Simple LW, ADD, and SW test with hazards
	test5.as: More complex LW, ADD, and SW test with hazards
	test6.as: load reg 1 with 1 # Complex LW, SW test with hazards
	test7.as: Test simple BEQ loop with hazards 
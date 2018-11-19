#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR – not implemented in this project */
#define HALT 6
#define NOOP 7
#define NOOPINSTRUCTION 0x1c00000

typedef struct forwardstruct{
	int instr;
	int instr_change;
	int regAFlag;
	int regBFlag;
	int regAnewData;
	int regBnewData;
} ForwardUnit;

typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{
	int instr;
	int writedata;
} WBENDType;

typedef struct statestruct{
	int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; /* Number of cycles run so far */
	int fetched; /* Total number of instructions fetched */
	int retired; /* Total number of completed instructions */
	int branches; /* Total number of branches executed */
	int mispreds; /* Number of branch mispredictions*/
} statetype;

void printstate(statetype *stateptr);
void printinstruction(int instr);
int signExtend(int num);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);

int field0(int instruction){
	return( (instruction>>19) & 0x7);
}
int field1(int instruction){
	return( (instruction>>16) & 0x7);
}
int field2(int instruction){
	return(instruction & 0xFFFF);
}
int opcode(int instruction){
	return(instruction>>22);
}

int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}

void printinstruction(int instr) {
	char opcodestring[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodestring, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodestring, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodestring, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodestring, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodestring, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodestring, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodestring, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodestring, "noop");
	} else {
		strcpy(opcodestring, "data");
	}
	if(opcode(instr) == ADD || opcode(instr) == NAND){
		printf("%s %d %d %d\n", opcodestring, field2(instr), field0(instr), field1(instr));
	}else if(0 == strcmp(opcodestring, "data")){
		printf("%s %d\n", opcodestring, signExtend(field2(instr)));
	}else{
		printf("%s %d %d %d\n", opcodestring, field0(instr), field1(instr),
		signExtend(field2(instr)));
	}
}

void printstate(statetype *stateptr){
	int i;
	printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
	printf("\tpc %d\n", stateptr->pc);

	printf("\tdata memory:\n");
		for (i=0; i<stateptr->numMemory; i++) {
			printf("\t\tdatamem[ %d ] %d\n", i, stateptr->datamem[i]);
		}

	printf("\tregisters:\n");
		for (i=0; i<NUMREGS; i++) {
			printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
		}

	printf("\tIFID:\n");
		printf("\t\tinstruction ");
		printinstruction(stateptr->IFID.instr);
		printf("\t\tpcplus1 %d\n", stateptr->IFID.pcplus1);
	printf("\tIDEX:\n");
		printf("\t\tinstruction ");
		printinstruction(stateptr->IDEX.instr);
		printf("\t\tpcplus1 %d\n", stateptr->IDEX.pcplus1);
		printf("\t\treadregA %d\n", stateptr->IDEX.readregA);
		printf("\t\treadregB %d\n", stateptr->IDEX.readregB);
		printf("\t\toffset %d\n", stateptr->IDEX.offset);
	printf("\tEXMEM:\n");
		printf("\t\tinstruction ");
		printinstruction(stateptr->EXMEM.instr);
		printf("\t\tbranchtarget %d\n", stateptr->EXMEM.branchtarget);
		printf("\t\taluresult %d\n", stateptr->EXMEM.aluresult);
		printf("\t\treadreg %d\n", stateptr->EXMEM.readreg);
	printf("\tMEMWB:\n");
		printf("\t\tinstruction ");
		printinstruction(stateptr->MEMWB.instr);
		printf("\t\twritedata %d\n", stateptr->MEMWB.writedata);
	printf("\tWBEND:\n");
		printf("\t\tinstruction ");
		printinstruction(stateptr->WBEND.instr);
		printf("\t\twritedata %d\n", stateptr->WBEND.writedata);
}

//regDetecting, the register index we want to check
// if there is a data hazard in the specific buffer

//bufferIndex:
//      0 -> IFID
//      1 -> IDEX
//      2 -> EXMEM
//      3 -> MEMWB
//      4 -> WBEND

//return:
//		0 -> no hazard
// 		1 -> r type hazard
// 		2 -> lw hazard
int hasDataHazard(statetype state, int bufferIndex, int regDetecting){
    int instruction;

    if(bufferIndex == 2){
        //only doing forwarding in EXMEM, MEMWB & WBEND
        instruction = state.EXMEM.instr;
    }else if(bufferIndex == 3){
        instruction = state.MEMWB.instr;
    }else if(bufferIndex == 4){
        instruction = state.WBEND.instr;
    }else{
        //dont care otherwise
        return 0;
    }

    int operation = opcode(instruction);
    int regcauseHazard;
    int returnHazardType = 0;

    //only ADD, NAND and LW causing the data hazard
    if(operation == ADD || operation == NAND){
        regcauseHazard = field2(instruction);

        if(regcauseHazard == regDetecting){
        	return 1;
        }else{
        	return 0;
        }
    }else if(operation == LW){
        regcauseHazard = field0(instruction);

        if(regcauseHazard  == regDetecting){
        	return 2;
        }else{
        	return 0;
        }
    }else{
        return 0;
    }

}

ForwardUnit forwarding(statetype *state, statetype *newstate){
	int regA_forworded = 0;
	int regB_forworded = 0;

	int instr = state->IDEX.instr;
	int regA = field0(instr);
	int regB = field1(instr);

	ForwardUnit unit;
	unit.instr = instr;
	unit.instr_change = 0;
	unit.regAFlag = 0;
	unit.regBFlag = 0;

	unit.regAnewData = 0;
	unit.regBnewData = 0;

	/////////////////////////EXMEM////////////////////////////////
	int regA_hazard_EXMEM = hasDataHazard(*state, 2, regA);
	int regB_hazard_EXMEM = hasDataHazard(*state, 2, regB);

	//lw data hazard in either regA or regB
	if(regA_hazard_EXMEM == 2 || regB_hazard_EXMEM == 2){
		unit.instr_change = 1;
		unit.instr = NOOPINSTRUCTION;
		return unit;
	}

	//r type forwarding
	if(regA_hazard_EXMEM == 1){
		regA_forworded = 1;
		unit.regAFlag = 1;
		unit.regAnewData = state->EXMEM.aluresult;
	}

	if(regB_hazard_EXMEM == 1){
		regB_forworded = 1;
		unit.regBFlag = 1;
		unit.regBnewData = state->EXMEM.aluresult;
	}

	if(regA_forworded == 1 && regB_forworded == 1){
		return unit;
	}

	/////////////////////////////////////////////////////////////


	/////////////////////////MEMWB////////////////////////////////

	int regA_hazard_MEMWB = hasDataHazard(*state, 3, regA);
	int regB_hazard_MEMWB = hasDataHazard(*state, 3, regB);

	//if there is hazard in MEMWB buffer, forwarding data for both r type and lw
	if(regA_hazard_MEMWB != 0 && regA_forworded == 0){
		regA_forworded = 1;
		unit.regAFlag = 1;
		unit.regAnewData = state->MEMWB.writedata;
	}

	if(regB_hazard_MEMWB != 0 && regB_forworded == 0){
		regB_forworded = 1;
		unit.regBFlag = 1;
		unit.regBnewData = state->MEMWB.writedata;
	}

	if(regA_forworded == 1 && regB_forworded == 1){
		return unit;
	}
	/////////////////////////////////////////////////////////////


	/////////////////////////WBEND////////////////////////////////

	int regA_hazard_WBEND = hasDataHazard(*state, 4, regA);
	int regB_hazard_WBEND = hasDataHazard(*state, 4, regB);

	//if there is hazard in WBEND buffer, forwarding data for both r type and lw
	if(regA_hazard_WBEND != 0 && regA_forworded == 0){
		regA_forworded = 1;
		unit.regAFlag = 1;
		unit.regAnewData = state->WBEND.writedata;
	}

	if(regB_hazard_WBEND != 0 && regB_forworded == 0){
		regB_forworded = 1;
		unit.regBFlag = 1;
		unit.regBnewData = state->WBEND.writedata;
	}

	if(regA_forworded == 1 && regB_forworded == 1){
		return unit;
	}
	/////////////////////////////////////////////////////////////

	return unit;
}


void fetchStage(statetype *state, statetype *newstate){
	int instruction = state->instrmem[state->pc];

	newstate->pc = state->pc + 1;
	newstate->fetched = state->fetched + 1;

	//set instruction in IFID buffer in newstate
	//fetching the new instruction
	newstate->IFID.instr = instruction;

	//set pcplu1 in IFID buffer in newstate
	newstate->IFID.pcplus1 = state->pc + 1;
}

void decodeStage(statetype *state, statetype *newstate){

	newstate->IDEX.pcplus1 = state->IFID.pcplus1;
	newstate->IDEX.instr = state->IFID.instr;

	newstate->IDEX.readregA = state->reg[field0(state->IFID.instr)];

	newstate->IDEX.readregB = state->reg[field1(state->IFID.instr)];

	newstate->IDEX.offset = signExtend(field2(state->IFID.instr));
}

void executeStage(statetype *state, statetype *newstate){

	int instr;
	int regA_data = state->IDEX.readregA;
	int regB_data = state->IDEX.readregB;
	int offset = state->IDEX.offset;

	ForwardUnit unit = forwarding(state, newstate);

	printf("unit.instr: \n");
	printinstruction(unit.instr);
	printf("unit.instr_change: %d \n", unit.instr_change);
	printf("unit.regAFlag: %d\n", unit.regAFlag);
	printf("unit.regAnewData: %d\n", unit.regAnewData);
	printf("unit.regBFlag: %d\n", unit.regBFlag);
	printf("unit.regBnewData: %d\n", unit.regBnewData);


	instr = unit.instr;

	if(unit.instr_change == 1){
		newstate->retired = state->retired - 1;

		//set pc in newstate

		newstate->pc = state->pc;
		//set fetched num in newstate
		newstate->fetched = state->fetched;

		//pause IFID and IDEX
		newstate->IFID = state->IFID;
		newstate->IDEX = state->IDEX;

		regA_data = 0;
		regB_data = 0;
		offset = 0;
	}

	if(unit.regAFlag == 1){
		if(opcode(instr) != LW) {
			regA_data = unit.regAnewData;
		}
	}

	if(unit.regBFlag == 1){
		regB_data = unit.regBnewData;
	}

	//set for newstate
	newstate->EXMEM.instr = instr;

	newstate->EXMEM.branchtarget = offset + state->IDEX.pcplus1;

	int operation = opcode(instr);
	if(operation == ADD){
		newstate->EXMEM.aluresult = regA_data + regB_data;
	}else if(operation == NAND){
		newstate->EXMEM.aluresult = ~(regA_data & regB_data);
	}else if(operation == LW || operation == SW){
		newstate->EXMEM.aluresult = regB_data + offset;
	}else if(operation == BEQ){
		newstate->EXMEM.aluresult = regA_data - regB_data;
	}else if(operation == NOOP){
		newstate->EXMEM.aluresult = 0;
	}else if(operation == JALR || operation == HALT){
		//do nothing 
	}else{
		fprintf(stderr,"%s %d\n" ,"FUNCTION: executeStage. REASON: Failed to get opcode from the instruction. INSTR: ", opcode(state->IDEX.instr));
		exit(0);
	}

	//set readreg in EXMEM buffer in newstate
	newstate->EXMEM.readreg = regA_data;
}

void memoryStage(statetype *state, statetype *newstate){
	/*
	typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;
	*/

	// writeData defult value is 0
	int writeData = 0;

	// Get our instruction from the previous EXMEM buffer
	int instr = state->EXMEM.instr;
	// set the new state MEMWB buffer to the current instruction
	newstate->MEMWB.instr = instr;
	// get the alu result
	int aluresult = state->EXMEM.aluresult;

	// Get operation
	int operation = opcode(instr);

	// If instruction is a NOOP
	if(operation ==  NOOP || operation == HALT){
		writeData = 0;
	}
	// If instruction is a BEQ
	else if(operation ==  BEQ){
		newstate->branches = state->branches + 1;
		// If the branch is to be taken
		if (aluresult == 0){
			// Set the new states pc to the branch target

			newstate->pc = state->EXMEM.branchtarget;
			newstate->mispreds = state->mispreds + 1;

			newstate->IFID.instr = NOOPINSTRUCTION;
			newstate->IFID.pcplus1 = 0;

			newstate->IDEX.instr = NOOPINSTRUCTION;
			newstate->IDEX.pcplus1 = 0;
			newstate->IDEX.readregA = 0;
			newstate->IDEX.readregB = 0;
			newstate->IDEX.offset = 0;

			newstate->EXMEM.instr = NOOPINSTRUCTION;
			newstate->EXMEM.branchtarget = 0;
			newstate->EXMEM.aluresult = 0;
			newstate->EXMEM.readreg = 0;
		}
	}
	// if the instruction is a LW, get the data from memory
	else if (operation == LW) {
		// get the data from memory
		writeData = state->datamem[aluresult];
	}
	// if the instruction is a SW, Write to memory 
	else if (operation == SW) {
		newstate->datamem[aluresult] = state->EXMEM.readreg;
	}
	// for all other functions pass the alu result
	else{
		writeData = aluresult;
	}

	newstate->MEMWB.writedata = writeData;
}

void writeBackStage(statetype *state, statetype *newstate){

	// Get instruction
	int instr = state->MEMWB.instr;
	
	//set instr in WBEND buffer in newstate
	newstate->WBEND.instr = instr;

	//set writedata in WBEND buffer in newstate
	newstate->WBEND.writedata = state->MEMWB.writedata;

	// increse retired
	newstate->retired = state->retired + 1;

	//write back to the register file
	int operation = opcode(instr);
	int regDest;

	if(operation == ADD || operation == NAND){
		int regDest = field2(instr);
		//write back here
		newstate->reg[regDest] = state->MEMWB.writedata;
	}else if(operation == LW){
		int regDest = field0(instr);
		newstate->reg[regDest] = state->MEMWB.writedata;
	}
}

int main(int argc, char** argv){

	/** Get command line arguments **/
	char* fname;

	opterr = 0;

	int cin = 0;

	while((cin = getopt(argc, argv, "i:")) != -1){
		switch(cin)
		{
			case 'i':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				printf("FILE: %s\n", fname);
				break;
			case '?':
				if(optopt == 'i'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option '-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character '\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				abort();
		}
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

		/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
    while (EOF != (c=getc(fp))) {
        if ( c == '\n' ){
            line_count++;
		}
    }
	// reset fp to the beginning of the file
	rewind(fp);

	// INITALIZATION
	// Make sure to initialize all values correctly:
	// a. state.numMemory should be set to the number of memory words in the machine-code file.
	// b. state.cycles/fetched/retired/branches/mispreds should be initialized to 0.
	// c. pc and all registers should be initialized to 0.
	// d. the instruction field in all pipeline registers should be set to the noop instruction (0x1c00000).
	statetype state;
	statetype newstate;

	state.numMemory = line_count;
	state.cycles = 0;
	state.fetched = 0;
	state.retired = 0;
	state.branches = 0;
	state.mispreds = 0; // because we predict branch not taken will mis predictions and branches be the same
	state.pc = 0;
	// Set registers to 0
	for(int i = 0; i < NUMREGS; i++){
		state.reg[i] = 0;
	}

	// Populate instruction memory
	char line[256];
	
	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
        /* note that fgets doesn't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
		state.instrmem[i] = atoi(line);
		state.datamem[i] = atoi(line);
		
		i++;
    }
    fclose(fp);

	// Set inital insturctions in pipeline
	state.IFID.instr = NOOPINSTRUCTION;
	state.IDEX.instr = NOOPINSTRUCTION;
	state.EXMEM.instr = NOOPINSTRUCTION;
	state.MEMWB.instr = NOOPINSTRUCTION;
	state.WBEND.instr = NOOPINSTRUCTION;

	while(1){

		printstate(&state);
		/* check for halt */
		if(HALT == opcode(state.MEMWB.instr)) {
			printf("machine halted\n");
			printf("total of %d cycles executed\n", state.cycles);
			// -3 to account for 'instructions' fetched befor HALT hit
			printf("total of %d instructions fetched\n", (state.fetched - 3));
			// retired - the number of bubles inserted - the number of branch mispradictions * 2 for the 2 noop's loded per misprediciton
			// -3 to acount for 3 stages of noting
			printf("total of %d instructions retired\n", (state.retired - (state.mispreds * 3) - 3));
			printf("total of %d branches executed\n", state.branches);
			printf("total of %d branch mispredictions\n", state.mispreds);
			exit(0);
		}
		newstate = state;

		newstate.cycles = state.cycles + 1;


		/*------------------ IF stage ----------------- */

		fetchStage(&state, &newstate);

		/*------------------ ID stage ----------------- */

		decodeStage(&state, &newstate);

		/*------------------ EX stage ----------------- */

		executeStage(&state, &newstate);
		//calling forwardingUnit in execusion

		/*------------------ MEM stage ----------------- */

		memoryStage(&state, &newstate);

		/*------------------ WB stage ----------------- */
		writeBackStage(&state, &newstate);


		state = newstate; /* this is the last statement before the end of the loop.
							It marks the end of the cycle and updates the current
							state with the values calculated in this cycle
							– AKA "Clock Tick". */
	}
}
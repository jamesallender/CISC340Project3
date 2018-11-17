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

int bubbleInsertions = 0;

void printstate(statetype *stateptr);
void printinstruction(int instr);
int signExtend(int num);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
int isInstruction(int instruction);

// Check if a value is an instruction
int isInstruction(int instruction){
	printf("instruction: %d\n", instruction);
	int returnVal = 1;

	// This will not work for an add 0 0 0s
	if (instruction == 0 || (opcode(instruction) != ADD && opcode(instruction) != NAND && opcode(instruction) != LW && opcode(instruction) != SW && 
		opcode(instruction) != BEQ && opcode(instruction) != JALR && opcode(instruction) != HALT && opcode(instruction) != NOOP)){
		returnVal = 0;

	}
	printf("returnVal: %d\n", returnVal);
	return returnVal;
}


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
    //only ADD, NAND and LW causing the data hazard
    //otherwise return 0, meaning no hazard

    if(operation == ADD || operation == NAND){
        regcauseHazard = field2(instruction);
    }else if(operation == LW){
        regcauseHazard = field0(instruction);
    }else{
        return 0;
    }

    if(regcauseHazard == regDetecting){
        return 1;
    }else{
        return 0;
    }
}

void forwardingProcess(int hazard_instr, int reg_update, statetype *newstate){

	int hazard_opcode = opcode(hazard_instr);
	int reg_cause_hazard; 

	if(hazard_opcode == ADD || hazard_opcode == NAND){
		reg_cause_hazard = field2(hazard_instr);			
			
		if(reg_update == reg_cause_hazard){
			newstate->IDEX.readregA = newstate->EXMEM.aluresult;
		}

	}else if(hazard_opcode == LW){
		reg_cause_hazard = field0(hazard_instr);
	}else{
		//the rest opcode will not cause hazard
		//run into some strange case
		fprintf(stderr, "%s ***", "in forwarding unit, read a instruction that does not have ADD or NAND or LW\n");
		printinstruction(hazard_instr);
	}
}

void forwardingUnit(statetype *state, statetype *newstate){
	//only use in the execution stage

	//opcodes might cause data hazard
	//ADD and NAND will overwrite value in destination with alu result
	//LW will overwrite value in destination with data read from DataMem

	//opcodes might take in data hazard
	//ADD & NAND & BEQ & SW will all need to know the changes in RegA & RegB
	//LW need to know the changes only in RegB(Destination register)
	//halt & noop will not care

	//Buffer that might contain hazard
	// EXMEM & MEMWB & WBEND

	int instr = state->IDEX.instr;

	int operation = opcode(instr);
	int regA = field0(instr);
	int regB = field1(instr);	


	if(operation != ADD || operation != NAND || operation != LW || operation != SW || operation != BEQ){
		// if the opcode is JALR or NOOP or HALT, do nothing
		return;
	}

//bufferIndex:
//      0 -> IFID
//      1 -> IDEX
//      2 -> EXMEM
//      3 -> MEMWB
//      4 -> WBEND

	int regA_forwarded = 0;
	int regB_forwarded = 0;

	int regA_hazard_EXMEM = hasDataHazard(*state, 2, regA);
	int regA_hazard_MEMWB = hasDataHazard(*state, 3, regA);
	int regA_hazard_WBEND = hasDataHazard(*state, 4, regA);

	int regB_hazard_EXMEM = hasDataHazard(*state, 2, regB);
    int regB_hazard_MEMWB = hasDataHazard(*state, 3, regB);
    int regB_hazard_WBEND = hasDataHazard(*state, 4, regB);

	int regA_hazard = regA_hazard_EXMEM || regA_hazard_MEMWB || regA_hazard_WBEND;
	int regB_hazard = regB_hazard_EXMEM || regB_hazard_MEMWB || regB_hazard_WBEND;

	if(regA_hazard == 0 && regA_hazard == 0){
		//no hazard at all
		return;
	}
	
	if(regA_hazard_EXMEM == 1){
		regA_forwarded = 1;
		
		int hazard_instr = state->EXMEM.instr;
		forwardingProcess(hazard_instr, regA, newstate);
	}

	if(regA_hazard_MEMWB == 1 && regA_forwarded == 0){
		regA_forwarded = 1;
			
		int hazard_instr = state->MEMWB.instr;
		forwardingProcess(hazard_instr, regA, newstate);
	}

	if(regA_hazard_WBEND == 1 && regA_forwarded == 0){
		regA_forwarded = 1;

        int hazard_instr = state->WBEND.instr;
        forwardingProcess(hazard_instr, regA, newstate);
	}


	
	//regB
	if(regB_hazard_EXMEM == 1){
        regB_forwarded = 1;

        int hazard_instr = state->EXMEM.instr;
        forwardingProcess(hazard_instr, regB, newstate);
    }

    if(regB_hazard_MEMWB == 1 && regB_forwarded == 0){
        regB_forwarded = 1;

        int hazard_instr = state->MEMWB.instr;
        forwardingProcess(hazard_instr, regB, newstate);
    }

    if(regB_hazard_WBEND == 1 && regB_forwarded == 0){
        regB_forwarded = 1;

        int hazard_instr = state->WBEND.instr;
        forwardingProcess(hazard_instr, regB, newstate);
    }
}

void fetchStage(statetype *state, statetype *newstate){
	int instruction = state->instrmem[state->pc];

	//set pc in newstate
	newstate->pc = state->pc + 1;

	//set fetched num in newstate
	if (isInstruction(instruction) == 1){
		newstate->fetched = state->fetched + 1;
	}	

	
	//set instruction in IFID buffer in newstate
	//fetching the new instruction
	newstate->IFID.instr = instruction;
	
	//set pcplu1 in IFID buffer in newstate
	newstate->IFID.pcplus1 = state->pc + 1;
}

void decodeStage(statetype *state, statetype *newstate){
	// typedef struct IDEXstruct{
	// 	int instr;
	// 	int pcplus1;
	// 	int readregA;
	// 	int readregB;
	// 	int offset;
	// } IDEXType;

	newstate->IDEX.pcplus1 = state->IFID.pcplus1;
	newstate->IDEX.instr = state->IFID.instr;

	newstate->IDEX.readregA = field0(state->IFID.instr);
	newstate->IDEX.readregB = field1(state->IFID.instr);
	newstate->IDEX.offset = signExtend(field2(state->IFID.instr));
}

void executeStage(statetype *state, statetype *newstate){
	//set instr in EXMEM buffer in newstate
	newstate->EXMEM.instr = state->IDEX.instr;
    
	//set branch target address in EXMEM buffer in newstate
	newstate->EXMEM.branchtarget = state->IDEX.pcplus1 + state->IDEX.offset;


	//set ALU result in EXMEM buffer in newstate
    int operation = opcode(state->IDEX.instr);

    if(operation == ADD){
        newstate->EXMEM.aluresult = state->IDEX.readregA + state->IDEX.readregB;
    }else if(operation == NAND){
        newstate->EXMEM.aluresult = ~(state->IDEX.readregA & state->IDEX.readregB);
    }else if(operation == LW || operation == SW){
		newstate->EXMEM.aluresult = state->IDEX.readregB + state->IDEX.offset;
    }else if(operation == BEQ){
		newstate->EXMEM.aluresult = state->IDEX.readregA - state->IDEX.readregB;
    }else if(operation == NOOP){
		newstate->EXMEM.aluresult = -1;
    }else{
		fprintf(stderr,"%s %d\n" ,"FUNCTION: executeStage. REASON: Failed to get opcode from the instruction. INSTR: ", state->IDEX.instr);   
	 }


	//set readreg in EXMEM buffer in newstate
	newstate->EXMEM.readreg = state->IDEX.readregA;
}

void memoryStage(statetype *state, statetype *newstate){
	// typedef struct EXMEMstruct{
	// 	int instr;
	// 	int branchtarget;
	// 	int aluresult;
	// 	int readreg;
	// } EXMEMType;

	// typedef struct MEMWBstruct{
	// 	int instr;
	// 	int writedata;
	// } MEMWBType;

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
	if(operation ==  NOOP){
		writeData = 0;
	}
	// If instruction is a BEQ
	else if(operation ==  BEQ){
		// If the branch is to be taken
		if (aluresult == 0){
			// Set the new states pc to the branch target
			newstate->pc = state->EXMEM.branchtarget;
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
	}else if(operation == LW){
		int regDest = field0(instr);
	}else{
		// In this case, we dont need to write back to the register file.
		// SW, BEQ, NOOP, and HALT belong to this case.
		return;
	}

	//write back here
	newstate->reg[regDest] = state->MEMWB.writedata;
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
			printf("total of %d instructions fetched\n", state.fetched);
			printf("total of %d instructions retired\n", (state.retired - bubbleInsertions - (state.mispreds * 2)));
			printf("total of %d branches executed\n", state.branches);
			printf("total of %d branch mispredictions\n", state.mispreds);
			exit(0);
		}
		newstate = state;

		newstate.cycles = state.cycles + 1;


		// typedef struct IFIDstruct{
		// 	int instr;
		// 	int pcplus1;
		// } IFIDType;

		// typedef struct IDEXstruct{
		// 	int instr;
		// 	int pcplus1;
		// 	int readregA;
		// 	int readregB;
		// 	int offset;
		// } IDEXType;

		// typedef struct EXMEMstruct{
		// 	int instr;
		// 	int branchtarget;
		// 	int aluresult;
		// 	int readreg;
		// } EXMEMType;

		// typedef struct MEMWBstruct{
		// 	int instr;
		// 	int writedata;
		// } MEMWBType;

		// typedef struct WBENDstruct{
		// 	int instr;
		// 	int writedata;
		// } WBENDType;


		/*------------------ IF stage ----------------- */

		fetchStage(&state, &newstate);

		/*------------------ ID stage ----------------- */

		decodeStage(&state, &newstate);

		/*------------------ EX stage ----------------- */

		executeStage(&state, &newstate);

		/*------------------ MEM stage ----------------- */

		memoryStage(&state, &newstate);

		/*------------------ WB stage ----------------- */
		writeBackStage(&state, &newstate);

		//calling forwardingUnit right here
		//so that we can passing the newest data (hazard) before next execute stage

		state = newstate; /* this is the last statement before the end of the loop.
							It marks the end of the cycle and updates the current
							state with the values calculated in this cycle
							– AKA "Clock Tick". */
	}
}

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

int main(int argc, char** argv){
	while(1){
		printstate(&state);
		/* check for halt */
		if(HALT == opcode(state.MEMWB.instr)) {
			printf(“machine halted\n”);
			printf(“total of %d cycles executed\n”, state.cycles);
			printf("total of %d instructions fetched\n", state.fetched);
			printf(“total of %d instructions retired\n”, state.retired);
			printf("total of %d branches executed\n", state.branches);
			printf("total of %d branch mispredictions\n", state.mispreds);
			exit(0);
		}
		newstate = state;
		newstate.cycles++;
		/*------------------ IF stage ----------------- */



		/*------------------ ID stage ----------------- */



		/*------------------ EX stage ----------------- */



		/*------------------ MEM stage ----------------- */



		/*------------------ WB stage ----------------- */



		state = newstate; /* this is the last statement before the end of the loop.
							It marks the end of the cycle and updates the current
							state with the values calculated in this cycle
							– AKA “Clock Tick”. */
	}
}
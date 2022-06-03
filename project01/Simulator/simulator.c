/* LC-2K Instruction-level simulator */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8       /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct
{
  int pc;
  int mem[NUMMEMORY];
  int reg[NUMREGS];
  int numMemory;
} stateType;

enum OpCode
{
  OP_ADD = 0,
  OP_NOR = 1,
  OP_LW = 2,
  OP_SW = 3,
  OP_BEQ = 4,
  OP_JALR = 5,
  OP_HALT = 6,
  OP_NOOP = 7
};

void printState(stateType *);

/** Simulator Functions */
void parseInst(stateType *statePtr, int *opcode, int *arg0, int *arg1, int *arg2);
void RTypeInst(stateType *statePtr, int opcode, int arg0, int arg1, int dest);
void ITypeInst(stateType *statePtr, int opcode, int arg0, int arg1, int offset);
void JTypeInst(stateType *statePtr, int opcode, int arg0, int arg1);

int main(int argc, char *argv[])
{
  char line[MAXLINELENGTH];
  stateType state;
  FILE *filePtr;
  int executionCount = 0;
  if (argc != 2)
  {
    printf("error: usage: %s <machine-code file>\n", argv[0]);
    exit(1);
  }
  filePtr = fopen(argv[1], "r");
  if (filePtr == NULL)
  {
    printf("error: can't open file %s", argv[1]);
    perror("fopen");
    exit(1);
  }
  /* read in the entire machine-code file into memory */
  for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
       state.numMemory++)
  {
    if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
    {
      printf("error in reading address %d\n", state.numMemory);
      exit(1);
    }
    printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
  }

  // Print initial state
  printState(&state);
  while (1)
  {
    int isHalt = 0;
    int opcode, arg0, arg1, arg2;
    parseInst(&state, &opcode, &arg0, &arg1, &arg2);

    state.pc++;
    executionCount++;

    if (state.pc < 0 || state.pc >= NUMMEMORY)
    {
      printf("PC out of memory");
      exit(1);
    }

    switch (opcode)
    {
    case OP_ADD:
    case OP_NOR:
      RTypeInst(&state, opcode, arg0, arg1, arg2);
      break;
    case OP_LW:
    case OP_SW:
    case OP_BEQ:
      ITypeInst(&state, opcode, arg0, arg1, arg2);
      break;
    case OP_JALR:
      JTypeInst(&state, opcode, arg0, arg1);
      break;
    case OP_HALT:
      isHalt = 1;
      break;
    case OP_NOOP:
      // just skip
      break;
    default:
      printf("Do not support its opcode.");
      exit(1);
      break;
    }
    if (isHalt)
    {
      break;
    }

    // Print state
    printState(&state);
  }

  printf("machine halted\n");
  printf("total of %d instructions executed\n", executionCount);
  printf("final state of machine:\n");

  printState(&state);

  fclose(filePtr);
  exit(0);
}

void printState(stateType *statePtr)
{
  int i;
  printf("\n@@@\nstate:\n");
  printf("\tpc %d\n", statePtr->pc);
  printf("\tmemory:\n");
  for (i = 0; i < statePtr->numMemory; i++)
  {
    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
  }
  printf("\tregisters:\n");
  for (i = 0; i < NUMREGS; i++)
  {
    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
  }
  printf("end state\n");
}

/*Register Valid Check*/
int isValidReg(int reg)
{
  return (int)(reg >= 0 && reg < NUMREGS);
}


int convertNum(int num)
{
  /* convert a 16-bit number into a 32-bit */
  if (num & (1 << 15))
  {
    num -= (1 << 16);
  }

  return (num);
}


void parseInst(stateType *statePtr, int *opcode, int *arg0, int *arg1, int *arg2)
{
  int memValue = statePtr->mem[statePtr->pc];

  // 25 ~ 22 bit => opcode
  *opcode = (memValue >> 22) & 0b111;
  //21-19 bit  => binary to arg0
  *arg0 = (memValue >> 19) & 0b111;
  //18-16 bit  => binary to arg1
  *arg1 = (memValue >> 16) & 0b111;
  //15-0 bit a =>binary to arg2
  *arg2 = (memValue & 0xFFFF);
}

/**
  OP_ADD, OP_NOR
 */
void RTypeInst(stateType *statePtr, int opcode, int arg0, int arg1, int dest)
{

  if (!isValidReg(arg0) || !isValidReg(arg1) || !isValidReg(dest))
  {
    printf("Register is not valid.");
    exit(1);
  }

  switch (opcode)
  {
  case 0: // add
    statePtr->reg[dest] = statePtr->reg[arg0] + statePtr->reg[arg1];
    break;
  case 1: // nor
    statePtr->reg[dest] = ~(statePtr->reg[arg0] | statePtr->reg[arg1]);
    break;
  default:
    printf("Do not support its opcode");
    exit(1);
    break;
  }
}

/**
 *
 * LW일 때는 (opcode 2)
 *
 * SW일 때는 (opcode 3)
 * Store arg1 into memory.

 *
 * BEQ일 때는 (opcode 4)
 *
 * Format
 * bits 24-22: opcode
 * bits 21-19: reg A
 * bits 18-16: reg B
 * bits 15-0: offsetField (a 16-bit, 2's complement number with a range of -32768 to 32767)
 */
void ITypeInst(stateType *statePtr, int opcode, int arg0, int arg1, int offset)
{
  offset = convertNum(offset);

  if (!isValidReg(arg0) || !isValidReg(arg1))
  {
    printf("Register is not valid.");
    exit(1);
  }

  if (offset > 32767 || offset < -32768)
  {
    printf("Offset out of range");
    exit(1);
  }

  switch (opcode)
  {
  case 2:
    statePtr->reg[arg1] = statePtr->mem[statePtr->reg[arg0] + offset];
    break;
  case 3:
    statePtr->mem[statePtr->reg[arg0] + offset] = statePtr->reg[arg1];
    break;
  case 4:
    if (statePtr->reg[arg0] == statePtr->reg[arg1])
    {
      statePtr->pc += offset;
    }
    break;
  default:
    printf("Do not support its opcode");
    exit(1);
    break;
  }
}

/* JALR*/
void JTypeInst(stateType *statePtr, int opcode, int arg0, int arg1)
{
  if (!isValidReg(arg0) || !isValidReg(arg1))
  {
    printf("Register is not valid.");
    exit(1);
  }

  switch (opcode)
  {
  case 5:
    statePtr->reg[arg1] = statePtr->pc;
    statePtr->pc = statePtr->reg[arg0];
    break;
  default:
    printf("Do not support its opcode");
    exit(1);
    break;
  }
}

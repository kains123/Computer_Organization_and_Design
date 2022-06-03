/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INSTRUCTION 1024
#define MAXLINELENGTH 1000
typedef char string_t[MAX_LINE_LENGTH];

/* use this when return error */
enum Error
{
  ERR,
  ERR_UNDEFINED_LABEL,
  ERR_LACK_ARGUMENTS,
  ERR_UNRECOGNIZED_OPCODE,
  ERR_ARGUMENT,
  ERR_OVERFLOW,
};

/* use this when return error */
typedef union
{
  unsigned int code;

  struct
  {
    unsigned int destReg : 3;
    unsigned int unused1 : 13;
    unsigned int regB : 3;
    unsigned int regA : 3;
    unsigned int opcode : 3;
    unsigned int unused0 : 7;
  } r;

  struct
  {
    int offset : 16;
    unsigned int regB : 3;
    unsigned int regA : 3;
    unsigned int opcode : 3;
    unsigned int unused : 7;
  } i;

  struct
  {
    unsigned int unused1 : 16;
    unsigned int regB : 3;
    unsigned int regA : 3;
    unsigned int opcode : 3;
    unsigned int unused0 : 7;
  } j;

  struct
  {
    unsigned int unused1 : 22;
    unsigned int opcode : 3;
    unsigned int unused0 : 7;
  } o;

} inst_t;

struct
{
  struct
  {
    string_t label;
    int addr;
  } labels[MAX_INSTRUCTION];

  int numAddrs;
} labelTable;

/* formatting functions */
int IType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg);
int RType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg);
int JType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg);
int OType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg);

/* new functions */
int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(const char *);

void procFirstPass(FILE *, FILE *);
void procSecondPass(FILE *, FILE *);

int getAddress(char *, int, char[NUMLABELS][LABELSIZE], int label_addy[]);
int isUpper(char *);                                // if the character is uppercase, returns 1
int isDef(char *, int, char[NUMLABELS][LABELSIZE]); // if the label is already defined, returns 1

int main(int argc, char *argv[])
{
  char *inFileString, *outFileString;
  FILE *inFilePtr, *outFilePtr;
  char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
      arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
  if (argc != 3)
  {
    printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
    exit(1);
  }
  inFileString = argv[1];
  outFileString = argv[2];
  inFilePtr = fopen(inFileString, "r");
  if (inFilePtr == NULL)
  {
    printf("error in opening %s\n", inFileString);
    exit(1);
  }
  outFilePtr = fopen(outFileString, "w");
  if (outFilePtr == NULL)
  {
    printf("error in opening %s\n", outFileString);
    exit(1);
  }

  // number of instructions in the file(text, data, symbol, relocation)
  int t = 0;
  int d = 0;
  int s = 0;
  int r = 0;

  // number of defined globals
  int glob = 0;

  // global labels
  char defglobals[NUMLABELS][LABELSIZE]; // defined globals
  char globals[NUMLABELS][LABELSIZE];    // all globals
  char symbol[NUMLABELS][2];             // for all globals, the associated code of "T", "D", or "U"

  int symbol_offset[NUMLABELS]; // the line offset from the start of the T/D sections
  /* here is an example for how to use readAndParse to read a line from inFilePtr */
  if (!readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
  { /* reached end of file */
  }
  /* this is how to rewind the file ptr so that you start reading from the
  beginning of the file */
  rewind(inFilePtr);
  /* after doing a readAndParse, you may want to do the following to test the
  opcode */
  if (!strcmp(opcode, "add"))
  {
/* do whatever you need to do for opcode "add" */ }
return (0);
}

/** * * * * * * * * */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2)
{
  char line[MAXLINELENGTH];
  char *ptr = line;
  /* delete prior values */
  label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0'; /* read the line from the assembly-language file */
  if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
  { /* reached end of file */
    return (0);
  }
  /* check for line too long (by looking for a \n) */ if (strchr(line, '\n') == NULL)
  {
    /* line too long */ printf("error: line too long\n");
    exit(1);
  }
  /* is there a label? */
  ptr = line;
  if (sscanf(ptr, "%[^\t\n\r ]", label))
  {
    /* successfully read label; advance pointer over the label */
    ptr += strlen(label);
  }
  /*
   * Parse the rest of the line. Would be nice to have real regular
   * expressions, but scanf will suffice. */
  sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
         opcode, arg0, arg1, arg2);
  return (1);
}

// return 1 if string is a number.
int isNumber(char *string)
{
  int i;
  return ((sscanf(string, "%d", &i)) == 1);
}

// return 1 if string is a Alphabet.
int isAlphabet(char *string)
{
  return (int)((*string >= 'a' && *string <= 'z') || (*string >= 'A' && *string <= 'Z'));
}

// return -1 if no consistent label.
int checkOffsetRange(const char *string)
{
  int val = atoi(string);

  return (val > 32767) || (val < -32768);
}

// return -1 if no consistent label.
int findLabelAddress(const char *label)
{
  int i;
  for (i = 0; i < labelTable.numAddrs; ++i)
  {
    if (strcmp(label, labelTable.labels[i].label) == 0)
      return labelTable.labels[i].addr;
  }

  return -1;
}

int labelOrImmediate(int labelOffset, const char *arg, enum ErrorCode *err)
{
  int addr;

  if (isNumber(arg))
  {
    return atoi(arg);
  }

  if ((addr = findLabelAddress(arg)) == -1)
  {
    *err = ERR_UNDEFINED_LABEL;
    return -1;
  }

  return addr - labelOffset;
}

int RType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg)
{
  if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
    return ERR_NOT_ENOUGH_ARGUMENTS;

  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_INVALID_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_INVALID_ARGUMENT;
  }

  if (!isNumber(arg2))
  {
    *errArg = 2;
    return ERR_INVALID_ARGUMENT;
  }

  inst->r.opcode = opcode;
  inst->r.regA = atoi(arg0);
  inst->r.regB = atoi(arg1);
  inst->r.destReg = atoi(arg2);

  return ERR_OK;
}

int IType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg)
{

  int mc = 0, address = 0;

  enum ErrorCode err = ERR_OK;

  if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
    return ERR_NOT_ENOUGH_ARGUMENTS;

  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_INVALID_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_INVALID_ARGUMENT;
  }

  if (isNumber(arg2) && checkOffsetRange(arg2))
  {
    *errArg = 2;
    return ERR_ARG_OVERFLOW;
  }

  inst->i.opcode = opcode;
  inst->i.regA = atoi(arg0);
  inst->i.regB = atoi(arg1);
  inst->i.offset = labelOrImmediate((opcode == OP_BEQ ? curAddr + 1 : 0), arg2, &err);

  // maybe error is in arg2
  if (err != ERR_OK)
    *errArg = 2;

  return err;
}

int JType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg)
{

  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_INVALID_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_INVALID_ARGUMENT;
  }

  if (strlen(arg0) == 0 || strlen(arg1) == 0)
    return ERR_NOT_ENOUGH_ARGUMENTS;

  inst->j.opcode = opcode;
  inst->j.regA = atoi(arg0);
  inst->j.regB = atoi(arg1);

  return ERR_OK;
}

int OType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, inst_t *inst, int *errArg)
{
  inst->o.opcode = opcode;

  return ERR_OK;
}

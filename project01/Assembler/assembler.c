/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INSTRUCTION 1024
#define MAXLINELENGTH 1000
typedef char stringType[MAXLINELENGTH];

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(const char *);


void formatWrite(FILE *, FILE *);
/* use this when return error */

int findLabelAddr(const char *label);

enum Error
{
  ERR, //default
  ERR_UNDEFINED_LABEL,
  ERR_LACK_ARGUMENTS,
  ERR_OVERFLOW,
  ERR_UNRECOGNIZED_OPCODE,
  ERR_ARGUMENT,
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

} instType;

struct
{
  struct
  {
    stringType label;
    int addr;
  } labels[MAX_INSTRUCTION];

  int numAddrs;
} labelTable;

enum OpCode
{
  OP_ADD = 0b000,
  OP_NOR = 0b001,
  OP_LW = 0b010,
  OP_SW = 0b011,
  OP_BEQ = 0b100,
  OP_JALR = 0b101,
  OP_HALT = 0b110,
  OP_NOOP = 0b111
};

/* formatting functions */
int IType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg);
int RType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg);
int JType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg);
int OType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg);

int main(int argc, char *argv[])
{
  char *inFileString, *outFileString;
  FILE *inFilePtr, *outFilePtr;

  char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
  arg1[MAXLINELENGTH], arg2[MAXLINELENGTH]; if (argc != 3) 
  {
    printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
    exit(1); 
  }
  
  inFileString = argv[1]; outFileString = argv[2];
  inFilePtr = fopen(inFileString, "r"); 
  if (inFilePtr == NULL) {
    printf("error in opening %s\n", inFileString);
    exit(1); 
  }
  outFilePtr = fopen(outFileString, "w"); if (outFilePtr == NULL) {
  printf("error in opening %s\n", outFileString);
  exit(1); }
  /* here is an example for how to use readAndParse to read a line from inFilePtr */

  stringType temp;
  int curAddr;

  labelTable.numAddrs = 0;


  /*Read and parse a line of the assembly-language file. Fields are returned in label, opcode, arg0, arg1, arg2 (these strings must have memory already allocated to them). */

  for (curAddr = 0; readAndParse(inFilePtr, label, temp, temp, temp, temp); ++curAddr)
  {
      if (strlen(label) > 0)
      {
          if (findLabelAddr(label) != -1)
          {
              printf("!err! duplicate label\n");
              fclose(inFilePtr);
              fclose(outFilePtr);
              exit(1);
          }
          strncpy(labelTable.labels[labelTable.numAddrs].label, label, MAXLINELENGTH);
          labelTable.labels[labelTable.numAddrs].addr = curAddr;
          ++labelTable.numAddrs;
      }
  }

  /* this is how to rewind the file ptr so that you start reading from the
  beginning of the file */
  rewind(inFilePtr);

  /* do whatever you need to do for opcode "somthing" */
  formatWrite(inFilePtr, outFilePtr);
  fclose(inFilePtr);
  fclose(outFilePtr);
  return (0);
}


void formatWrite(FILE *inFilePtr, FILE *outFilePtr)
{
    instType inst;
    stringType label, opcode, arg0, arg1, arg2;
    int curAddr, temp, errArg; /* which arg err*/

    for (curAddr = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++curAddr)
    {
        if (curAddr)
        {
            fputc('\n', outFilePtr);
        }

        memset(&inst, 0, sizeof inst);
        temp = ERR;
        errArg = -1;
        /* after doing a readAndParse, you may want to do the following to test the opcode */
        
        if (strcmp(opcode, ".fill") == 0)
        {
            if (strlen(arg0) == 0)
            {
                temp = ERR_LACK_ARGUMENTS;

                goto err;
            }

            if (isNumber(arg0))
            {
                fprintf(outFilePtr, "%d", atoi(arg0));
            }
            else
            {
                if ((temp = findLabelAddr(arg0)) == -1)
                {
                    temp = ERR_UNDEFINED_LABEL;
                    errArg = 0;

                    goto err;
                }

                fprintf(outFilePtr, "%d", temp);
            }
        } else {
            if (strcmp(opcode, "add") == 0)
                temp = RType(OP_ADD, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "nor") == 0)
                temp = RType(OP_NOR, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "lw") == 0)
                temp = IType(OP_LW, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "sw") == 0)
                temp = IType(OP_SW, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "beq") == 0)
                temp = IType(OP_BEQ, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "jalr") == 0)
                temp = JType(OP_JALR,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "halt") == 0)
                temp = OType(OP_HALT,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "noop") == 0)
                temp = OType(OP_NOOP,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else
                temp = ERR_UNRECOGNIZED_OPCODE;

                
            if (temp != ERR)
                goto err;

            fprintf(outFilePtr, "%u", inst.code);
        }
    }

    return;

err:
    /*Error Checking*/
    if (temp == ERR_LACK_ARGUMENTS)
    {
        printf("!err! lack arguments\n");
    }
    else if (temp == ERR_UNDEFINED_LABEL)
    {
        printf("!err! undefined label\n");
        switch (errArg)
        {
        case 0:
            printf("%s", arg0);
            break;
        case 1:
            printf("%s", arg1);
            break;
        case 2:
            printf("%s", arg2);
            break;
        }
        printf("\n");
    }
    else if (temp == ERR_LACK_ARGUMENTS)
    {
        printf("!err! lack argument\n");
        switch (errArg)
        {
        case 0:
            printf("%s", arg0);
            break;
        case 1:
            printf("%s", arg1);
            break;
        case 2:
            printf("%s", arg2);
            break;
        }
        printf("\n");
    }
    else if (temp == ERR_UNRECOGNIZED_OPCODE)
    {
        printf("!err! unrecognized opcode\n%s\n", opcode);
    }
    else if (temp == ERR_OVERFLOW)
    {
        printf("!err! argument overflow\n");
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(1);
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
  /* check for line too long (by looking for a \n) */
  if (strchr(line, '\n') == NULL)
  {
    printf("!err! line too long\n");
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
int isNumber(const char *string)
{
  int i;
  return ((sscanf(string, "%d", &i)) == 1);
}

int checkVal(const char *string)
{
  int val = atoi(string);

  return (val < -32768) || (val > 32767) ;
}

int labelOrImmediate(int labelOffset, const char *arg, enum Error *err)
{
  int addr;

  if (isNumber(arg))
  {
    return atoi(arg);
  }

  if ((addr = findLabelAddr(arg)) == -1)
  {
    *err = ERR_UNDEFINED_LABEL;
    return -1;
  }

  return addr - labelOffset;
}

int findLabelAddr(const char *label)
{
  int i;
  for (i = 0; i < labelTable.numAddrs; ++i)
  {
    if (strcmp(label, labelTable.labels[i].label) == 0)
      return labelTable.labels[i].addr;
  }

  return -1;
}


int RType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg)
{
  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_ARGUMENT;
  }

  if (!isNumber(arg2))
  {
    *errArg = 2;
    return ERR_ARGUMENT;
  }

  if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
    return ERR_LACK_ARGUMENTS;

  inst->r.opcode = opcode;
  inst->r.regA = atoi(arg0);
  inst->r.regB = atoi(arg1);
  inst->r.destReg = atoi(arg2);

  return ERR;
}

int IType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg)
{

  int mc = 0, address = 0;

  enum Error err = ERR;


  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_ARGUMENT;
  }

  if (isNumber(arg2) && checkVal(arg2))
  {
    *errArg = 2;
    return ERR_OVERFLOW;
  }
  if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
    return ERR_LACK_ARGUMENTS;

  inst->i.opcode = opcode;
  inst->i.regA = atoi(arg0);
  inst->i.regB = atoi(arg1);
  inst->i.offset = labelOrImmediate((opcode == OP_BEQ ? curAddr + 1 : 0), arg2, &err);

  // maybe error is in arg2
  if (err != ERR)
    *errArg = 2;

  return err;
}

int JType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg)
{

  if (!isNumber(arg0))
  {
    *errArg = 0;
    return ERR_ARGUMENT;
  }

  if (!isNumber(arg1))
  {
    *errArg = 1;
    return ERR_ARGUMENT;
  }

  if (strlen(arg0) == 0 || strlen(arg1) == 0)
    return ERR_LACK_ARGUMENTS;

  inst->j.opcode = opcode;
  inst->j.regA = atoi(arg0);
  inst->j.regB = atoi(arg1);

  return ERR;
}





int OType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char *arg2, instType *inst, int *errArg)
{
  inst->o.opcode = opcode;
  return ERR;
}
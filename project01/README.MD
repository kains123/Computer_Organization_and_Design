# Project - Computer Architecture (Spring 2022) (LC-2K)

In this project, we implemented simple assembler and simulator for LC-2K Instruction-Set Architecture.

## Purpose

This project is intended to help you understand the instructions of a very
simple assembly language and how to assemble programs into machine language.

## LC-2K Instruction-Set Architecture

There are 4 instruction formats (bit 0 is the least-significant bit).

- R-type instructions (add, nor):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-3: unused (should all be 0)
  - bits 2-0: desgReg
- I-type instructions (lw, sw, beq):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-0: offsetField (a 16-bit, 2's complement number with a range of -32768 to 32767)
- J-type instructions (jalr):
  - bits 24-22: opcode
  - bits 21-19: reg A
  - bits 18-16: reg B
  - bits 15-0: unused (should all be 0)
- O-type instructions (halt, noop):
  - bits 24-22: opcode
  - bits 21-0: unused (should all be 0)

## How to build

You should clone this repository.

```bash
git clone https://github.com/kains123/
bash
cd project01
```

## Build & Run

```bash
cd Assembler
gcc assembler.c -o assembler
./assembler test/test1.as test/test1.mc

test2 > undefined label err!

test3 > invalid arguments err!

test4 > line too long err!

```

```bash
cd Simulator
gcc simulator.c -o simulator

gcc -o simulate simulator.c -lm
./simulator test/test1.mc > test/test1.as
```

## Test cases

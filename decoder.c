#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "base_types.h"

#define AL "al"
#define CL "cl"
#define DL "dl"
#define BL "bl"
#define AH "ah"
#define CH "ch"
#define DH "dh"
#define BH "bh"
#define AX "ax"
#define CX "cx"
#define DX "dx"
#define BX "bx"
#define SP "sp"
#define BP "bp"
#define SI "si"
#define DI "di"

// REG (register) field encoding table
const char *register_table[16] = {AL, CL, DL, BL, AH, CH, DH, BH,
                                  AX, CX, DX, BX, SP, BP, SI, DI};

typedef enum InstructionEnum { MOV } Instruction;

const char *InstructionToString(Instruction i) {
  switch (i) {
  case MOV:
    return "mov";

  default:
    printf("Invalid instruction (%d) to parse as string", i);
    exit(1);
  }
}

// Returns how many bytes has read
u32 DecodeInstruction(u8 *bytes) {
  Instruction instruction;
  int cursor = 0;

  // OPCODE. extract bits 7 to 4 from first byte
  u8 op_code = bytes[cursor] >> 4;
  switch (op_code) {
  case 0xB:
    // check if W bit is set (00001000)
    u8 w_bit = (bytes[cursor] & (1 << 3)) != 0;
    // REG. extract bits 2 to 0 (00000111) from first byte
    u8 reg = bytes[cursor] & 0x7;
    cursor += 1;

    char source[32];
    const char *destination = register_table[(w_bit << 3) | reg];
    // printf("W: %d | REG: %d | register: %s\n", w_bit, reg, destination);
    if (w_bit == 1) {
      u8 low = bytes[cursor];
      cursor += 1;
      u8 hi = bytes[cursor];
      u16 byte = (hi << 7) | low;
      sprintf(source, "%d", byte);
    } else {
      sprintf(source, "%d", bytes[cursor]);
    }

    printf("%s %s, %s\n", "mov", destination, source);
    return cursor;

  default:
    break;
  }
  // OPCODE. extract bits 7 to 2 from first byte
  op_code = bytes[cursor] >> 2;
  switch (op_code) {
  case 0x22:
  case 0x23:
  case 0x28:
  case 0x29:
  case 0x31:
    instruction = MOV;
    break;

  default:
    printf("This instruction (%d) is not implemented yet!", op_code);
    exit(1);
  }

  // check if D bit is set
  u8 d_bit = (bytes[cursor] & (1 << 2));
  (void)d_bit;

  // check if W bit is set
  u8 w_bit = (bytes[cursor] & 1);
  (void)w_bit;
  cursor += 1;

  // MOD. extract bits 7 to 6 from second byte
  u8 mod = bytes[cursor] >> 6;
  switch (mod) {
  case 3:
    break;

  default:
    printf("This MOD value (%d) is not implemented yet!", mod);
    exit(1);
  }

  // R/M. extract bits 2 to 0 from second byte
  u8 rm = bytes[cursor] & 0x7;
  switch (rm) {
  case 56:
    break;

  default:
    // MOD is set to Register Mode (11) so R/M identifies
    // the second register operand.
    if (mod == 3) {
      break;
    }
    printf("This R/M value (%d) is not implemented yet!", rm);
    exit(1);
  }

  // REG. extract bits 5 to 3 (00111000) from second byte
  u8 reg = (bytes[cursor] & 0x38) >> 3;

  const char *source;
  const char *destination;
  if (d_bit == 0) {
    source = register_table[(w_bit << 3) | reg];
    destination = register_table[(w_bit << 3) | rm];
  } else {
    source = register_table[(w_bit << 3) | rm];
    destination = register_table[(w_bit << 3) | reg];
  }

  printf("%s %s, %s\n", InstructionToString(instruction), destination, source);

  return cursor;
}

i32 main(i32 argc, char **argv) {
  (void)argc;
  if (!argv[1]) {
    printf("the name of the binary file to decode must be provided");
    exit(1);
  }

  FILE *file_ptr = fopen(argv[1], "rb");
  if (file_ptr == NULL) {
    perror("fopen");
    exit(1);
  }

  // calculate the file size
  fseek(file_ptr, 0, SEEK_END);
  u64 file_length = ftell(file_ptr);
  rewind(file_ptr);

  // allocate and copy the file contents
  u8 *buffer = (u8 *)malloc(file_length * sizeof(char));
  fread(buffer, file_length, 1, file_ptr);
  fclose(file_ptr);

  printf("bits 16\n");
  u32 instruction_start = 0;
  while (instruction_start < file_length) {
    // last instruction read
    instruction_start += DecodeInstruction(buffer + instruction_start);
    assert(instruction_start < file_length);
    // start at the next one
    instruction_start += 1;
  }

  return 0;
}

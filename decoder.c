#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define BX_SI "bx + si"
#define BX_DI "bx + di"
#define BP_SI "bp + si"
#define BP_DI "bp + di"
#define SI "si"
#define DI "di"
#define BP "bp"
#define BX "bx"

#define BX_SI_D "bx + si + "
#define BX_DI_D "bx + di + "
#define BP_SI_D "bp + si + "
#define BP_DI_D "bp + di + "
#define SI_D "si + "
#define DI_D "di + "
#define BP_D "bp + "
#define BX_D "bx + "

// REG (register) field encoding table
const char *register_table[16] = {AL, CL, DL, BL, AH, CH, DH, BH,
                                  AX, CX, DX, BX, SP, BP, SI, DI};

const char *effective_address_register_table[] = {
    BX_SI,   BX_DI,   BP_SI,   BP_DI,   SI,   DI,   BP,   BX,
    BX_SI_D, BX_DI_D, BP_SI_D, BP_DI_D, SI_D, DI_D, BP_D, BX_D,
    BX_SI_D, BX_DI_D, BP_SI_D, BP_DI_D, SI_D, DI_D, BP_D, BX_D,
};

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

u8 verbose = 0;

void ConsumeByte(u8 *bytes, char *buff, i32 *pos) {
  *pos += 1;
  assert(*pos >= 0);

  if (!verbose) {
    return;
  }

  char bit[8];
  for (i32 i = 7; i >= 0; i--) {
    snprintf(bit, 8, "%d", (bytes[*pos] & (1 << i)) != 0);
    strcat(buff, bit);
  }
  strcat(buff, " ");
}

// Returns how many bytes has read
u32 DecodeInstruction(u8 *bytes) {
  Instruction instruction;
  i32 cursor = -1;
  char bytes_str[1024] = {0};
  ConsumeByte(bytes, bytes_str, &cursor);

  // OPCODE. extract bits 7 to 4 from first byte
  u8 op_code = bytes[cursor] >> 4;
  switch (op_code) {
  case 0xB: {
    // printf("[%hx]", op_code);
    // check if W bit is set (00001000)
    u8 w_bit = (bytes[cursor] & (1 << 3)) != 0;
    // REG. extract bits 2 to 0 (00000111) from first byte
    u8 reg = bytes[cursor] & 0x7;
    ConsumeByte(bytes, bytes_str, &cursor);

    char source[32];
    const char *destination = register_table[(w_bit << 3) | reg];
    if (w_bit == 1) {
      u8 low = bytes[cursor];
      ConsumeByte(bytes, bytes_str, &cursor);
      u8 hi = bytes[cursor];
      u16 byte = (hi << 7) | low;
      sprintf(source, "%d", byte);
    } else {
      sprintf(source, "%d", bytes[cursor]);
    }

    if (verbose) {
      printf("\n");
      printf("OPCODE: %hx | W: %d | REG: %d | register: %s", op_code, w_bit,
             reg, destination);
      printf("\n%s\n", bytes_str);
    }
    printf("%s %s, %s\n", "mov", destination, source);
    return cursor;
  }

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
    // printf("[%hx]", op_code);
    instruction = MOV;
    break;

  default:
    printf("This instruction (%d) is not implemented yet!", op_code);
    exit(1);
  }

  // check if D bit is set
  u8 d_bit = (bytes[cursor] & (1 << 1)) != 0;
  // check if W bit is set
  u8 w_bit = (bytes[cursor] & 1) != 0;
  ConsumeByte(bytes, bytes_str, &cursor);

  // MOD. extract bits 7 and 6 (11000000) from second byte
  u8 mod = bytes[cursor] >> 6;
  // R/M. extract bits 2 to 0 (00000111) from second byte
  u8 rm = bytes[cursor] & 0x7;
  // REG. extract bits 5 to 3 (00111000) from second byte
  u8 reg = (bytes[cursor] & 0x38) >> 3;

  const char *source;
  const char *destination;
  char from_reg[32] = {0};
  char from_rm[32] = {0};
  sprintf(from_reg, "%s", register_table[(w_bit << 3) | reg]);
  sprintf(from_rm, "%s", register_table[(w_bit << 3) | rm]);
  switch (mod) {
  // register mode
  case 0x3:
    break;

  // 16-bit displacement
  case 0x2: {
    ConsumeByte(bytes, bytes_str, &cursor);
    u8 low = bytes[cursor];
    ConsumeByte(bytes, bytes_str, &cursor);
    u8 hi = bytes[cursor];
    i16 byte = (hi << 8) | low;

    sprintf(from_rm, "[%s%d]",
            effective_address_register_table[(mod << 3) | rm], byte);
    break;
  }

  // 8-bit displacement
  case 0x1: {
    ConsumeByte(bytes, bytes_str, &cursor);
    u8 low = bytes[cursor];
    u8 idx = (mod << 3) | rm;
    sprintf(from_rm, "[%s%d]", effective_address_register_table[idx], low);
    break;
  }

  // memory mode
  case 0x0: {
    // is direct mode
    if (rm == 0x6) {
      ConsumeByte(bytes, bytes_str, &cursor);
      u8 low = bytes[cursor];
      ConsumeByte(bytes, bytes_str, &cursor);
      u8 hi = bytes[cursor];
      u16 byte = (hi << 8) | low;
      sprintf(from_rm, "[%d]", byte);
      break;
    }

    sprintf(from_rm, "[%s]", effective_address_register_table[(mod << 2) | rm]);
    break;
  }

  default:
    printf("This MOD (%d) is not supported!", mod);
    exit(1);
  }

  if (!d_bit) {
    source = from_reg;
    destination = from_rm;
  } else {
    source = from_rm;
    destination = from_reg;
  }

  if (verbose) {
    printf("\n");
    printf("OPCODE: %hx | D: %d | W: %d | MOD: %d | REG: %d | R/M: %d", op_code,
           d_bit, w_bit, mod, reg, rm);
    printf("\n%s\n", bytes_str);
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

  if (argv[2]) {
    verbose = strcmp(argv[2], "-v") == 0;
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

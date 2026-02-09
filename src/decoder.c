#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decoder.h"

u8 verbose = 0;

// REG (register) field encoding table
const char *register_table[16] = {AL, CL, DL, BL, AH, CH, DH, BH,
                                  AX, CX, DX, BX, SP, BP, SI, DI};

const char *effective_address_register_table[45] = {
    BX_SI,   BX_DI,   BP_SI,   BP_DI,   SI,   DI,   BP,   BX,
    BX_SI_D, BX_DI_D, BP_SI_D, BP_DI_D, SI_D, DI_D, BP_D, BX_D,
    BX_SI_D, BX_DI_D, BP_SI_D, BP_DI_D, SI_D, DI_D, BP_D, BX_D,
};

// register/memory to/from register
// immediate to register/memory
// immediate to register
// memory to accumulator
// register/memory to segment register
// segment register to register/memory
//
// register_memory__register
// immediate__register_memory
// immediate__register
// memory__accumulator
// register_memory_segment_register
// segment_register__register_memory

const char *instructionToString(Instruction i) {
  switch (i) {
  case ADD:
    return "add";
  // case SUB:
  //   return "sub";
  // case CMP:
  //   return "cmp";
  // case JNZ:
  //   return "jnz";
  case MOV:
    return "mov";

  default:
    printf("Invalid instruction (%d) to parse as string", i);
    exit(1);
  }
}

u8 consumeByte(Decoder *decoder) {
  u8 current_byte = decoder->bytes[decoder->cursor];

  if (!verbose) {
    // leave cursor on the next byte
    decoder->cursor += 1;
    return current_byte;
  }

  char bit[8];
  for (i32 i = 7; i >= 0; i--) {
    snprintf(bit, 8, "%d", (current_byte & (1 << i)) != 0);
    strcat(decoder->bit_pattern_str, bit);
  }
  strcat(decoder->bit_pattern_str, " ");

  // leave cursor on the next byte
  decoder->cursor += 1;
  return current_byte;
}

u8 isImediateToRegister(Decoder *decoder, u8 first_byte) {
  u8 op_code = first_byte >> 4;
  if (op_code != 0xB) {
    return 0;
  }

  // check if W bit is set (00001000)
  u8 w_bit = (first_byte & (1 << 3)) != 0;
  // REG. extract bits 2 to 0 (00000111) from first byte
  u8 reg = first_byte & 0x7;

  char source[32];
  const char *destination = register_table[(w_bit << 3) | reg];
  u8 low = consumeByte(decoder);
  if (w_bit == 1) {
    u8 hi = consumeByte(decoder);
    u16 byte = (hi << 8) | low;
    sprintf(source, "%d", byte);
  } else {
    sprintf(source, "%d", low);
  }

  if (verbose) {
    printf("\n");
    printf("OPCODE: %hx | W: %d | REG: %d | register: %s", op_code, w_bit, reg,
           destination);
    printf("\n%s\n", decoder->bit_pattern_str);
  }
  printf("%s %s, %s\n", "mov", destination, source);
  return 1;
}

u8 isRegisterMemory(Decoder *decoder, u8 first_byte) {
  Instruction instruction;

  u8 op_code = first_byte >> 2;
  switch (op_code) {
  case 0x0: {
    instruction = ADD;
    break;
  }

  case 0x31:
  case 0x22: {
    instruction = MOV;
    break;
  }

  case 0xA: {
    instruction = SUB;
    break;
  }

  case 0xE: {
    instruction = CMP;
    break;
  }

  default:
    return 0;
  }

  // check if D bit is set
  u8 d_bit = (first_byte & (1 << 1)) != 0;
  // check if W bit is set
  u8 w_bit = (first_byte & 1) != 0;

  u8 second_byte = consumeByte(decoder);
  // MOD. extract bits 7 and 6 (11000000) from second byte
  u8 mod = second_byte >> 6;
  // R/M. extract bits 2 to 0 (00000111) from second byte
  u8 rm = second_byte & 0x7;
  // REG. extract bits 5 to 3 (00111000) from second byte
  u8 reg = (second_byte & 0x38) >> 3;

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
    u8 low = consumeByte(decoder);
    u8 hi = consumeByte(decoder);
    i16 byte = (hi << 8) | low;

    sprintf(from_rm, "[%s%d]",
            effective_address_register_table[(mod << 3) | rm], byte);
    break;
  }

  // 8-bit displacement
  case 0x1: {
    u8 low = consumeByte(decoder);
    u8 idx = (mod << 3) | rm;
    sprintf(from_rm, "[%s%d]", effective_address_register_table[idx], low);
    break;
  }

  // memory mode
  case 0x0: {
    // is direct mode
    if (rm == 0x6) {
      u8 low = consumeByte(decoder);
      u8 hi = consumeByte(decoder);
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
    printf("\n%s\n", decoder->bit_pattern_str);
  }

  printf("%s %s, %s\n", instructionToString(instruction), destination, source);

  return 1;
}

// Decode the current instruction under cursor
void decodeInstruction(Decoder *decoder) {
  u8 first_byte = consumeByte(decoder);

  // check if is imediate to register operation
  if (isImediateToRegister(decoder, first_byte))
    return;

  // check if is register/memory to/from register mode
  if (isRegisterMemory(decoder, first_byte))
    return;

  printf("\nThis byte (");
  for (i32 i = 7; i >= 0; i--) {
    printf("%d", (first_byte & (1 << i)) != 0);
  }
  printf(") does not encode a known instruction!");
  exit(1);
}

i32 main(i32 argc, char **argv) {
#ifdef DEBUG
  // print realtime without buffering (only when program stop running)
  // during debug sessions
  setbuf(stdout, NULL);
#endif
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
  u8 *file_content = (u8 *)malloc(file_length * sizeof(char));
  fread(file_content, file_length, 1, file_ptr);
  fclose(file_ptr);

  // buffer used to print the textual representation of the bytes consumed when
  // verbose flag is set
  char bit_pattern_str[32] = {0};
  Decoder decoder = {
      .cursor = 0, .bytes = file_content, .bit_pattern_str = bit_pattern_str};

  printf("bits 16\n");
  while (decoder.cursor < file_length) {
    decodeInstruction(&decoder);
    // reset buff
    bit_pattern_str[0] = '\0';
  }

  return 0;
}

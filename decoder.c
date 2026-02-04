#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

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

#define MOV 34

// REG (register) field encoding table
const char *register_table[16] = {AL, CL, DL, BL, AH, CH, DH, BH,
                                  AX, CX, DX, BX, SP, BP, SI, DI};

void DecodeInstruction(u_int8_t *instruction_line) {
  const char *instruction;

  // OPCODE. extract bits 7 to 2 from first byte
  u_int8_t op_code = (instruction_line[0] & 0xFC) >> 2;
  switch (op_code) {
  case MOV:
    instruction = "mov";
    break;

  default:
    printf("This instruction (%d) is not implemented yet!", op_code);
    exit(1);
  }

  // check if D bit is set
  u_int8_t d_bit = (instruction_line[0] & (1 << 2));
  // printf("D: %d\n", d_bit != 0);
  (void)d_bit;

  // check if W bit is set
  u_int8_t w_bit = (instruction_line[0] & 1);
  // printf("W: %d\n", w_bit != 0);
  (void)w_bit;

  // MOD. extract bits 7 to 6 from second byte
  u_int8_t mod = (instruction_line[1] & 0xC0) >> 6;
  switch (mod) {
  case 3:
    break;

  default:
    printf("This MOD value (%d) is not implemented yet!", mod);
    exit(1);
  }

  // R/M. extract bits 2 to 0 from second byte
  u_int8_t rm = instruction_line[1] & 0x7;
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

  // REG. extract bits 5 to 3 from second byte
  u_int8_t reg = (instruction_line[1] & 0x38) >> 3;
  const char *source_register = register_table[(w_bit << 3) | reg];
  const char *destination_register = register_table[(w_bit << 3) | rm];

  printf("bits 16\n");
  printf("%s %s, %s\n", instruction, destination_register, source_register);
}

int main(int argc, char **argv) {
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
  long file_length = ftell(file_ptr);
  printf("[debug] file length: %ld\n", file_length);
  rewind(file_ptr);

  // allocate and copy the file contents
  u_int8_t *buffer = (u_int8_t *)malloc(file_length * sizeof(char));
  fread(buffer, file_length, 1, file_ptr);
  fclose(file_ptr);

  // print file content as bytes
  printf("%d %d\n", buffer[0], buffer[1]);
  // print file content bits
  printf("00000000: ");
  for (int i = 7; i >= 0; i--) {
    printf("%d", (buffer[0] & (1 << i)) != 0);
  }
  printf(" ");
  for (int i = 7; i >= 0; i--) {
    printf("%d", (buffer[0] & (1 << i)) != 0);
  }
  printf("\n\n");

  DecodeInstruction(buffer);

  return 0;
}

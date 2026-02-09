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

typedef enum InstructionEnum { MOV, ADD, SUB, CMP, JNZ } Instruction;

typedef struct {
  u64 cursor;
  u8 *bytes;

  // String representation of individual bits of each byte consumed
  //
  // eg:. 11010010 01101010
  char *bit_pattern_str;
} Decoder;

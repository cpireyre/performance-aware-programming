typedef int i32;
typedef unsigned char   u8;
typedef unsigned short  u16;

#define REG2REG 0b10001000
#define IMM2REG 0b10110000

typedef enum
{
    reg2reg,
    imm2reg,
    unknown
} Op;

u8 match(u8 op, u8 mask) { return (op & mask) == mask; }

Op decode(u8 opcode)
{
    /* This is order-complected somehow oh geez */
    if (match(opcode, IMM2REG)) return imm2reg;
    if (match(opcode, REG2REG)) return reg2reg;
    return unknown;
}

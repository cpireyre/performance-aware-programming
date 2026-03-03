#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef int             i32;

#include "io.c"

typedef enum
{ /* Assign masks in here to allow array indexing? */
    mov_reg2reg,
    mov_imm2reg,
    add_reg2reg,
    add_imm2reg,
    add_imm2acc,
    unknown
} Op;

u8 match(u8 op, u8 mask) { return (op & mask) == mask; }

Op decode(u8 opcode)
{
    /* This is order-complected somehow oh geez */
    if (match(opcode, 0b10110000)) return mov_imm2reg;
    if (match(opcode, 0b10001000)) return mov_reg2reg;
    if ((opcode & 0b11111100) == 0)return add_reg2reg;
    if (match(opcode, 0b10000000)) return add_imm2reg;
    if (match(opcode, 0b00000100)) return add_imm2acc;

    return unknown;
}

#define MAX_FILE_SIZE_BYTES 4096

void           run(u8 *program, i32 size);

int main(int argc, char **argv)
{
    i32 fd, read_bytes;
    u8  buf[MAX_FILE_SIZE_BYTES];

    if (argc != 2) return (0);
    fd = Open(argv[1]);
    read_bytes = Read(fd, (u8*)&buf, MAX_FILE_SIZE_BYTES);
    /* Assumes we fully read file */
    close(fd);
    printf("; %s\n\nbits 16\n\n", argv[1]);
    run(buf, read_bytes);
    return (0);
}

void    run(u8 *program, i32 size)
{
    i32     pc;
    u8      d, w, mod, reg, rem, opcode;
    u8      disp8;
    u16     disp16;
    char    *arg1;

    static char *reg_names[] = {
        "al", "ax", "cl", "cx", "dl", "dx", "bl", "bx",
        "ah", "sp", "ch", "bp", "dh", "si", "bh", "di"
    };
    static char *rem_names[] = {
        "[bx + si]",
        "[bx + di]",
        "[bp + si]",
        "[bp + di]",
        "[si]",
        "[di]",
        "[bp + si]", /* Should be "DIRECT ADDRESS"? */
        "[bx]"
    };
    static char *reg_names_disp8[] = {
        "[bx + si + %u]", "[bx + di + %u]",
        "[bp + si + %u]", "[bp + di + %u]",
        "[si + %u]", "[di + %u]",
        "[bp + %u]", "[bx + %u]"
    };
    static char *reg_names_disp16[] = {
        "[bx + si + %d]", "[bx + di + %d]",
        "[bp + si + %d]", "[bp + di + %d]",
        "[si + %d]", "[di + %d]",
        "[bp + %d]", "[bx + %d]"
    };

    /* d = 0: src in reg field. 1: dest in reg */
    /* d is always 0 in the first 2 listings */
    /* dest first in Intel assembly */
    /* w = 0: byte, 1: word */

    pc = 0;
    while (pc < size)
    {
        opcode = program[pc];
        d = opcode & 0b10;
        w = opcode & 0b01;
        mod = (program[pc + 1] & 0b11000000) >> 6;
        reg = (program[pc + 1] & 0b00111000) >> 2;
        rem =  program[pc + 1] & 0b00000111;
        arg1 = reg_names[reg | w];

        switch (decode(program[pc]))
        {
            case mov_reg2reg:
                {
                    printf("mov ");
                    if (mod == 0b11)
                        printf("%s, %s",
                                reg_names[(rem << 1) | w],
                                arg1);
                    else
                    {
                        if (d) printf("%s, ", arg1);
                        switch (mod)
                        {
                            case 0b00:
                                printf(rem_names[rem]);
                                break;
                            case 0b01:
                                disp8 = program[pc + 2];
                                printf(reg_names_disp8[rem], disp8);
                                pc += 1;
                                break;
                            case 0b10:
                                disp16 = *(u16*)(program + pc + 2);
                                printf(reg_names_disp16[rem], disp16);
                                pc += 2;
                                break;
                        }
                        if (!d) printf(", %s", arg1);
                    }
                    break;
                }
            case mov_imm2reg:
                {
                    reg = opcode & 0b111;
                    w = !(!(opcode & (1 << 3)));
                    printf("mov ");
                    printf(reg_names[(reg << 1) | w]);
                    if (!w)
                        printf(", %d", program[pc + 1]);
                    else
                        printf(", %d", *(u16*)(program + pc + 1));
                    pc += !!w;
                    break;
                }
            case add_reg2reg:
                printf("add ");
                if (mod == 0b11)
                    printf("%s, %s",
                            reg_names[(rem << 1) | w],
                            arg1);
                else
                {
                    if (d) printf("%s, ", arg1);
                    switch (mod)
                    {
                        case 0b00:
                            printf(rem_names[rem]);
                            break;
                        case 0b01:
                            disp8 = program[pc + 2];
                            printf(reg_names_disp8[rem], disp8);
                            pc += 1;
                            break;
                        case 0b10:
                            disp16 = *(u16*)(program + pc + 2);
                            printf(reg_names_disp16[rem], disp16);
                            pc += 2;
                            break;
                    }
                    if (!d) printf(", %s", arg1);
                }
                break;

            case add_imm2reg:
                printf("add ");
                if (mod == 0b11) /* Register mode */
                {
                    reg = (program[pc + 1]) & 0b111;
                    w = opcode & 1;
                    printf(reg_names[(reg << 1) | w]);
                    u8 _16bit = (d == 0 && w == 1) ? 1 : 0;
                    if (_16bit)
                        printf(", %d", *(u16*)(program + pc + 2));
                    else
                        printf(", %d", program[pc + 2]);
                    pc += 1 + _16bit;
                }
                else /* Memory modes, maybe displacement */
                {
                    printf("%s ", w ? "word" : "byte");
                    switch (mod)
                    {
                        case 0b00:
                            printf(rem_names[rem]);
                            break;
                        case 0b01:
                            disp8 = program[pc + 2];
                            printf(reg_names_disp8[rem], disp8);
                            pc += 1;
                            break;
                        case 0b10:
                            disp16 = *(u16*)(program + pc + 2);
                            printf(reg_names_disp16[rem], disp16);
                            pc += 2;
                            break;
                    }
                    u8 _16bit = (d == 0 && w == 1) ? 1 : 0;
                    if (_16bit)
                        printf(", %d", *(u16*)(program + pc + 2));
                    else
                        printf(", %d", program[pc + 2]);
                    pc += 1 + _16bit;
                }
                break;
            case add_imm2acc:
                {
                    printf("add %s, ", w == 1 ? "ax" : "al");
                    if (w == 1)
                        printf("%d", *(u16*)(program + pc + 1));
                    else
                        printf("%d", program[pc + 1]);
                    pc += w;
                }
                break;
            default:
                {
                    printf("unrecognised opcode");
                    printb(opcode);
                    break;
                }
        }
        printf("\n");
        pc += 2;
    }
}

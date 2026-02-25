/* TODO: Use bit field for decoding as in https://pastebin.com/hQ6PKSB3
 * Although some endianness and future-proofing concerns,
 * if some later opcodes break the rules.
*/

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mov.c"
#include "io.c"

#define MAX_FILE_SIZE_BYTES 4096
#define MOD_MASK        0b11000000
#define REG_MASK        0b00111000
#define REM_MASK        0b00000111

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
    run(buf, read_bytes);
    return (0);
}

void    run(u8 *program, i32 size)
{
    i32  pc;
    u8   d, w, mod, reg, rem, opcode;
    u8   disp8;
    u16  disp16;

    static char *reg_names[] = {
        "al", "ax",
        "cl", "cx",
        "dl", "dx",
        "bl", "bx",
        "ah", "sp",
        "ch", "bp",
        "dh", "si",
        "bh", "di"
    };
    static char *rem_names[] = {
        "[bx + si]",
        "[bx + di]",
        "[bp + si]",
        "[bp + di]",
        "[si]",
        "[di]",
        "[bp + si]",
        "[bx]"
    };
    static char *reg_names_disp8[] = {
        "[bx + si + %u]",
        "[bx + di + %u]",
        "[bp + si + %u]",
        "[bp + di + %u]",
        "[si + %u]",
        "[di + %u]",
        "[bp + %u]",
        "[bx + %u]"
    };
    static char *reg_names_disp16[] = {
        "[bx + si + %d]",
        "[bx + di + %d]",
        "[bp + si + %d]",
        "[bp + di + %d]",
        "[si + %d]",
        "[di + %d]",
        "[bp + %d]",
        "[bx + %d]"
    };

    Op  op;

    /* d = 0: src in reg field. 1: dest in reg */
    /* d is always 0 in the first 2 listings */
    /* dest first in Intel assembly */
    /* w = 0: byte, 1: word */

    pc = 0;
    while (pc < size)
    {
        opcode = program[pc];
        op = decode(program[pc]);
        d = opcode & (1 << 1);
        w = opcode & (1 << 0);
        mod = (program[pc + 1] & MOD_MASK) >> 6;
        reg = (program[pc + 1] & REG_MASK) >> 2;
        rem = program[pc + 1] & REM_MASK;
        switch (op)
        {
            case reg2reg:
                {
                    printf("mov ");
                    if (mod == 0b11)
                        printf("%s, %s",
                                reg_names[(rem << 1) + w],
                                reg_names[reg + w]);
                    else
                    {
                        if (d) printf("%s, ", reg_names[reg + w]);
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
                        if (!d) printf(", %s", reg_names[reg + w]);
                    }
                    break;
                }
            case imm2reg:
                {
                    reg = opcode & 0b111;
                    w = !(!(opcode & (1 << 3)));
                    printf("mov ");
                    printf(reg_names[(reg << 1) + w]);
                    if (!w)
                        printf(", %d", program[pc + 1]);
                    else
                        printf(", %d", *(u16*)(program + pc + 1));
                    pc += !!w;
                    break;
                }
            default:
                {
                    printf("Unrecognised opcode");
                    printb(opcode);
                    break;
                }
        }
        printf("\n");
        pc += 2;
    }
}

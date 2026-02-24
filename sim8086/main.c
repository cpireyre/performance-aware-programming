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

#define MAX_FILE_SIZE_BYTES 4096
#define MOD_MASK        0b11000000
#define REG_MASK        0b00111000
#define REM_MASK        0b00000111

static i32     Open(char *path);
static i32     Read(i32 fd, u8 *buf, i32 size);
static void    printb(u8 byte);
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
        /* printb(mod); */
        /* printb(mod >> 6); */
        /* write(1, "---\n", 4); */
        reg = program[pc + 1] & REG_MASK;
        rem = program[pc + 1] & REM_MASK;
        switch (op)
        {
            case reg2reg:
                {
                    printf("mov ");
                    if (mod == 0b11)
                    {
                        printf(reg_names[(rem << 1) + w]);
                        printf(", ");
                        printf(reg_names[(reg >> 2) + w]);
                    }
                    else if (mod == 0b00)
                    {
                        if (d)
                        {
                            printf(reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        /* Direct memory, no displacement */
                        printf("%s", rem_names[rem]);
                        if (!d)
                        {
                            printf(", ");
                            printf(reg_names[(reg >> 2) + w]);
                        }
                    }
                    else if (mod == 0b01) /* 8bit displacement */
                    {
                        u8 disp = program[pc + 2];
                        if (d)
                        {
                            printf(reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        printf(reg_names_disp8[rem], disp);
                        if (!d)
                        {
                            printf(", ");
                            printf(reg_names[(reg >> 2) + w]);
                        }
                        pc += 1;
                    }
                    else if (mod == 0b10) /* 16bit displacement */
                    {
                        /* I don't understand why pc + 2 is correct */
                        u16 disp = *(u16*)(program + pc + 2);
                        /* printb(program[pc]); */
                        /* printb(program[pc + 1]); */
                        /* printb(program[pc + 2]); */
                        if (d)
                        {
                            printf(reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        printf(reg_names_disp16[rem], disp);
                        if (!d)
                        {
                            printf(", ");
                            printf("%s", reg_names[(reg >> 2) + w]);
                        }
                        pc += 2;
                    }
                    else
                    {
                        printb(program[pc]);
                        printb(program[pc + 1]);
                        printb(program[pc + 2]);
                    }
                    (void)d;
                    /* printf("%d%d %d.%d.%d\n", d, w, mod, reg, rem); */
                    break;
                }
            case imm2reg:
                {
                    reg = opcode & 0b111;
                    w = !(!(opcode & (1 << 3)));
                    printf("mov ");
                    printf("%s", reg_names[(reg << 1) + w]);
                    printf(", ");
                    if (!w)
                        printf("%d", program[pc + 1]);
                    else
                        printf("%d", *(u16*)(program + pc + 1));
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

static i32 Read(i32 fd, u8 *buf, i32 size)
{
    i32 read_bytes;

    read_bytes = read(fd, buf, size);
    if (read_bytes < 0)
    {
        printf("Error: read\n");
        exit(1);
    }
    return (read_bytes);
}

static i32 Open(char *path)
{
    i32 fd;

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        printf("Error: open\n");
        exit(1);
    }
    return (fd);
}

static void    printb(u8 byte)
{
    char    *xs = "01";
    i32     i;

    i = 7;
    while (i >= 0)
    {
        write(1, xs + ((byte >> i) & 1), 1);
        i -= 1;
    }
    write(1, "\n", 1);
}

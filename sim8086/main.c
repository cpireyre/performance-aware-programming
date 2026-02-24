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
    char *reg_names[] = {
        "al", "ax",
        "cl", "cx",
        "dl", "dx",
        "bl", "bx",
        "ah", "sp",
        "ch", "bp",
        "dh", "si",
        "bh", "di"
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
        switch (op)
        {
            case reg2reg:
                {
                    d = opcode & (1 << 1);
                    w = opcode & (1 << 0);
                    mod = program[pc + 1] & MOD_MASK;
                    reg = program[pc + 1] & REG_MASK;
                    rem = program[pc + 1] & REM_MASK;
                    printf("mov ");
                    if (mod == 0b11000000)
                    {
                        printf("%s", reg_names[(rem << 1) + w]);
                        printf(", ");
                        printf("%s", reg_names[(reg >> 2) + w]);
                    }
                    else if (mod == 0)
                    {
                        if (d)
                        {
                            printf("%s", reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        /* Direct memory, no displacement */
                        switch (rem)
                        {
                            case 0b000: printf("[bx + si]"); break;
                            case 0b001: printf("[bx + di]"); break;
                            case 0b010: printf("[bp + si]"); break;
                            case 0b011: printf("[bp + di]"); break;
                            case 0b100: printf("[si]");      break;
                            case 0b101: printf("[di]");      break;
                            case 0b110: printf("[bp + si]"); break;
                            case 0b111: printf("[bx]");      break;
                        }
                        if (!d)
                        {
                            printf(", ");
                            printf("%s", reg_names[(reg >> 2) + w]);
                        }
                    }
                    else if (mod == 0b01000000) /* 8bit displacement */
                    {
                        u8 disp = program[pc + 2];
                        if (d)
                        {
                            printf("%s", reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        switch (rem)
                        {
                            case 0b000: printf("[bx + si + %u]", disp); break;
                            case 0b001: printf("[bx + di + %u]", disp); break;
                            case 0b010: printf("[bp + si + %u]", disp); break;
                            case 0b011: printf("[bp + di + %u]", disp); break;
                            case 0b100: printf("[si + %u]", disp); break;
                            case 0b101: printf("[di + %u]", disp); break;
                            case 0b110: printf("[bp + %u]", disp); break;
                            case 0b111: printf("[bx + %u]", disp); break;
                        }
                        if (!d)
                        {
                            printf(", ");
                            printf("%s", reg_names[(reg >> 2) + w]);
                        }
                        pc += 1;
                    }
                    else if (mod == 0b10000000) /* 16bit displacement */
                    {
                        /* I don't understand why pc + 2 is correct */
                        u16 disp = *(u16*)(program + pc + 2);
                        /* printb(program[pc]); */
                        /* printb(program[pc + 1]); */
                        /* printb(program[pc + 2]); */
                        if (d)
                        {
                            printf("%s", reg_names[(reg >> 2) + w]);
                            printf(", ");
                        }
                        switch (rem)
                        {
                            case 0b000: printf("[bx + si + %d]", disp); break;
                            case 0b001: printf("[bx + di + %d]", disp); break;
                            case 0b010: printf("[bp + si + %d]", disp); break;
                            case 0b011: printf("[bp + di + %d]", disp); break;
                            case 0b100: printf("[si + %d]", disp); break;
                            case 0b101: printf("[di + %d]", disp); break;
                            case 0b110: printf("[bp + %d]", disp); break;
                            case 0b111: printf("[bx + %d]", disp); break;
                        }
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
                    {
                        printf("%d", *(u16*)(program + pc + 1));
                        pc += 1;
                    }
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

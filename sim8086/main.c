/* TODO: Use bit field for decoding as in https://pastebin.com/hQ6PKSB3
 * Although some endianness and future-proofing concerns,
 * if some later opcodes break the rules.
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "mov.c"

#define MAX_FILE_SIZE_BYTES 4096
#define MOV_CODE        0b10001000
#define MOV_IMM_CODE    0b10110000
#define OP_MASK         0b11111000
#define MOD_MASK        0b11000000
#define REG_MASK        0b00111000
#define REM_MASK        0b00000111

void    printb(u8 byte)
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
                    assert(mod == MOD_MASK);
                    reg = program[pc + 1] & REG_MASK;
                    rem = program[pc + 1] & REM_MASK;
                    printf("mov ");
                    printf("%s", reg_names[(rem << 1) + w]);
                    printf(", ");
                    printf("%s", reg_names[(reg >> 2) + w]);
                    printf("\n");
                    (void)d; (void)mod;
                    /* printf("%d%d %d.%d.%d\n", d, w, mod, reg, rem); */
                    break;
                }
           case imm2reg:
                {
                    reg = opcode & 0b111;
                    w = opcode & (1 << 3);
                    printf("mov ");
                    printf("%s", reg_names[(reg << 1) + w]);
                    printf(", ");
                    if (w) printf("16bit immediate\n");
                    else printf("%d", program[pc + 1]);
                    printf("\n");
                    break;
                }
          default:
                {
                    printf("Unrecognised opcode");
                    printb(opcode);
                    break;
                }
        }
        pc += 2;
    }
}

int main(int argc, char **argv)
{
    i32 fd, read_bytes;
    u8  buf[MAX_FILE_SIZE_BYTES];

    if (argc != 2) return (0);
    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        printf("Error: open\n");
        return (-1);
    }
    read_bytes = read(fd, buf, MAX_FILE_SIZE_BYTES);
    if (read_bytes < 0)
    {
        printf("Error: read\n");
        return (-1);
    }
    write(1, "; ", 2);
    write(1, argv[1], strlen(argv[1]));
    write(1, "\n\nbits 16\n\n", 11);
    run(buf, read_bytes);
    close(fd);
    return (0);
}

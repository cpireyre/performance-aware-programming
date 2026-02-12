#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

typedef int i32;
typedef char u8;
typedef unsigned short u16;

#define MAX_FILE_SIZE_BYTES 4096
#define MOV_CODE 0b100010
#define OP_MASK  0b111111

int main(int argc, char **argv)
{
    i32 fd, read_bytes;
    u8  buf[MAX_FILE_SIZE_BYTES];
    u8  offset;
    u8  hi, lo;

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
    offset = 0;
    while (offset < read_bytes)
    {
        hi = buf[offset];
        lo = buf[offset + 1];
        if ((hi & OP_MASK) == MOV_CODE)
            printf("Looks like `mov`\n");
        else
            write(1, buf + offset, 2);
        offset += 2;
    }
    close(fd);
    return (0);
}

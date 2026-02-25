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

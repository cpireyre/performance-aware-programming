all: main.c
	$(CC) -Wall -Wextra -Wshadow -Werror main.c -o sim8086
	./diff.sh test/listing_0037_single_register_mov
	./diff.sh test/listing_0038_many_register_mov
	@echo "All clear!"

clean:
	$(RM) sim8086

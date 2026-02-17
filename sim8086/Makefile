.PHONY: all
all: main.c
	$(CC) -Wall -Wextra -Wshadow -Werror main.c -o sim8086

.PHONY: clean
clean:
	$(RM) sim8086

.PHONY: t
t: all
	./diff.sh test/listing_0037_single_register_mov
	./diff.sh test/listing_0038_many_register_mov
	# ./diff.sh test/listing_0039_more_movs
	@echo "All clear!"

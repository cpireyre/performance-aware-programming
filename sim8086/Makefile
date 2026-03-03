CFLAGS := -Wall -Wextra -Werror -Wshadow -Wno-format-security

.PHONY: all
all: main.c
	$(CC) $(CFLAGS) main.c -o sim8086

.PHONY: clean
clean:
	$(RM) sim8086

.PHONY: t
t: all
	./diff.sh test/listing_0037_single_register_mov
	./diff.sh test/listing_0038_many_register_mov
	./diff.sh test/imm2reg
	./diff.sh test/mov16imm
	./diff.sh test/addr-calc
	./diff.sh test/disp8
	./diff.sh test/disp16
	./diff.sh test/dest-calc
	./diff.sh test/listing_0039_more_movs
	./diff.sh test/test-add
	./diff.sh test/sub
	@echo "All clear!"

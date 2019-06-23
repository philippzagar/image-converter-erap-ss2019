CFLAGS=-g -Wall -Wextra -no-pie

.PHONY: all
all: BMP
BMP: Program.c BMP.s
    $(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f BMP

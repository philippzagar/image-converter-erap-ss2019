CFLAGS=-g -Wall -Wextra

.PHONY: all
all: BMP
BMP: Program.c BMP_parsing.c BMP.s

.PHONY: clean
clean:
	rm -f BMP

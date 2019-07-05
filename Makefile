CFLAGS=-O3 -g -Wall -Wextra -no-pie 

.PHONY: all
all: main
main: main.c BMP.s
    

.PHONY: clean
clean:
	rm -f main

CFLAGS= -O3 -g -Wall -Wextra -no-pie

.PHONY: all
all: main
main: main.c grey.S blur.S


.PHONY: clean
clean:
	rm -f main

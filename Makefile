CFLAGS= -O3 -g -Wall -Wextra -no-pie -w

.PHONY: all
all: main
main: main.c -ljpeg ./assembly/grey.S ./assembly/grey_SIMD.S ./assembly/blur.S ./assembly/blur_SIMD.S


.PHONY: clean
clean:
	rm -f main

CFLAGS= -O3 -g -Wall -Wextra -no-pie -w

.PHONY: all
all: main
main: main.c -ljpeg grey.S grey_SIMD.S blur.S blur_SIMD.S


.PHONY: clean
clean:
	rm -f main

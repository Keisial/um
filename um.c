/* Implements the Universal machine of the Cult of the Bound Variable
 * as described in the 9th annual ICFP PRogramming Contest
 * 
 * Must be compiled on an architecture with 32bit pointers
 */
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "um.h"

static int debug = 0;
static int dumpregs = 0;
#define option(X) if (argc > 1 && !strcmp(argv[1], "--"#X)) { argv++; argc--; X = 1; }

int main(int argc, char** argv) {
	option(debug)
	option(dumpregs)

	if (argc < 2) {
		errx(1, "Usage: %s [--debug] [--dumpregs] <scroll>", argv[0]);
	}
	run_scroll_file(argv[1]);

	return 0;
}

void run_scroll_file(const char* filename) {
	struct stat st;
	int fd = open(filename, O_RDONLY);
	if (fd == -1) {
		err(1, "Error opening %s", filename);
	}
	if (fstat(fd, &st) == -1) {
		err(1, "Error obtaining information from scroll %s", filename);
	}
	if (st.st_size & 3) {
		errx(1, "The scroll has incomplete platters");
	}
	if (!st.st_size) {
		errx(1, "Empty scroll %s", filename);
	}
	
	platter scroll;
	FILE* f = fdopen(fd, "rb");
	int i, ok;
	scroll.ptr = malloc(st.st_size);
	for (i = 0; i < st.st_size / 4; i++) {
		uint32_t temp;
		ok = fread(&temp, 1, 4, f);
		if (ok) {
			scroll.ptr[i] = htonl(temp);
		} else {
			/* Either an error or an EOF (the file was truncated?) */
			err(1, "Error reading scroll");
		}
	}
	fclose(f);

	run_scroll(scroll, st.st_size / 4);
	/* Do not try to free scroll, run_scroll() can free it.
	 * Program ends here, anyway.
	 */
}

static platter registers[8];

enum registers { C, B, A };
#define PLATTER(x) registers[(instruction >> (x*3))& 7]
#define REG(x) PLATTER(x).v
#define ARRAY(x) ((REG(x) ? PLATTER(x) : scroll).ptr)
#define Fail(x) errx(2, x)

#define Operation(x, y) case x: if (debug) {  if (dumpregs) dumpregisters(); printf("[%08x] " #y "\t%08x %08x %08x\n", instruction, REG(A), REG(B), REG(C)); }

void dumpregisters() {
	int i;
	puts("");
	for (i=0; i < 8; i++)
		printf("%08x ", registers[i].v);
	puts("");
}

void run_scroll(platter scroll, size_t size) {
	size_t ef; /* Execution finger */

	for (ef = 0; ef < size || !size; ef++) {
		uint32_t instruction = scroll.ptr[ef];

		switch (instruction >> 28) {
           Operation( 0, Conditional Move )
				/*
                  The register A receives the value in register B,
                  unless the register C contains 0.
				 */
				if (REG(C)) REG(A) = REG(B);
				break;

           Operation( 1, Array Index )
				/*
                  The register A receives the value stored at offset
                  in register C in the array identified by B.
                 */
                 REG(A) = ARRAY(B)[REG(C)];
                 break;

           Operation( 2, Array Amendment. )
				/*
                  The array identified by A is amended at the offset
                  in register B to store the value in register C.
                 */
                 ARRAY(A)[REG(B)] = REG(C);
                 break;

           Operation( 3, Addition )
				/*
                  The register A receives the value in register B plus 
                  the value in register C, modulo 2^32.
                 */
                 REG(A) = REG(B) + REG(C);
                 break;

           Operation( 4, Multiplication )
				/*
                  The register A receives the value in register B times
                  the value in register C, modulo 2^32.
                 */
                 REG(A) = REG(B) * REG(C);
                 break;

           Operation( 5, Division )
				/*
                  The register A receives the value in register B
                  divided by the value in register C, if any, where
                  each quantity is treated treated as an unsigned 32
                  bit number.
                 */
                 REG(A) = REG(B) / REG(C);
                 break;

           Operation( 6, Not-And )
				/*
                  Each bit in the register A receives the 1 bit if
                  either register B or register C has a 0 bit in that
                  position.  Otherwise the bit in register A receives
                  the 0 bit.
                 */
                 REG(A) = ~(REG(B) & REG(C));
                 break;


           Operation( 7, Halt. )
				/*
                  The universal machine stops computation.
                 */
                 return;

           Operation( 8, Allocation )
				/*
                  A new array is created with a capacity of platters
                  commensurate to the value in the register C. This
                  new array is initialized entirely with platters
                  holding the value 0. A bit pattern not consisting of
                  exclusively the 0 bit, and that identifies no other
                  active allocated array, is placed in the B register.
                 */
                 PLATTER(B).ptr = calloc(REG(C), 4);
                 break;

           Operation( 9, Abandonment )
				/*
                  The array identified by the register C is abandoned.
                  Future allocations may then reuse that identifier.
                 */
                 free(ARRAY(C));
                 if (!REG(C)) Fail("Array '0' was abandoned");
                 break;

          Operation( 10, Output )
				/*
                  The value in the register C is displayed on the console
                  immediately. Only values between and including 0 and 255
                  are allowed.
                 */
                 putchar(REG(C) & 255);
                 break;

          Operation( 11, Input )
				/*
                  The universal machine waits for input on the console.
                  When input arrives, the register C is loaded with the
                  input, which must be between and including 0 and 255.
                  If the end of input has been signaled, then the 
                  register C is endowed with a uniform value pattern
                  where every place is pregnant with the 1 bit.
                 */
                 REG(C) = getchar();
                 break;

          Operation( 12, Load Program )
				/*
                  The array identified by the B register is duplicated
                  and the duplicate shall replace the '0' array,
                  regardless of size. The execution finger is placed
                  to indicate the platter of this array that is
                  described by the offset given in C, where the value
                  0 denotes the first platter, 1 the second, et
                  cetera.

                  The '0' array shall be the most sublime choice for
                  loading, and shall be handled with the utmost
                  velocity.
                 */
                if (REG(B)) {
					free(scroll.ptr);

					/* Duplicate the array and use the duplicate as array '0' */
					size = malloc_usable_size(PLATTER(B).ptr);
					scroll.ptr = malloc(size);
					memcpy(scroll.ptr, ARRAY(B), size);
				}
                ef = REG(C) - 1;
				break;

          Operation( 13, Orthography )
				/*
				  One special operator does not describe registers in the same way.
				  Instead the three bits immediately less significant than the four
				  instruction indicator bits describe a single register A. The
				  remainder twenty five bits indicate a value, which is loaded
				  forthwith into the register A.
                 */
                 registers[(instruction >> 25) & 7].v = instruction & ((1 << 25) - 1);
                 break;
           default:
                 Fail("Invalid instruction");
		}
	}
}

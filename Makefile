CC=gcc
CFLAGS=-Wall -O3 -Wextra -m32

um: um.c um.h
	$(CC) $(CFLAGS) -o $@ um.c

clean:
	rm -f um

.PHONY: clean

CC=gcc
CFLAGS=-Wall -O3 -Wextra -m32

um: um.c um.h
	$(CC) $(CFLAGS) -o $@ um.c

clean:
	rm -f um sandmark.tmp

check:
	./um docs/sandmark.umz | tee sandmark.tmp
	cmp sandmark.tmp docs/sandmark-output.txt
	rm sandmark.tmp

.PHONY: clean

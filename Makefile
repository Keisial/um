CC=gcc
CFLAGS=-Wall -O3 -Wextra -m32
LDFLAGS=-m32
OBJS=um.o um_malloc.o

um: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -f um $(OBJS) sandmark.tmp

check:
	./um docs/sandmark.umz | tee sandmark.tmp
	cmp sandmark.tmp docs/sandmark-output.txt
	rm sandmark.tmp

.PHONY: clean

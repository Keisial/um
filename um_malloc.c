#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "um.h"

int memdebug = 0;
int memstats = 0;

static struct {
	int allocations, reallocations, duplications, frees;
	uint32_t maxsize;
	int concurrent, maxconcurrent;
} statistics;

#define HIT_N(x,c) do { if (__builtin_expect(memstats, 0)) { statistics.x += c; } } while (0)
#define HIT(x) HIT_N(x,1)
#define MEMDEBUG(x, ...) do { if (__builtin_expect(memdebug, 0)) printf(x, __VA_ARGS__); } while (0)

platter um_alloc(platter size) {
	platter p = { .ptr = calloc(size.v, 4) };
	HIT(allocations); HIT(concurrent); if (memstats && statistics.concurrent > statistics.maxconcurrent) statistics.maxconcurrent = statistics.concurrent;
	if (memstats && size.v > statistics.maxsize) statistics.maxsize = size.v;

	MEMDEBUG("alloc(%u) = %p\n", size.v, p.ptr);
	return p;
}

platter um_realloc(platter p, uint32_t size) {
	HIT(reallocations);
	MEMDEBUG("realloc(%p, %u) =", p.ptr, size);
	p.ptr = realloc(p.ptr, size * 4);
	MEMDEBUG("%p\n", p.ptr);
	return p;
}

uint32_t um_alloc_size(platter p) {
	return malloc_usable_size(p.ptr) / 4;
}

platter um_duplicate(platter old) {
	platter new;
	HIT(duplications); HIT(concurrent);
	size_t size = um_alloc_size(old) * 4;
	new.ptr = malloc(size);
	memcpy(new.ptr, old.ptr, size);

	MEMDEBUG("duplicate(%p) = %p (%zu)\n", old.ptr, new.ptr, size);
	return new;
}

void um_free(platter ptr) {
	HIT(frees);
	HIT_N(concurrent, -1);
	free(ptr.ptr);

	MEMDEBUG("free(%p)\n", ptr.ptr);
}

void um_memory_stats() {
	if (!memstats) return;

	#define STAT(x) printf(#x ": %d\n", statistics.x)
	STAT(allocations);
	STAT(reallocations);
	STAT(duplications);
	STAT(frees);
	STAT(maxsize);
	STAT(concurrent);
	STAT(maxconcurrent);
}

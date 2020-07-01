#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "um.h"

platter um_alloc(platter size) {
	platter p = { .ptr = calloc(size.v, 4) };
	
	return p;
}

platter um_realloc(platter p, uint32_t size) {
	p.ptr = realloc(p.ptr, size * 4);
	return p;
}

uint32_t um_alloc_size(platter p) {
	return malloc_usable_size(p.ptr) / 4;
}

platter um_duplicate(platter old) {
	platter new;
	size_t size = um_alloc_size(old) * 4;
	new.ptr = malloc(size);
	memcpy(new.ptr, old.ptr, size);

	return new;
}

void um_free(platter ptr) {
	free(ptr.ptr);
}

#include <stdint.h>

typedef union { uint32_t v; uint32_t* ptr; } platter;

void load_scroll_file(const char* filename);
void run_scroll();

platter um_alloc(platter size);
uint32_t um_alloc_size(platter size);
platter um_realloc(platter ptr, uint32_t size);
platter um_duplicate(platter ptr);
void um_free(platter ptr);

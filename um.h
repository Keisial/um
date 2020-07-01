typedef union { uint32_t v; uint32_t* ptr; } platter;

void run_scroll_file(const char* filename);
void run_scroll(const platter scroll, size_t size);

#ifndef LIBC_MEMORY_H
#define LIBC_MEMORY_H
#include <libc/types.h>

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif

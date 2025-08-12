#include <klib/memory.h>

void *operator new(usize size) {
  return kmalloc(size);
}

void *operator new[](usize size) {
  return kmalloc(size);
}

void operator delete(void *p) {
  kfree(p);
}

void operator delete[](void *p) {
  kfree(p);
}

void operator delete(void *p, usize) {
  kfree(p);
}

void operator delete[](void *p, usize) {
  kfree(p);
}

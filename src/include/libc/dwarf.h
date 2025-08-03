#pragma once
#include <libc/types.h>
#include <libc/elf.h>

struct Addr2LineResult {
  const char* file;
  u32 line;
  bool found;
};

namespace DWARF {
  Addr2LineResult addr2line_lookup(struct elf_desc *k_desc, u64 addr);
}


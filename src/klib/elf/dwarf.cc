#include <klib/dwarf.h>
#include <klib/memory.h>
#include <klib/assert.h>
#include <klib/string.h>

using namespace DWARF;

static u64 read_uleb128(const u8*& p, const u8* end) {
  u64 result = 0;
  int shift = 0;
  while (p < end) {
    u8 byte = *p++;
    result |= (u64)(byte & 0x7F) << shift;
    if ((byte & 0x80) == 0)
      break;
    shift += 7;
  }
  return result;
}

static s64 read_sleb128(const u8*& p, const u8* /* end */) {
  s64 result = 0;
  int shift = 0;
  u8 byte;
  do {
    byte = *p++;
    result |= (s64)(byte & 0x7F) << shift;
    shift += 7;
  } while ((byte & 0x80) != 0);

  if (shift < 64 && (byte & 0x40))
    result |= -(1LL << shift);

  return result;
}

static u16 read_u16(const u8*& p) {
  u16 v = p[0] | (p[1] << 8);
  p += 2;
  return v;
}

static u32 read_u32(const u8*& p) {
  u32 v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
  p += 4;
  return v;
}

static u64 read_u64(const u8*& p) {
  u64 v = 0;
  for (int i = 0; i < 8; i++) {
    v |= (u64)p[i] << (i * 8);
  }
  p += 8;
  return v;
}

struct LineNumberState {
  u64 address;
  u32 file;
  u32 line;
  u32 column;
  bool is_stmt;
  bool basic_block;
  bool end_sequence;
  bool prologue_end;
  bool epilogue_begin;
  u32 isa;
  u32 discriminator;
  
  void reset(bool default_is_stmt) {
    address = 0;
    file = 1;
    line = 1;
    column = 0;
    is_stmt = default_is_stmt;
    basic_block = false;
    end_sequence = false;
    prologue_end = false;
    epilogue_begin = false;
    isa = 0;
    discriminator = 0;
  }
};

static const char* read_string(const u8*& p, const u8* end) {
  const char* str = (const char*)p;
  while (p < end && *p != 0) {
    p++;
  }
  if (p < end) {
    p++;
  }
  return str;
}

static const char* build_full_path(const char* dir, const char* file) {
  if (!dir || !file) return file;
  if (dir[0] == '.' && dir[1] == '\0') return file;
  
  size_t dir_len = strlen(dir);
  size_t file_len = strlen(file);
  size_t total_len = dir_len + 1 + file_len + 1;
  
  char* full_path = (char*)kmalloc(total_len);
  if (!full_path) return file;
  
  strcpy(full_path, dir);
  if (dir[dir_len - 1] != '/') {
    strcat(full_path, "/");
  }
  strcat(full_path, file);
  
  return full_path;
}

Addr2LineResult DWARF::addr2line_lookup(struct elf_desc *k_desc, u64 addr) {
  Addr2LineResult result = {nullptr, 0, false};
  
  if (!k_desc || !k_desc->debug.debug_line) {
    return result;
  }
  
  const u8 *data = (const u8*)(k_desc->debug.debug_line->sh_offset + k_desc->raw_ptr);
  size_t size = k_desc->debug.debug_line->sh_size;
  
  if (size == 0) {
    return result;
  }
  
  const u8* p = data;
  const u8* end = data + size;
  
  Addr2LineResult best_global_match = {nullptr, 0, false};
  u64 best_global_addr = 0;
  
  while (p < end) {
    u32 unit_length = read_u32(p);
    if (unit_length == 0xFFFFFFFF) {
      return result;
    }
    
    const u8* unit_end = p + unit_length;
    if (unit_end > end) {
      break;
    }
    
    u16 version = read_u16(p);
    if (version > 4) {
      p = unit_end;
      continue;
    }
    
    u32 header_length = read_u32(p);
    const u8* program_start = p + header_length;
    
    u8 minimum_instruction_length = *p++;
    if (version >= 4) { (void)*p++; }
    u8 default_is_stmt = *p++;
    s8 line_base = (s8)*p++;
    u8 line_range = *p++;
    u8 opcode_base = *p++;
    
    u8 opcode_lengths[256] = {0};
    for (u8 i = 1; i < opcode_base && p < program_start; i++) {
      opcode_lengths[i] = *p++;
    }
    
    const char* include_dirs[256];
    u32 include_dir_count = 0;
    include_dirs[0] = ".";
    include_dir_count = 1;
    
    while (p < program_start && *p != 0) {
      if (include_dir_count < 255) {
        include_dirs[include_dir_count] = read_string(p, program_start);
        include_dir_count++;
      } else {
        read_string(p, program_start);
      }
    }
    if (p < program_start) p++;
    
    const char* file_names[256];
    u32 file_dirs[256];
    u32 file_count = 0;
    file_names[0] = nullptr;
    file_dirs[0] = 0;
    file_count = 1;
    
    while (p < program_start && *p != 0) {
      if (file_count < 255) {
        file_names[file_count] = read_string(p, program_start);
        file_dirs[file_count] = read_uleb128(p, program_start);
        read_uleb128(p, program_start);
        read_uleb128(p, program_start);
        file_count++;
      } else {
        read_string(p, program_start);
        read_uleb128(p, program_start);
        read_uleb128(p, program_start);
        read_uleb128(p, program_start);
      }
    }
    if (p < program_start) p++;
    
    LineNumberState state;
    state.reset(default_is_stmt != 0);
    
    Addr2LineResult best_match = {nullptr, 0, false};
    u64 best_addr = 0;
    
    p = program_start;
    while (p < unit_end) {
      u8 opcode = *p++;
      
      if (opcode == 0) {
        u64 length = read_uleb128(p, unit_end);
        const u8* ext_end = p + length;
        if (ext_end > unit_end) break;
        
        u8 ext_opcode = *p++;
        switch (ext_opcode) {
          case 1:
            state.end_sequence = true;
            state.reset(default_is_stmt != 0);
            break;
            
          case 2:
            if (length == 9) {
              state.address = read_u64(p);
            }
            break;
            
          case 3:
            p = ext_end;
            break;
            
          default:
            p = ext_end;
            break;
        }
      } else if (opcode < opcode_base) {
        switch (opcode) {
          case 1:
            if (state.address <= addr && state.address > best_addr && 
                state.file > 0 && state.file < file_count) {
              const char* dir = (file_dirs[state.file] < include_dir_count) ? 
                                include_dirs[file_dirs[state.file]] : ".";
              best_match.file = build_full_path(dir, file_names[state.file]);
              best_match.line = state.line;
              best_match.found = true;
              best_addr = state.address;
            }
            state.basic_block = false;
            state.prologue_end = false;
            state.epilogue_begin = false;
            state.discriminator = 0;
            break;
            
          case 2:
            state.address += read_uleb128(p, unit_end) * minimum_instruction_length;
            break;
            
          case 3:
            state.line += read_sleb128(p, unit_end);
            break;
            
          case 4:
            state.file = read_uleb128(p, unit_end);
            break;
                        
          case 5:
            state.column = read_uleb128(p, unit_end);
            break;
            
          case 6:
            state.is_stmt = !state.is_stmt;
            break;
            
          case 7:
            state.basic_block = true;
            break;
            
          case 8:
            state.address += ((255 - opcode_base) / line_range) * minimum_instruction_length;
            break;
            
          case 9:
            state.address += read_u16(p);
            break;
            
          case 10:
            state.prologue_end = true;
            break;
            
          case 11:
            state.epilogue_begin = true;
            break;
            
          case 12:
            state.isa = read_uleb128(p, unit_end);
            break;
            
          default:
            for (u8 i = 0; i < opcode_lengths[opcode]; i++) {
              read_uleb128(p, unit_end);
            }
            break;
        }
      } else {
        u8 adjusted_opcode = opcode - opcode_base;
        u32 addr_advance = (adjusted_opcode / line_range) * minimum_instruction_length;
        s32 line_advance = line_base + (adjusted_opcode % line_range);
        
        state.address += addr_advance;
        state.line += line_advance;
        
        if (state.address <= addr && state.address > best_addr && 
            state.file > 0 && state.file < file_count) {
          const char* dir = (file_dirs[state.file] < include_dir_count) ? 
                            include_dirs[file_dirs[state.file]] : ".";
          best_match.file = build_full_path(dir, file_names[state.file]);
          best_match.line = state.line;
          best_match.found = true;
          best_addr = state.address;
        }
        
        state.basic_block = false;
        state.prologue_end = false;
        state.epilogue_begin = false;
        state.discriminator = 0;
      }
    }
    
    if (best_match.found && best_addr > best_global_addr) {
      best_global_match = best_match;
      best_global_addr = best_addr;
    }
    
    p = unit_end;
  }
  
  return best_global_match;
}

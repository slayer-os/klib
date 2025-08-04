#include <klib/types.h>

namespace Log {
  void __failed_assert(const char *assertion, const char *message, const char *file, u32 line, const char *function);
}

void __lc_assert(bool condition, const char *assertion, const char *message, const char *file, u32 line, const char *function) {
    if (!condition) {
        Log::__failed_assert(assertion, message, file, line, function);
        for(;;);
        // TODO: Kernel panic
    }
}

#pragma once
#include <atomic>

class Spinlock {
private:
  volatile std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
  Spinlock() {}
  
  void lock() {
    while (flag.test_and_set(std::memory_order_acquire)) __builtin_ia32_pause();
  }

  u8 test_and_set() {
    return flag.test_and_set(std::memory_order_acquire);
  }

  void nowaitlock() {
    flag.test_and_set(std::memory_order_acquire);
  }

  void unlock() {
    flag.clear(std::memory_order_release);
  }

  u8 test() {
    return flag.test();
  }
};

#pragma once
#include <klib/types.h>

#define NLIMBS_512 8

class u512 {
public:
  u64 limbs[NLIMBS_512] = {0};

  // -- Constructors --
  u512();
  template<typename T>
  u512(T value);
  u512(u128 v);
  static u512 from_hex(const char *s);
  static u512 from_dec(const char *s);

  static u512 expmod(const u512& x, const u512& e, const u512& m);
};

// -- Operator Overloads --
// Arithmetic
u512 operator+(const u512& x, const u512& y);
u512 operator-(const u512& x, const u512& y);
u512 operator*(const u512& x, const u512& y);
u512 operator/(const u512& x, const u512& y);
// Bitwise
u512 operator<<(const u512& x, int n);
u512 operator>>(const u512& x, int n);
u512 operator|(const u512& x, const u512 &y);
// Modular Arithmetic
u512 operator%(const u512& x, const u512& y);

// Comparison
bool operator<(const u512& x, const u512& y);
bool operator>(const u512& x, const u512& y);
bool operator<=(const u512& x, const u512& y);
bool operator>=(const u512& x, const u512& y);
bool operator==(const u512& x, const u512& y);
bool operator!=(const u512& x, const u512& y);
// Conversion
u512 operator""_u512(const char *s);

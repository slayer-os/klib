#include <libc/intx.h>
#include <libc/string.h>
#include <libc/assert.h>

int bit_length(const u512& x) {
  for (int i = NLIMBS_512 - 1; i >= 0; i--) {
    if (x.limbs[i] != 0) {
      int bits = 64 - __builtin_clzll(x.limbs[i]);
      return i * 64 + bits;
    }
  }
  return 0;
}

u512 operator+(const u512& x, const u512& y) {
  u512 z;
  u64 carry = 0;
  for (int i = 0; i < NLIMBS_512; i++) {
    z.limbs[i] = x.limbs[i] + y.limbs[i] + carry;
    carry = (x.limbs[i] > ~y.limbs[i]);
  }
  return z;
}

u512 operator-(const u512& x, const u512& y) {
  u512 z;
  u64 borrow = 0;
  for (int i = 0; i < NLIMBS_512; i++) {
    z.limbs[i] = x.limbs[i] - y.limbs[i] - borrow;
    borrow = (x.limbs[i] < y.limbs[i]);
  }
  return z;
}

u512 operator*(const u512& x, const u512& y) {
  u512 z;
  for (int i = 0; i < NLIMBS_512; i++) {
    u64 carry = 0;
    for (int j = 0; j < NLIMBS_512 - i; j++) {
      u128 t = (u128)x.limbs[i] * y.limbs[j] + z.limbs[i + j] + carry;
      z.limbs[i + j] = (u64)t;
      carry = (u64)(t >> 64);
    }
  }
  return z;
}

u512 operator<<(const u512& x, int n) {
  u512 z;
  int k = n / 64;
  int s = n % 64;
  for (int i = 0; i < NLIMBS_512; i++) {
    if (i + k < NLIMBS_512) {
      z.limbs[i + k] = x.limbs[i] << s;
      if (s && i + k + 1 < NLIMBS_512) {
        z.limbs[i + k + 1] |= x.limbs[i] >> (64 - s);
      }
    }
  }
  return z;
}

u512 operator>>(const u512& x, int n) {
  u512 z;
  int k = n / 64;
  int s = n % 64;
  for (int i = 0; i < NLIMBS_512; i++) {
    if (i >= k) {
      z.limbs[i - k] = x.limbs[i] >> s;
      if (s && i - k - 1 >= 0) {
        z.limbs[i - k - 1] |= x.limbs[i] << (64 - s);
      }
    }
  }
  return z;
}

u512 operator/(const u512& dividend, const u512& divisor) {
  assert(!(divisor == 0), "Division by zero");

  u512 quotient;
  u512 remainder = dividend;

  int n = bit_length(dividend);
  int d = bit_length(divisor);

  if (n < d) {
      return quotient; 
  }

  int shift = n - d;
  u512 shiftedDivisor = divisor << shift;

  for (int i = shift; i >= 0; i--) {
    if (remainder >= shiftedDivisor) {
      remainder = remainder - shiftedDivisor;
      quotient = quotient + (u512(1) << i);
    }
    shiftedDivisor = shiftedDivisor >> 1;
  }
  return quotient;
}

// Modular
u512 operator%(const u512& x, const u512& y) {
  return x - (x / y) * y;
}

u512 expmod(const u512& x, const u512& y, const u512& m) {
  if (m == 1) return 0;
  u512 r = 1;
  u512 base = x % m;
  u512 e = y;
  
  while (e > 0) {
    if (e.limbs[0] & 1) {
      r = (r * base) % m;
    }
    e = e >> 1;
    base = (base * base) % m;
  }

  return r;
}


bool operator<(const u512& x, const u512& y) {
  for (int i = NLIMBS_512 - 1; i >= 0; i--) {
    if (x.limbs[i] < y.limbs[i]) {
      return true;
    }
    if (x.limbs[i] > y.limbs[i]) {
      return false;
    }
  }
  return false;
}

bool operator>(const u512& x, const u512& y) {
  for (int i = NLIMBS_512 - 1; i >= 0; i--) {
    if (x.limbs[i] > y.limbs[i]) {
      return true;
    }
    if (x.limbs[i] < y.limbs[i]) {
      return false;
    }
  }
  return false;
}

bool operator<=(const u512& x, const u512& y) {
  return !(x > y);
}

bool operator>=(const u512& x, const u512& y) {
  return !(x < y);
}

bool operator==(const u512& x, const u512& y) {
  for (int i = 0; i < NLIMBS_512; i++) {
    if (x.limbs[i] != y.limbs[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const u512& x, const u512& y) {
  return !(x == y);
}

// -- Constructors --
u512::u512() {
  for (int i = 0; i < 8; i++) {
    limbs[i] = 0;
  }
}

template<typename T>
u512::u512(T value) {
  static_assert(sizeof(T) <= 8, "T must be at most 64 bits");

  for (int i = 0; i < 8; i++) {
    limbs[i] = 0;
  }

  for (size_t i = 0; i < sizeof(T); i++) {
    limbs[i / 8] |= (static_cast<u64>(value) & 0xFF) << (8 * (i % 8));
    value >>= 8;
  }
}
template u512::u512(u8);
template u512::u512(u16);
template u512::u512(u32);
template u512::u512(u64);
template u512::u512<long>(long);
template u512::u512<long long>(long long);

u512::u512(u128 v) {
  limbs[0] = (u64)v;
  limbs[1] = (u64)(v >> 64);
}

u512 u512::from_hex(const char* hex) {
  u512 x = {0};

  for (int i = 0; hex[i] != '\0'; i++) {
    for (int j = NLIMBS_512 - 1; j > 0; j--) {
      x.limbs[j] = (x.limbs[j] << 4) | (x.limbs[j-1] >> 60);
    }
    x.limbs[0] <<= 4;
    if (hex[i] >= '0' && hex[i] <= '9') {
      x.limbs[0] |= (hex[i] - '0');
    } else if (hex[i] >= 'a' && hex[i] <= 'f') {
      x.limbs[0] |= (hex[i] - 'a' + 10);
    } else if (hex[i] >= 'A' && hex[i] <= 'F') {
      x.limbs[0] |= (hex[i] - 'A' + 10);
    } else {
      break;
    }
  }

  return x;
}


u512 u512::from_dec(const char *s) {
  u512 x;
  int len = strlen(s);
  for (int i = 0; i < len; i++) {
    x = x * 10;
    x.limbs[0] += s[i] - '0';
  }
  return x;
}

// Conversion
u512 operator""_u512(const char *s) {
  if (s[0] == '0' && s[1] == 'x') {
    return u512::from_hex(s+2);
  }
  return u512::from_dec(s);
}

#pragma once
// Not actually part of a libc but it's a good idea to have it here


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long s64;

typedef u64 size_t;
typedef s64 ssize_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef __uint128_t u128;

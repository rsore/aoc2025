#ifndef BASIC_H
#define BASIC_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Fundamental types
typedef size_t usize;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8 ;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8 ;

typedef double f64;
typedef float f32;

#define UNUSED(x) (void)(x);
#define ARRAY_LENGTH(arr) (sizeof((arr)) / sizeof(*(arr)))

char *read_entire_file(const char *path);
char *sprint(const char *fmt, ...);

static inline u64
absolute_value(s64 val)
{
    if (val < 0) val *= -1;
    return val;
}

static inline s64
max_s64(s64 a, s64 b)
{
    if (a > b) return a;
    return b;
}

#endif

#pragma once

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <util/Except.h>

__attribute((used)) volatile int _fltused = 0;

static void _memcpy(void* dest, void* src, UINTN n)
{
    for (UINTN i = 0; i < n; i++)
        ((UINT8*)dest)[i] = ((UINT8*)src)[i];
}

static void _memset(void* s, UINT8 c, UINTN n)
{
    for (UINTN i = 0; i < n; i++)
        ((UINT8*)s)[i] = c;
}

static UINTN _strlen(char* s)
{
    for (int i = 0;; i++)
        if (s[i] == '\0')
            return i;
}

static void _assert(BOOLEAN n, char* msg)
{
    if (!(n)) {
        TRACE("Assert %s failed!\n", msg);
        while (TRUE)
            ;
    }
}

static double _sqrt(double n)
{
    double out;
    __asm__ volatile("fldl %[input];"
                     "fsqrt;"
                     "fstpl %[output];"
                     : [ output ] "=g"(out)
                     : [ input ] "g"(n)
                     :);
    return out;
}

static double _cos(double n)
{
    double out;
    __asm__ volatile("fldl %[input];"
                     "fcos;"
                     "fstpl %[output];"
                     : [ output ] "=g"(out)
                     : [ input ] "g"(n)
                     :);
    return out;
}

static double _acos(double n)
{
    double imd = _sqrt(1 - (n * n));
    double out;
    __asm__ volatile("fldl %[imd];"
                     "fldl %[input];"
                     "fpatan;"
                     "fstpl %[output];"
                     : [ output ] "=g"(out)
                     : [ input ] "g"(n), [ imd ] "g"(imd)
                     :);
    return out;
}

static int _ifloor(double d)
{
    if ((((double)((int)d)) == d) || (d >= 0))
        return (int)d;
    return (int)(d - 1);
}

static int _iceil(double d)
{
    if ((((double)((int)d)) == d) || (d < 0))
        return (int)d;
    return (int)(d + 1);
}

static double _pow(double a, int n)
{
    double prod = 1;
    BOOLEAN neg = (n < 0);
    n = neg ? -n : n;
    for (int i = 0; i < n; i++)
        prod = prod * a;
    if (neg)
        prod = 1 / prod;
    return prod;
}

static double _fmod(double x, double y)
{
    return x - ((int)(x / y)) * y;
}

#define STB_TRUETYPE_IMPLEMENTATION

#define STBTT_ifloor(x) _ifloor(x)
#define STBTT_iceil(x) _iceil(x)
#define STBTT_sqrt(x) _sqrt(x)
#define STBTT_pow(x, y) _pow(x, y)
#define STBTT_fmod(x, y) _fmod(x, y)
#define STBTT_cos(x) _cos(x)
#define STBTT_acos(x) _acos(x)
#define STBTT_fabs(x) __builtin_fabs(x)

#define STBTT_malloc(x, u) ((void)(u), AllocatePool(x))
#define STBTT_free(x, u) ((void)(u), FreePool(x))

#define STBTT_assert(x) _assert(x, #x)

#define STBTT_strlen(x) _strlen(x)

#define STBTT_memcpy _memcpy
#define STBTT_memset _memset
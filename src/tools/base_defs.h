#pragma once
#include <stdint.h>

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long dword;
// typedef unsigned short word;
// typedef unsigned char byte;
typedef const char* cstr;
typedef void* pvoid;
#ifdef _WIN32
#if _MSC_VER > 1200
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef int int64;
typedef unsigned int uint64;
#endif
#else
#include <inttypes.h>
typedef int64_t int64;
typedef uint64_t uint64;
#endif

typedef ulong CLR;

#ifndef MAX
  #define MAX(a,b)    (((a) > (b)) ? (a) : (b))
  #define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX_PATH
  #define MAX_PATH 260
#endif

// and here it would be good to use the compiler marker, not the OS
#ifdef _WIN32
#define _CDECL _cdecl
#else
#define _CDECL __attribute__((__cdecl__))
#endif


// Because int64_t and friends may be any integral type of the
// correct size, we have to explicitly ask for the correct
// integer type to get the correct mangling with ddmd
#if __LP64__
// Be careful not to care about sign when using dinteger_t
// use this instead of integer_t to
// avoid conflicts with system #include's
typedef unsigned long dinteger_t;
// Signed and unsigned variants
typedef long sinteger_t;
typedef unsigned long uinteger_t;
#else
typedef unsigned long long dinteger_t;
typedef long long sinteger_t;
typedef unsigned long long uinteger_t;
#endif

typedef unsigned char ubyte;

typedef long double longdouble;
typedef volatile long double volatile_longdouble;

typedef int8_t                  d_int8;
typedef uint8_t                 d_uns8;
typedef int16_t                 d_int16;
typedef uint16_t                d_uns16;
typedef int32_t                 d_int32;
typedef uint32_t                d_uns32;
typedef int64_t                 d_int64;
typedef uint64_t                d_uns64;

typedef float                   d_float32;
typedef double                  d_float64;
typedef longdouble              d_float80;

typedef d_uns8                  d_char;
typedef d_uns16                 d_wchar;
typedef d_uns32                 d_dchar;

typedef longdouble real_t;

/// A UTF-8 code unit
typedef /*unsigned*/ char   utf8_t;
/// A UTF-16 code unit
//typedef unsigned short  utf16_t;
typedef wchar_t utf16_t;


/// A UTF-32 code unit
typedef unsigned int    utf32_t;
typedef utf32_t         dchar_t;

#if _MSC_VER
//#include <alloca.h>
#include <malloc.h>
typedef __int64 longlong;
typedef unsigned __int64 ulonglong;
#else
typedef long long longlong;
typedef unsigned long long ulonglong;
#endif







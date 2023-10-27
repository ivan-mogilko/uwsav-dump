//=============================================================================
//
// Platform definitions.
//
//=============================================================================
#ifndef COMMON_UTILS__PLATFORM_H__
#define COMMON_UTILS__PLATFORM_H__

#if defined(_WIN32)
    #define PLATFORM_OS_WINDOWS     (1)
#else
    #define PLATFORM_OS_WINDOWS     (0)
#endif

#if defined(__LP64__)
    // LP64 machine, OS X or Linux
    // int 32bit | long 64bit | long long 64bit | void* 64bit
    #define PLATFORM_64BIT          (1)
#elif defined(_WIN64)
    // LLP64 machine, Windows
    // int 32bit | long 32bit | long long 64bit | void* 64bit
    #define PLATFORM_64BIT          (1)
#else
    // 32-bit machine, Windows or Linux or OS X
    // int 32bit | long 32bit | long long 64bit | void* 32bit
    #define PLATFORM_64BIT          (0)
#endif

#if defined(_WIN32)
    #define PLATFORM_ENDIAN_LITTLE  (1)
    #define PLATFORM_ENDIAN_BIG     (0)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define PLATFORM_ENDIAN_LITTLE  (1)
    #define PLATFORM_ENDIAN_BIG     (0)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define PLATFORM_ENDIAN_LITTLE  (0)
    #define PLATFORM_ENDIAN_BIG     (1)
#else
    #error "Unknown platform"
#endif

#if defined(_DEBUG)
    #define PLATFORM_DEBUG  (1)
#elif ! defined(NDEBUG)
    #define PLATFORM_DEBUG  (1)
#else
    #define PLATFORM_DEBUG  (0)
#endif

#endif // COMMON_UTILS__PLATFORM_H__

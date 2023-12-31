//=============================================================================
//
// Various utility bit and byte operations.
//
//=============================================================================
#ifndef COMMON_UTILS__BBOP_H__
#define COMMON_UTILS__BBOP_H__

#include "platform.h"
#include <stdint.h>

#if PLATFORM_ENDIAN_BIG || defined (TEST_BIGENDIAN)
#define BITBYTE_BIG_ENDIAN
#endif

namespace BitByteOperations
{
    inline int16_t SwapBytesInt16(const int16_t val)
    {
        return ((val >> 8) & 0xFF) | ((val << 8) & 0xFF00);
    }

    inline int32_t SwapBytesInt32(const int32_t val)
    {
        return ((val >> 24) & 0xFF) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | ((val << 24) & 0xFF000000);
    }

    inline int64_t SwapBytesInt64(const int64_t val)
    {
        return ((val >> 56) & 0xFF) | ((val >> 40) & 0xFF00) | ((val >> 24) & 0xFF0000) |
              ((val >> 8) & 0xFF000000) | ((val << 8) & 0xFF00000000LL) |
              ((val << 24) & 0xFF0000000000LL) | ((val << 40) & 0xFF000000000000LL) | ((val << 56) & 0xFF00000000000000LL);
    }

    inline float SwapBytesFloat(const float val)
    {
        // (c) SDL2
        union
        {
            float f;
            uint32_t ui32;
        } swapper;
        swapper.f = val;
        swapper.ui32 = SwapBytesInt32(swapper.ui32);
        return swapper.f;
    }

    inline int16_t Int16FromLE(const int16_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return SwapBytesInt16(val);
#else
        return val;
#endif
    }

    inline int32_t Int32FromLE(const int32_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return SwapBytesInt32(val);
#else
        return val;
#endif
    }

    inline int64_t Int64FromLE(const int64_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return SwapBytesInt64(val);
#else
        return val;
#endif
    }

    inline float FloatFromLE(const float val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return SwapBytesFloat(val);
#else
        return val;
#endif
    }

    inline int16_t Int16FromBE(const int16_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return val;
#else
        return SwapBytesInt16(val);
#endif
    }

    inline int32_t Int32FromBE(const int32_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return val;
#else
        return SwapBytesInt32(val);
#endif
    }

    inline int64_t Int64FromBE(const int64_t val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return val;
#else
        return SwapBytesInt64(val);
#endif
    }

    inline float FloatFromBE(const float val)
    {
#if defined (BITBYTE_BIG_ENDIAN)
        return val;
#else
        return SwapBytesFloat(val);
#endif
    }

} // namespace BitByteOperations


// Aliases for easier calling
namespace BBOp  = BitByteOperations;

#endif // COMMON_UTILS__BBOP_H__

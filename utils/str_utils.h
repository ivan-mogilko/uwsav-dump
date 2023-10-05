//=============================================================================
//
// Various string operations.
//
//=============================================================================
#ifndef COMMON_UTILS__STRUTILS_H__
#define COMMON_UTILS__STRUTILS_H__

#include <stdarg.h>
#include <string>

std::string StrPrint(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    va_list ap_cpy;
    va_copy(ap_cpy, ap);
    size_t length = vsnprintf(nullptr, 0u, fmt, ap);
    std::string s(length, 0);
    vsnprintf(&s[0], length + 1, fmt, ap_cpy);
    va_end(ap_cpy);
    va_end(ap);
    return s;
}

#endif // COMMON_UTILS__STRUTILS_H__

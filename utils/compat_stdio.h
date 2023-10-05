//=============================================================================
//
// Compatibility layer over stdio functions.
//
//=============================================================================
#ifndef COMMON_UTILS__COMPAT_STDIO_H__
#define COMMON_UTILS__COMPAT_STDIO_H__

#include <stdio.h>
#include <stdint.h>

typedef int64_t file_off_t;

// Size of the buffer enough to accomodate a UTF-8 path
#ifdef __cplusplus
const size_t MAX_PATH_SZ = 1024u;
#else
#define MAX_PATH_SZ (1024u)
#endif

#ifdef __cplusplus
extern "C" {
#endif

FILE *compat_fopen(const char *path, const char *mode);
int   compat_fseek(FILE * stream, file_off_t offset, int whence);
file_off_t compat_ftell(FILE * stream);

#ifdef __cplusplus
}
#endif

#endif // COMMON_UTILS__COMPAT_STDIO_H__

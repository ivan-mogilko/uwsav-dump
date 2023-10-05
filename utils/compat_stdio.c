#include "compat_stdio.h"

#include "platform.h"
#include <stdio.h>

#if defined(_WIN32)
#include <shlwapi.h>
#endif

FILE *compat_fopen(const char *path, const char *mode)
{
#if defined(_WIN32)
    WCHAR wpath[MAX_PATH_SZ];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH_SZ);
    WCHAR wmode[10];
    MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 10);
    FILE *f = NULL;
    _wfopen_s(&f, wpath, wmode);
    return f;
#else
    return fopen(path, mode);
#endif
}

int	compat_fseek(FILE * stream, file_off_t offset, int whence)
{
#if defined(HAVE_FSEEKO) // Contemporary POSIX libc
    return fseeko(stream, offset, whence);
#elif defined(_WIN32) // MSVC
    return _fseeki64(stream, offset, whence); 
#else // No distinct interface with off_t
    return fseek(stream, offset, whence);
#endif
}

file_off_t compat_ftell(FILE * stream) 
{
#if defined(HAVE_FSEEKO) // Contemporary POSIX libc
    return ftello(stream);
#elif defined(_WIN32) // MSVC
    return _ftelli64(stream); 
#else // No distinct interface with off_t
    return ftell(stream);
#endif
}

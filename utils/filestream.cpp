#include "filestream.h"
#include <stdexcept>
#include "compat_stdio.h"

static std::string GetCMode(FileOpenMode open_mode, StreamMode work_mode)
{
    switch (open_mode)
    {
    case kFileMode_Open:
        return (work_mode == kStream_Read) ? "rb" : "r+b";
    case kFileMode_Create:
        return (work_mode == kStream_Write) ? "ab" : "a+b";
    case kFileMode_CreateAlways:
        return (work_mode == kStream_Write) ? "wb" : "w+b";
    default:
        return "";
    }
}

FileStream::FileStream(const std::string &path, FileOpenMode open_mode, StreamMode work_mode)
    : StreamBase(path)
    , _file(nullptr)
    , _openMode(open_mode)
    , _workMode(work_mode)
{
    OpenImpl(path, open_mode, work_mode);
}

FileStream::~FileStream()
{
    CloseImpl();
}

std::unique_ptr<FileStream> FileStream::TryOpen(
    const std::string &path, FileOpenMode open_mode, StreamMode work_mode)
{
    std::unique_ptr<FileStream> fs;
    try
    {
        fs.reset(new FileStream(path, open_mode, work_mode));
        if (fs && !fs->IsValid())
            fs = nullptr;
    }
    catch(std::runtime_error)
    {
        fs = nullptr;
    }
    return fs;
}

void FileStream::OpenImpl(const std::string &path, FileOpenMode open_mode, StreamMode work_mode)
{
    std::string mode = GetCMode(open_mode, work_mode);
    if (mode.size() == 0)
        throw std::runtime_error("Error determining open mode.");
    _file = compat_fopen(path.c_str(), mode.c_str());
    if (_file == nullptr)
        throw std::runtime_error("Error opening file.");
}

void FileStream::CloseImpl()
{
    if (_file)
        fclose(_file);
    _file = nullptr;
}

bool FileStream::IsValid() const
{
    return _file != nullptr;
}

bool FileStream::EOS() const
{
    return feof(_file) != 0;
}

soff_t FileStream::GetLength() const
{
    soff_t pos = (soff_t)compat_ftell(_file);
    compat_fseek(_file, 0, SEEK_END);
    soff_t end = (soff_t)compat_ftell(_file);
    compat_fseek(_file, pos, SEEK_SET);
    return end;
}

   soff_t FileStream::GetPosition() const
{
    return static_cast<soff_t>(compat_ftell(_file));
}

bool FileStream::CanRead() const
{
    return IsValid() && _workMode != kStream_Read;
}

bool FileStream::CanWrite() const
{
    return IsValid() && _workMode == kStream_Write;
}

bool FileStream::CanSeek() const
{
    return IsValid();
}

size_t FileStream::Read(void *buffer, size_t size)
{
    return fread(buffer, sizeof(uint8_t), size, _file);
}

int32_t FileStream::ReadByte()
{
    return fgetc(_file);
}

size_t FileStream::Write(const void *buffer, size_t size)
{
    return fwrite(buffer, sizeof(uint8_t), size, _file);
}

int32_t FileStream::WriteByte(uint8_t val)
{
    return fputc(val, _file);
}

bool FileStream::Seek(soff_t offset, StreamSeek origin)
{
    int whence;
    switch (origin)
    {
    case kSeekBegin:    whence = SEEK_SET; break;
    case kSeekCurrent:  whence = SEEK_CUR; break;
    case kSeekEnd:      whence = SEEK_END; break;
    default:
        return false;
    }
    return compat_fseek(_file, static_cast<file_off_t>(offset), whence) == 0;
}

bool FileStream::Flush()
{
    return fflush(_file) == 0;
}

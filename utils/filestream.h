//=============================================================================
//
// File stream implementation.
// TODO: proper explanation, comments.
//
//=============================================================================
#ifndef COMMON_UTILS__FILESTREAM_H__
#define COMMON_UTILS__FILESTREAM_H__

#include "stream.h"

enum FileOpenMode
{
    kFileMode_Open,         // Open existing file
    kFileMode_Create,       // Create new file, or open existing one
    kFileMode_CreateAlways  // Always create a new file, replacing any existing one
};


class FileStream : public StreamBase
{
public:
    // Represents an open file object
    // The constructor may raise std::runtime_error if 
    // - there is an issue opening the file (does not exist, locked, permissions, etc)
    // - the open mode could not be determined
    FileStream(const std::string &path, FileOpenMode open_mode, StreamMode work_mode);
    ~FileStream() override;

    static std::unique_ptr<FileStream> TryOpen(
        const std::string &path, FileOpenMode open_mode, StreamMode work_mode);

    FileOpenMode GetOpenMode() const { return _openMode; }
    StreamMode GetWorkMode() const { return _workMode; }

    bool    IsValid() const override;
    bool    EOS() const override;
    soff_t  GetLength() const override;
    soff_t  GetPosition() const override;
    bool    CanRead() const override;
    bool    CanWrite() const override;
    bool    CanSeek() const override;

    size_t  Read(void *buffer, size_t size) override;
    int32_t ReadByte() override;
    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;

    bool    Seek(soff_t offset, StreamSeek origin) override;

    bool    Flush() override;

private:
    void    OpenImpl(const std::string &path, FileOpenMode open_mode, StreamMode work_mode);
    void    CloseImpl();

    FILE                *_file = nullptr;
    const FileOpenMode  _openMode;
    const StreamMode    _workMode;
};

#endif // COMMON_UTILS__FILESTREAM_H__

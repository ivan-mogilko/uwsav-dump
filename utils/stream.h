//=============================================================================
//
// Base stream class.
// TODO: proper explanation, comments.
//
//=============================================================================
#ifndef COMMON_UTILS__STREAM_H__
#define COMMON_UTILS__STREAM_H__

#include <memory>
#include <string>
#include "bbop.h"

// Stream offset type
typedef int64_t soff_t;

enum StreamMode
{
    kStream_Read,
    kStream_Write
};

enum StreamSeek
{
   kSeekBegin,
   kSeekCurrent,
   kSeekEnd
};


class StreamBase
{
public:
    StreamBase() = default;
    StreamBase(const std::string &path)
        : _path(path) {}
    virtual ~StreamBase() = default;

    // Returns an optional path of a stream's source, such as a filepath;
    // primarily for diagnostic purposes
    const std::string &GetPath() const { return _path; }

    virtual bool    IsValid() const = 0;
    virtual bool    EOS() const = 0;
    virtual soff_t  GetLength() const = 0;
    virtual soff_t  GetPosition() const = 0;
    virtual bool    CanRead() const = 0;
    virtual bool    CanWrite() const = 0;
    virtual bool    CanSeek() const = 0;

    virtual size_t  Read(void *buffer, size_t size) = 0;
    virtual int32_t ReadByte() = 0;
    virtual size_t  Write(const void *buffer, size_t size) = 0;
    virtual int32_t WriteByte(uint8_t b) = 0;

    virtual bool    Seek(soff_t offset, StreamSeek origin) = 0;

    // Closes the stream
    virtual void    Close() = 0;
    // Flush stream buffer to the underlying device
    virtual bool    Flush() = 0;

private:
    std::string _path; // optional name of the stream's source (e.g. filepath)
};


class Stream
{
public:
    Stream(std::unique_ptr<StreamBase> base)
        : _base(std::move(base)) {}

    bool    IsValid() const     { return _base && _base->IsValid(); }
    bool    EOS() const         { return _base->EOS(); }
    soff_t  GetLength() const   { return _base->GetLength(); }
    soff_t  GetPosition() const { return _base->GetPosition(); }
    bool    CanRead() const     { return _base && _base->CanRead(); }
    bool    CanWrite() const    { return _base && _base->CanWrite(); }
    bool    CanSeek() const     { return _base && _base->CanSeek(); }

    operator bool() const       { return IsValid(); }

    // Closes the stream
    void    Close()             { _base->Close(); }
    // Flush stream buffer to the underlying device
    void    Flush()             { _base->Flush(); }

    // Reads number of bytes in the provided buffer
    size_t  Read(void *buffer, size_t size) { return _base->Read(buffer, size); }
    // ReadByte conforms to fgetc behavior:
    // - if stream is valid, then returns an *unsigned char* packed in the int
    // - if EOS, then returns -1
    int32_t ReadByte() { return _base->ReadByte(); }
    // Writes number of bytes from the provided buffer
    size_t  Write(const void *buffer, size_t size) { return _base->Write(buffer, size); }
    // WriteByte conforms to fputc behavior:
    // - on success, returns the unsigned char packed in the int
    // - on failure, returns -1
    int32_t WriteByte(uint8_t b) { return _base->WriteByte(b); }

    bool Seek(soff_t offset, StreamSeek origin)
    {
        return _base->Seek(offset, origin);
    }

    //
    // Following are helper methods for reading & writing particular values.
    //

    int8_t ReadInt8()
    {
        int8_t val = 0;
        Read(&val, sizeof(int8_t));
        return val;
    }
    int16_t ReadInt16LE()
    {
        int16_t val = 0;
        Read(&val, sizeof(int16_t));
        return BBOp::Int16FromLE(val);
    }
    int32_t ReadInt32LE()
    {
        int32_t val = 0;
        Read(&val, sizeof(int32_t));
        return BBOp::Int32FromLE(val);
    }
    int64_t ReadInt64LE()
    {
        int32_t val = 0;
        Read(&val, sizeof(int32_t));
        return BBOp::Int64FromLE(val);
    }

    size_t WriteInt8(int8_t val)
    {
        return Write(&val, sizeof(int8_t));
    }
    size_t WriteInt16LE(int16_t val)
    {
        val = BBOp::Int16FromLE(val);
        return Write(&val, sizeof(int16_t));
    }
    size_t WriteInt32LE(int32_t val)
    {
        val = BBOp::Int32FromLE(val);
        return Write(&val, sizeof(int32_t));
    }
    size_t WriteInt64LE(int64_t val)
    {
        val = BBOp::Int64FromLE(val);
        return Write(&val, sizeof(int64_t));
    }

protected:
    std::unique_ptr<StreamBase> _base;
};

#endif // COMMON_UTILS__STREAM_H__

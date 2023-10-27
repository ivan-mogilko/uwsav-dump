//=============================================================================
//
// MemoryStream does reading and writing over the buffer of bytes stored in
// memory. Currently has rather trivial implementation. Does not own a buffer
// itself, but works with the provided C-buffer pointer, which means that the
// buffer object *must* persist until stream is closed.
//
// VectorStream is a specialized implementation that works with std::vector.
// Unlike base MemoryStream provides continiously resizing buffer for writing.
//
//=============================================================================
#ifndef COMMON_UTILS__MEMORYSTREAM_H__
#define COMMON_UTILS__MEMORYSTREAM_H__

#include <vector>
#include "stream.h"

class MemoryStream : public StreamBase
{
public:
    // Construct memory stream in the read-only mode over a const C-buffer;
    // reading will never exceed buf_sz bytes;
    // buffer must persist in memory until the stream is closed.
    MemoryStream(const uint8_t *cbuf, size_t buf_sz);
    // Construct memory stream in the chosen mode over a given C-buffer;
    // neither reading nor writing will ever exceed buf_sz bytes;
    // buffer must persist in memory until the stream is closed.
    MemoryStream(uint8_t *buf, size_t buf_sz, StreamMode mode);
    ~MemoryStream() override = default;

    // Is stream valid (underlying data initialized properly)
    bool    IsValid() const override;
    // Is end of stream
    bool    EOS() const override;
    // Total length of stream (if known)
    soff_t  GetLength() const override;
    // Current position (if known)
    soff_t  GetPosition() const override;
    bool    CanRead() const override;
    bool    CanWrite() const override;
    bool    CanSeek() const override;

    size_t  Read(void *buffer, size_t size) override;
    int32_t ReadByte() override;
    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;

    bool    Seek(soff_t offset, StreamSeek origin) override;

    void    Close() override;
    bool    Flush() override;

protected:
    const uint8_t           *_cbuf = nullptr; // readonly buffer ptr
    size_t                   _buf_sz = 0u; // hard buffer limit
    size_t                   _len = 0u; // calculated length of stream
    const StreamMode         _mode;
    size_t                   _pos = 0u; // current stream pos

private:
    uint8_t                 *_buf = nullptr; // writeable buffer ptr
};


class VectorStream : public MemoryStream
{
public:
    // Construct memory stream in the read-only mode over a const std::vector;
    // vector must persist in memory until the stream is closed.
    VectorStream(const std::vector<uint8_t> &cbuf);
    // Construct memory stream in the chosen mode over a given std::vector;
    // vector must persist in memory until the stream is closed.
    VectorStream(std::vector<uint8_t> &buf, StreamMode mode);
    ~VectorStream() override = default;

    bool    CanRead() const override;
    bool    CanWrite() const override;

    size_t  Write(const void *buffer, size_t size) override;
    int32_t WriteByte(uint8_t b) override;

    void    Close() override;

private:
    std::vector<uint8_t> *_vec = nullptr; // writeable vector (may be null)
};

#endif // COMMON_UTILS__MEMORYSTREAM_H__

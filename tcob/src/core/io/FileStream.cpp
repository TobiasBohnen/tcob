// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/FileStream.hpp>

#include <physfs.h>

#include <tcob/core/io/Logger.hpp>

namespace tcob::io::detail {
inline void check(const std::string& msg, i32 c)
{
    if (c == 0) {
        Log(msg + ": " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }
}

void FileStreamBase::close() const
{
    check("close", PHYSFS_close(_handle));
}

auto FileStreamBase::flush() const -> bool
{
    return PHYSFS_flush(_handle) != 0;
}

auto FileStreamBase::eof() const -> bool
{
    return PHYSFS_eof(_handle) != 0;
}

auto FileStreamBase::tell() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_tell(_handle));
}

auto FileStreamBase::length() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_fileLength(_handle));
}

auto FileStreamBase::seek(std::streamoff off, std::ios_base::seekdir way) const -> std::streampos
{
    PHYSFS_sint64 pos { off };
    if (way == std::ios_base::cur) {
        const PHYSFS_sint64 cur { PHYSFS_tell(_handle) };
        pos = cur + off;
    } else if (way == std::ios_base::end) {
        const PHYSFS_sint64 end { PHYSFS_fileLength(_handle) };
        pos = end + off;
    }

    check("seek", PHYSFS_seek(_handle, static_cast<PHYSFS_uint64>(pos)));

    return static_cast<std::streampos>(pos);
}

auto FileStreamBase::OpenRead(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openRead(path.c_str());
}

auto FileStreamBase::OpenWrite(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openWrite(path.c_str());
}

auto FileStreamBase::OpenAppend(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openAppend(path.c_str());
}

void FileStreamBase::buffer(u64 size)
{
    check("setBuffer", PHYSFS_setBuffer(_handle, size));
}

auto FileStreamBase::read(void* s, std::streamsize n, isize size) const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_readBytes(_handle, s, n * size));
}

auto FileStreamBase::write(const void* s, std::streamsize n, isize size) const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_writeBytes(_handle, s, n * size));
}
}
// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/FileStream.hpp>

#include <physfs.h>

#include <tcob/core/io/Logger.hpp>

namespace tcob::detail::io {
inline auto check(const std::string& msg, i32 c) -> bool
{
    if (c == 0) {
        Log(msg + ": " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()), LogLevel::Error);
    }

    return c != 0;
}

auto FileStream::close() -> bool
{
    _closed = true;
    return check("close", PHYSFS_close(_handle));
}

auto FileStream::flush() const -> bool
{
    return check("flush", PHYSFS_flush(_handle));
}

auto FileStream::eof() const -> bool
{
    return PHYSFS_eof(_handle) != 0;
}

auto FileStream::tell() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_tell(_handle));
}

auto FileStream::length() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_fileLength(_handle));
}

auto FileStream::seek(std::streamoff off, std::ios_base::seekdir way) const -> bool
{
    PHYSFS_sint64 pos { off };
    if (way == std::ios_base::cur) {
        const PHYSFS_sint64 cur { PHYSFS_tell(_handle) };
        pos = cur + off;
    } else if (way == std::ios_base::end) {
        const PHYSFS_sint64 end { PHYSFS_fileLength(_handle) };
        pos = end + off;
    }

    return check("seek", PHYSFS_seek(_handle, static_cast<PHYSFS_uint64>(pos)));
}

auto FileStream::OpenRead(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openRead(path.c_str());
}

auto FileStream::OpenWrite(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openWrite(path.c_str());
}

auto FileStream::OpenAppend(const std::string& path) -> PHYSFS_File*
{
    return PHYSFS_openAppend(path.c_str());
}

void FileStream::buffer(u64 size)
{
    check("setBuffer", PHYSFS_setBuffer(_handle, size));
}

auto FileStream::read(void* s, std::streamsize n, isize size) const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_readBytes(_handle, s, n * size));
}

auto FileStream::write(const void* s, std::streamsize n, isize size) const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_writeBytes(_handle, s, n * size));
}
}
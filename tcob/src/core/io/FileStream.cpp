// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/FileStream.hpp"

#include <cassert>

#include <physfs.h>

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::io {

static inline auto check(string const& msg, i32 c) -> bool
{
    if (c == 0) {
        logger::Error(msg + ": " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return false;
    }

    return true;
}

file_sink::file_sink(PHYSFS_File* handle)
    : _handle {handle}
{
    assert(handle);
}

file_sink::~file_sink()
{
    if (_handle) {
        close();
    }
}

file_sink::file_sink(file_sink&& other) noexcept
    : _handle {std::exchange(other._handle, nullptr)}
{
}

auto file_sink::operator=(file_sink&& other) noexcept -> file_sink&
{
    std::swap(_handle, other._handle);
    return *this;
}

auto file_sink::close() -> bool
{
    if (check("close", PHYSFS_close(_handle))) {
        _handle = nullptr;
        return true;
    }

    return false;
}

auto file_sink::flush() const -> bool
{
    return check("flush", PHYSFS_flush(_handle));
}

auto file_sink::is_eof() const -> bool
{
    return PHYSFS_eof(_handle) != 0;
}

auto file_sink::tell() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_tell(_handle));
}

auto file_sink::size_in_bytes() const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_fileLength(_handle));
}

auto file_sink::seek(std::streamoff off, seek_dir way) const -> bool
{
    PHYSFS_sint64 pos {off};
    if (way == seek_dir::Current) {
        pos = PHYSFS_tell(_handle) + off;
    } else if (way == seek_dir::End) {
        pos = PHYSFS_fileLength(_handle) + off;
    }

    if (pos < 0) { return false; }

    return check("seek", PHYSFS_seek(_handle, static_cast<PHYSFS_uint64>(pos)));
}

auto file_sink::OpenRead(path const& path) -> file_sink
{
    return file_sink {PHYSFS_openRead(path.c_str())};
}

auto file_sink::OpenWrite(path const& path) -> file_sink
{
    return file_sink {PHYSFS_openWrite(path.c_str())};
}

auto file_sink::OpenAppend(path const& path) -> file_sink
{
    return file_sink {PHYSFS_openAppend(path.c_str())};
}

void file_sink::set_buffer_size(u64 size)
{
    check("set_buffer_size", PHYSFS_setBuffer(_handle, size));
}

auto file_sink::read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    return static_cast<std::streamsize>(PHYSFS_readBytes(_handle, s, static_cast<u64>(sizeInBytes)));
}

auto file_sink::write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    auto const retValue {static_cast<std::streamsize>(PHYSFS_writeBytes(_handle, s, static_cast<u64>(sizeInBytes)))};
    if (retValue != sizeInBytes) {
        logger::Error("write_bytes: " + string {PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())});
    }
    return retValue;
}

////////////////////////////////////////////////////////////

ifstream::ifstream(path const& path, u64 bufferSize)
    : sink_istream<file_sink> {&_sink}
    , _sink {file_sink::OpenRead(path)}
{
    _sink.set_buffer_size(bufferSize);
}

auto ifstream::close() -> bool
{
    return _sink.close();
}

auto ifstream::flush() -> bool
{
    return _sink.flush();
}

auto ifstream::Open(path const& path, u64 bufferSize) -> std::optional<ifstream>
{
    if (io::is_file(path)) {
        return std::optional<ifstream> {std::in_place, path, bufferSize};
    }

    return std::nullopt;
}

////////////////////////////////////////////////////////////

ofstream::ofstream(path const& path, u64 bufferSize, bool append)
    : sink_ostream<file_sink> {&_sink}
    , _sink {append ? file_sink::OpenAppend(path) : file_sink::OpenWrite(path)}
{
    _sink.set_buffer_size(bufferSize);
}

auto ofstream::close() -> bool
{
    return _sink.close();
}

auto ofstream::flush() -> bool
{
    return _sink.flush();
}

}

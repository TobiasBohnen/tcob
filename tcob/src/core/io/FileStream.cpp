// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/FileStream.hpp"

#include <cassert>
#include <expected>
#include <ios>
#include <utility>

#include <physfs.h>

#include "tcob/core/Logger.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

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
}

file_sink::~file_sink()
{
    if (_handle) { close(); }
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
    if (!is_valid()) { return false; }

    if (check("close", PHYSFS_close(_handle))) {
        _handle = nullptr;
        return true;
    }

    return false;
}

auto file_sink::flush() const -> bool
{
    if (!is_valid()) { return false; }
    return check("flush", PHYSFS_flush(_handle));
}

auto file_sink::is_eof() const -> bool
{
    if (!is_valid()) { return true; }
    return PHYSFS_eof(_handle) != 0;
}

auto file_sink::tell() const -> std::streamsize
{
    if (!is_valid()) { return 0; }
    return static_cast<std::streamsize>(PHYSFS_tell(_handle));
}

auto file_sink::size_in_bytes() const -> std::streamsize
{
    if (!is_valid()) { return 0; }
    return static_cast<std::streamsize>(PHYSFS_fileLength(_handle));
}

auto file_sink::seek(std::streamoff off, seek_dir way) const -> bool
{
    if (!is_valid()) { return false; }

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
    if (!is_valid()) { return; }
    check("set_buffer_size", PHYSFS_setBuffer(_handle, size));
}

auto file_sink::read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    if (!is_valid()) { return 0; }
    return static_cast<std::streamsize>(PHYSFS_readBytes(_handle, s, static_cast<u64>(sizeInBytes)));
}

auto file_sink::write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    if (!is_valid()) { return 0; }
    auto const retValue {static_cast<std::streamsize>(PHYSFS_writeBytes(_handle, s, static_cast<u64>(sizeInBytes)))};
    if (retValue != sizeInBytes) {
        logger::Error("write_bytes: " + string {PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())});
    }
    return retValue;
}

auto file_sink::is_valid() const -> bool
{
    return _handle != nullptr;
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

auto ifstream::is_valid() const -> bool
{
    return _sink.is_valid();
}

auto ifstream::Open(path const& path, u64 bufferSize) -> std::expected<ifstream, error_code>
{
    if (io::is_file(path)) {
        return std::expected<ifstream, error_code> {std::in_place, path, bufferSize};
    }

    return std::unexpected<error_code> {error_code::FileNotFound};
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

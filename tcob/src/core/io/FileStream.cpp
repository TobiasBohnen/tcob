// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/FileStream.hpp"

#include <cassert>
#include <expected>
#include <utility>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/io/FileSystem.hpp"

namespace tcob::io {

////////////////////////////////////////////////////////////

ifstream::ifstream(path const& path, u64 bufferSize)
    : _source {locate_service<file_system>().open_read(path, bufferSize)}
{
}

auto ifstream::close() -> bool
{
    return _source->close();
}

auto ifstream::is_valid() const -> bool
{
    return _source->is_valid();
}

auto ifstream::Open(path const& path, u64 bufferSize) -> std::expected<ifstream, error_code>
{
    if (io::is_file(path)) {
        return std::expected<ifstream, error_code> {std::in_place, path, bufferSize};
    }

    return std::unexpected<error_code> {error_code::FileNotFound};
}

auto ifstream::get_source() -> file_device*
{
    return _source.get();
}

auto ifstream::get_source() const -> file_device const*
{
    return _source.get();
}

////////////////////////////////////////////////////////////

ofstream::ofstream(path const& path, u64 bufferSize, bool append)
    : _sink {append ? locate_service<file_system>().open_append(path, bufferSize) : locate_service<file_system>().open_write(path, bufferSize)}
{
}

auto ofstream::close() -> bool
{
    return _sink->close();
}

auto ofstream::flush() -> bool
{
    return _sink->flush();
}

auto ofstream::get_sink() -> file_device*
{
    return _sink.get();
}

auto ofstream::get_sink() const -> file_device const*
{
    return _sink.get();
}

}

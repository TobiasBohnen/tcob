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
    : _sink {locate_service<file_system>().open_read(path, bufferSize)}
{
}

auto ifstream::close() -> bool
{
    return _sink->close();
}

auto ifstream::is_valid() const -> bool
{
    return _sink->is_valid();
}

auto ifstream::Open(path const& path, u64 bufferSize) -> std::expected<ifstream, error_code>
{
    if (io::is_file(path)) {
        return std::expected<ifstream, error_code> {std::in_place, path, bufferSize};
    }

    return std::unexpected<error_code> {error_code::FileNotFound};
}

auto ifstream::get_sink() -> file_sink*
{
    return _sink.get();
}

auto ifstream::get_sink() const -> file_sink const*
{
    return _sink.get();
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

auto ofstream::get_sink() -> file_sink*
{
    return _sink.get();
}

auto ofstream::get_sink() const -> file_sink const*
{
    return _sink.get();
}

}

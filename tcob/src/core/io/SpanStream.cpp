// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/SpanStream.hpp>

#include <cstring>

namespace tcob::io {

ispan_sink::ispan_sink(std::span<ubyte const> span)
    : _span {span}
{
}

auto ispan_sink::size_in_bytes() const -> std::streamsize
{
    return static_cast<std::streamsize>(_span.size_bytes());
}

auto ispan_sink::is_eof() const -> bool
{
    return _pos >= std::ssize(_span);
}

auto ispan_sink::tell() const -> std::streamsize
{
    return _pos;
}

auto ispan_sink::seek(std::streamoff off, seek_dir way) -> bool
{
    switch (way) {
    case seek_dir::Current:
        _pos += off;
        break;
    case seek_dir::Begin:
        _pos = off;
        break;
    case seek_dir::End:
        _pos = std::ssize(_span) + off;
        break;
    }

    if (_pos < 0) {
        _pos = 0;
        return false;
    }

    return true;
}

auto ispan_sink::read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize
{
    auto const retValue {std::min(std::max(sizeInBytes, std::streamsize {0}), size_in_bytes() - _pos)};

    if (retValue > 0) {
        memcpy(s, _span.data() + _pos, static_cast<usize>(retValue));
        _pos += retValue;
    }

    return retValue;
}

////////////////////////////////////////////////////////////

isstream::isstream(std::span<ubyte const> span)
    : sink_istream<ispan_sink> {&_sink}
    , _sink {span}
{
}

////////////////////////////////////////////////////////////

ospan_sink::ospan_sink(std::span<ubyte> span)
    : _span {span}
{
}

auto ospan_sink::tell() const -> std::streamsize
{
    return _pos;
}

auto ospan_sink::seek(std::streamoff off, seek_dir way) -> bool
{
    switch (way) {
    case seek_dir::Current:
        _pos += off;
        break;
    case seek_dir::Begin:
        _pos = off;
        break;
    case seek_dir::End:
        _pos = std::ssize(_span) + off;
        break;
    }

    return true;
}

auto ospan_sink::write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize
{
    auto const retValue {std::min(std::max(sizeInBytes, std::streamsize {0}), static_cast<std::streamsize>(_span.size_bytes()) - _pos)};

    if (retValue > 0) {
        memcpy(_span.data() + _pos, s, static_cast<usize>(retValue));
        _pos += retValue;
    }

    return retValue;
}

////////////////////////////////////////////////////////////

osstream::osstream(std::span<ubyte> span)
    : sink_ostream<ospan_sink> {&_sink}
    , _sink {span}
{
}

}

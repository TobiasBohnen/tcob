// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/SpanStream.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <ios>
#include <iterator>
#include <span>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {

span_source::span_source(std::span<std::byte const> span)
    : _span {span}
{
}

auto span_source::size_in_bytes() const -> std::streamsize
{
    return static_cast<std::streamsize>(_span.size_bytes());
}

auto span_source::is_eof() const -> bool
{
    return _pos >= std::ssize(_span);
}

auto span_source::tell() const -> std::streamoff
{
    return _pos;
}

auto span_source::seek(std::streamoff off, seek_dir way) -> bool
{
    auto const totalSize {size_in_bytes()};

    switch (way) {
    case seek_dir::Current: _pos += off; break;
    case seek_dir::Begin:   _pos = off; break;
    case seek_dir::End:     _pos = totalSize + off; break;
    }

    if (_pos < 0) {
        _pos = 0;
        return false;
    }

    if (_pos > totalSize) {
        _pos = totalSize;
        return false;
    }

    return true;
}

auto span_source::read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    std::streamsize const totalSize {size_in_bytes()};
    std::streamsize const remaining {totalSize - _pos};
    if (remaining <= 0) { return 0; }

    std::streamsize const bytesToRead {std::min(sizeInBytes, remaining)};
    std::memcpy(s, _span.data() + _pos, static_cast<usize>(bytesToRead));
    _pos += bytesToRead;

    return bytesToRead;
}

////////////////////////////////////////////////////////////

isstream::isstream(std::span<std::byte const> span)
    : _source {span}
{
}

auto isstream::get_source() -> span_source*
{
    return &_source;
}

auto isstream::get_source() const -> span_source const*
{
    return &_source;
}

////////////////////////////////////////////////////////////

span_sink::span_sink(std::span<std::byte> span)
    : _span {span}
{
}

auto span_sink::tell() const -> std::streamoff
{
    return _pos;
}

auto span_sink::seek(std::streamoff off, seek_dir way) -> bool
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

auto span_sink::write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    auto const retValue {std::min(sizeInBytes, static_cast<std::streamsize>(_span.size_bytes()) - static_cast<std::streamsize>(_pos))};

    if (retValue > 0) {
        std::memcpy(_span.data() + _pos, s, static_cast<usize>(retValue));
        _pos += retValue;
    }

    return retValue;
}

////////////////////////////////////////////////////////////

osstream::osstream(std::span<std::byte> span)
    : _sink {span}
{
}

auto osstream::get_sink() -> span_sink*
{
    return &_sink;
}

auto osstream::get_sink() const -> span_sink const*
{
    return &_sink;
}

}

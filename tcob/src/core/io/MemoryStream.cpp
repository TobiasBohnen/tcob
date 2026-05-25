// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/MemoryStream.hpp>

#include <algorithm>
#include <cstring>
#include <ios>
#include <iterator>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {

auto memory_sink::size_in_bytes() const -> std::streamsize
{
    return static_cast<std::streamsize>(_buf.size());
}

auto memory_sink::is_eof() const -> bool
{
    return _pos >= std::ssize(_buf);
}

auto memory_sink::tell() const -> std::streamoff
{
    return _pos;
}

auto memory_sink::seek(std::streamoff off, seek_dir way) -> bool
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

    return true;
}

auto memory_sink::read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    auto const totalSize {size_in_bytes()};
    if (_pos > totalSize) { return 0; }

    auto const remaining {totalSize - _pos};
    if (remaining <= 0) { return 0; }

    auto const bytesToRead {std::min(sizeInBytes, remaining)};
    memcpy(s, _buf.data() + static_cast<usize>(_pos), static_cast<usize>(bytesToRead));
    _pos += bytesToRead;

    return bytesToRead;
}

auto memory_sink::write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    if (std::ssize(_buf) < sizeInBytes + _pos) {
        _buf.resize(static_cast<usize>(sizeInBytes + _pos));
    }

    memcpy(_buf.data() + static_cast<usize>(_pos), s, static_cast<usize>(sizeInBytes));
    _pos += sizeInBytes;

    return sizeInBytes;
}

////////////////////////////////////////////////////////////

iomstream::iomstream()
    : sink_istream<memory_sink> {&_sink}
    , sink_ostream<memory_sink> {&_sink}
{
}

auto iomstream::tell() const -> std::streamoff
{
    return _sink.tell();
}

auto iomstream::seek(std::streamoff off, seek_dir way) -> bool
{
    return _sink.seek(off, way);
}

}

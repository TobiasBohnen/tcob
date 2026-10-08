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

auto memory_device::size_in_bytes() const -> std::streamsize
{
    return static_cast<std::streamsize>(_buf.size());
}

auto memory_device::is_eof() const -> bool
{
    return _pos >= std::ssize(_buf);
}

auto memory_device::tell() const -> std::streamoff
{
    return _pos;
}

auto memory_device::seek(std::streamoff off, seek_dir way) -> bool
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

auto memory_device::read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    std::streamsize const totalSize {size_in_bytes()};
    std::streamsize const remaining {totalSize - _pos};
    if (remaining <= 0) { return 0; }

    std::streamsize const bytesToRead {std::min(sizeInBytes, remaining)};
    std::memcpy(s, _buf.data() + static_cast<usize>(_pos), static_cast<usize>(bytesToRead));
    _pos += bytesToRead;

    return bytesToRead;
}

auto memory_device::write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }

    if (std::ssize(_buf) < sizeInBytes + _pos) {
        _buf.resize(static_cast<usize>(sizeInBytes + _pos));
    }

    std::memcpy(_buf.data() + static_cast<usize>(_pos), s, static_cast<usize>(sizeInBytes));
    _pos += sizeInBytes;

    return sizeInBytes;
}

void memory_device::reserve(usize capacity)
{
    _buf.reserve(capacity);
}

auto memory_device::capacity() const -> usize
{
    return _buf.capacity();
}

////////////////////////////////////////////////////////////

iomstream::iomstream() = default;

auto iomstream::tell() const -> std::streamoff
{
    return _device.tell();
}

auto iomstream::seek(std::streamoff off, seek_dir way) -> bool
{
    return _device.seek(off, way);
}

void iomstream::reserve(usize capacity)
{
    _device.reserve(capacity);
}

auto iomstream::capacity() const -> usize
{
    return _device.capacity();
}

auto iomstream::get_source() -> memory_device*
{
    return &_device;
}

auto iomstream::get_source() const -> memory_device const*
{
    return &_device;
}

auto iomstream::get_sink() -> memory_device*
{
    return &_device;
}

auto iomstream::get_sink() const -> memory_device const*
{
    return &_device;
}

}

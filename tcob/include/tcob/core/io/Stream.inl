// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Stream.hpp"

#include "tcob/core/Common.hpp"

namespace tcob::io {

template <POD T, std::endian Endianess>
inline auto istream::read() -> T
{
    T s {};
    read_bytes(&s, sizeof(T));
    if constexpr (Endianess != std::endian::native) {
        s = helper::byteswap(s);
    }
    return s;
}

template <POD T>
inline auto istream::read_to(std::span<T> target) -> std::streamsize
{
    return read_bytes(target.data(), static_cast<std::streamsize>(target.size_bytes()));
}

template <POD T>
inline auto istream::read_n(std::streamsize n) -> std::vector<T>
{
    std::vector<T> retValue {};
    retValue.resize(static_cast<usize>(n));
    retValue.resize(static_cast<usize>(read_bytes(retValue.data(), n)));
    return retValue;
}

template <POD T>
inline auto istream::read_filtered(std::streamsize n, auto&&... filters) -> std::vector<T>
{
    auto vec {read_n<T>(n)};
    ((vec = filters.from(vec)), ...);
    return vec;
}

template <POD T>
inline auto istream::read_all() -> std::vector<T>
{
    auto constexpr size {static_cast<std::streamsize>(sizeof(T))};
    std::vector<T> retValue {};
    retValue.reserve(static_cast<usize>((size_in_bytes() - tell()) / size));

    if constexpr (size <= 8) {
        std::array<T, 1024> buffer {};
        do {
            auto constexpr bufferSize {static_cast<std::streamsize>(buffer.size() * size)};
            auto const readItems {static_cast<std::ptrdiff_t>(read_bytes(buffer.data(), bufferSize) / size)};
            retValue.insert(retValue.end(), buffer.begin(), buffer.begin() + readItems);
        } while (!is_eof());
    } else {
        do {
            retValue.push_back(read<T>());
        } while (!is_eof());
    }

    return retValue;
}

template <typename T>
inline auto operator>>(istream& is, T& m) -> istream&
{
    m = is.template read<T>();
    return is;
}

////////////////////////////////////////////////////////////

template <ISink Sink>
inline sink_istream<Sink>::sink_istream(Sink* sink)
    : _sink {sink}
{
}

template <ISink Sink>
inline auto sink_istream<Sink>::size_in_bytes() const -> std::streamsize
{
    return _sink->size_in_bytes();
}

template <ISink Sink>
inline auto sink_istream<Sink>::is_eof() const -> bool
{
    return _sink->is_eof();
}

template <ISink Sink>
inline auto sink_istream<Sink>::read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize
{
    return _sink->read_bytes(s, sizeInBytes);
}

template <ISink Sink>
inline auto sink_istream<Sink>::tell() const -> std::streamsize
{
    return _sink->tell();
}

template <ISink Sink>
inline auto sink_istream<Sink>::seek(std::streamoff off, seek_dir way) -> bool
{
    return _sink->seek(off, way);
}

////////////////////////////////////////////////////////////

template <NotStringLikePOD T, std::endian Endianess>
inline auto ostream::write(T s) -> std::streamsize
{
    if constexpr (Endianess != std::endian::native) {
        s = helper::byteswap(s);
    }

    return write_bytes(&s, sizeof(T));
}

template <StringLike T>
inline auto ostream::write(T s) -> std::streamsize
{
    return write_string(s);
}

template <POD T>
inline auto ostream::write(std::span<T const> s) -> std::streamsize
{
    return write_bytes(s.data(), static_cast<std::streamsize>(s.size_bytes()));
}

template <POD T>
inline auto ostream::write_filtered(std::span<T const> s, auto&& filter, auto&&... filters) -> std::streamsize
{
    auto vec {filter.to(s)};
    if constexpr (sizeof...(filters) > 0) {
        return write_filtered<T>(vec, filters...);
    }

    return write<T>(vec);
}

template <typename T>
inline auto operator<<(ostream& os, T const& m) -> ostream&
{
    os.write(m);
    return os;
}

////////////////////////////////////////////////////////////

template <OSink Sink>
inline sink_ostream<Sink>::sink_ostream(Sink* sink)
    : _sink {sink}
{
}

template <OSink Sink>
inline auto sink_ostream<Sink>::write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize
{
    if (sizeInBytes == 0) {
        return 0;
    }

    return _sink->write_bytes(s, sizeInBytes);
}

template <OSink Sink>
inline auto sink_ostream<Sink>::tell() const -> std::streamsize
{
    return _sink->tell();
}

template <OSink Sink>
inline auto sink_ostream<Sink>::seek(std::streamoff off, seek_dir way) -> bool
{
    return _sink->seek(off, way);
}

}

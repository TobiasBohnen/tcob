// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Stream.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <ios>
#include <span>
#include <vector>

namespace tcob::io {

template <POD T, std::endian Endianess>
inline auto istream::read() -> T
{
    T s {};
    read_bytes(&s, sizeof(T));
    if constexpr (Integral<T> && Endianess != std::endian::native) {
        s = std::byteswap(s);
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
    std::vector<T> retValue(static_cast<usize>(n));
    retValue.resize(static_cast<usize>(read_bytes(retValue.data(), n * sizeof(T)) / sizeof(T)));
    return retValue;
}

template <POD T, std::streamsize Size>
inline auto istream::read_n() -> std::array<T, Size>
{
    std::array<T, Size> retValue;
    read_bytes(retValue.data(), Size * sizeof(T));
    return retValue;
}

inline auto istream::read_filtered(std::streamsize n, auto&&... filters) -> std::vector<std::byte>
{
    auto vec {read_n<std::byte>(n)};
    ((vec = filters.from(vec)), ...);
    return vec;
}

template <POD T>
inline auto istream::read_all() -> std::vector<T>
{
    auto const length {static_cast<std::streamsize>((size_in_bytes() - tell()) / sizeof(T))};
    return read_n<T>(static_cast<std::streamsize>(length));
}

inline auto istream::read_string_until(auto&& pred) -> string
{
    string retValue;
    while (!is_eof()) {
        char const c {read<char>()};
        if (pred(c)) { break; }
        retValue += c;
    }

    return retValue;
}

template <POD T, std::endian Endianess>
inline auto istream::peek() -> T
{
    auto const pos {tell()};
    T          value {read<T, Endianess>()};
    seek(pos, seek_dir::Begin);
    return value;
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
inline auto sink_istream<Sink>::tell() const -> std::streamoff
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
    if constexpr (Integral<T> && Endianess != std::endian::native) {
        s = std::byteswap(s);
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

inline auto ostream::write_filtered(std::span<std::byte const> s, auto&& filter, auto&&... filters) -> std::streamsize
{
    auto vec {filter.to(s)};
    if constexpr (sizeof...(filters) > 0) {
        return write_filtered(vec, filters...);
    } else {
        return write<std::byte>(vec);
    }
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
inline auto sink_ostream<Sink>::tell() const -> std::streamoff
{
    return _sink->tell();
}

template <OSink Sink>
inline auto sink_ostream<Sink>::seek(std::streamoff off, seek_dir way) -> bool
{
    return _sink->seek(off, way);
}

}

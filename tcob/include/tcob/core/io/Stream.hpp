// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ios>
#include <span>
#include <vector>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/Interfaces.hpp"

namespace tcob::io {

////////////////////////////////////////////////////////////

enum class seek_dir {
    Begin   = std::ios::beg,
    Current = std::ios::cur,
    End     = std::ios::end
};

////////////////////////////////////////////////////////////

class TCOB_API istream : public non_copyable {
public:
    virtual ~istream() = default;

    template <POD T>
    auto read() -> T;

    template <POD T>
    auto read(std::endian endianness) -> T;

    template <POD T>
    auto read_to(std::span<T> target) -> std::streamsize;
    template <POD T>
    auto read_n(std::streamsize n) -> std::vector<T>;
    template <POD T>
    auto read_all() -> std::vector<T>;

    auto read_string(std::streamsize length) -> string;
    auto read_string_until(char delim) -> string;

    auto virtual tell() const -> std::streamsize                = 0;
    auto virtual seek(std::streamoff off, seek_dir way) -> bool = 0;
    auto virtual size_in_bytes() const -> std::streamsize       = 0;
    auto virtual is_eof() const -> bool                         = 0;

protected:
    auto virtual read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize = 0;
};

template <typename T>
inline auto operator>>(istream& is, T& m) -> istream&;

////////////////////////////////////////////////////////////

template <typename T>
concept ISink =
    requires(T& t, void* s, std::streamsize sib, std::streamoff off, seek_dir way) {
        {
            t.size_in_bytes()
        } -> std::same_as<std::streamsize>;
        {
            t.is_eof()
        } -> std::same_as<bool>;
        {
            t.read_bytes(s, sib)
        } -> std::same_as<std::streamsize>;
        {
            t.tell()
        } -> std::same_as<std::streamsize>;
        {
            t.seek(off, way)
        } -> std::same_as<bool>;
    };

////////////////////////////////////////////////////////////

template <ISink Sink>
class sink_istream : public istream {
public:
    explicit sink_istream(Sink* sink);

    auto size_in_bytes() const -> std::streamsize override;
    auto is_eof() const -> bool override;

    auto tell() const -> std::streamsize override;
    auto seek(std::streamoff off, seek_dir way) -> bool override;

protected:
    auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize override;

private:
    Sink* _sink;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API ostream : public non_copyable {
public:
    virtual ~ostream() = default;

    template <NotStringLikePOD T>
    auto write(T s) -> std::streamsize;

    template <NotStringLikePOD T>
    auto write(T s, std::endian endianness) -> std::streamsize;

    template <StringLike T>
    auto write(T s) -> std::streamsize;

    template <POD T>
    auto write(std::span<T const> s) -> std::streamsize;

    auto virtual tell() const -> std::streamsize                = 0;
    auto virtual seek(std::streamoff off, seek_dir way) -> bool = 0;
    // TODO: add flush
protected:
    auto virtual write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize = 0;
    auto write_string(string_view s) -> std::streamsize;
};

template <typename T>
inline auto operator<<(ostream& os, T const& m) -> ostream&;

////////////////////////////////////////////////////////////

template <typename T>
concept OSink =
    requires(T& t, void const* cs, std::streamsize sib, std::streamoff off, seek_dir way) {
        {
            t.write_bytes(cs, sib)
        } -> std::same_as<std::streamsize>;
        {
            t.tell()
        } -> std::same_as<std::streamsize>;
        {
            t.seek(off, way)
        } -> std::same_as<bool>;
    };

////////////////////////////////////////////////////////////

template <OSink Sink>
class sink_ostream : public ostream {
public:
    explicit sink_ostream(Sink* sink);

    auto tell() const -> std::streamsize override;
    auto seek(std::streamoff off, seek_dir way) -> bool override;

protected:
    auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize override;

private:
    Sink* _sink;
};

}

namespace tcob {
using istream = io::istream;
using ostream = io::ostream;
}

#include "Stream.inl"

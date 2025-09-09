// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <bit>
#include <ios>
#include <span>
#include <vector>

#include "tcob/core/Concepts.hpp"
#include "tcob/core/Interfaces.hpp"

namespace tcob::io {

////////////////////////////////////////////////////////////

enum class seek_dir : u8 {
    Begin   = std::ios::beg,
    Current = std::ios::cur,
    End     = std::ios::end
};

////////////////////////////////////////////////////////////

class TCOB_API istream : public non_copyable {
public:
    virtual ~istream() = default;

    template <POD T, std::endian Endianess = std::endian::native>
    auto read() -> T;

    template <POD T>
    auto read_to(std::span<T> target) -> std::streamsize;
    template <POD T>
    auto read_n(std::streamsize n) -> std::vector<T>;
    template <POD T, std::streamsize Size>
    auto read_n() -> std::array<T, Size>;
    template <POD T>
    auto read_filtered(std::streamsize n, auto&&... filters) -> std::vector<T>;

    template <POD T>
    auto read_all() -> std::vector<T>;

    auto read_string(std::streamsize length) -> string;
    auto read_string_until(char delim) -> string;

    virtual auto tell() const -> std::streamsize                = 0;
    virtual auto seek(std::streamoff off, seek_dir way) -> bool = 0;
    virtual auto size_in_bytes() const -> std::streamsize       = 0;
    virtual auto is_eof() const -> bool                         = 0;

    explicit     operator bool() const;
    virtual auto is_valid() const -> bool;

protected:
    virtual auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize = 0;
};

template <typename T>
inline auto operator>>(istream& is, T& m) -> istream&;

////////////////////////////////////////////////////////////

template <typename T>
concept ISink =
    requires(T& t, void* s, std::streamsize sib, std::streamoff off, seek_dir way) {
        { t.size_in_bytes() } -> std::same_as<std::streamsize>;
        { t.is_eof() } -> std::same_as<bool>;
        { t.read_bytes(s, sib) } -> std::same_as<std::streamsize>;
        { t.tell() } -> std::same_as<std::streamsize>;
        { t.seek(off, way) } -> std::same_as<bool>;
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

    template <NotStringLikePOD T, std::endian Endianess = std::endian::native>
    auto write(T s) -> std::streamsize;

    template <StringLike T>
    auto write(T s) -> std::streamsize;

    template <POD T>
    auto write(std::span<T const> s) -> std::streamsize;

    template <POD T>
    auto write_filtered(std::span<T const> s, auto&& filter, auto&&... filters) -> std::streamsize;

    virtual auto tell() const -> std::streamsize                = 0;
    virtual auto seek(std::streamoff off, seek_dir way) -> bool = 0;
    // TODO: add flush
protected:
    virtual auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize = 0;
    auto         write_string(string_view s) -> std::streamsize;
};

template <typename T>
inline auto operator<<(ostream& os, T const& m) -> ostream&;

////////////////////////////////////////////////////////////

template <typename T>
concept OSink =
    requires(T& t, void const* cs, std::streamsize sib, std::streamoff off, seek_dir way) {
        { t.write_bytes(cs, sib) } -> std::same_as<std::streamsize>;
        { t.tell() } -> std::same_as<std::streamsize>;
        { t.seek(off, way) } -> std::same_as<bool>;
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

#include "Stream.inl"

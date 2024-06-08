// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <span>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API ispan_sink final {
public:
    explicit ispan_sink(std::span<byte const> span);

    auto size_in_bytes() const -> std::streamsize;
    auto is_eof() const -> bool;

    auto tell() const -> std::streamsize;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize;

private:
    std::span<byte const> _span;
    std::streamsize       _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API isstream final : public sink_istream<ispan_sink> {
public:
    explicit isstream(std::span<byte const> span);

private:
    ispan_sink _sink;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API ospan_sink final {
public:
    explicit ospan_sink(std::span<byte> span);

    auto tell() const -> std::streamsize;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize;

private:
    std::span<byte> _span;
    std::streamsize _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API osstream final : public sink_ostream<ospan_sink> {
public:
    explicit osstream(std::span<byte> span);

private:
    ospan_sink _sink;
};

////////////////////////////////////////////////////////////

}

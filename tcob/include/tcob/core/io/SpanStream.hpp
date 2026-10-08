// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <ios>
#include <span>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API span_source final {
public:
    explicit span_source(std::span<std::byte const> span);

    auto size_in_bytes() const -> std::streamsize;
    auto is_eof() const -> bool;

    auto tell() const -> std::streamoff;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize;

private:
    std::span<std::byte const> _span;
    std::streamoff             _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API isstream final : public source_istream<span_source> {
public:
    explicit isstream(std::span<std::byte const> span);

protected:
    auto get_source() -> span_source* override;
    auto get_source() const -> span_source const* override;

private:
    span_source _source;
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCOB_API span_sink final {
public:
    explicit span_sink(std::span<std::byte> span);

    auto tell() const -> std::streamoff;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize;

private:
    std::span<std::byte> _span;
    std::streamoff       _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API osstream final : public sink_ostream<span_sink> {
public:
    explicit osstream(std::span<std::byte> span);

protected:
    auto get_sink() -> span_sink* override;
    auto get_sink() const -> span_sink const* override;

private:
    span_sink _sink;
};

////////////////////////////////////////////////////////////

}

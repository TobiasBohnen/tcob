// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API memory_sink final {
public:
    auto size_in_bytes() const -> std::streamsize;
    auto is_eof() const -> bool;

    auto tell() const -> std::streamsize;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize;
    auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize;

private:
    std::vector<byte> _buf;
    std::streamsize   _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API iomstream final : public sink_istream<memory_sink>, public sink_ostream<memory_sink> {
public:
    iomstream();

    auto tell() const -> std::streamsize override;
    auto seek(std::streamoff off, seek_dir way) -> bool override;

private:
    memory_sink _sink {};
};

////////////////////////////////////////////////////////////

}

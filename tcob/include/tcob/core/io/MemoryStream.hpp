// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <cstddef>
#include <ios>
#include <vector>

#include "tcob/core/io/Stream.hpp"

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API memory_device final {
public:
    auto size_in_bytes() const -> std::streamsize;
    auto is_eof() const -> bool;

    auto tell() const -> std::streamoff;
    auto seek(std::streamoff off, seek_dir way) -> bool;

    auto read_bytes(void* s, std::streamsize sizeInBytes) -> std::streamsize;
    auto write_bytes(void const* s, std::streamsize sizeInBytes) -> std::streamsize;

    void reserve(usize capacity);
    auto capacity() const -> usize;

private:
    std::vector<std::byte> _buf;
    std::streamoff         _pos {0};
};

////////////////////////////////////////////////////////////

class TCOB_API iomstream final : public source_istream<memory_device>, public sink_ostream<memory_device> {
public:
    iomstream();

    auto tell() const -> std::streamoff override;
    auto seek(std::streamoff off, seek_dir way) -> bool override;

    void reserve(usize capacity);
    auto capacity() const -> usize;

protected:
    auto get_source() -> memory_device* override;
    auto get_source() const -> memory_device const* override;

    auto get_sink() -> memory_device* override;
    auto get_sink() const -> memory_device const* override;

private:
    memory_device _device {};
};

////////////////////////////////////////////////////////////

}

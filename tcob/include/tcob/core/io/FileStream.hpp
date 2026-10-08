// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <expected>
#include <ios>
#include <memory>

#include "tcob/core/io/Stream.hpp"

struct PHYSFS_File;

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API file_device {
public:
    virtual ~file_device() = default;

    virtual auto size_in_bytes() const -> std::streamsize = 0;
    virtual auto is_eof() const -> bool                   = 0;

    virtual auto close() -> bool       = 0;
    virtual auto flush() const -> bool = 0;

    virtual auto tell() const -> std::streamoff                       = 0;
    virtual auto seek(std::streamoff off, seek_dir way) const -> bool = 0;

    virtual auto read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize        = 0;
    virtual auto write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize = 0;

    virtual auto is_valid() const -> bool = 0;
};

////////////////////////////////////////////////////////////

enum class error_code : u8 {
    FileNotFound
};

////////////////////////////////////////////////////////////

class TCOB_API ifstream final : public source_istream<file_device> {
public:
    explicit ifstream(path const& path, u64 bufferSize = 4096);

    auto close() -> bool;

    auto is_valid() const -> bool override;

    static auto Open(path const& path, u64 bufferSize = 4096) -> std::expected<ifstream, error_code>;

protected:
    auto get_source() -> file_device* override;
    auto get_source() const -> file_device const* override;

private:
    std::unique_ptr<file_device> _source;
};

////////////////////////////////////////////////////////////

class TCOB_API ofstream final : public sink_ostream<file_device> {
public:
    explicit ofstream(path const& path, u64 bufferSize = 4096, bool append = false);

    auto close() -> bool;
    auto flush() -> bool;

protected:
    auto get_sink() -> file_device* override;
    auto get_sink() const -> file_device const* override;

private:
    std::unique_ptr<file_device> _sink;
};

////////////////////////////////////////////////////////////

}

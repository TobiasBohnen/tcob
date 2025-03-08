// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ios>
#include <optional>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/io/Stream.hpp"

struct PHYSFS_File;

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API file_sink final : public non_copyable {
public:
    file_sink(file_sink const&)                    = delete;
    auto operator=(file_sink const&) -> file_sink& = delete;
    file_sink(file_sink&& other) noexcept;
    auto operator=(file_sink&& other) noexcept -> file_sink&;
    ~file_sink();

    auto size_in_bytes() const -> std::streamsize;
    auto is_eof() const -> bool;

    auto close() -> bool;
    auto flush() const -> bool;

    auto tell() const -> std::streamsize;
    auto seek(std::streamoff off, seek_dir way) const -> bool;

    void set_buffer_size(u64 size);

    auto read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize;
    auto write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize;

    auto is_valid() const -> bool;

    auto static OpenRead(path const& path) -> file_sink;
    auto static OpenWrite(path const& path) -> file_sink;
    auto static OpenAppend(path const& path) -> file_sink;

private:
    explicit file_sink(PHYSFS_File* handle);

    PHYSFS_File* _handle {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API ifstream final : public sink_istream<file_sink> {
public:
    explicit ifstream(path const& path, u64 bufferSize = 4096);

    auto close() -> bool;
    auto flush() -> bool;

    auto is_valid() const -> bool override;

    auto static Open(path const& path, u64 bufferSize = 4096) -> std::optional<ifstream>; // TODO: change to result

private:
    file_sink _sink;
};

////////////////////////////////////////////////////////////

class TCOB_API ofstream final : public sink_ostream<file_sink> {
public:
    explicit ofstream(path const& path, u64 bufferSize = 4096, bool append = false);

    auto close() -> bool;
    auto flush() -> bool;

private:
    file_sink _sink;
};

////////////////////////////////////////////////////////////

}

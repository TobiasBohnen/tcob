// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <ios>
#include <memory>
#include <unordered_set>

#include "tcob/core/Interfaces.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API physfs_file_sink final : public file_sink, public non_copyable {
public:
    physfs_file_sink(PHYSFS_File* handle, usize bufferSize);
    ~physfs_file_sink() override;

    auto size_in_bytes() const -> std::streamsize override;
    auto is_eof() const -> bool override;

    auto close() -> bool override;
    auto flush() const -> bool override;

    auto tell() const -> std::streamoff override;
    auto seek(std::streamoff off, io::seek_dir way) const -> bool override;

    auto read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize override;
    auto write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize override;

    auto is_valid() const -> bool override;

private:
    void set_buffer_size(usize size);

    PHYSFS_File* _handle {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API physfs_file_system final : public file_system {
public:
    physfs_file_system(string const& name, string const& orgName);
    ~physfs_file_system() override;

    auto mount(path const& folderOrArchive, string const& mp) -> bool override;
    auto unmount(path const& folderOrArchive) -> bool override;

    auto is_folder_empty(path const& folder) -> bool override;

    auto get_stat(path const& fileOrFolder) -> stat override;

    auto exists(path const& fileOrFolder) -> bool override;

    auto delete_file(path const& file) -> bool override;

    auto delete_folder(path const& folder) -> bool override;

    auto create_file(path const& file) -> bool override;

    auto create_folder(path const& folder) -> bool override;

    auto enumerate(path const& folder, pattern const& pattern, bool recursive) -> std::unordered_set<string> override;

    auto get_sub_folders(path const& folder) -> std::unordered_set<string> override;

    auto open_read(path const& path, usize bufferSize) -> std::unique_ptr<file_sink> override;
    auto open_write(path const& path, usize bufferSize) -> std::unique_ptr<file_sink> override;
    auto open_append(path const& path, usize bufferSize) -> std::unique_ptr<file_sink> override;
};

}

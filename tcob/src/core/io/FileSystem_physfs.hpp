// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/io/FileSystem.hpp"
#include <unordered_set>

namespace tcob::io {
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
};

}

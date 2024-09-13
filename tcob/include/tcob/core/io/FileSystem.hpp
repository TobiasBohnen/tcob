// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <unordered_set>

namespace tcob::io {
////////////////////////////////////////////////////////////

class TCOB_API file_hasher final {
public:
    explicit file_hasher(path file);

    auto crc32 [[nodiscard]] () const -> u32;

private:
    path _path;
};

////////////////////////////////////////////////////////////

enum class file_type : u8 {
    File,
    Folder,
    Symlink,
    Other
};

struct stat {
    i64       FileSize {0};
    i64       ModTime {0};
    i64       CreateTime {0};
    i64       AccessTime {0};
    file_type Type {};
    bool      ReadOnly {false};
};

struct pattern {
    string String {"*.*"};
    bool   MatchWholePath {true};
};

////////////////////////////////////////////////////////////
namespace detail {
    TCOB_API void simple_init(char const* argv0);
    TCOB_API void init(char const* argv0, string const& name, string const& orgName);
    TCOB_API void done();
}

TCOB_API auto mount(path const& folderOrArchive, string const& mp) -> bool;
TCOB_API auto unmount(path const& folderOrArchive) -> bool;

TCOB_API auto create_file(path const& file) -> bool;
TCOB_API auto create_folder(path const& folder) -> bool;

TCOB_API auto delete_file(path const& file) -> bool;
TCOB_API auto delete_folder(path const& folder) -> bool;

TCOB_API auto is_file(path const& file) -> bool;
TCOB_API auto is_folder(path const& folder) -> bool;
TCOB_API auto get_stat(path const& fileOrFolder) -> stat;

TCOB_API auto exists(path const& fileOrFolder) -> bool;

TCOB_API auto is_folder_empty(path const& folder) -> bool;

TCOB_API auto enumerate(path const& folder, pattern const& pattern = {}, bool recursive = true) -> std::unordered_set<string>;
TCOB_API auto get_sub_folders(path const& folder) -> std::unordered_set<string>;

TCOB_API auto read_as_string(path const& file) -> string;

TCOB_API auto zip(path const& srcFileOrFolder, path const& dstFile, bool relative = false, i32 level = -1) -> bool;
TCOB_API auto unzip(path const& srcFile, path const& dstFolder) -> bool;

TCOB_API auto get_file_size(path const& file) -> i64;

TCOB_API auto get_extension(path const& file) -> string;
TCOB_API auto get_stem(path const& file) -> string;
TCOB_API auto get_filename(path const& file) -> string;
TCOB_API auto get_parent_folder(path const& file) -> string;
}

// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include <tcob/tcob_config.hpp>

namespace tcob::FileSystem {
enum class FileType {
    File,
    Folder,
    Symlink,
    Other
};
struct Stat {
    i64 FileSize;
    i64 ModTime;
    i64 CreateTime;
    i64 AccessTime;
    FileType Type;
    bool ReadOnly;
};

void init(const char* argv0, const std::string& name);
void done();

void mount(const std::string& loc, const std::string& mp);
void unmount(const std::string& loc);

void create_file(const std::string& path);
void create_folder(const std::string& path);

void delete_file(const std::string& path);
void delete_folder(const std::string& path);

auto is_file(const std::string& path) -> bool;
auto is_folder(const std::string& path) -> bool;
auto stat(const std::string& path) -> Stat;

auto exists(const std::string& path) -> bool;
auto filesize(const std::string& path) -> i64;

auto enumerate(const std::string& dir, const std::string& pattern = "*.*", bool recursive = true) -> const std::vector<std::string>;

auto read_as_string(const std::string& path) -> std::string;

auto extension(const std::string& path) -> std::string;
auto stem(const std::string& path) -> std::string;
}
// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/io/FileSystem.hpp>

#include <filesystem>
#include <physfs.h>

#include <tcob/core/io/FileStream.hpp>
#include <tcob/core/io/Logger.hpp>

namespace tcob {
inline auto check(const std::string& msg, i32 c) -> bool
{
    if (c == 0) {
        Log(msg + ": " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return c != 0;
}

void FileSystem::init(const char* argv0, const std::string& name)
{
    check("init", PHYSFS_init(argv0));
    check("setSaneConfig", PHYSFS_setSaneConfig("tcob", name.c_str(), "", 0, 0));
    mount(".", "/");
}

void FileSystem::done()
{
    Logger::Instance().done();
    PHYSFS_deinit();
}

void FileSystem::mount(const std::string& loc, const std::string& mp)
{
    check("mount", PHYSFS_mount(loc.c_str(), mp.c_str(), true));
}

void FileSystem::unmount(const std::string& loc)
{
    check("ummount", PHYSFS_unmount(loc.c_str()));
}

auto FileSystem::filesize(const std::string& path) -> i64
{
    if (!is_file(path)) {
        return -1;
    }

    return stat(path).FileSize;
}

auto FileSystem::read_as_string(const std::string& path) -> std::string
{
    if (!exists(path)) {
        return "";
    }

    std::string retValue;
    retValue.reserve(filesize(path));

    PHYSFS_File* handle { PHYSFS_openRead(path.c_str()) };
    i64 read { 0 };
    do {
        std::array<byte, 1024> buffer {};
        read = PHYSFS_readBytes(handle, buffer.data(), buffer.size());
        retValue.append(buffer.data(), read);
    } while (read != 0);

    check("close", PHYSFS_close(handle));
    return retValue;
}

auto FileSystem::extension(const std::string& path) -> std::string
{
    return std::filesystem::path { path }.extension().string();
}

auto FileSystem::stem(const std::string& path) -> std::string
{
    return std::filesystem::path { path }.stem().string();
}

auto FileSystem::is_file(const std::string& path) -> bool
{
    if (!exists(path)) {
        return false;
    }

    return stat(path).Type == FileType::File;
}

auto FileSystem::is_folder(const std::string& path) -> bool
{
    if (!exists(path)) {
        return false;
    }

    return stat(path).Type == FileType::Folder;
}

auto FileSystem::stat(const std::string& path) -> Stat
{
    PHYSFS_Stat stat;
    if (check("stat", PHYSFS_stat(path.c_str(), &stat))) {
        FileType type;
        switch (stat.filetype) {
        case PHYSFS_FILETYPE_REGULAR:
            type = FileType::File;
            break;
        case PHYSFS_FILETYPE_DIRECTORY:
            type = FileType::Folder;
            break;
        case PHYSFS_FILETYPE_SYMLINK:
            type = FileType::Symlink;
            break;
        default:
            type = FileType::Other;
            break;
        }

        return {
            .FileSize = stat.filesize,
            .ModTime = stat.modtime,
            .CreateTime = stat.createtime,
            .AccessTime = stat.accesstime,
            .Type = type,
            .ReadOnly = stat.readonly != 0
        };
    }

    return {};
}

auto FileSystem::exists(const std::string& path) -> bool
{
    return PHYSFS_exists(path.c_str());
}

void FileSystem::delete_file(const std::string& path)
{
    if (!is_file(path)) {
        return;
    }

    check("delete", PHYSFS_delete(path.c_str()));
}

void FileSystem::delete_folder(const std::string& path)
{
    if (!is_folder(path)) {
        return;
    }

    const char* folder { path.c_str() };
    char** items { PHYSFS_enumerateFiles(folder) };
    for (char** item { items }; *item != nullptr; item++) {
        std::string file { path + "/" + *item };
        if (is_folder(file)) {
            delete_folder(file);
        } else {
            delete_file(file);
        }
    }

    PHYSFS_freeList(items);
    check("delete", PHYSFS_delete(folder));
}

void FileSystem::create_file(const std::string& path)
{
    delete_file(path);

    const isize idx { path.find_last_of('/') };
    if (idx != std::string::npos && idx > 0) {
        std::string folder { path.substr(0, idx) };
        if (!exists(folder)) {
            create_folder(folder);
        }
    }

    PHYSFS_File* handle { PHYSFS_openWrite(path.c_str()) };
    PHYSFS_close(handle);
}

void FileSystem::create_folder(const std::string& path)
{
    check("create folder", PHYSFS_mkdir(path.c_str()));
}

struct CallbackData {
    std::vector<std::string> files;
    std::vector<std::string> folders;
    std::string pattern;
};

//based on: https://www.geeksforgeeks.org/wildcard-pattern-matching/
auto wildcard_match(std::string_view str, std::string_view pattern) -> bool
{
    if (pattern.empty())
        return str.empty();

    auto n { str.size() };
    auto m { pattern.size() };

    std::vector<bool> lookup((n + 1) * (m + 1), false);

    lookup[0] = true;

    for (isize j { 1 }; j <= m; j++)
        if (pattern[j - 1] == '*')
            lookup[j] = lookup[j - 1];

    for (isize i { 1 }; i <= n; ++i) {
        for (isize j { 1 }; j <= m; j++) {
            if (pattern[j - 1] == '*')
                lookup[i * m + j] = lookup[i * m + j - 1] || lookup[(i - 1) * m + j];
            else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1])
                lookup[i * m + j] = lookup[(i - 1) * m + j - 1];
            else
                lookup[i * m + j] = false;
        }
    }

    return lookup[n * m + m];
}

auto enumCallback(void* data, const char* origdir, const char* fname) -> PHYSFS_EnumerateCallbackResult
{
    auto* cd { static_cast<CallbackData*>(data) };
    std::string entry { std::string(origdir) + "/" + std::string(fname) };

    switch (FileSystem::stat(entry).Type) {
    case FileSystem::FileType::File:
        if (wildcard_match(entry, cd->pattern)) {
            cd->files.push_back(entry);
        }
        break;
    case FileSystem::FileType::Folder:
        cd->folders.push_back(entry);
        break;
    default:
        break;
    }

    return PHYSFS_ENUM_OK;
}

auto FileSystem::enumerate(const std::string& dir, const std::string& pattern, bool recursive) -> const std::vector<std::string>
{
    CallbackData cd;
    cd.pattern = pattern;
    PHYSFS_enumerate(dir.c_str(), &enumCallback, &cd);
    if (recursive) {
        for (const auto& folder : cd.folders) {
            auto files { enumerate(folder, pattern, true) };
            cd.files.insert(cd.files.end(), files.begin(), files.end());
        }
    }

    return cd.files;
}
}
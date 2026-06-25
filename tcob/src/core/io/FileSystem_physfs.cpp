// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FileSystem_physfs.hpp"

#include <filesystem>
#include <format>
#include <ios>
#include <memory>
#include <unordered_set>

#include <physfs.h>

#include "Archiver_tar.hpp"

#include "tcob/core/Logger.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/FileSystem.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::io {

static auto Check(string const& msg, i32 c) -> bool
{
    if (c == 0) {
        logger::Error("{}: {}", msg, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return c != 0;
}
extern "C" {
struct callback_data {
    std::unordered_set<string> Files;
    std::unordered_set<string> Folders;
    pattern                    Pattern;
};

static auto EnumerateCallback(void* data, char const* origdir, char const* fname) -> PHYSFS_EnumerateCallbackResult
{
    if (!origdir || !fname) { return PHYSFS_ENUM_ERROR; }

    auto* cd {static_cast<callback_data*>(data)};

    string       folder {origdir};
    string const file {fname};
    if (!folder.ends_with("/")) { folder += "/"; }
    string const entry {folder == "/" ? file : folder + file};

    switch (get_stat(entry).Type) {
    case file_type::File: {
        if (cd->Pattern.String == "*.*" || helper::wildcard_match(cd->Pattern.MatchWholePath ? entry : file, cd->Pattern.String)) {
            cd->Files.insert(entry);
        }
    } break;
    case file_type::Folder: cd->Folders.insert(entry); break;
    default:                break;
    }

    return PHYSFS_ENUM_OK;
}

static auto EmptyEnumCallback(void* data, char const*, char const*) -> PHYSFS_EnumerateCallbackResult
{
    bool* ok {static_cast<bool*>(data)};
    *ok = false;

    return PHYSFS_ENUM_STOP;
}
}

////////////////////////////////////////////////////////////

physfs_file_sink::physfs_file_sink(PHYSFS_File* handle, usize bufferSize)
    : _handle {handle}
{
    set_buffer_size(bufferSize);
}

physfs_file_sink::~physfs_file_sink()
{
    if (_handle) { close(); }
}

auto physfs_file_sink::close() -> bool
{
    if (!is_valid()) { return false; }

    if (Check("close", PHYSFS_close(_handle))) {
        _handle = nullptr;
        return true;
    }

    return false;
}

auto physfs_file_sink::flush() const -> bool
{
    if (!is_valid()) { return false; }
    return Check("flush", PHYSFS_flush(_handle));
}

auto physfs_file_sink::is_eof() const -> bool
{
    if (!is_valid()) { return true; }
    return PHYSFS_eof(_handle) != 0;
}

auto physfs_file_sink::tell() const -> std::streamoff
{
    if (!is_valid()) { return 0; }
    return static_cast<std::streamoff>(PHYSFS_tell(_handle));
}

auto physfs_file_sink::size_in_bytes() const -> std::streamsize
{
    if (!is_valid()) { return 0; }
    return static_cast<std::streamsize>(PHYSFS_fileLength(_handle));
}

auto physfs_file_sink::seek(std::streamoff off, seek_dir way) const -> bool
{
    if (!is_valid()) { return false; }

    PHYSFS_sint64 pos {off};
    if (way == seek_dir::Current) {
        pos = PHYSFS_tell(_handle) + off;
    } else if (way == seek_dir::End) {
        pos = PHYSFS_fileLength(_handle) + off;
    }

    if (pos < 0) { return false; }

    return Check("seek", PHYSFS_seek(_handle, static_cast<PHYSFS_uint64>(pos)));
}

void physfs_file_sink::set_buffer_size(usize size)
{
    if (!is_valid()) { return; }
    Check("set_buffer_size", PHYSFS_setBuffer(_handle, size));
}

auto physfs_file_sink::read_bytes(void* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }
    if (!is_valid()) { return 0; }

    return static_cast<std::streamsize>(PHYSFS_readBytes(_handle, s, static_cast<u64>(sizeInBytes)));
}

auto physfs_file_sink::write_bytes(void const* s, std::streamsize sizeInBytes) const -> std::streamsize
{
    if (s == nullptr || sizeInBytes <= 0) { return 0; }
    if (!is_valid()) { return 0; }

    auto const retValue {static_cast<std::streamsize>(PHYSFS_writeBytes(_handle, s, static_cast<u64>(sizeInBytes)))};
    if (retValue != sizeInBytes) {
        logger::Error("write_bytes: " + string {PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())});
    }
    return retValue;
}

auto physfs_file_sink::is_valid() const -> bool
{
    return _handle != nullptr;
}

////////////////////////////////////////////////////////////

physfs_file_system::physfs_file_system(string const& name, string const& orgName)
{
    auto const cp {std::filesystem::current_path()};
    Check("init", PHYSFS_init(reinterpret_cast<char const*>(cp.c_str())));
    if (!orgName.empty() && !name.empty()) {
        Check("setSaneConfig", PHYSFS_setSaneConfig(orgName.c_str(), name.c_str(), "", 0, 0));
    } else {
        Check("setWriteDir", PHYSFS_setWriteDir("."));
    }
    PHYSFS_init_tar();
    mount(".", "/");
}

physfs_file_system::~physfs_file_system()
{
    PHYSFS_deinit();
}

auto physfs_file_system::mount(path const& folderOrArchive, string const& mp) -> bool
{
    return Check(std::format("mount (mount point: {}, folder: {})", mp, folderOrArchive), PHYSFS_mount(folderOrArchive.c_str(), mp.c_str(), true));
}

auto physfs_file_system::unmount(path const& folderOrArchive) -> bool
{
    return Check("ummount", PHYSFS_unmount(folderOrArchive.c_str()));
}

auto physfs_file_system::is_folder_empty(path const& folder) -> bool
{
    bool retValue {true};
    PHYSFS_enumerate(folder.c_str(), &EmptyEnumCallback, &retValue);
    return retValue;
}

auto physfs_file_system::get_stat(path const& fileOrFolder) -> stat
{
    PHYSFS_Stat stat;
    if (Check("stat", PHYSFS_stat(fileOrFolder.c_str(), &stat))) {
        file_type type {};
        switch (stat.filetype) {
        case PHYSFS_FILETYPE_REGULAR:
            type = file_type::File;
            break;
        case PHYSFS_FILETYPE_DIRECTORY:
            type = file_type::Folder;
            break;
        case PHYSFS_FILETYPE_SYMLINK:
            type = file_type::Symlink;
            break;
        default:
            type = file_type::Other;
            break;
        }

        return {
            .FileSize   = stat.filesize,
            .ModTime    = stat.modtime,
            .CreateTime = stat.createtime,
            .AccessTime = stat.accesstime,
            .Type       = type,
            .ReadOnly   = stat.readonly != 0};
    }

    return {};
}

auto physfs_file_system::exists(path const& fileOrFolder) -> bool
{
    return PHYSFS_exists(fileOrFolder.c_str());
}

auto physfs_file_system::delete_file(path const& file) -> bool
{
    return Check(std::format("delete file ({})", file), PHYSFS_delete(file.c_str()));
}

auto physfs_file_system::delete_folder(path const& folder) -> bool
{
    char** items {PHYSFS_enumerateFiles(folder.c_str())};
    for (char** item {items}; *item != nullptr; item++) {
        path const file {folder + "/" + *item};
        if (is_folder(file)) {
            delete_folder(file);
        } else {
            delete_file(file);
        }
    }

    PHYSFS_freeList(items);
    return Check(std::format("delete folder ({})", folder), PHYSFS_delete(folder.c_str()));
}

auto physfs_file_system::create_file(path const& file) -> bool
{
    return PHYSFS_close(PHYSFS_openWrite(file.c_str())) != 0;
}

auto physfs_file_system::create_folder(path const& folder) -> bool
{
    return Check(std::format("create folder ({})", folder), PHYSFS_mkdir(folder.c_str()));
}

auto physfs_file_system::enumerate(path const& folder, pattern const& pattern, bool recursive) -> std::unordered_set<string>
{
    callback_data cd;
    cd.Pattern = pattern;
    PHYSFS_enumerate(folder.c_str(), &EnumerateCallback, &cd);
    if (recursive) {
        for (auto const& f : cd.Folders) {
            auto files {enumerate(f, pattern, true)};
            cd.Files.insert(files.begin(), files.end());
        }
    }

    return cd.Files;
}

auto physfs_file_system::get_sub_folders(path const& folder) -> std::unordered_set<string>
{
    callback_data cd;
    PHYSFS_enumerate(folder.c_str(), &EnumerateCallback, &cd);

    return cd.Folders;
}

auto physfs_file_system::open_read(path const& path, usize bufferSize) -> std::unique_ptr<file_sink>
{
    return std::make_unique<physfs_file_sink>(PHYSFS_openRead(path.c_str()), bufferSize);
}
auto physfs_file_system::open_write(path const& path, usize bufferSize) -> std::unique_ptr<file_sink>
{
    return std::make_unique<physfs_file_sink>(PHYSFS_openWrite(path.c_str()), bufferSize);
}
auto physfs_file_system::open_append(path const& path, usize bufferSize) -> std::unique_ptr<file_sink>
{
    return std::make_unique<physfs_file_sink>(PHYSFS_openAppend(path.c_str()), bufferSize);
}

}

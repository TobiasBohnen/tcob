// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/FileSystem.hpp"

#include <filesystem>
#include <vector>

#include <miniz/miniz.h>
#undef crc32
#include <physfs.h>

#include "tcob/core/Logger.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileStream.hpp"

namespace tcob::io {

file_hasher::file_hasher(path file)
    : _path {std::move(file)}
{
}

auto file_hasher::crc32() const -> u32
{
    if (!is_file(_path)) {
        return 0;
    }

    ifstream           fs {_path};
    std::vector<ubyte> fileData {fs.read_all<ubyte>()};
    return mz_crc32(MZ_CRC32_INIT, fileData.data(), fileData.size());
}

auto static check(string const& msg, i32 c) -> bool
{
    if (c == 0) {
        logger::Error(msg + ": " + PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    }

    return c != 0;
}
extern "C" {
struct callback_data {
    std::unordered_set<string> Files;
    std::unordered_set<string> Folders;
    pattern                    Pattern;
};

auto static EnumerateCallback(void* data, char const* origdir, char const* fname) -> PHYSFS_EnumerateCallbackResult
{
    if (!origdir || !fname) { return PHYSFS_ENUM_ERROR; }

    auto* cd {static_cast<callback_data*>(data)};

    string folder {origdir};
    string file {fname};
    if (!folder.ends_with("/")) { folder += "/"; }
    string const entry {folder == "/" ? file : folder + file};

    switch (get_stat(entry).Type) {
    case file_type::File: {
        if (cd->Pattern.String == "*.*" || helper::wildcard_match(cd->Pattern.MatchWholePath ? entry : file, cd->Pattern.String)) {
            cd->Files.insert(entry);
        }
    } break;
    case file_type::Folder:
        cd->Folders.insert(entry);
        break;
    default:
        break;
    }

    return PHYSFS_ENUM_OK;
}

auto static emptyEnumCallback(void* data, char const*, char const*) -> PHYSFS_EnumerateCallbackResult
{
    bool* ok {static_cast<bool*>(data)};
    *ok = false;

    return PHYSFS_ENUM_STOP;
}
}

////////////////////////////////////////////////////////////

void detail::simple_init(char const* argv0)
{
    check("init", PHYSFS_init(argv0));
    check("setWriteDir", PHYSFS_setWriteDir("."));
    mount(".", "/");
}

void detail::init(char const* argv0, string const& name, string const& orgName)
{
    check("init", PHYSFS_init(argv0));
    if (!orgName.empty() || !name.empty()) {
        check("setSaneConfig", PHYSFS_setSaneConfig(orgName.c_str(), name.c_str(), "", 0, 0));
    }
    mount(".", "/");
}

void detail::done()
{
    PHYSFS_deinit();
}

////////////////////////////////////////////////////////////
extern "C" {
auto static mz_read(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n) -> size_t
{
    auto* fs {static_cast<ifstream*>(pOpaque)};
    fs->seek(file_ofs, seek_dir::Begin);
    return static_cast<size_t>(fs->read_to<byte>({static_cast<byte*>(pBuf), n}));
}

auto static mz_write(void* pOpaque, mz_uint64 file_ofs, void const* pBuf, size_t n) -> size_t
{
    auto* fs {static_cast<ofstream*>(pOpaque)};
    fs->seek(file_ofs, seek_dir::Begin);
    return static_cast<size_t>(fs->write<byte>({static_cast<byte const*>(pBuf), n}));
}
}

////////////////////////////////////////////////////////////

auto zip(path const& srcFileOrFolder, path const& dstFile, bool relative, i32 level) -> bool
{
    ofstream stream {dstFile};

    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);
    zip.m_pWrite     = &mz_write;
    zip.m_pIO_opaque = &stream;

    if (!mz_zip_writer_init(&zip, 0)) { return false; }

    if (is_folder(srcFileOrFolder)) {
        auto files {enumerate(srcFileOrFolder)};
        for (auto const& file : files) {
            ifstream istream {file};
            string   name {relative ? file.substr(srcFileOrFolder.size()) : file};
            if (!mz_zip_writer_add_read_buf_callback(&zip, name.c_str(), &mz_read, &istream, istream.size_in_bytes(), nullptr, nullptr, 0, level, nullptr, 0, nullptr, 0)) {
                return false;
            }
        }
    } else if (is_file(srcFileOrFolder)) {
        ifstream istream {srcFileOrFolder};
        string   name {relative ? std::filesystem::path {srcFileOrFolder}.filename().string() : srcFileOrFolder};
        if (!mz_zip_writer_add_read_buf_callback(&zip, name.c_str(), &mz_read, &istream, istream.size_in_bytes(), nullptr, nullptr, 0, level, nullptr, 0, nullptr, 0)) {
            return false;
        }
    } else {
        return false;
    }

    if (!mz_zip_writer_finalize_archive(&zip)) { return false; }

    return mz_zip_writer_end(&zip);
}

auto unzip(path const& srcFile, path const& dstFolder) -> bool
{
    ifstream stream {srcFile};

    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);
    zip.m_pRead      = &mz_read;
    zip.m_pIO_opaque = &stream;

    if (!mz_zip_reader_init(&zip, stream.size_in_bytes(), 0)) { return false; }

    mz_uint const n {mz_zip_reader_get_num_files(&zip)};
    for (mz_uint i {0}; i < n; ++i) {
        std::array<char, 260> buf {};
        mz_zip_reader_get_filename(&zip, i, buf.data(), static_cast<mz_uint>(buf.size()));
        string const file {dstFolder.empty() ? string {buf.data()} : dstFolder + "/" + string {buf.data()}};
        create_file(file);
        ofstream ostream {file};
        mz_zip_reader_extract_to_callback(&zip, i, &mz_write, &ostream, 0);
    }

    return mz_zip_reader_end(&zip);
}

auto mount(path const& folderOrArchive, string const& mp) -> bool
{
    return check("mount", PHYSFS_mount(folderOrArchive.c_str(), mp.c_str(), true));
}

auto unmount(path const& folderOrArchive) -> bool
{
    return check("ummount", PHYSFS_unmount(folderOrArchive.c_str()));
}

auto get_file_size(path const& file) -> i64
{
    if (!is_file(file)) { return -1; }

    return get_stat(file).FileSize;
}

auto read_as_string(path const& file) -> string
{
    if (!is_file(file)) {
        return "";
    }

    string retValue {};
    retValue.reserve(static_cast<usize>(get_file_size(file)));

    PHYSFS_File* handle {PHYSFS_openRead(file.c_str())};
    i64          read {0};
    do {
        std::array<byte, 1024> buffer {};
        read = PHYSFS_readBytes(handle, buffer.data(), buffer.size());
        retValue.append(buffer.data(), read);
    } while (read != 0);

    check("close", PHYSFS_close(handle));
    return retValue;
}

auto get_extension(path const& file) -> string
{
    return std::filesystem::path {file}.extension().string();
}

auto get_stem(path const& file) -> string
{
    return std::filesystem::path {file}.stem().string();
}

auto get_parent_folder(path const& file) -> string
{
    return std::filesystem::path {file}.parent_path().string();
}

auto is_file(path const& file) -> bool
{
    if (!io::exists(file)) { return false; }

    return get_stat(file).Type == file_type::File;
}

auto is_folder(path const& folder) -> bool
{
    if (!io::exists(folder)) { return false; }

    return get_stat(folder).Type == file_type::Folder;
}

auto is_folder_empty(path const& folder) -> bool
{
    if (!is_folder(folder)) { return false; }

    bool retValue {true};
    PHYSFS_enumerate(folder.c_str(), &emptyEnumCallback, &retValue);
    return retValue;
}

auto get_stat(path const& fileOrFolder) -> stat
{
    PHYSFS_Stat stat;
    if (check("stat", PHYSFS_stat(fileOrFolder.c_str(), &stat))) {
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

auto exists(path const& fileOrFolder) -> bool
{
    return fileOrFolder.empty() ? false : PHYSFS_exists(fileOrFolder.c_str());
}

auto delete_file(path const& file) -> bool
{
    if (!is_file(file)) { return false; }

    return check("delete", PHYSFS_delete(file.c_str()));
}

auto delete_folder(path const& folder) -> bool
{
    if (!is_folder(folder)) { return false; }

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
    return check("delete", PHYSFS_delete(folder.c_str()));
}

auto create_file(path const& file) -> bool
{
    delete_file(file);

    usize const idx {file.find_last_of('/')};
    if (idx != string::npos && idx > 0) {
        string const folder {file.substr(0, idx)};
        if (!exists(folder)) {
            if (!create_folder(folder)) { return false; }
        }
    }

    return PHYSFS_close(PHYSFS_openWrite(file.c_str())) != 0;
}

auto create_folder(path const& folder) -> bool
{
    return check("create folder", PHYSFS_mkdir(folder.c_str()));
}

auto enumerate(path const& folder, pattern const& pattern, bool recursive) -> std::unordered_set<string>
{
    if (!is_folder(folder)) { return {}; }

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

}

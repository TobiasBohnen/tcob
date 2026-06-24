// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/io/FileSystem.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <unordered_set>
#include <utility>
#include <vector>

#include <miniz/miniz.h>
#undef crc32

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/io/FileStream.hpp"
#include "tcob/core/io/Stream.hpp"

namespace tcob::io {

file_hasher::file_hasher(path file)
    : _path {std::move(file)}
{
}

auto file_hasher::crc32() const -> u32
{
    if (!is_file(_path)) { return 0; }

    ifstream        fs {_path};
    std::vector<u8> fileData {fs.read_all<u8>()};
    return static_cast<u32>(mz_crc32(MZ_CRC32_INIT, fileData.data(), fileData.size()));
}

////////////////////////////////////////////////////////////
extern "C" {
static auto Read(void* pOpaque, mz_uint64 file_ofs, void* pBuf, size_t n) -> size_t
{
    auto* fs {static_cast<ifstream*>(pOpaque)};
    fs->seek(static_cast<std::streamoff>(file_ofs), seek_dir::Begin);
    return static_cast<size_t>(fs->read_to<std::byte>({static_cast<std::byte*>(pBuf), n}));
}

static auto Write(void* pOpaque, mz_uint64 file_ofs, void const* pBuf, size_t n) -> size_t
{
    auto* fs {static_cast<ofstream*>(pOpaque)};
    fs->seek(static_cast<std::streamoff>(file_ofs), seek_dir::Begin);
    return static_cast<size_t>(fs->write<std::byte>({static_cast<std::byte const*>(pBuf), n}));
}
}

////////////////////////////////////////////////////////////

auto zip(path const& srcFileOrFolder, ofstream& dstStream, bool relative, i32 level) -> bool
{
    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);
    zip.m_pWrite     = &Write;
    zip.m_pIO_opaque = &dstStream;

    if (!mz_zip_writer_init(&zip, 0)) { return false; }

    if (is_folder(srcFileOrFolder)) {
        auto files {enumerate(srcFileOrFolder)};
        for (auto const& file : files) {
            ifstream istream {file};
            string   name {relative ? file.substr(srcFileOrFolder.size()) : file};
            if (!mz_zip_writer_add_read_buf_callback(&zip, name.c_str(), &Read, &istream, static_cast<u64>(istream.size_in_bytes()), nullptr, nullptr, 0, static_cast<u32>(level), nullptr, 0, nullptr, 0)) {
                return false;
            }
        }
    } else if (is_file(srcFileOrFolder)) {
        ifstream istream {srcFileOrFolder};
        string   name {relative ? std::filesystem::path {srcFileOrFolder}.filename().string() : srcFileOrFolder};
        if (!mz_zip_writer_add_read_buf_callback(&zip, name.c_str(), &Read, &istream, static_cast<u64>(istream.size_in_bytes()), nullptr, nullptr, 0, static_cast<u32>(level), nullptr, 0, nullptr, 0)) {
            return false;
        }
    } else {
        return false;
    }

    if (!mz_zip_writer_finalize_archive(&zip)) { return false; }

    return mz_zip_writer_end(&zip);
}

auto unzip(ifstream& srcStream, path const& dstFolder) -> bool
{
    mz_zip_archive zip;
    mz_zip_zero_struct(&zip);
    zip.m_pRead      = &Read;
    zip.m_pIO_opaque = &srcStream;

    if (!mz_zip_reader_init(&zip, static_cast<u64>(srcStream.size_in_bytes()), 0)) { return false; }

    mz_uint const n {mz_zip_reader_get_num_files(&zip)};
    for (mz_uint i {0}; i < n; ++i) {
        std::array<char, 260> buf {};
        mz_zip_reader_get_filename(&zip, i, buf.data(), static_cast<mz_uint>(buf.size()));
        string const file {dstFolder.empty() ? string {buf.data()} : dstFolder + "/" + string {buf.data()}};
        create_file(file);
        ofstream ostream {file};
        mz_zip_reader_extract_to_callback(&zip, i, &Write, &ostream, 0);
    }

    return mz_zip_reader_end(&zip);
}

auto mount(path const& folderOrArchive, string const& mp) -> bool
{
    return locate_service<file_system>().mount(folderOrArchive, mp);
}

auto unmount(path const& folderOrArchive) -> bool
{
    return locate_service<file_system>().unmount(folderOrArchive);
}

auto get_file_size(path const& file) -> i64
{
    if (!is_file(file)) { return -1; }

    return get_stat(file).FileSize;
}

auto read_as_string(path const& file) -> string
{
    if (!is_file(file)) { return ""; }

    ifstream str {file};
    return str.read_string(str.size_in_bytes());
}

auto get_extension(path const& file) -> string
{
    return std::filesystem::path {file}.extension().string();
}

auto get_stem(path const& file) -> string
{
    return std::filesystem::path {file}.stem().string();
}

auto get_filename(path const& file) -> string
{
    return std::filesystem::path {file}.filename().string();
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
    return locate_service<file_system>().is_folder_empty(folder);
}

auto get_stat(path const& fileOrFolder) -> stat
{
    return locate_service<file_system>().get_stat(fileOrFolder);
}

auto exists(path const& fileOrFolder) -> bool
{
    return fileOrFolder.empty() ? false : locate_service<file_system>().exists(fileOrFolder);
}

auto delete_file(path const& file) -> bool
{
    if (!is_file(file)) { return false; }
    return locate_service<file_system>().delete_file(file);
}

auto delete_folder(path const& folder) -> bool
{
    if (!is_folder(folder)) { return false; }
    return locate_service<file_system>().delete_folder(folder);
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

    return locate_service<file_system>().create_file(file);
}

auto create_folder(path const& folder) -> bool
{
    return locate_service<file_system>().create_folder(folder);
}

auto enumerate(path const& folder, pattern const& pattern, bool recursive) -> std::unordered_set<string>
{
    if (!is_folder(folder)) { return {}; }
    return locate_service<file_system>().enumerate(folder, pattern, recursive);
}

auto get_sub_folders(path const& folder) -> std::unordered_set<string>
{
    if (!is_folder(folder)) { return {}; }
    return locate_service<file_system>().get_sub_folders(folder);
}

}

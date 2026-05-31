// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Archiver_tar.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include <physfs.h>

namespace tcob::detail {

constexpr usize TAR_BLOCK {512};

struct entry {
    string        name {};
    PHYSFS_uint64 size {};
    PHYSFS_uint64 offset {};
    bool          isDir {};
};

struct archive {
    PHYSFS_Io*                        io {};
    std::unordered_map<string, entry> files {};
};

static auto parse_octal(char const* str, i32 size) -> PHYSFS_uint64
{
    PHYSFS_uint64 retValue {};

    for (i32 i {0}; i < size && str[i]; ++i) {
        if (str[i] >= '0' && str[i] <= '7') {
            retValue = (retValue << 3) + (str[i] - '0');
        }
    }

    return retValue;
}

static void add_dir(archive* arc, string const& path)
{
    if (arc->files.contains(path)) { return; }
    arc->files.emplace(path, entry {.name = path, .size = 0, .offset = 0, .isDir = true});
}

static void ensure_dirs(archive* arc, string const& file)
{
    usize pos {0};

    while ((pos = file.find('/', pos)) != string::npos) {
        add_dir(arc, file.substr(0, pos));
        ++pos;
    }
}

static void build_index(archive* arc)
{
    auto* io {arc->io};

    std::array<char, TAR_BLOCK> header {};
    PHYSFS_uint64               offset {0};

    add_dir(arc, "");

    for (;;) {
        io->seek(io, offset);

        if (io->read(io, header.data(), TAR_BLOCK) != TAR_BLOCK) { break; }

        bool empty {true};
        for (char c : header) {
            if (c != 0) {
                empty = false;
                break;
            }
        }
        if (empty) { break; }

        std::array<char, 101> name {};
        std::memcpy(name.data(), header.data(), 100);

        string const        fname {name.data()};
        PHYSFS_uint64 const size {parse_octal(header.data() + 124, 12)};

        if (!fname.empty()) {
            bool const isDir {fname.back() == '/'};
            ensure_dirs(arc, fname);
            if (!isDir) {
                arc->files.emplace(fname, entry {.name = fname, .size = size, .offset = offset + TAR_BLOCK, .isDir = isDir});
            }
        }

        PHYSFS_uint64 const dataBlocks {(size + TAR_BLOCK - 1) / TAR_BLOCK};
        offset += TAR_BLOCK + (dataBlocks * TAR_BLOCK);
    }
}

struct file_io {
    PHYSFS_Io*    src {};
    PHYSFS_uint64 start {};
    PHYSFS_uint64 size {};
    PHYSFS_uint64 pos {};
};

static auto tar_read(PHYSFS_Io* io, void* buf, PHYSFS_uint64 len) -> PHYSFS_sint64
{
    auto* t {static_cast<file_io*>(io->opaque)};

    if (t->pos >= t->size) {
        return 0;
    }

    PHYSFS_uint64 const rem {t->size - t->pos};
    len = std::min(len, rem);

    t->src->seek(t->src, t->start + t->pos);
    PHYSFS_sint64 const r {t->src->read(t->src, buf, len)};

    if (r > 0) {
        t->pos += static_cast<PHYSFS_uint64>(r);
    }

    return r;
}

static auto tar_tell(PHYSFS_Io* io) -> PHYSFS_sint64 { return static_cast<file_io*>(io->opaque)->pos; }
static auto tar_length(PHYSFS_Io* io) -> PHYSFS_sint64 { return static_cast<file_io*>(io->opaque)->size; }

static auto tar_seek(PHYSFS_Io* io, PHYSFS_uint64 offset) -> int
{
    auto* t {static_cast<file_io*>(io->opaque)};

    if (offset > t->size) { return 0; }
    t->pos = offset;
    return 1;
}

static void tar_destroy(PHYSFS_Io* io)
{
    if (!io) { return; }
    auto* t {static_cast<file_io*>(io->opaque)};
    if (t) {
        if (t->src) { t->src->destroy(t->src); }
        delete t; // NOLINT
    }
    delete io;    // NOLINT
}

static auto tar_openRead(void* opaque, char const* fnm) -> PHYSFS_Io*
{
    auto* arc {static_cast<archive*>(opaque)};

    auto const it {arc->files.find(fnm)};
    if (it == arc->files.end()) { return nullptr; }

    auto const& e {it->second};
    if (e.isDir) { return nullptr; }

    auto* t {new file_io {
        .src   = arc->io->duplicate(arc->io),
        .start = e.offset,
        .size  = e.size,
        .pos   = 0}};

    auto* io {new PHYSFS_Io {}};

    io->opaque  = t;
    io->read    = tar_read;
    io->write   = nullptr;
    io->seek    = tar_seek;
    io->tell    = tar_tell;
    io->length  = tar_length;
    io->flush   = nullptr;
    io->destroy = tar_destroy;

    return io;
}

static auto tar_enumerate(
    void*                    opaque,
    char const*              dirname,
    PHYSFS_EnumerateCallback cb,
    char const*              origdir,
    void*                    data) -> PHYSFS_EnumerateCallbackResult
{
    auto* arc {static_cast<archive*>(opaque)};

    string prefix {dirname ? dirname : ""};
    if (!prefix.empty() && prefix.back() != '/') { prefix += '/'; }

    std::unordered_set<string_view> emitted {};

    for (auto const& kv : arc->files) {
        string_view const path {kv.first};

        if (!path.starts_with(prefix)) { continue; }

        string_view const rest {path.substr(prefix.size())};
        if (rest.empty()) { continue; }

        usize const       slash {rest.find('/')};
        string_view const entry {slash == string_view::npos ? rest : rest.substr(0, slash)};

        if (emitted.contains(entry)) { continue; }
        emitted.insert(entry);
        auto const res {cb(data, origdir, string {entry}.c_str())};
        if (res == PHYSFS_ENUM_ERROR) {
            PHYSFS_setErrorCode(PHYSFS_ERR_APP_CALLBACK);
            return PHYSFS_ENUM_ERROR;
        }
        if (res == PHYSFS_ENUM_STOP) { return PHYSFS_ENUM_STOP; }
    }

    return PHYSFS_ENUM_OK;
}

static auto tar_stat(void* opaque, char const* fn, PHYSFS_Stat* st) -> int
{
    auto* arc {static_cast<archive*>(opaque)};

    auto const it {arc->files.find(fn)};
    if (it == arc->files.end()) {
        return 0;
    }

    *st = {};
    auto const& e {it->second};
    st->filesize = static_cast<PHYSFS_sint64>(e.size);
    st->filetype = e.isDir ? PHYSFS_FILETYPE_DIRECTORY : PHYSFS_FILETYPE_REGULAR;

    st->readonly   = 1;
    st->modtime    = -1;
    st->createtime = -1;
    st->accesstime = -1;

    return 1;
}

static void tar_closeArchive(void* opaque)
{
    auto* arc {static_cast<archive*>(opaque)};

    if (arc->io) {
        arc->io->destroy(arc->io);
    }

    delete arc; // NOLINT
}

static auto tar_openArchive(PHYSFS_Io* io,
                            char const*,
                            int  forWrite,
                            int* claimed) -> void*
{
    *claimed = 0;

    if (forWrite) { return nullptr; }

    io->seek(io, 0);

    auto* arc {new archive {}};
    arc->io = io->duplicate(io);

    if (!arc->io) {
        delete arc; // NOLINT
        return nullptr;
    }

    std::array<char, TAR_BLOCK> header {};
    if (arc->io->read(arc->io, header.data(), TAR_BLOCK) != TAR_BLOCK
        || std::memcmp(header.data() + 257, "ustar", 5) != 0) {
        arc->io->destroy(arc->io);
        delete arc; // NOLINT
        return nullptr;
    }

    arc->io->seek(arc->io, 0);

    *claimed = 1;

    build_index(arc);

    return arc;
}

static auto tar_openWrite(void*, char const*) -> PHYSFS_Io*
{
    PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
    return nullptr;
}

static auto tar_openAppend(void*, char const*) -> PHYSFS_Io*
{
    PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
    return nullptr;
}

static auto tar_remove(void*, char const*) -> int
{
    PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
    return 0;
}

static auto tar_mkdir(void*, char const*) -> int
{
    PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
    return 0;
}

static PHYSFS_Archiver const tar_archiver {
    .version      = 0,
    .info         = {.extension        = "TAR",
                     .description      = "TAR archiver",
                     .author           = "",
                     .url              = "",
                     .supportsSymlinks = 0},
    .openArchive  = tar_openArchive,
    .enumerate    = tar_enumerate,
    .openRead     = tar_openRead,
    .openWrite    = tar_openWrite,
    .openAppend   = tar_openAppend,
    .remove       = tar_remove,
    .mkdir        = tar_mkdir,
    .stat         = tar_stat,
    .closeArchive = tar_closeArchive};
}

extern "C" auto PHYSFS_init_tar() -> int
{
    return PHYSFS_registerArchiver(&tcob::detail::tar_archiver);
}

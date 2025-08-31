// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SqliteVFS.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <cassert>
    #include <cstring>

    #include <sqlite3/sqlite3.h>

    #include "tcob/core/io/FileStream.hpp"
    #include "tcob/core/io/FileSystem.hpp"
    #include "tcob/core/io/Stream.hpp"

namespace tcob::db::detail {

namespace fs = tcob::io;

extern "C" {
struct physfs_sqlite3_file {
    sqlite3_file SqliteFile;
    char const*  FileName;
};

static auto xClose(sqlite3_file* /* f */) -> int
{
    return SQLITE_OK;
}

static auto xRead(sqlite3_file* f, void* dst, int iAmt, sqlite3_int64 iOfst) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) { return SQLITE_IOERR; }

    fs::ifstream stream {file->FileName};

    if (!stream.seek(iOfst, io::seek_dir::Begin)) { return SQLITE_IOERR_READ; }

    auto* dstBuffer {static_cast<byte*>(dst)};
    auto  nRead {stream.read_to<byte>({dstBuffer, static_cast<usize>(iAmt)})};

    if (nRead < 0) { return SQLITE_IOERR_READ; }
    if (nRead < iAmt) { std::memset(dstBuffer + nRead, 0, iAmt - nRead); }
    return SQLITE_OK;
}

static auto xWrite(sqlite3_file* f, void const* src, int iAmt, sqlite3_int64 iOfst) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) { return SQLITE_IOERR; }

    fs::ofstream stream {file->FileName, 4096, true};

    if (!stream.seek(iOfst, io::seek_dir::Begin)) { return SQLITE_IOERR_SEEK; }

    auto const* srcBuffer {static_cast<byte const*>(src)};
    auto const  written {stream.write<byte>({srcBuffer, static_cast<usize>(iAmt)})};

    return written == iAmt ? SQLITE_OK : SQLITE_IOERR_WRITE;
}

static auto xTruncate(sqlite3_file* /* f */, sqlite3_int64 /* size */) -> int
{
    return SQLITE_OK;
}

static auto xSync(sqlite3_file* /* f */, int /* flags */) -> int
{
    return SQLITE_OK;
}

static auto xFileSize(sqlite3_file* f, sqlite3_int64* pSize) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) { return SQLITE_IOERR; }

    *pSize = fs::get_file_size(file->FileName);
    return SQLITE_OK;
}

static auto xLock(sqlite3_file* /* f */, int /* level */) -> int
{
    return SQLITE_OK;
}

static auto xUnlock(sqlite3_file* /* f */, int /* level */) -> int
{
    return SQLITE_OK;
}

static auto xCheckReservedLock(sqlite3_file* /* f */, int* pResOut) -> int
{
    *pResOut = false;
    return SQLITE_OK;
}

static auto xFileControl(sqlite3_file* /* f */, int /* op */, void* /* pArg */) -> int
{
    return SQLITE_NOTFOUND;
}

static auto xSectorSize(sqlite3_file* /* f */) -> int
{
    return 4096;
}

static auto xDeviceCharacteristics(sqlite3_file* /* f */) -> int
{
    return 0;
}

static sqlite3_io_methods const physfs_sqlite3_io_methods {
    .iVersion               = 1,
    .xClose                 = &xClose,
    .xRead                  = &xRead,
    .xWrite                 = &xWrite,
    .xTruncate              = &xTruncate,
    .xSync                  = &xSync,
    .xFileSize              = &xFileSize,
    .xLock                  = &xLock,
    .xUnlock                = &xUnlock,
    .xCheckReservedLock     = &xCheckReservedLock,
    .xFileControl           = &xFileControl,
    .xSectorSize            = &xSectorSize,
    .xDeviceCharacteristics = &xDeviceCharacteristics,
    .xShmMap                = nullptr,
    .xShmLock               = nullptr,
    .xShmBarrier            = nullptr,
    .xShmUnmap              = nullptr,
    .xFetch                 = nullptr,
    .xUnfetch               = nullptr,
};

static auto xOpen(sqlite3_vfs*, char const* zName, sqlite3_file* f, int flags, int* pOutFlags) -> int
{
    auto* file {reinterpret_cast<physfs_sqlite3_file*>(f)};
    assert(&file->SqliteFile == f);

    file->SqliteFile.pMethods = &physfs_sqlite3_io_methods;
    file->FileName            = zName;

    bool const exists {fs::is_file(zName)};

    if ((flags & SQLITE_OPEN_CREATE) && !exists) {
        if (!fs::create_file(zName)) {
            return SQLITE_IOERR;
        }
    } else if (!exists) {
        return SQLITE_CANTOPEN;
    }

    if (pOutFlags) { *pOutFlags = flags; }

    return SQLITE_OK;
}

static auto xDelete(sqlite3_vfs*, char const* zName, int /* syncDir */) -> int
{
    return fs::delete_file(zName) ? SQLITE_OK : SQLITE_IOERR_DELETE;
}

static auto xAccess(sqlite3_vfs*, char const* zName, int flags, int* pResOut) -> int
{
    switch (flags) {
    case SQLITE_ACCESS_EXISTS:    *pResOut = fs::is_file(zName); break;
    case SQLITE_ACCESS_READ:      *pResOut = fs::is_file(zName); break;
    case SQLITE_ACCESS_READWRITE:
    default:                      *pResOut = false; break;
    }

    return SQLITE_OK;
}

static auto xFullPathname(sqlite3_vfs*, char const* zName, int nOut, char* zOut) -> int
{
    return sqlite3_snprintf(nOut, zOut, "%s", zName) ? SQLITE_OK : SQLITE_IOERR;
}

static auto xDlOpen(sqlite3_vfs*, char const* /* zFilename */) -> void*
{
    return nullptr;
}

static void xDlError(sqlite3_vfs*, int, char*)
{
}

static auto xDlSym(sqlite3_vfs*, void*, char const*) -> void (*)(void)
{
    return nullptr;
}

static void xDlClose(sqlite3_vfs*, void*)
{
}

static auto xRandomness(sqlite3_vfs*, int, char*) -> int
{
    return 0;
}

static auto xSleep(sqlite3_vfs*, int) -> int
{
    return 0;
}

static auto xCurrentTime(sqlite3_vfs*, double*) -> int
{
    return 0;
}

static auto xGetLastError(sqlite3_vfs*, int /* nBuf */, char* /* zBuf */) -> int
{
    return 0;
}

static sqlite3_vfs physfs_sqlite3_vfs {
    .iVersion          = 1,                           /* Structure version number (currently 3) */
    .szOsFile          = sizeof(physfs_sqlite3_file), /* Size of subclassed sqlite3_file */
    .mxPathname        = 256,                         /* Maximum file pathname length */
    .pNext             = nullptr,                     /* Next registered VFS */
    .zName             = "physfs",                    /* Name of this virtual file system */
    .pAppData          = nullptr,                     /* Pointer to application-specific data */
    .xOpen             = &xOpen,
    .xDelete           = &xDelete,
    .xAccess           = &xAccess,
    .xFullPathname     = &xFullPathname,
    .xDlOpen           = &xDlOpen,
    .xDlError          = &xDlError,
    .xDlSym            = &xDlSym,
    .xDlClose          = &xDlClose,
    .xRandomness       = &xRandomness,
    .xSleep            = &xSleep,
    .xCurrentTime      = &xCurrentTime,
    .xGetLastError     = &xGetLastError,
    .xCurrentTimeInt64 = nullptr,
    .xSetSystemCall    = nullptr,
    .xGetSystemCall    = nullptr,
    .xNextSystemCall   = nullptr};
}

auto register_vfs() -> string
{
    return sqlite3_vfs_register(&physfs_sqlite3_vfs, true) == SQLITE_OK ? physfs_sqlite3_vfs.zName : "";
}

} // namespace sqlite

#endif

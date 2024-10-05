// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "SqliteVFS.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <sqlite3/sqlite3.h>

    #include "tcob/core/io/FileStream.hpp"
    #include "tcob/core/io/FileSystem.hpp"

namespace tcob::data::sqlite::detail {

namespace fs = tcob::io;

extern "C" {
struct physfs_sqlite3_file {
    sqlite3_file SqliteFile;
    char const*  FileName;
};

auto static xClose(sqlite3_file* /* f */) -> int
{
    return SQLITE_OK;
}

auto static xRead(sqlite3_file* f, void* dst, int iAmt, sqlite3_int64 iOfst) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) {
        return SQLITE_IOERR;
    }

    fs::ifstream stream {file->FileName};

    if (!stream.seek(iOfst, io::seek_dir::Begin)) {
        return SQLITE_IOERR_READ;
    }

    auto* dstBuffer {static_cast<byte*>(dst)};
    auto  nRead {stream.read_to<byte>({dstBuffer, static_cast<usize>(iAmt)})};

    if (nRead < 0) {
        return SQLITE_IOERR_READ;
    }
    if (nRead < iAmt) {
        return SQLITE_IOERR_SHORT_READ;
    }
    return SQLITE_OK;
}

auto static xWrite(sqlite3_file* f, void const* src, int iAmt, sqlite3_int64 iOfst) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) {
        return SQLITE_IOERR;
    }

    fs::ofstream stream {file->FileName, 4096, true};

    if (!stream.seek(iOfst, io::seek_dir::Begin)) {
        return SQLITE_IOERR_SEEK;
    }

    auto const* srcBuffer {static_cast<byte const*>(src)};
    auto const  written {stream.write<byte>({srcBuffer, static_cast<usize>(iAmt)})};

    return written == iAmt ? SQLITE_OK : SQLITE_IOERR_WRITE;
}

auto static xTruncate(sqlite3_file* /* f */, sqlite3_int64 /* size */) -> int
{
    return SQLITE_OK;
}

auto static xSync(sqlite3_file* /* f */, int /* flags */) -> int
{
    return SQLITE_OK;
}

auto static xFileSize(sqlite3_file* f, sqlite3_int64* pSize) -> int
{
    auto const* file {reinterpret_cast<physfs_sqlite3_file const*>(f)};
    assert(&file->SqliteFile == f);

    if (!fs::is_file(file->FileName)) {
        return SQLITE_IOERR;
    }

    *pSize = fs::get_file_size(file->FileName);
    return SQLITE_OK;
}

auto static xLock(sqlite3_file* /* f */, int /* level */) -> int
{
    return SQLITE_OK;
}

auto static xUnlock(sqlite3_file* /* f */, int /* level */) -> int
{
    return SQLITE_OK;
}

auto static xCheckReservedLock(sqlite3_file* /* f */, int* pResOut) -> int
{
    *pResOut = false;
    return SQLITE_OK;
}

auto static xFileControl(sqlite3_file* /* f */, int /* op */, void* /* pArg */) -> int
{
    return 0;
}

auto static xSectorSize(sqlite3_file* /* f */) -> int
{
    return 4096;
}

auto static xDeviceCharacteristics(sqlite3_file* /* f */) -> int
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

auto static xOpen(sqlite3_vfs*, char const* zName, sqlite3_file* f, int /* flags */, int* /* pOutFlags */) -> int
{
    auto* file {reinterpret_cast<physfs_sqlite3_file*>(f)};
    assert(&file->SqliteFile == f);

    file->SqliteFile.pMethods = &physfs_sqlite3_io_methods;
    file->FileName            = zName;

    if (!fs::is_file(file->FileName)) { // FIXME: check flags
        fs::create_file(file->FileName);
    }

    return file->FileName ? SQLITE_OK : SQLITE_IOERR;
}

auto static xDelete(sqlite3_vfs*, char const* zName, int /* syncDir */) -> int
{
    return fs::delete_file(zName) ? SQLITE_OK : SQLITE_IOERR_DELETE;
}

auto static xAccess(sqlite3_vfs*, char const* zName, int flags, int* pResOut) -> int
{
    switch (flags) {
    case SQLITE_ACCESS_EXISTS:
        *pResOut = fs::is_file(zName);
        break;

    case SQLITE_ACCESS_READ: {
        *pResOut = fs::is_file(zName);
        break;
    }
    case SQLITE_ACCESS_READWRITE:
    default:
        *pResOut = false;
    }

    return SQLITE_OK;
}

auto static xFullPathname(sqlite3_vfs*, char const* zName, int nOut, char* zOut) -> int
{
    return sqlite3_snprintf(nOut, zOut, "%s", zName) ? SQLITE_OK : SQLITE_IOERR;
}

auto static xDlOpen(sqlite3_vfs*, char const* /* zFilename */) -> void*
{
    return nullptr;
}

void static xDlError(sqlite3_vfs*, int, char*)
{
}

auto static xDlSym(sqlite3_vfs*, void*, char const*) -> void (*)(void)
{
    return nullptr;
}

void static xDlClose(sqlite3_vfs*, void*)
{
}

auto static xRandomness(sqlite3_vfs*, int, char*) -> int
{
    return 0;
}

auto static xSleep(sqlite3_vfs*, int) -> int
{
    return 0;
}

auto static xCurrentTime(sqlite3_vfs*, double*) -> int
{
    return 0;
}

auto static xGetLastError(sqlite3_vfs*, int /* nBuf */, char* /* zBuf */) -> int
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

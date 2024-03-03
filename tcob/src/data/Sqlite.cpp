// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/Sqlite.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <cassert>
    #include <format>

    #include <sqlite3/sqlite3.h>

    #include "SqliteVFS.hpp"
    #include "tcob/core/Logger.hpp"

namespace tcob::data::sqlite {

////////////////////////////////////////////////////////////
statement_view::statement_view(sqlite3_stmt* stmt)
    : _stmt {stmt}
{
}

statement_view::operator bool() const
{
    return _stmt != nullptr;
}

auto statement_view::step() const -> step_status
{
    assert(_stmt);

    i32 const res {sqlite3_step(_stmt)};
    switch (res) {
    case SQLITE_ROW:
        return step_status::Row;
    case SQLITE_DONE:
        return step_status::Done;
    default:
        return step_status::Error;
    }
}

auto statement_view::column_double(i32 col) const -> f64
{
    assert(_stmt);
    return sqlite3_column_double(_stmt, col);
}

auto statement_view::column_int(i32 col) const -> i32
{
    assert(_stmt);
    return sqlite3_column_int(_stmt, col);
}

auto statement_view::column_int64(i32 col) const -> i64
{
    assert(_stmt);
    return sqlite3_column_int64(_stmt, col);
}

auto statement_view::column_text(i32 col) const -> utf8_string
{
    assert(_stmt);
    return reinterpret_cast<char const*>(sqlite3_column_text(_stmt, col));
}

auto statement_view::column_blob(i32 col) const -> void const*
{
    assert(_stmt);
    return sqlite3_column_blob(_stmt, col);
}

auto statement_view::bind(i32 idx, f64 value) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_double(_stmt, idx, value) == SQLITE_OK;
}

void statement_view::finalize() const
{
    if (_stmt) {
        sqlite3_finalize(_stmt);
    }
}

auto statement_view::get_column_name(i32 col) const -> utf8_string
{
    assert(_stmt);
    return sqlite3_column_name(_stmt, col);
}

auto statement_view::get_column_type(i32 col) const -> type
{
    assert(_stmt);
    switch (sqlite3_column_type(_stmt, col)) {
    case SQLITE_INTEGER:
        return type::Integer;
    case SQLITE_FLOAT:
        return type::Real;
    case SQLITE_BLOB:
        return type::Blob;
    case SQLITE_NULL:
        return type::Null;
    case SQLITE_TEXT:
        return type::Text;
    default:
        return type::Blob;
    }
}

auto statement_view::get_column_count() const -> i32
{
    assert(_stmt);
    return sqlite3_column_count(_stmt);
}

auto statement_view::bind(i32 idx, i32 value) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_int(_stmt, idx, value) == SQLITE_OK;
}

auto statement_view::bind(i32 idx, i64 value) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_int64(_stmt, idx, value) == SQLITE_OK;
}

auto statement_view::bind(i32 idx, utf8_string_view value) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_text(_stmt, idx, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT) == SQLITE_OK; // NOLINT
}

auto statement_view::bind(i32 idx, char const* value) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_text(_stmt, idx, value, -1, SQLITE_TRANSIENT) == SQLITE_OK; // NOLINT
}

auto statement_view::bind(i32 idx, void const* value, i64 size) const -> bool
{
    assert(_stmt);
    return sqlite3_bind_blob64(_stmt, idx, value, size, SQLITE_TRANSIENT) == SQLITE_OK; // NOLINT
}

auto statement_view::bind_null(i32 idx) -> bool
{
    assert(_stmt);
    return sqlite3_bind_null(_stmt, idx) == SQLITE_OK;
}

////////////////////////////////////////////////////////////

database_view::database_view(sqlite3* db)
    : _db {db}
{
}

database_view::operator bool() const
{
    return _db != nullptr;
}

auto database_view::open(path const& file) -> bool
{
    [[maybe_unused]] auto static reg {detail::register_vfs()};
    return sqlite3_open_v2(file.c_str(), &_db, SQLITE_OPEN_READWRITE, reg.c_str()) == SQLITE_OK;
}

auto database_view::close() -> bool
{
    if (_db) {
        auto err {sqlite3_close(_db)};
        _db = nullptr;
        return err == SQLITE_OK;
    }

    return true;
}

auto database_view::prepare(utf8_string_view sql) const -> statement_view
{
    assert(_db);
    logger::Debug("SQLite: prepare: {}", sql);
    sqlite3_stmt* stmt {};
    sqlite3_prepare_v2(_db, sql.data(), static_cast<int>(sql.size()), &stmt, nullptr);
    return statement_view {stmt};
}

auto database_view::exec(utf8_string const& sql) const -> bool
{
    assert(_db);
    logger::Debug("SQLite: exec: {}", sql);
    [[maybe_unused]] char* err {nullptr};
    return sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK;
}

auto database_view::get_error_message() const -> utf8_string
{
    assert(_db);
    return sqlite3_errmsg(_db);
}

void database_view::commit_hook(i32 (*callback)(void*), void* userdata) const
{
    assert(_db);
    sqlite3_commit_hook(_db, callback, userdata);
}

void database_view::rollback_hook(void (*callback)(void*), void* userdata) const
{
    assert(_db);
    sqlite3_rollback_hook(_db, callback, userdata);
}

void database_view::update_hook(void (*callback)(void*, int, char const*, char const*, long long int), void* userdata) const
{
    assert(_db);
    sqlite3_update_hook(_db, callback, userdata);
}

auto quote_string(utf8_string_view str) -> utf8_string
{
    return std::format("\"{}\"", str);
}

}

#endif

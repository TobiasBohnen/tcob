// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteDatabase.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <functional>
    #include <optional>
    #include <set>
    #include <utility>

    #include "tcob/core/io/FileSystem.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteSavepoint.hpp"
    #include "tcob/data/SqliteSchema.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::db {

extern "C" {
auto static commit(void* ptr) -> i32
{
    auto* db {static_cast<database*>(ptr)};
    return db->call_commit_hook();
}

void static rollback(void* ptr)
{
    auto* db {static_cast<database*>(ptr)};
    db->call_rollback_hook();
}

void static update(void* ptr, i32 mode, char const* dbName, char const* table, long long int rowid)
{
    auto*       db {static_cast<database*>(ptr)};
    update_mode upMode {};
    switch (mode) {
    case 9:  upMode = update_mode::Delete; break;
    case 18: upMode = update_mode::Insert; break;
    case 23: upMode = update_mode::Update; break;
    default:
        break;
    }

    db->call_update_hook(upMode, dbName, table, rowid);
}
}

database::database()
    : database {database_view {nullptr}}
{
}

database::database(database_view db)
    : _db {db}
    , _main {db, "main"}
{
}

database::database(database&& other) noexcept
    : _db {std::exchange(other._db, database_view {nullptr})}
    , _main {std::exchange(other._main, schema {database_view {nullptr}, ""})}
    , _commitHookFunc {std::exchange(other._commitHookFunc, {})}
    , _rbHookFunc {std::exchange(other._rbHookFunc, {})}
    , _updateHookFunc {std::exchange(other._updateHookFunc, {})}
{
}

auto database::operator=(database&& other) noexcept -> database&
{
    std::swap(_db, other._db);
    std::swap(_main, other._main);
    std::swap(_commitHookFunc, other._commitHookFunc);
    std::swap(_rbHookFunc, other._rbHookFunc);
    std::swap(_updateHookFunc, other._updateHookFunc);
    return *this;
}

database::~database()
{
    close();
}

void database::set_journal_mode(journal_mode mode) const
{
    switch (mode) {
    case journal_mode::Delete: _db.exec("PRAGMA journal_mode=DELETE;"); break;
    case journal_mode::Memory: _db.exec("PRAGMA journal_mode=MEMORY;"); break;
    case journal_mode::Wal:    _db.exec("PRAGMA journal_mode=WAL;"); break;
    case journal_mode::Off:    _db.exec("PRAGMA journal_mode=OFF;"); break;
    }
}

auto database::schema_names() const -> std::set<utf8_string>
{
    std::set<utf8_string> result;

    statement stmt {_db};
    if (stmt.prepare("PRAGMA database_list;")) {
        while (stmt.step() == step_status::Row) {
            result.insert(stmt.get_column_value<utf8_string>(1)); // column 1 = schema name
        }
    }

    return result;
}

auto database::table_names() const -> std::set<utf8_string>
{
    return _main.table_names();
}

auto database::view_names() const -> std::set<utf8_string>
{
    return _main.view_names();
}

auto database::schema_exists(utf8_string const& schema) const -> bool
{
    statement select {_db};
    if (!select.prepare("PRAGMA database_list;")) {
        return false;
    }

    while (select.step() == step_status::Row) {
        if (select.get_column_value<utf8_string>(1) == schema) {
            return true;
        }
    }
    return false;
}

auto database::table_exists(utf8_string const& tableName) const -> bool
{
    return _main.table_exists(tableName);
}

auto database::view_exists(utf8_string const& viewName) const -> bool
{
    return _main.view_exists(viewName);
}

auto database::create_savepoint(utf8_string const& name) const -> savepoint
{
    return {_db, name};
}

auto database::get_schema(utf8_string const& schemaName) const -> std::optional<schema>
{
    return schema_exists(schemaName)
        ? std::make_optional<schema>(_db, schemaName)
        : std::nullopt;
}

auto database::get_table(utf8_string const& tableName) const -> std::optional<table>
{
    return _main.get_table(tableName);
}

auto database::get_view(utf8_string const& viewName) const -> std::optional<view>
{
    return _main.get_view(viewName);
}

auto database::drop_table(utf8_string const& tableName) const -> bool
{
    return _main.drop_table(tableName);
}

auto database::drop_view(utf8_string const& viewName) const -> bool
{
    return _main.drop_view(viewName);
}

void database::set_commit_hook(std::function<i32(database*)>&& func)
{
    _commitHookFunc = std::move(func);
    _db.commit_hook(&commit, this);
}

auto database::call_commit_hook() -> i32
{
    return _commitHookFunc(this);
}

void database::set_rollback_hook(std::function<void(database*)>&& func)
{
    _rbHookFunc = std::move(func);
    _db.rollback_hook(&rollback, this);
}

void database::call_rollback_hook()
{
    _rbHookFunc(this);
}

void database::set_update_hook(std::function<void(database*, update_mode, utf8_string, utf8_string, i64)>&& func)
{
    _updateHookFunc = std::move(func);
    _db.update_hook(&update, this);
}

void database::call_update_hook(update_mode mode, utf8_string const& dbName, utf8_string const& table, i64 rowId)
{
    _updateHookFunc(this, mode, dbName, table, rowId);
}

auto database::Open(path const& file) -> std::optional<database>
{
    if (!io::is_file(file)) { io::create_file(file); }

    database_view db {nullptr};
    if (db.open(file)) {
        db.config(1002, 1); // foreign key
        return std::make_optional<database>(db);
    }

    return std::nullopt;
}

auto database::OpenMemory() -> database
{
    database_view db {nullptr};
    db.open(":memory:");
    db.config(1002, 1); // foreign key
    return database {db};
}

void database::close()
{
    _db.close();
}

auto database::vacuum_into(path const& file) const -> bool
{
    return _main.vacuum_into(file);
}

auto database::attach_memory(utf8_string const& alias) const -> std::optional<schema>
{
    return attach(":memory:", alias);
}

auto database::attach(path const& file, utf8_string const& alias) const -> std::optional<schema>
{
    statement stmt {_db};
    return stmt.prepare(std::format("ATTACH DATABASE '{}' AS '{}';", file, alias)) && stmt.step() == step_status::Done
        ? get_schema(alias)
        : std::nullopt;
}

}

#endif

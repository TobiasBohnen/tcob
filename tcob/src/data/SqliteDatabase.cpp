// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteDatabase.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>

    #include "tcob/core/io/FileSystem.hpp"

namespace tcob::data::sqlite {

auto static create_master_query(utf8_string const& name, utf8_string const& type) -> utf8_string
{
    return std::format(
        "SELECT name FROM sqlite_master WHERE name = '{0}' and type = '{1}' "
        "UNION ALL "
        "SELECT name FROM sqlite_temp_master WHERE name = '{0}' and type = '{1}';",
        name, type);
}

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
    case 9:
        upMode = update_mode::Delete;
        break;
    case 18:
        upMode = update_mode::Insert;
        break;
    case 23:
        upMode = update_mode::Update;
        break;
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
{
}

database::database(database&& other) noexcept
    : _db {std::exchange(other._db, database_view {nullptr})}
    , _commitHookFunc {std::exchange(other._commitHookFunc, {})}
    , _rbHookFunc {std::exchange(other._rbHookFunc, {})}
    , _updateHookFunc {std::exchange(other._updateHookFunc, {})}
{
}

auto database::operator=(database&& other) noexcept -> database&
{
    std::swap(_db, other._db);
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
    case journal_mode::Delete:
        _db.exec("PRAGMA journal_mode=DELETE;");
        break;
    case journal_mode::Memory:
        _db.exec("PRAGMA journal_mode=MEMORY;");
        break;
    case journal_mode::Wal:
        _db.exec("PRAGMA journal_mode=WAL;");
        break;
    case journal_mode::Off:
        _db.exec("PRAGMA journal_mode=OFF;");
        break;
    }
}

auto database::table_names() const -> std::set<utf8_string>
{
    // SELECT name FROM sqlite_master WHERE type='table' ORDER BY name
    statement select {_db};
    select.prepare(
        "SELECT name FROM sqlite_master "
        "WHERE type='table' "
        "UNION ALL "
        "SELECT name FROM sqlite_temp_master "
        "WHERE type='table';");

    return select.get_column_value<std::set<utf8_string>>(0);
}

auto database::view_names() const -> std::set<utf8_string>
{
    // SELECT name FROM sqlite_master WHERE type='view' ORDER BY name
    statement select {_db};
    select.prepare(
        "SELECT name FROM sqlite_master "
        "WHERE type='view' "
        "UNION ALL "
        "SELECT name FROM sqlite_temp_master "
        "WHERE type='view';");

    return select.get_column_value<std::set<utf8_string>>(0);
}

auto database::table_exists(utf8_string const& tableName) const -> bool
{
    //  SELECT name FROM sqlite_master WHERE name='tableName' and type='table';
    statement select {_db};
    select.prepare(create_master_query(tableName, "table"));
    return select.step() == step_status::Row;
}

auto database::view_exists(utf8_string const& viewName) const -> bool
{
    //  SELECT name FROM sqlite_master WHERE name='viewName' and type='view';
    statement select {_db};
    select.prepare(create_master_query(viewName, "view"));
    return select.step() == step_status::Row;
}

auto database::create_savepoint(utf8_string const& name) const -> savepoint
{
    return {_db, name};
}

auto database::create_statement() const -> statement
{
    return statement {_db};
}

auto database::get_table(utf8_string const& tableName) const -> std::optional<table>
{
    return table_exists(tableName)
        ? std::optional {table {_db, tableName}}
        : std::nullopt;
}

auto database::get_view(utf8_string const& viewName) const -> std::optional<view>
{
    return view_exists(viewName)
        ? std::optional {view {_db, viewName}}
        : std::nullopt;
}

auto database::drop_table(utf8_string const& tableName) const -> bool
{
    // DROP TABLE [IF EXISTS] [schema_name.]table_name;
    return _db.exec("DROP TABLE IF EXISTS " + tableName + ";");
}

auto database::drop_view(utf8_string const& viewName) const -> bool
{
    // DROP VIEW  [IF EXISTS] [schema_name.]table_name;
    return _db.exec("DROP VIEW IF EXISTS " + viewName + ";");
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
    return _db.exec("VACUUM main INTO '" + file + "';");
}

}

#endif

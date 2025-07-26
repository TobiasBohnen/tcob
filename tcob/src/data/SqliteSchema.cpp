// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteSchema.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <optional>
    #include <set>
    #include <utility>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::db {

schema::schema(database_view db, utf8_string name)
    : _db {db}
    , _name {std::move(name)}
{
}

auto schema::table_names() const -> std::set<utf8_string>
{
    // SELECT name FROM sqlite_schema WHERE type='table' ORDER BY name
    statement select {_db};
    if (_name == "main") {
        select.prepare("SELECT name FROM sqlite_schema WHERE type='table';");
    } else {
        select.prepare(std::format("SELECT name FROM {}.sqlite_schema WHERE type='table';", _name));
    }
    return select.get_column_value<std::set<utf8_string>>(0);
}
auto schema::view_names() const -> std::set<utf8_string>
{
    // SELECT name FROM sqlite_schema WHERE type='view' ORDER BY name
    statement select {_db};
    if (_name == "main") {
        select.prepare("SELECT name FROM sqlite_schema WHERE type='view';");
    } else {
        select.prepare(std::format("SELECT name FROM {}.sqlite_schema WHERE type='view';", _name));
    }
    return select.get_column_value<std::set<utf8_string>>(0);
}

auto schema::table_exists(utf8_string const& tableName) const -> bool
{
    // SELECT name FROM sqlite_schema WHERE name='tableName' and type='table';
    statement select {_db};
    if (_name == "main") {
        select.prepare("SELECT name FROM sqlite_schema WHERE name = ? and type = 'table';");
    } else {
        select.prepare(std::format("SELECT name FROM {0}.sqlite_schema WHERE name = ? and type = 'table';", _name));
    }

    i32 idx {1};
    select.bind_parameter(idx, tableName);
    return select.step() == step_status::Row;
}

auto schema::view_exists(utf8_string const& viewName) const -> bool
{
    //  SELECT name FROM sqlite_schema WHERE name='viewName' and type='view';
    statement select {_db};
    if (_name == "main") {
        select.prepare("SELECT name FROM sqlite_schema WHERE name = ? and type = 'view';");
    } else {
        select.prepare(std::format("SELECT name FROM {0}.sqlite_schema WHERE name = ? and type = 'view';", _name));
    }

    i32 idx {1};
    select.bind_parameter(idx, viewName);
    return select.step() == step_status::Row;
}

auto schema::get_table(utf8_string const& tableName) const -> std::optional<table>
{
    return table_exists(tableName)
        ? std::optional {table {_db, _name, tableName}}
        : std::nullopt;
}

auto schema::get_view(utf8_string const& viewName) const -> std::optional<view>
{
    return view_exists(viewName)
        ? std::optional {view {_db, _name, viewName}}
        : std::nullopt;
}

auto schema::drop_table(utf8_string const& tableName) const -> bool
{
    // DROP TABLE [IF EXISTS] [schema_name.]table_name;
    return _db.exec(std::format("DROP TABLE IF EXISTS {}.{};", quote_identifier(_name), quote_identifier(tableName)));
}

auto schema::drop_view(utf8_string const& viewName) const -> bool
{
    // DROP VIEW  [IF EXISTS] [schema_name.]table_name;
    return _db.exec(std::format("DROP VIEW IF EXISTS {}.{};", quote_identifier(_name), quote_identifier(viewName)));
}

auto schema::vacuum_into(path const& file) const -> bool
{
    return _db.exec(std::format("VACUUM {} INTO '{}';", _name, file));
}

auto schema::detach() const -> bool
{
    statement stmt {_db};
    return stmt.prepare(std::format("DETACH DATABASE '{}';", _name)) && stmt.step() == step_status::Done;
}

}

#endif

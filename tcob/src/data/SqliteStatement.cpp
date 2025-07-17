// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteStatement.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <utility>
    #include <vector>

    #include "tcob/core/Logger.hpp"
    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/Sqlite.hpp"

namespace tcob::db {

////////////////////////////////////////////////////////////

statement::statement(database_view db)
    : _db {db}
    , _stmt {nullptr}
{
}

statement::~statement()
{
    _stmt.finalize();
}

statement::statement(statement&& other) noexcept
    : _db {std::exchange(other._db, database_view {nullptr})}
    , _stmt {std::exchange(other._stmt, statement_view {nullptr})}
{
}

auto statement::operator=(statement&& other) noexcept -> statement&
{
    std::swap(_db, other._db);
    std::swap(_stmt, other._stmt);
    return *this;
}

auto statement::prepare(utf8_string const& sql) -> bool
{
    _stmt.finalize();
    _stmt = _db.prepare(sql);
    if (!is_valid()) {
        logger::Error("SQLite: {}", _db.error_message());
        return false;
    }
    return true;
}

auto statement::step() const -> step_status
{
    auto retValue {_stmt.step()};
    if (retValue == step_status::Error) {
        logger::Error("SQLite: {}", _db.error_message());
    }
    return retValue;
}

auto statement::get_column_name(i32 col) const -> utf8_string
{
    return _stmt.get_column_name(col);
}

auto statement::column_count() const -> i32
{
    return _stmt.column_count();
}

auto statement::get_db() const -> database_view
{
    return _db;
}

auto statement::is_valid() const -> bool
{
    return static_cast<bool>(_stmt);
}

////////////////////////////////////////////////////////////

update_statement::update_statement(database_view db, utf8_string const& schemaName, utf8_string const& table, utf8_string const& columns)
    : statement {db}
{
    // UPDATE table_name
    // SET column1 = value1, column2 = value2...., columnN = valueN
    // WHERE [condition];

    // create query
    _sql = std::format(R"(UPDATE "{}"."{}" SET {})", schemaName, table, columns);
}

auto update_statement::query_string() const -> utf8_string
{
    return std::format("{} WHERE {};", _sql, _where);
}

////////////////////////////////////////////////////////////

insert_statement::insert_statement(database_view db, mode mode, utf8_string const& schemaName, utf8_string const& table, utf8_string const& columns)
    : statement {db}
{
    // INSERT (OR IGNORE) INTO TABLE_NAME [(column1, column2, column3,...columnN)]
    // VALUES (value1, value2, value3,...valueN);

    string modeStr;
    switch (mode) {
    case Normal:  modeStr = " "; break;
    case Ignore:  modeStr = " OR IGNORE "; break;
    case Replace: modeStr = " OR REPLACE "; break;
    }

    // create query
    _sql = std::format(R"(INSERT{}INTO "{}"."{}" ({}))", modeStr, schemaName, table, columns);
}

auto insert_statement::query_string(usize valueSize, usize valueCount) const -> utf8_string
{
    // values
    auto const paramLine {"(" + helper::join("?", valueSize, ", ") + ")"};
    auto const paramLines {std::vector<utf8_string>(valueCount, paramLine)};
    return std::format("{} VALUES {};",
                       _sql, helper::join(paramLines, ", "));
}

////////////////////////////////////////////////////////////

delete_statement::delete_statement(database_view db, utf8_string const& schemaName, utf8_string const& table)
    : statement {db}
{
    // DELETE FROM table_name
    // WHERE [condition];
    _sql = std::format(R"(DELETE FROM "{}"."{}")", schemaName, table);
}

auto delete_statement::query_string() const -> utf8_string
{
    return std::format("{} WHERE {};", _sql, _where);
}
}

#endif

// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteSchema.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <optional>
    #include <vector>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::db {

inline auto schema::create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>
{
    // CREATE TABLE database_name.table_name(
    // column1 datatype PRIMARY KEY(one or more columns),
    // column2 datatype,
    // column3 datatype,
    // .....
    // columnN datatype
    // );
    std::vector<utf8_string> colStrings {columns.str()...};

    utf8_string const sql {std::format(R"(CREATE TABLE IF NOT EXISTS "{}"."{}" ({});)",
                                       _name, tableName, helper::join(colStrings, ", "))};

    return _db.exec(sql) ? get_table(tableName) : std::nullopt;
}

template <typename... Values>
inline auto schema::create_view(utf8_string const& viewName, select_statement<Values...>& stmt) -> std::optional<view>
{
    utf8_string const sql {std::format(R"(CREATE VIEW IF NOT EXISTS "{}"."{}" AS {};)", // TODO: Temp
                                       _name, viewName, stmt.query_string())};

    return _db.exec(sql) ? get_view(viewName) : std::nullopt;
}
}

#endif

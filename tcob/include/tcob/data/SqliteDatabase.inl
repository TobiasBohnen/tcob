// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteDatabase.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <optional>
    #include <vector>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::data::sqlite {

inline auto database::create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>
{
    // CREATE TABLE database_name.table_name(
    // column1 datatype PRIMARY KEY(one or more columns),
    // column2 datatype,
    // column3 datatype,
    // .....
    // columnN datatype
    // );
    std::vector<utf8_string> colStrings {columns.str()...};

    utf8_string const sql {std::format("CREATE TABLE IF NOT EXISTS {} ({});",
                                       tableName, helper::join(colStrings, ", "))};

    if (_db.exec(sql)) { return get_table(tableName); }

    return std::nullopt;
}

template <typename... Values>
inline auto database::create_view(utf8_string const& viewName, select_statement<Values...>& stmt, bool temp) -> std::optional<view>
{
    utf8_string const sql {std::format("CREATE {} VIEW IF NOT EXISTS {} AS {};",
                                       temp ? "TEMP" : "", viewName, stmt.query_string())};

    return (stmt.prepare(sql) && stmt.step() == step_status::Done)
        ? std::optional<view> {view {_db, viewName}}
        : std::nullopt;
}

}

#endif

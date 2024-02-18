// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteDatabase.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

namespace tcob::data::sqlite {

inline auto database::create_table(string const& tableName, auto&&... columns) const -> std::optional<table>
{
    // CREATE TABLE database_name.table_name(
    // column1 datatype PRIMARY KEY(one or more columns),
    // column2 datatype,
    // column3 datatype,
    // .....
    // columnN datatype
    // );
    std::vector<string> colStrings {};
    ((colStrings.push_back(columns.str())), ...);

    string const sql {std::format("CREATE TABLE IF NOT EXISTS {} ({});",
                                  tableName, helper::join(colStrings, ", "))};

    if (_db.exec(sql)) { return get_table(tableName); }

    return std::nullopt;
}

template <typename... Values>
inline auto database::create_view(string const& viewName, select_statement<Values...>& stmt, bool temp) -> std::optional<view>
{
    string const sql {std::format("CREATE {} VIEW IF NOT EXISTS {} AS {};",
                                  temp ? "TEMP" : "", viewName, stmt.get_query())};

    return (stmt.prepare(sql) && stmt.step() == step_status::Done)
        ? std::optional<view> {view {_db, viewName}}
        : std::nullopt;
}

}

#endif

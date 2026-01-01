// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteDatabase.hpp"

#include <optional>

#include "tcob/data/SqliteStatement.hpp"
#include "tcob/data/SqliteTable.hpp"

namespace tcob::db {

inline auto database::create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>
{
    return _main.create_table(tableName, columns...);
}

template <typename... Values>
inline auto database::create_view(utf8_string const& viewName, select_statement<Values...>& stmt) -> std::optional<view>
{
    return _main.create_view(viewName, stmt);
}

}

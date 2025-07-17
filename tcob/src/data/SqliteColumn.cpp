// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <utility>

    #include "tcob/data/Sqlite.hpp"

namespace tcob::db {

auto no_constraint::str() const -> utf8_string
{
    return "";
}

auto unique::str() const -> utf8_string
{
    if (Columns.empty()) { return "UNIQUE"; }

    return std::format("UNIQUE({})", Columns);
}

auto primary_key::str() const -> utf8_string
{
    return "PRIMARY KEY";
}

auto foreign_key::str() const -> utf8_string
{
    return Column.empty() ? std::format(R"(REFERENCES {}("{}"))", ForeignTable, ForeignColumn)
                          : std::format(R"(FOREIGN KEY("{}") REFERENCES {}("{}"))", Column, ForeignTable, ForeignColumn);
}

auto check::str() const -> utf8_string
{
    return "CHECK(" + Check + ")";
}

////////////////////////////////////////////////////////////

avg::avg(utf8_string column)
    : Column {std::move(column)}
{
}

auto avg::str() const -> utf8_string
{
    return std::format("AVG({})", quote_identifier(Column));
}

count::count(utf8_string column)
    : Column {std::move(column)}
{
}

auto count::str() const -> utf8_string
{
    return std::format("COUNT({})", quote_identifier(Column));
}

max::max(utf8_string column)
    : Column {std::move(column)}
{
}

auto max::str() const -> utf8_string
{
    return std::format("MAX({})", quote_identifier(Column));
}

min::min(utf8_string column)
    : Column {std::move(column)}
{
}

auto min::str() const -> utf8_string
{
    return std::format("MIN({})", quote_identifier(Column));
}

sum::sum(utf8_string column)
    : Column {std::move(column)}
{
}

auto sum::str() const -> utf8_string
{
    return std::format("SUM({})", quote_identifier(Column));
}

////////////////////////////////////////////////////////////

auto on::str(utf8_string const& table, utf8_string const& otherTable) const -> utf8_string
{
    return std::format(R"({}."{}" = {}."{}")", table, LeftColumn, otherTable, RightColumn);
}
}

#endif

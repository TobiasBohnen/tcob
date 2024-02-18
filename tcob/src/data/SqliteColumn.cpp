// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <utility>

#include "tcob/data/SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)
namespace tcob::data::sqlite {

auto no_constraint::str() const -> string
{
    return "";
}

auto unique::str() const -> string
{
    return "UNIQUE";
}

auto primary_key::str() const -> string
{
    return "PRIMARY KEY";
}

check::check(string check)
    : Check {std::move(check)}
{
}

auto check::str() const -> string
{
    return "CHECK(" + Check + ")";
}

////////////////////////////////////////////////////////////

avg::avg(string column)
    : Column {std::move(column)}
{
}

auto avg::str() const -> string
{
    return "AVG(\"" + Column + "\")";
}

count::count(string column)
    : Column {std::move(column)}
{
}

auto count::str() const -> string
{
    return "COUNT(\"" + Column + "\")";
}

max::max(string column)
    : Column {std::move(column)}
{
}

auto max::str() const -> string
{
    return "MAX(\"" + Column + "\")";
}

min::min(string column)
    : Column {std::move(column)}
{
}

auto min::str() const -> string
{
    return "MIN(\"" + Column + "\")";
}

sum::sum(string column)
    : Column {std::move(column)}
{
}

auto sum::str() const -> string
{
    return "SUM(\"" + Column + "\")";
}

////////////////////////////////////////////////////////////

auto literals::operator""_col(char const* str, usize) -> detail::column_builder
{
    return detail::column_builder {str};
}

}
#endif

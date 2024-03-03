// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <utility>

namespace tcob::data::sqlite {

auto no_constraint::str() const -> utf8_string
{
    return "";
}

auto unique::str() const -> utf8_string
{
    return "UNIQUE";
}

auto primary_key::str() const -> utf8_string
{
    return "PRIMARY KEY";
}

check::check(utf8_string check)
    : Check {std::move(check)}
{
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
    return std::format("AVG({})", quote_string(utf8_string {Column}));
}

count::count(utf8_string column)
    : Column {std::move(column)}
{
}

auto count::str() const -> utf8_string
{
    return std::format("COUNT({})", quote_string(utf8_string {Column}));
}

max::max(utf8_string column)
    : Column {std::move(column)}
{
}

auto max::str() const -> utf8_string
{
    return std::format("MAX({})", quote_string(utf8_string {Column}));
}

min::min(utf8_string column)
    : Column {std::move(column)}
{
}

auto min::str() const -> utf8_string
{
    return std::format("MIN({})", quote_string(utf8_string {Column}));
}

sum::sum(utf8_string column)
    : Column {std::move(column)}
{
}

auto sum::str() const -> utf8_string
{
    return std::format("SUM({})", quote_string(utf8_string {Column}));
}

}
#endif

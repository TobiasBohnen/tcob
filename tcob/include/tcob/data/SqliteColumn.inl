// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <string>
    #include <unordered_set>

    #include "tcob/core/Common.hpp"
    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/Sqlite.hpp"

namespace tcob::data::sqlite {

template <typename T>
inline default_value<T>::default_value(T defaultValue)
    : DefaultValue {defaultValue}
{
}

template <typename T>
inline auto default_value<T>::str() const -> utf8_string
{
    return std::format(" DEFAULT {}", DefaultValue);
}

////////////////////////////////////////////////////////////

inline unique::unique(auto&&... columns)
    : Columns {helper::join(std::unordered_set<utf8_string> {("\"" + utf8_string {columns} + "\"")...}, ",")}
{
}

////////////////////////////////////////////////////////////

template <order Order>
inline auto ordering<Order>::str() const -> utf8_string
{
    utf8_string order;
    // type
    switch (Order) {
    case order::Ascending:  order = "ASC"; break;
    case order::Descending: order = "DESC"; break;
    }

    utf8_string const column {
        std::visit(overloaded {
                       [](string const& name) { return quote_string(name); },
                       [](i32 column) { return std::to_string(column); }},
                   Column)};

    return std::format("{} {}", column, order);
}

////////////////////////////////////////////////////////////

template <type Type, typename C>
inline auto column<Type, C>::str() const -> utf8_string
{
    utf8_string type;
    // type
    switch (Type) {
    case type::Text:    type = "TEXT"; break;
    case type::Numeric: type = "NUMERIC"; break;
    case type::Integer: type = "INTEGER"; break;
    case type::Real:    type = "REAL"; break;
    case type::Blob:    type = "BLOB"; break;
    case type::Null:    break;
    }

    return std::format("{} {} {} {}", quote_string(Name), type, NotNull ? "NOT NULL" : "", Constraint.str());
}

////////////////////////////////////////////////////////////
}

#endif

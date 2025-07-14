// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <cassert>
    #include <format>
    #include <string>
    #include <type_traits>
    #include <unordered_set>
    #include <vector>

    #include "tcob/core/Common.hpp"
    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::data::sqlite {

template <typename T>
inline default_value<T>::default_value(T defaultValue)
    : DefaultValue {defaultValue}
{
}

template <typename T>
inline auto default_value<T>::str() const -> utf8_string
{
    return std::format("DEFAULT {}", DefaultValue);
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
    switch (Order) {
    case order::Ascending:  order = "ASC"; break;
    case order::Descending: order = "DESC"; break;
    }

    utf8_string const column {
        std::visit(overloaded {
                       [](string const& name) { return quote_identifier(name); },
                       [](i32 column) { return std::to_string(column); }},
                   Column)};

    return std::format("{} {}", column, order);
}

////////////////////////////////////////////////////////////

template <op Operator>
template <typename T>
inline conditional<Operator>::conditional(T const& column, auto&&... params)
{
    if constexpr (sizeof...(params) > 0) {
        (_params.push_back(params), ...);
    }

    if constexpr (detail::HasStr<std::remove_cvref_t<T>>) {
        _column = column.str();
    } else {
        _column = column;
    }
}

template <op Operator>
inline auto conditional<Operator>::str() const -> utf8_string
{
    utf8_string op;

    switch (Operator) {
    case op::Equal:        op = "="; break;
    case op::NotEqual:     op = "<>"; break;
    case op::Greater:      op = ">"; break;
    case op::GreaterEqual: op = ">="; break;
    case op::Less:         op = "<"; break;
    case op::LessEqual:    op = "<="; break;
    case op::Like:         op = "LIKE"; break;
    case op::In:           return std::format("{} IN ({})", _column, helper::join(std::vector<utf8_string>(_params.size(), "?"), ", "));
    case op::Between:      return std::format("{} BETWEEN ? AND ?", _column);
    }

    return std::format("{} {} ?", _column, op);
}

template <op Operator>
inline auto conditional<Operator>::bind() const -> bind_func
{
    return [values = _params](i32& idx, statement& view) {
        if (values.empty()) { return; }
        for (auto const& value : values) {
            std::visit([&](auto&& item) { view.bind_parameter(idx, item); }, value);
        }
    };
}

////////////////////////////////////////////////////////////

template <type Type, typename C>
inline auto column<Type, C>::str() const -> utf8_string
{
    utf8_string type;
    switch (Type) {
    case type::Text:    type = "TEXT"; break;
    case type::Numeric: type = "NUMERIC"; break;
    case type::Integer: type = "INTEGER"; break;
    case type::Real:    type = "REAL"; break;
    case type::Blob:    type = "BLOB"; break;
    case type::Null:    break;
    }

    auto const constraint {Constraint.str()};
    return std::format("{} {}{}{}{}",
                       quote_identifier(Name), type,
                       NotNull ? " NOT NULL" : "",
                       constraint.empty() ? "" : " ", constraint);
}

////////////////////////////////////////////////////////////
}

#endif

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

namespace tcob::db {

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

template <combine_op Operator, typename C0, typename C1>
inline combined_condition<Operator, C0, C1>::combined_condition(C0 const& cond1, C1 const& cond2)
    : _cond0 {cond1}
    , _cond1 {cond2}
{
}

template <combine_op Operator, typename C0, typename C1>
inline auto combined_condition<Operator, C0, C1>::str() const -> utf8_string
{
    constexpr utf8_string_view op {[] {
        if constexpr (Operator == combine_op::And) { return "AND"; }
        if constexpr (Operator == combine_op::Or) { return "OR"; }
    }()};

    return _not ? std::format("(NOT ({} {} {}))", _cond0.str(), op, _cond1.str())
                : std::format("({} {} {})", _cond0.str(), op, _cond1.str());
}

template <combine_op Operator, typename C0, typename C1>
inline auto combined_condition<Operator, C0, C1>::bind() const -> bind_func
{
    return [cond0 = _cond0, cond2 = _cond1](i32& idx, statement& view) {
        cond0.bind()(idx, view);
        cond2.bind()(idx, view);
    };
}

template <combine_op Operator, typename C0, typename C1>
template <typename C>
inline auto combined_condition<Operator, C0, C1>::operator||(C const& other) const -> combined_condition<combine_op::Or, combined_condition, C>
{
    return {*this, other};
}

template <combine_op Operator, typename C0, typename C1>
template <typename C>
inline auto combined_condition<Operator, C0, C1>::operator&&(C const& other) const -> combined_condition<combine_op::And, combined_condition, C>
{
    return {*this, other};
}

template <combine_op Operator, typename C0, typename C1>
inline auto combined_condition<Operator, C0, C1>::operator!() const -> combined_condition
{
    auto copy {*this};
    copy._not = !copy._not;
    return copy;
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
    switch (Operator) {
    case op::In:      return std::format("{} {} ({})", _column, _not ? "NOT IN" : "IN", helper::join(std::vector<utf8_string>(_params.size(), "?"), ", "));
    case op::Between: return std::format("{} {} ? AND ?", _column, _not ? "NOT BETWEEN" : "BETWEEN");
    default:          break;
    }

    constexpr utf8_string_view op {[] {
        if constexpr (Operator == op::Equal) { return "="; }
        if constexpr (Operator == op::NotEqual) { return "<>"; }
        if constexpr (Operator == op::Greater) { return ">"; }
        if constexpr (Operator == op::GreaterEqual) { return ">="; }
        if constexpr (Operator == op::Less) { return "<"; }
        if constexpr (Operator == op::LessEqual) { return "<="; }
        if constexpr (Operator == op::Like) { return "LIKE"; }
        return "";
    }()};

    auto const base {std::format("{} {} ?", _column, op)};
    return _not ? std::format("NOT ({})", base) : base;
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

template <op Operator>
template <typename C>
inline auto conditional<Operator>::operator||(C const& other) const -> combined_condition<combine_op::Or, conditional, C>
{
    return {*this, other};
}

template <op Operator>
template <typename C>
inline auto conditional<Operator>::operator&&(C const& other) const -> combined_condition<combine_op::And, conditional, C>
{
    return {*this, other};
}

template <op Operator>
inline auto conditional<Operator>::operator!() const -> conditional
{
    auto copy {*this};
    copy._not = !copy._not;
    return copy;
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

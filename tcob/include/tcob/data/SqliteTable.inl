// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteTable.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/SqliteColumn.hpp"

namespace tcob::data::sqlite {

namespace detail {
    auto get_column_string(auto&& column) -> utf8_string
    {
        if constexpr (Aggregate<decltype(column)>) {
            return column.str();
        } else {
            return quote_string(utf8_string {column});
        }
    }

    template <typename... Values>
    auto create_select(database_view db, utf8_string const& name, bool distinct, auto&&... columns) -> select_statement<Values...>
    {
        if constexpr (sizeof...(columns) == 0) {
            return select_statement<Values...> {db, distinct, name, "*"};
        } else {
            std::vector<utf8_string> columnStrings;
            ((columnStrings.push_back(get_column_string(columns))), ...);
            return select_statement<Values...> {db, distinct, name, helper::join(columnStrings, ", ")};
        }
    }
}

template <typename... Values>
inline auto table::select_from(auto&&... columns) const -> select_statement<Values...>
{
    return detail::create_select<Values...>(_db, _name, false, columns...);
}

template <typename... Values>
inline auto table::select_from(distinct_t, auto&&... columns) const -> select_statement<Values...>
{
    return detail::create_select<Values...>(_db, _name, true, columns...);
}

inline auto table::insert_into(auto&&... columns) const -> insert_statement
{
    return insert_into(insert_statement::mode::Normal, std::move(columns)...);
}

inline auto table::insert_into(replace_t, auto&&... columns) const -> insert_statement
{
    return insert_into(insert_statement::mode::Replace, std::move(columns)...);
}

inline auto table::insert_into(ignore_t, auto&&... columns) const -> insert_statement
{
    return insert_into(insert_statement::mode::Ignore, std::move(columns)...);
}

inline auto table::insert_into(insert_statement::mode mode, auto&&... columns) const -> insert_statement
{
    assert(check_columns(columns...));

    std::vector<utf8_string> columnStrings;
    ((columnStrings.push_back(quote_string(utf8_string {columns}))), ...);
    return insert_statement {_db, mode, _name, helper::join(columnStrings, ", ")};
}

inline auto table::update(auto&&... columns) const -> update_statement
{
    assert(check_columns(columns...));

    // SET column1 = value1, column2 = value2...., columnN = valueN
    std::vector<utf8_string> setStrings;
    ((setStrings.push_back(quote_string(utf8_string {columns}) + " = ?")), ...);
    return update_statement {_db, _name, helper::join(setStrings, ", ")};
}

inline auto table::check_columns(auto&&... columns) const -> bool
{
    auto const tableColumns {column_names()};

    auto const check {
        [&tableColumns](auto const& value) {
            if constexpr (requires { decltype(value)::Column; }) {
                return tableColumns.contains(value.Column);
            } else {
                return tableColumns.contains(value);
            }
        }};

    return ((check(columns)) && ...);
}

////////////////////////////////////////////////////////////

template <typename... Values>
inline auto view::select_from(auto&&... columns) const -> select_statement<Values...>
{
    return detail::create_select<Values...>(_db, _name, false, columns...);
}

template <typename... Values>
inline auto view::select_from(distinct_t, auto&&... columns) const -> select_statement<Values...>
{
    return detail::create_select<Values...>(_db, _name, true, columns...);
}
}

#endif

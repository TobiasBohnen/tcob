// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteStatement.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>

    #include "tcob/data/SqliteConversions.hpp" // IWYU pragma: keep

namespace tcob::data::sqlite {

template <typename T>
inline auto statement::get_column_value(i32 col) const -> T
{
    T value {};
    converter<T>::From(statement_view {_stmt}, col, value);
    return value;
}

template <typename T>
inline auto statement::bind_parameter(i32& idx, T&& value) const -> bool
{
    return converter<std::remove_cvref_t<T>>::To(statement_view {_stmt}, idx, std::forward<T>(value));
}

////////////////////////////////////////////////////////////

template <typename... Values>
inline select_statement<Values...>::select_statement(database_view db, bool addDistinct, utf8_string const& table, utf8_string const& columns)
    : statement {db}
    , _distinct {addDistinct}
{
    // SELECT column1, column2, columnN FROM table_name;
    // create query
    _values.Columns = columns;
    _values.Table   = table;
}

template <typename... Values>
inline auto select_statement<Values...>::where(utf8_string const& expr) -> select_statement<Values...>&
{
    _values.Where = std::format(" WHERE {}", expr);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::order_by(utf8_string const& term) -> select_statement<Values...>&
{
    _values.OrderBy = std::format(" ORDER BY {}", term);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::limit(i32 value, std::optional<i32> offset) -> select_statement<Values...>&
{
    _values.Limit = std::format(" LIMIT {} ", value);
    if (offset.has_value()) {
        _values.Offset = std::format(" OFFSET {} ", *offset);
    }

    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::group_by(utf8_string const& column) -> select_statement<Values...>&
{
    _values.GroupBy = std::format(" GROUP BY \"{}\" ", column);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::left_join(utf8_string const& table, utf8_string const& on) -> select_statement<Values...>&
{
    _values.Join = std::format(" LEFT JOIN {} ON {}", table, on);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::inner_join(utf8_string const& table, utf8_string const& on) -> select_statement<Values...>&
{
    _values.Join = std::format(" INNER JOIN {} ON {}", table, on);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::cross_join(utf8_string const& table) -> select_statement<Values...>&
{
    _values.Join = std::format(" CROSS JOIN {}", table);
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::get_query() const -> utf8_string
{
    return std::format(
        "SELECT {} {} FROM {} {} {} {} {} {} {};",
        _distinct ? "DISTINCT" : "",
        _values.Columns, _values.Table,
        _values.Where, _values.OrderBy, _values.Limit, _values.Offset,
        _values.GroupBy, _values.Join);
}

template <typename... Values>
inline auto select_statement<Values...>::operator()()
{
    if constexpr (sizeof...(Values) > 1) {
        std::vector<std::tuple<Values...>> retValue;

        // prepare
        if (prepare(get_query())) {
            if (sizeof...(Values) != get_column_count()) {
                return retValue;
            }

            // get columns
            retValue = get_column_value<std::vector<std::tuple<Values...>>>(0);
        }

        return retValue;
    } else {
        std::vector<Values...> retValue;

        // prepare
        if (prepare(get_query())) {
            if (get_column_count() != 1) { return retValue; }

            // get columns
            retValue = get_column_value<std::vector<Values...>>(0);
        }

        return retValue;
    }
}

template <typename... Values>
template <typename T>
inline select_statement<Values...>::operator std::vector<T>()
{
    std::vector<T> retValue;

    if constexpr (sizeof...(Values) > 1) {
        auto const values {operator()()};
        for (auto const& tup : values) {
            retValue.push_back(std::make_from_tuple<T>(tup));
        }
    } else {
        retValue = operator()();
    }

    return retValue;
}

////////////////////////////////////////////////////////////

inline auto update_statement::operator()(auto&&... values) -> bool
{
    // prepare
    if (!prepare(get_query())) { return false; }

    // bind parameters
    i32 idx {1};
    ((bind_parameter(idx, values)), ...);

    // execute
    return step() == step_status::Done;
}

////////////////////////////////////////////////////////////

namespace detail {
    template <typename T>
    struct value_size {
        static constexpr usize Size {1};
    };

    template <typename... Ts>
    struct value_size<std::tuple<Ts...>> {
        static constexpr usize Size {sizeof...(Ts)};
    };

    template <typename... Ts>
    struct value_size<std::vector<std::tuple<Ts...>>> {
        static constexpr usize Size {sizeof...(Ts)};
    };

    auto value_count(auto&& value, auto&&... values) -> usize
    {
        usize retValue {};
        if constexpr (HasSize<std::remove_cvref_t<decltype(value)>>) {
            retValue = value.size();
        } else {
            retValue = 1;
        }

        if constexpr (sizeof...(values) > 0) {
            return value_count(values...) + retValue;
        } else {
            return retValue;
        }
    }
}

inline auto insert_statement::operator()(auto&& value, auto&&... values) -> bool
{
    // prepare
    usize const valueSize {detail::value_size<std::remove_cvref_t<decltype(value)>>::Size};
    usize const valueCount {detail::value_count(value, values...)};
    if (!prepare(get_query(valueSize, valueCount))) {
        return false;
    }

    // bind parameters
    i32 idx {1};
    bind_parameter(idx, value);
    ((bind_parameter(idx, values)), ...);

    // execute
    return step() == step_status::Done;
}

////////////////////////////////////////////////////////////

inline auto delete_statement::operator()() -> bool
{
    // prepare
    if (!prepare(get_query())) { return false; }

    // execute
    return step() == step_status::Done;
}

}

#endif

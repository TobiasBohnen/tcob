// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteStatement.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <optional>
    #include <tuple>
    #include <type_traits>
    #include <vector>

    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/Sqlite.hpp"

namespace tcob::data::sqlite {

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

////////////////////////////////////////////////////////////

template <typename T>
inline auto statement::get_column_value(i32 col) const -> T
{
    T value {};
    converter<T>::From(_stmt, col, value);
    return value;
}

template <typename T>
inline auto statement::bind_parameter(i32& idx, T&& value) const -> bool
{
    return converter<std::remove_cvref_t<T>>::To(_stmt, idx, std::forward<T>(value));
}

////////////////////////////////////////////////////////////

template <typename... Values>
inline select_statement<Values...>::select_statement(database_view db, bool addDistinct, utf8_string const& table, utf8_string const& columns)
    : statement {db}
    , _distinct {addDistinct}
{
    static_assert(sizeof...(Values) > 0);

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
inline auto select_statement<Values...>::order_by(auto&&... orderings) -> select_statement<Values...>&
{
    std::vector<utf8_string> colStrings {orderings.str()...};
    _values.OrderBy = std::format(" ORDER BY {}", helper::join(colStrings, ", "));
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
inline auto select_statement<Values...>::group_by(auto&&... columns) -> select_statement<Values...>&
{
    std::vector<utf8_string> colStrings {quote_string(columns)...};
    _values.GroupBy = std::format(" GROUP BY {} ", helper::join(colStrings, ", "));
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
inline auto select_statement<Values...>::query_string() const -> utf8_string
{
    return std::format(
        "SELECT {} {} FROM {} {} {} {} {} {} {};",
        _distinct ? "DISTINCT" : "",
        _values.Columns, _values.Table,
        _values.Where, _values.OrderBy, _values.Limit, _values.Offset,
        _values.GroupBy, _values.Join);
}

template <typename... Values>
inline auto select_statement<Values...>::operator() [[nodiscard]] (auto&&... params)
{
    // prepare
    bool const prepared {prepare(query_string())};

    if constexpr (sizeof...(params) > 0) {
        if (prepared) {
            // bind parameters
            i32 idx {1};
            ((bind_parameter(idx, params)), ...);
        }
    }

    if constexpr (sizeof...(Values) > 1) {
        std::vector<std::tuple<Values...>> retValue;

        // prepare
        if (prepared) {
            if (sizeof...(Values) != column_count()) { return retValue; }

            // get columns
            retValue = get_column_value<std::vector<std::tuple<Values...>>>(0);
        }

        return retValue;
    }

    if constexpr (sizeof...(Values) == 1) {
        std::vector<Values...> retValue;

        // prepare
        if (prepared) {
            if (column_count() != 1) { return retValue; }

            // get columns
            retValue = get_column_value<std::vector<Values...>>(0);
        }

        return retValue;
    }
}

template <typename... Values>
template <typename T>
inline auto select_statement<Values...>::exec [[nodiscard]] (auto&&... params) -> std::vector<T>
{
    static_assert(sizeof...(Values) > 1);

    // prepare
    bool const prepared {prepare(query_string())};

    if constexpr (sizeof...(params) > 0) {
        if (prepared) {
            // bind parameters
            i32 idx {1};
            ((bind_parameter(idx, params)), ...);
        }
    }

    std::vector<T> retValue;
    if (prepared) {
        if (sizeof...(Values) != column_count()) { return retValue; }

        // get columns
        auto const values {get_column_value<std::vector<std::tuple<Values...>>>(0)};
        retValue.reserve(values.size());
        for (auto const& tup : values) {
            retValue.push_back(std::make_from_tuple<T>(tup));
        }
    }

    return retValue;
}

////////////////////////////////////////////////////////////

inline auto update_statement::operator()(auto&&... values) -> bool
{
    // prepare
    if (!prepare(query_string())) { return false; }

    // bind parameters
    i32 idx {1};
    ((bind_parameter(idx, values)), ...);

    // execute
    return step() == step_status::Done;
}

////////////////////////////////////////////////////////////

inline auto insert_statement::operator()(auto&& value, auto&&... values) -> bool
{
    // prepare
    usize const valueSize {detail::value_size<std::remove_cvref_t<decltype(value)>>::Size};
    usize const valueCount {detail::value_count(value, values...)};
    if (!prepare(query_string(valueSize, valueCount))) {
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

inline auto delete_statement::operator()(auto&&... values) -> bool
{
    // prepare
    if (!prepare(query_string())) { return false; }

    if constexpr (sizeof...(values) > 0) {
        // bind parameters
        i32 idx {1};
        ((bind_parameter(idx, values)), ...);
    }

    // execute
    return step() == step_status::Done;
}
}

#endif

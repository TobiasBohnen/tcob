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

namespace tcob::db {

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
template <typename T>
inline auto select_statement<Values...>::where(T const& cond) -> select_statement<Values...>&
{
    if constexpr (detail::HasBind<T>) {
        _values.Where = std::format(" WHERE {}", cond.str());
        _whereBind    = cond.bind();
    } else {
        _values.Where = std::format(" WHERE {}", cond);
    }

    return *this;
}

template <typename... Values>
template <typename T>
inline auto select_statement<Values...>::having(T const& cond) -> select_statement&
{
    if constexpr (detail::HasBind<T>) {
        _values.Having = std::format(" HAVING {}", cond.str());
        _havingBind    = cond.bind();
    } else {
        _values.Having = std::format(" HAVING {}", cond);
    }

    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::order_by(auto&&... orders) -> select_statement<Values...>&
{
    std::vector<utf8_string> colStrings {[&]() {
        if constexpr (detail::HasStr<std::remove_cvref_t<decltype(orders)>>) {
            return orders.str();
        } else {
            return orders;
        }
    }()...};
    _values.OrderBy = std::format(" ORDER BY {}", helper::join(colStrings, ", "));
    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::limit(i32 value, std::optional<i32> offset) -> select_statement<Values...>&
{
    _values.Limit = std::format(" LIMIT {}", value);
    if (offset.has_value()) {
        _values.Offset = std::format(" OFFSET {}", *offset);
    }

    return *this;
}

template <typename... Values>
inline auto select_statement<Values...>::group_by(auto&&... columns) -> select_statement<Values...>&
{
    std::vector<utf8_string> colStrings {[&]() {
        if constexpr (detail::HasStr<std::remove_cvref_t<decltype(columns)>>) {
            return columns.str();
        } else {
            return quote_identifier(columns);
        }
    }()...};

    _values.GroupBy = std::format(" GROUP BY {}", helper::join(colStrings, ", "));
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
        "SELECT{}{} FROM {}{}{}{}{}{}{}{};",
        _distinct ? " DISTINCT " : " ",
        _values.Columns, _values.Table,
        _values.Join,
        _values.Where, _values.GroupBy,
        _values.Having, _values.OrderBy,
        _values.Limit, _values.Offset);
}

template <typename... Values>
inline auto select_statement<Values...>::prepare_and_bind(auto&&... params) -> bool
{
    // prepare
    if (!prepare(query_string())) { return false; };

    // bind parameters
    i32 idx {1};
    if constexpr (sizeof...(params) > 0) {
        ((bind_parameter(idx, params)), ...);
    }
    if (_whereBind) { (*_whereBind)(idx, *this); }
    if (_havingBind) { (*_havingBind)(idx, *this); }

    return true;
}

template <typename... Values>
inline auto select_statement<Values...>::operator() [[nodiscard]] (auto&&... params)
{
    bool const prepared(prepare_and_bind(params...));

    if constexpr (sizeof...(Values) > 1) {
        using return_type = std::vector<std::tuple<Values...>>;
        if (!prepared) { return return_type {}; }
        if (sizeof...(Values) != column_count()) { return return_type {}; }

        // get columns
        return get_column_value<return_type>(0);
    }

    if constexpr (sizeof...(Values) == 1) {
        using return_type = std::vector<Values...>;
        if (!prepared) { return return_type {}; }
        if (column_count() != 1) { return return_type {}; }

        // get columns
        return get_column_value<return_type>(0);
    }
}

template <typename... Values>
template <typename T>
inline auto select_statement<Values...>::exec [[nodiscard]] (auto&&... params) -> std::vector<T>
{
    static_assert(sizeof...(Values) > 1);
    using return_type = std::vector<T>;

    if (!prepare_and_bind(params...)) { return return_type {}; }
    if (sizeof...(Values) != column_count()) { return return_type {}; }

    // get columns
    auto const  values {get_column_value<std::vector<std::tuple<Values...>>>(0)};
    return_type retValue;
    retValue.reserve(values.size());
    for (auto const& tup : values) {
        retValue.push_back(std::make_from_tuple<T>(tup));
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
    if (_whereBind) { (*_whereBind)(idx, *this); }

    // execute
    return step() == step_status::Done;
}

template <typename T>
inline auto update_statement::where(T const& cond) -> update_statement&
{
    if constexpr (detail::HasBind<std::remove_cvref_t<T>>) {
        _where     = cond.str();
        _whereBind = cond.bind();
    } else {
        _where = cond;
    }

    return *this;
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

    // bind parameters
    i32 idx {1};
    if constexpr (sizeof...(values) > 0) {
        ((bind_parameter(idx, values)), ...);
    }
    if (_whereBind) { (*_whereBind)(idx, *this); }

    // execute
    return step() == step_status::Done;
}

template <typename T>
inline auto delete_statement::where(T const& cond) -> delete_statement&
{
    if constexpr (detail::HasBind<std::remove_cvref_t<T>>) {
        _where     = cond.str();
        _whereBind = cond.bind();
    } else {
        _where = cond;
    }

    return *this;
}

}

#endif

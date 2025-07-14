// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <functional>
    #include <optional>
    #include <vector>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteConversions.hpp"

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

class TCOB_API statement : public non_copyable {
public:
    explicit statement(database_view db);
    statement(statement&& other) noexcept;
    auto operator=(statement&& other) noexcept -> statement&;
    ~statement();

    auto prepare(utf8_string const& sql) -> bool;
    auto step() const -> step_status;

    auto column_count() const -> i32;
    template <typename T>
    auto get_column_value(i32 col) const -> T;
    auto get_column_name(i32 col) const -> utf8_string;

    template <typename T>
    auto bind_parameter(i32& idx, T&& value) const -> bool;

protected:
    auto get_db() const -> database_view;

    auto is_valid() const -> bool;

private:
    database_view  _db;
    statement_view _stmt;
};

using bind_func = std::function<void(i32&, statement&)>;

namespace detail {
    template <typename T>
    concept HasBind =
        requires(T t) {
            { t.str() } -> std::same_as<utf8_string>;
            { t.bind() } -> std::same_as<bind_func>;
        };
}

////////////////////////////////////////////////////////////

template <typename... Values>
class select_statement : public statement {
public:
    select_statement(database_view db, bool addDistinct, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... params);

    template <typename T>
    auto exec [[nodiscard]] (auto&&... params) -> std::vector<T>;

    template <typename T>
    auto where(T const& cond) -> select_statement&;

    auto order_by(auto&&... orders) -> select_statement&;
    auto limit(i32 value, std::optional<i32> offset = std::nullopt) -> select_statement&;
    auto group_by(auto&&... columns) -> select_statement&;

    auto left_join(utf8_string const& table, utf8_string const& on) -> select_statement&;
    auto inner_join(utf8_string const& table, utf8_string const& on) -> select_statement&;
    auto cross_join(utf8_string const& table) -> select_statement&;

    auto query_string() const -> utf8_string;

private:
    auto prepare_and_bind(auto&&... params) -> bool;

    struct values {
        utf8_string Columns;
        utf8_string Table;
        utf8_string Where;
        utf8_string OrderBy;
        utf8_string Limit;
        utf8_string Offset;
        utf8_string GroupBy;
        utf8_string Join;
    };

    values                   _values;
    std::optional<bind_func> _whereBind;
    bool                     _distinct;
};

////////////////////////////////////////////////////////////

class TCOB_API update_statement : public statement {
public:
    update_statement(database_view db, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    template <typename T>
    auto where(T const& cond) -> update_statement&;

private:
    auto query_string() const -> utf8_string;

    utf8_string              _where;
    std::optional<bind_func> _whereBind;
    utf8_string              _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API insert_statement : public statement {
public:
    enum mode : u8 {
        Normal,
        Ignore,
        Replace
    };

    insert_statement(database_view db, mode mode, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&& value, auto&&... values) -> bool;

private:
    auto query_string(usize valueSize, usize valueCount) const -> utf8_string;

    utf8_string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API delete_statement : public statement {
public:
    delete_statement(database_view db, utf8_string const& table);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    template <typename T>
    auto where(T const& cond) -> delete_statement&;

private:
    auto query_string() const -> utf8_string;

    utf8_string              _where;
    std::optional<bind_func> _whereBind;
    utf8_string              _sql;
};

}

    #include "SqliteStatement.inl"

#endif

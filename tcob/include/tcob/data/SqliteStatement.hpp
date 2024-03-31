// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <optional>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/data/Sqlite.hpp"

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

    auto get_column_count() const -> i32;
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

////////////////////////////////////////////////////////////

template <typename... Values>
class select_statement : public statement {
public:
    select_statement(database_view db, bool addDistinct, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... params);

    template <typename T>
    explicit operator std::vector<T>();

    auto where(utf8_string const& expr) -> select_statement&;

    auto order_by(utf8_string const& term) -> select_statement&;
    auto limit(i32 value, std::optional<i32> offset = std::nullopt) -> select_statement&;
    auto group_by(utf8_string const& column) -> select_statement&;

    auto left_join(utf8_string const& table, utf8_string const& on) -> select_statement&;
    auto inner_join(utf8_string const& table, utf8_string const& on) -> select_statement&;
    auto cross_join(utf8_string const& table) -> select_statement&;

    auto get_query() const -> utf8_string;

private:
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

    values _values;
    bool   _distinct;
};

////////////////////////////////////////////////////////////

class TCOB_API update_statement : public statement {
public:
    update_statement(database_view db, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    auto where(utf8_string const& expr) -> update_statement&;

private:
    auto get_query() const -> utf8_string;

    utf8_string _where;
    utf8_string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API insert_statement : public statement {
public:
    enum mode {
        Normal,
        Ignore,
        Replace
    };

    insert_statement(database_view db, mode mode, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&& value, auto&&... values) -> bool;

private:
    auto get_query(usize valueSize, usize valueCount) const -> utf8_string;

    utf8_string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API delete_statement : public statement {
public:
    delete_statement(database_view db, utf8_string const& table);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    auto where(utf8_string const& expr) -> delete_statement&;

private:
    auto get_query() const -> utf8_string;

    utf8_string _where;
    utf8_string _sql;
};

}

    #include "SqliteStatement.inl"

#endif

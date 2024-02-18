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

    auto prepare(string const& sql) -> bool;
    auto step() const -> step_status;

    auto get_column_count() const -> i32;
    template <typename T>
    auto get_column_value(i32 col) const -> T;
    auto get_column_name(i32 col) const -> string;

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
    select_statement(database_view db, bool distinct, string const& table, string const& columns);

    auto operator() [[nodiscard]] ();

    template <typename T>
    operator std::vector<T>();

    auto where(string const& expr) -> select_statement&;
    auto order_by(string const& term) -> select_statement&;
    auto limit(i32 value, std::optional<i32> offset = std::nullopt) -> select_statement&;
    auto group_by(string const& column) -> select_statement&;

    auto left_join(string const& table, string const& on) -> select_statement&;
    auto inner_join(string const& table, string const& on) -> select_statement&;
    auto cross_join(string const& table) -> select_statement&;

    auto get_query() const -> string;

private:
    struct values {
        string Columns;
        string Table;
        string Where;
        string OrderBy;
        string Limit;
        string Offset;
        string GroupBy;
        string Join;
    };

    values _values;
    bool   _distinct;
};

////////////////////////////////////////////////////////////

class TCOB_API update_statement : public statement {
public:
    update_statement(database_view db, string const& table, string const& columns);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    auto where(string const& expr) -> update_statement&;

private:
    auto get_query() const -> string;

    string _where;
    string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API insert_statement : public statement {
public:
    insert_statement(database_view db, string const& table, string const& columns);

    auto operator() [[nodiscard]] (auto&& value, auto&&... values) -> bool;

private:
    auto get_query(usize valueSize, usize valueCount) const -> string;

    string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API delete_statement : public statement {
public:
    delete_statement(database_view db, string const& table);

    auto operator() [[nodiscard]] () -> bool;

    auto where(string const& expr) -> delete_statement&;

private:
    auto get_query() const -> string;

    string _where;
    string _sql;
};

}

    #include "SqliteStatement.inl"

#endif

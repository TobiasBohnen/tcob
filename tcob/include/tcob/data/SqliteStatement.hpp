// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <functional>
    #include <optional>
    #include <utility>
    #include <vector>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteConversions.hpp"

namespace tcob::db {
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

////////////////////////////////////////////////////////////

template <typename... Values>
class select_statement : public statement {
public:
    select_statement(database_view db, bool addDistinct, utf8_string const& schemaName, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... params);

    template <typename T>
    auto exec [[nodiscard]] (auto&&... params) -> std::vector<T>;

    template <typename T>
    auto where(T const& cond) -> select_statement&;
    template <typename T>
    auto having(T const& cond) -> select_statement&;

    auto order_by(auto&&... orderings) -> select_statement&;
    auto limit(i32 value, std::optional<i32> offset = std::nullopt) -> select_statement&;
    auto group_by(auto&&... columns) -> select_statement&;

    auto left_join(auto const& table, auto const& on) -> select_statement&;
    auto right_join(auto const& table, auto const& on) -> select_statement&;
    auto full_join(auto const& table, auto const& on) -> select_statement&;
    auto inner_join(auto const& table, auto const& on) -> select_statement&;
    auto cross_join(auto const& table) -> select_statement&;

    auto union_all_with(select_statement const& other) -> select_statement&;
    auto union_with(select_statement const& other) -> select_statement&;
    auto intersect(select_statement const& other) -> select_statement&;
    auto except(select_statement const& other) -> select_statement&;

    auto query_string() const -> utf8_string;

private:
    template <typename T, typename O>
    auto on_str(T const& table, O const& on) -> string;
    template <typename T>
    auto table_str(T const& table) -> string;

    auto prepare_and_bind(auto&&... params) -> bool;

    struct values {
        utf8_string Columns;
        utf8_string Schema;
        utf8_string Table;
        utf8_string Where;
        utf8_string OrderBy;
        utf8_string Limit;
        utf8_string Offset;
        utf8_string GroupBy;
        utf8_string Having;
        utf8_string Join;
    };

    std::vector<std::pair<utf8_string, utf8_string>> _setOps;
    values                                           _values;
    bind_func                                        _whereBind;
    bind_func                                        _havingBind;
    bool                                             _distinct;
};

////////////////////////////////////////////////////////////

class TCOB_API update_statement : public statement {
public:
    update_statement(database_view db, utf8_string const& schemaName, utf8_string const& table, utf8_string const& columns);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    template <typename T>
    auto where(T const& cond) -> update_statement&;

private:
    auto query_string() const -> utf8_string;

    utf8_string _where;
    bind_func   _whereBind;
    utf8_string _sql;
};

////////////////////////////////////////////////////////////

class TCOB_API insert_statement : public statement {
public:
    enum mode : u8 {
        Normal,
        Ignore,
        Replace
    };

    insert_statement(database_view db, mode mode, utf8_string const& schemaName, utf8_string const& table, utf8_string const& columns, usize columnCount);

    auto operator() [[nodiscard]] (auto&& value, auto&&... values) -> bool;

private:
    auto query_string(usize columnCount, usize rowCount) const -> utf8_string;

    utf8_string _sql;
    usize       _columnCount;
};

////////////////////////////////////////////////////////////

class TCOB_API delete_statement : public statement {
public:
    delete_statement(database_view db, utf8_string const& schemaName, utf8_string const& table);

    auto operator() [[nodiscard]] (auto&&... values) -> bool;

    template <typename T>
    auto where(T const& cond) -> delete_statement&;

private:
    auto query_string() const -> utf8_string;

    utf8_string _where;
    bind_func   _whereBind;
    utf8_string _sql;
};

}

    #include "SqliteStatement.inl"

#endif

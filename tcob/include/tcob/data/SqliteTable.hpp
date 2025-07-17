// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <set>
    #include <vector>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::db {
////////////////////////////////////////////////////////////

struct distinct_t { };
inline constexpr distinct_t distinct;

struct ignore_t { };
inline constexpr ignore_t ignore;

struct replace_t { };
inline constexpr replace_t replace;

struct column_info {
    utf8_string Name;
    utf8_string Type;
    bool        NotNull {false};
    bool        IsPrimaryKey {false};
};

////////////////////////////////////////////////////////////

class TCOB_API table final {
public:
    table(database_view db, utf8_string schema, utf8_string name);

    auto name() const -> utf8_string const&;
    auto qualified_name() const -> utf8_string;
    auto column_names() const -> std::set<utf8_string>;
    auto row_count() const -> i32;
    auto info() const -> std::vector<column_info>;

    template <typename... Values>
    auto select_from(auto&&... columns) const -> select_statement<Values...>;
    template <typename... Values>
    auto select_from(distinct_t, auto&&... columns) const -> select_statement<Values...>;

    auto insert_into(auto&&... columns) const -> insert_statement;
    auto insert_into(replace_t, auto&&... columns) const -> insert_statement;
    auto insert_into(ignore_t, auto&&... columns) const -> insert_statement;

    auto update(auto&&... columns) const -> update_statement;

    auto delete_from() const -> delete_statement;

private:
    auto insert_into(insert_statement::mode mode, auto&&... columns) const -> insert_statement;

    auto check_columns(auto&&... columns) const -> bool;

    database_view _db;
    utf8_string   _schema;
    utf8_string   _name;
};

////////////////////////////////////////////////////////////

class TCOB_API view final {
public:
    view(database_view db, utf8_string schema, utf8_string name);

    template <typename... Values>
    auto select_from(auto&&... columns) const -> select_statement<Values...>;
    template <typename... Values>
    auto select_from(distinct_t, auto&&... columns) const -> select_statement<Values...>;

private:
    database_view _db;
    utf8_string   _schema;
    utf8_string   _name;
};

}

    #include "SqliteTable.inl"

#endif

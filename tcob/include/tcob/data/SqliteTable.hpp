// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <set>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteColumn.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

class TCOB_API table final {
public:
    table(database_view db, utf8_string name);

    auto get_name() const -> utf8_string const&;
    auto get_column_names() const -> std::set<utf8_string>;
    auto get_row_count() const -> i32;

    template <typename... Values>
    auto select_from(auto&&... columns) const -> select_statement<Values...>;
    template <typename... Values>
    auto select_from(distinct, auto&&... columns) const -> select_statement<Values...>;
    auto insert_into(auto&&... columns) const -> insert_statement;
    auto update(auto&&... columns) const -> update_statement;
    auto delete_from() const -> delete_statement;

private:
    auto check_columns(auto&&... columns) const -> bool;

    database_view _db;
    utf8_string   _name;
};

////////////////////////////////////////////////////////////

class TCOB_API view final {
public:
    view(database_view db, utf8_string name);

    template <typename... Values>
    auto select_from(auto&&... columns) const -> select_statement<Values...>;
    template <typename... Values>
    auto select_from(distinct, auto&&... columns) const -> select_statement<Values...>;

private:
    database_view _db;
    utf8_string   _name;
};

}

    #include "SqliteTable.inl"

#endif

// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <optional>
    #include <set>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::db {

////////////////////////////////////////////////////////////

class TCOB_API schema final {
public:
    schema(database_view db, utf8_string name);

    auto create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>;
    template <typename... Values>
    auto create_view(utf8_string const& viewName, select_statement<Values...>& stmt) -> std::optional<view>;

    auto table_names() const -> std::set<utf8_string>;
    auto view_names() const -> std::set<utf8_string>;

    auto table_exists(utf8_string const& tableName) const -> bool;
    auto view_exists(utf8_string const& viewName) const -> bool;

    auto get_table(utf8_string const& tableName) const -> std::optional<table>;
    auto get_view(utf8_string const& viewName) const -> std::optional<view>;

    auto drop_table(utf8_string const& tableName) const -> bool;
    auto drop_view(utf8_string const& viewName) const -> bool;

    auto vacuum_into(path const& file) const -> bool;

    auto detach() const -> bool;

private:
    database_view _db {nullptr};
    utf8_string   _name;
};

}

    #include "SqliteSchema.inl"

#endif

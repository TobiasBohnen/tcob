// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteTable.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <set>
    #include <utility>
    #include <vector>

    #include "tcob/core/StringUtils.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::db {

static auto get_column_names(auto&& select, utf8_string const& name) -> std::set<utf8_string>
{
    std::set<utf8_string> retValue;

    if (select.prepare(std::format("SELECT * FROM {};", name))) {
        i32 const count {select.column_count()};
        for (i32 i {0}; i < count; i++) {
            retValue.insert(select.get_column_name(i));
        }
    }

    return retValue;
}

static auto get_row_count(auto&& select, utf8_string const& name) -> i32
{
    // SELECT COUNT(1) FROM table
    if (select.prepare(std::format("SELECT COUNT(1) FROM {};", name))) {
        if (select.step() == step_status::Row) {
            return select.template get_column_value<i32>(0);
        }
    }

    return 0;
}

table::table(database_view db, utf8_string schema, utf8_string name)
    : _db {db}
    , _schema {std::move(schema)}
    , _name {std::move(name)}
{
}

auto table::name() const -> utf8_string const&
{
    return _name;
}

auto table::qualified_name() const -> utf8_string
{
    return std::format(R"("{}"."{}")", _schema, _name);
}

auto table::info() const -> std::vector<column_info>
{
    std::vector<column_info> retValue;

    statement pragma {_db};
    if (pragma.prepare(std::format("PRAGMA {}.table_info({});", _schema, _name))) {
        while (pragma.step() == step_status::Row) {
            column_info const info {
                .Name         = pragma.get_column_value<utf8_string>(1),
                .Type         = pragma.get_column_value<utf8_string>(2),
                .NotNull      = pragma.get_column_value<i32>(3) != 0,
                .IsPrimaryKey = pragma.get_column_value<i32>(5) != 0,
            };
            retValue.push_back(info);
        }
    }

    return retValue;
}

auto table::column_names() const -> std::set<utf8_string>
{
    statement select {_db};
    return get_column_names(select, qualified_name());
}

auto table::row_count() const -> i32
{
    statement select {_db};
    return get_row_count(select, qualified_name());
}

auto table::delete_from() const -> delete_statement
{
    return delete_statement {_db, _schema, _name};
}

auto table::rename(utf8_string const& newName) -> bool
{
    statement alter {_db};
    if (alter.prepare(std::format("ALTER TABLE {} RENAME TO {};", qualified_name(), quote_identifier(newName)))) {
        if (alter.step() == step_status::Done) {
            _name = newName;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////

view::view(database_view db, utf8_string schema, utf8_string name)
    : _db {db}
    , _schema {std::move(schema)}
    , _name {std::move(name)}
{
}

auto view::name() const -> utf8_string const&
{
    return _name;
}

auto view::qualified_name() const -> utf8_string
{
    return std::format(R"("{}"."{}")", _schema, _name);
}

auto view::column_names() const -> std::set<utf8_string>
{
    statement select {_db};
    return get_column_names(select, qualified_name());
}

auto view::row_count() const -> i32
{
    statement select {_db};
    return get_row_count(select, qualified_name());
}

}

#endif

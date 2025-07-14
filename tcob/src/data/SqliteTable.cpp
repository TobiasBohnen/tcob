// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteTable.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <set>
    #include <utility>
    #include <vector>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::data::sqlite {

table::table(database_view db, utf8_string name)
    : _db {db}
    , _name {std::move(name)}
{
}

auto table::name() const -> utf8_string const&
{
    return _name;
}

auto table::column_names() const -> std::set<utf8_string>
{
    std::set<utf8_string> retValue;

    statement select {_db};
    if (select.prepare("SELECT * FROM " + _name + ";")) {
        i32 const count {select.column_count()};
        for (i32 i {0}; i < count; i++) {
            retValue.insert(select.get_column_name(i));
        }
    }

    return retValue;
}

auto table::row_count() const -> i32
{
    // SELECT COUNT(1) FROM table
    statement select {_db};
    if (select.prepare("SELECT COUNT(1) FROM " + _name + ";")) {
        if (select.step() == step_status::Row) {
            return select.get_column_value<i32>(0);
        }
    }

    return 0;
}

auto table::schema() const -> std::vector<column_info>
{
    std::vector<column_info> result;

    statement pragma {_db};
    if (pragma.prepare("PRAGMA table_info(" + _name + ")")) {
        while (pragma.step() == step_status::Row) {
            column_info info;
            info.Name         = pragma.get_column_value<utf8_string>(1); // name
            info.Type         = pragma.get_column_value<utf8_string>(2); // type
            info.NotNull      = pragma.get_column_value<i32>(3) != 0;    // notnull
            info.IsPrimaryKey = pragma.get_column_value<i32>(5) != 0;    // pk
            result.push_back(std::move(info));
        }
    }

    return result;
}

auto table::delete_from() const -> delete_statement
{
    return delete_statement {_db, _name};
}

////////////////////////////////////////////////////////////

view::view(database_view db, utf8_string name)
    : _db {db}
    , _name {std::move(name)}
{
}

}

#endif

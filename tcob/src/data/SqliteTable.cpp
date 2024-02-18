// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteTable.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

namespace tcob::data::sqlite {

table::table(database_view db, string name)
    : _db {db}
    , _name {std::move(name)}
{
}

auto table::get_name() const -> string const&
{
    return _name;
}

auto table::get_column_names() const -> std::set<string>
{
    // SELECT name FROM sqlite_schema WHERE type='table' ORDER BY name
    std::set<string> retValue;

    statement select {_db};
    if (select.prepare("SELECT * FROM " + _name + ";")) {

        if (select.step() != step_status::Error) {
            i32 const count {select.get_column_count()};
            for (i32 i {0}; i < count; i++) {
                retValue.insert(select.get_column_name(i));
            }
        }
    }

    return retValue;
}

auto table::get_row_count() const -> i32
{
    // SELECT COUNT(1) FROM table
    statement select {_db};
    if (select.prepare("SELECT COUNT(1) FROM " + _name + ";")) {
        if (select.step() != step_status::Error) {
            return select.get_column_value<i32>(0);
        }
    }

    return 0;
}

auto table::delete_from() const -> delete_statement
{
    return delete_statement {_db, _name};
}

////////////////////////////////////////////////////////////

view::view(database_view db, string name)
    : _db {db}
    , _name {std::move(name)}
{
}

}

#endif

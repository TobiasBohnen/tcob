// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/data/SqliteSavepoint.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

namespace tcob::data::sqlite {

savepoint::savepoint(database_view db, utf8_string name)
    : _db {db}
    , _name {std::move(name)}
{
    db.exec("SAVEPOINT " + _name + ";");
}

savepoint::~savepoint()
{
    rollback();
    release();
}

void savepoint::release()
{
    if (!_released) {
        _db.exec("RELEASE SAVEPOINT " + _name + ";");
        _released = true;
    }
}

void savepoint::rollback()
{
    if (!_released) {
        _db.exec("ROLLBACK TO SAVEPOINT " + _name + ";");
    }
}

}

#endif

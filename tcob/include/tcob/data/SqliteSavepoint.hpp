// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include "tcob/core/Interfaces.hpp"
#include "tcob/data/Sqlite.hpp"

namespace tcob::db {
////////////////////////////////////////////////////////////

class TCOB_API savepoint : public non_copyable {
public:
    savepoint(database_view db, utf8_string name);
    ~savepoint();

    void release();
    void rollback();

private:
    database_view _db;
    utf8_string   _name;
    bool          _released {false};
};

}

// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <functional>
    #include <optional>
    #include <set>

    #include "tcob/core/Interfaces.hpp"
    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteSavepoint.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

enum class update_mode : u8 {
    Insert,
    Delete,
    Update
};

enum class journal_mode : u8 {
    // TRUNCATE | PERSIST
    Delete,
    Memory,
    Wal,
    Off
};

////////////////////////////////////////////////////////////

class TCOB_API database final : public non_copyable {
public:
    database();
    explicit database(database_view db);
    ~database();

    void set_journal_mode(journal_mode mode) const;

    auto get_table_names() const -> std::set<utf8_string>;
    auto get_view_names() const -> std::set<utf8_string>;

    auto create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>;
    template <typename... Values>
    auto create_view(utf8_string const& viewName, select_statement<Values...>& stmt, bool temp = true) -> std::optional<view>;
    auto create_savepoint(utf8_string const& name) const -> savepoint;
    auto create_statement() const -> statement;

    auto table_exists(utf8_string const& tableName) const -> bool;
    auto view_exists(utf8_string const& viewName) const -> bool;

    auto get_table(utf8_string const& tableName) const -> std::optional<table>;
    auto get_view(utf8_string const& viewName) const -> std::optional<view>;

    auto drop_table(utf8_string const& tableName) const -> bool;
    auto drop_view(utf8_string const& viewName) const -> bool;

    auto vacuum_into(path const& file) const -> bool;

    void set_commit_hook(std::function<i32(database*)>&& func);
    auto call_commit_hook() -> i32;

    void set_rollback_hook(std::function<void(database*)>&& func);
    void call_rollback_hook();

    void set_update_hook(std::function<void(database*, update_mode, utf8_string, utf8_string, i64)>&& func);
    void call_update_hook(update_mode mode, utf8_string const& dbName, utf8_string const& table, i64 rowId);

    auto static Open(path const& file) -> std::optional<database>; // TODO: change to result
    auto static OpenMemory() -> database;

private:
    void close();

    database_view _db {nullptr};

    std::function<i32(database*)>                                              _commitHookFunc;
    std::function<void(database*)>                                             _rbHookFunc;
    std::function<void(database*, update_mode, utf8_string, utf8_string, i64)> _updateHookFunc;
};

}

    #include "SqliteDatabase.inl"

#endif

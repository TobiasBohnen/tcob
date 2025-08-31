// Copyright (c) 2025 Tobias Bohnen
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
    #include "tcob/data/SqliteSchema.hpp"
    #include "tcob/data/SqliteStatement.hpp"
    #include "tcob/data/SqliteTable.hpp"

namespace tcob::db {
////////////////////////////////////////////////////////////

enum class update_mode : u8 {
    Insert,
    Delete,
    Update
};

enum class journal_mode : u8 {
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
    database(database&& other) noexcept;
    auto operator=(database&& other) noexcept -> database&;
    ~database();

    auto create_table(utf8_string const& tableName, auto&&... columns) const -> std::optional<table>;
    template <typename... Values>
    auto create_view(utf8_string const& viewName, select_statement<Values...>& stmt) -> std::optional<view>;
    auto create_savepoint(utf8_string const& name) const -> savepoint;

    auto schema_names() const -> std::set<utf8_string>;
    auto table_names() const -> std::set<utf8_string>;
    auto view_names() const -> std::set<utf8_string>;

    auto schema_exists(utf8_string const& schema) const -> bool;
    auto table_exists(utf8_string const& tableName) const -> bool;
    auto view_exists(utf8_string const& viewName) const -> bool;

    auto get_schema(utf8_string const& schemaName) const -> std::optional<schema>;
    auto get_table(utf8_string const& tableName) const -> std::optional<table>;
    auto get_view(utf8_string const& viewName) const -> std::optional<view>;

    auto drop_table(utf8_string const& tableName) const -> bool;
    auto drop_view(utf8_string const& viewName) const -> bool;

    auto vacuum() const -> bool;
    auto vacuum_into(path const& file) const -> bool;

    auto attach_memory(utf8_string const& alias) const -> std::optional<schema>;
    auto attach(path const& file, utf8_string const& alias) const -> std::optional<schema>;

    void set_commit_hook(std::function<i32(database*)>&& func);
    auto call_commit_hook() -> i32;

    void set_rollback_hook(std::function<void(database*)>&& func);
    void call_rollback_hook();

    void set_update_hook(std::function<void(database*, update_mode, utf8_string, utf8_string, i64)>&& func);
    void call_update_hook(update_mode mode, utf8_string const& dbName, utf8_string const& table, i64 rowId);

    static auto Open(path const& file) -> std::optional<database>;                    // TODO: change to result
    static auto Open(path const& file, journal_mode mode) -> std::optional<database>; // TODO: change to result
    static auto OpenMemory() -> database;

private:
    void set_journal_mode(journal_mode mode) const;

    void close();

    database_view _db {nullptr};
    schema        _main;

    std::function<i32(database*)>                                              _commitHookFunc {};
    std::function<void(database*)>                                             _rbHookFunc {};
    std::function<void(database*, update_mode, utf8_string, utf8_string, i64)> _updateHookFunc {};
};

}

    #include "SqliteDatabase.inl"

#endif

// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

struct sqlite3;
struct sqlite3_stmt;

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

class statement_view;

template <typename T>
struct converter;

template <typename T>
concept ConvertibleTo =
    requires(statement_view stmt, i32& idx, T const& t) {
        {
            converter<T>::To(stmt, idx, t)
        };
    } || requires(statement_view stmt, i32 idx, T const& t) {
        {
            converter<std::remove_cvref_t<T>>::To(stmt, idx, t)
        };
    };

template <typename T>
concept ConvertibleFrom = requires(statement_view stmt, i32 col, T& t) {
    {
        converter<T>::From(stmt, col, t)
    };
} || requires(statement_view stmt, i32 col, T& t) {
    {
        converter<std::remove_cvref_t<T>>::From(stmt, col, t)
    };
};

////////////////////////////////////////////////////////////

enum class type : u8 {
    Text,
    Numeric,
    Integer,
    Real,
    Blob,
    Null
};

////////////////////////////////////////////////////////////

enum class step_status : u8 {
    Row,
    Done,
    Error
};

////////////////////////////////////////////////////////////

class TCOB_API statement_view final {
public:
    explicit statement_view(sqlite3_stmt* stmt);

    explicit operator bool() const;

    auto get_column_count() const -> i32;

    auto step() const -> step_status;

    auto column_double(i32 col) const -> f64;
    auto column_int(i32 col) const -> i32;
    auto column_int64(i32 col) const -> i64;
    auto column_text(i32 col) const -> string;
    auto column_blob(i32 col) const -> void const*;

    auto bind(i32 idx, f64 value) const -> bool;
    auto bind(i32 idx, i32 value) const -> bool;
    auto bind(i32 idx, i64 value) const -> bool;
    auto bind(i32 idx, string_view value) const -> bool;
    auto bind(i32 idx, char const* value) const -> bool;
    auto bind(i32 idx, void const* value, i64 size) const -> bool;
    auto bind_null(i32 idx) -> bool;

    void finalize() const;

    auto get_column_name(i32 col) const -> string;
    auto get_column_type(i32 col) const -> type;

private:
    sqlite3_stmt* _stmt {nullptr};
};

////////////////////////////////////////////////////////////

class TCOB_API database_view final {
public:
    explicit database_view(sqlite3* db);

    explicit operator bool() const;

    auto get_error_message() const -> string;

    auto open(path const& file) -> bool;

    auto close() -> bool;

    auto prepare(string_view sql) const -> statement_view;

    auto exec(string const& sql) const -> bool;

    void commit_hook(i32 (*callback)(void*), void* userdata) const;
    void rollback_hook(void (*callback)(void*), void* userdata) const;
    void update_hook(void (*callback)(void*, int, char const*, char const*, long long int), void* userdata) const;

private:
    sqlite3* _db {nullptr};
};

}

#endif

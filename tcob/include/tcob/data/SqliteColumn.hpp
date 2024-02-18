// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include "tcob/data/Sqlite.hpp"

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

template <typename T>
concept Aggregate =
    requires(T t) {
        {
            t.str()
        } -> std::same_as<string>;
    };

class TCOB_API avg {
public:
    avg(string column);

    string Column;

    auto str() const -> string;
};
static_assert(Aggregate<avg>);

class TCOB_API count {
public:
    count(string column);

    string Column;

    auto str() const -> string;
};
static_assert(Aggregate<count>);

class TCOB_API max {
public:
    max(string column);

    string Column;

    auto str() const -> string;
};
static_assert(Aggregate<max>);

class TCOB_API min {
public:
    min(string column);

    string Column;

    auto str() const -> string;
};
static_assert(Aggregate<min>);

class TCOB_API sum {
public:
    sum(string column);

    string Column;

    auto str() const -> string;
};
static_assert(Aggregate<sum>);

////////////////////////////////////////////////////////////

class TCOB_API no_constraint {
public:
    auto str() const -> string;
};

template <typename T>
class default_value {
public:
    default_value(T defaultValue);

    T DefaultValue;

    auto str() const -> string;
};

class TCOB_API unique {
public:
    auto str() const -> string;
};

class TCOB_API primary_key {
public:
    auto str() const -> string;
};

class TCOB_API check {
public:
    check(string check);

    string Check;

    auto str() const -> string;
};

////////////////////////////////////////////////////////////

template <typename T = no_constraint>
struct column {
    string Name;
    type   Type {type::Integer};
    bool   NotNull {false};
    T      Constraint {};

    auto str() const -> string;
};

namespace detail {
    struct column_builder {
        string Name;

        template <typename T = no_constraint>
        auto operator()(type t = type::Integer, bool notNull = false, T constraint = no_constraint {}) const -> column<T>;
    };

}

namespace literals {
    auto operator""_col(char const* str, usize) -> detail::column_builder;
}

////////////////////////////////////////////////////////////

struct distinct {
};

////////////////////////////////////////////////////////////

}

    #include "SqliteColumn.inl"

#endif

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
        } -> std::same_as<utf8_string>;
    };

class TCOB_API avg {
public:
    avg(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(Aggregate<avg>);

class TCOB_API count {
public:
    count(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(Aggregate<count>);

class TCOB_API max {
public:
    max(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(Aggregate<max>);

class TCOB_API min {
public:
    min(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(Aggregate<min>);

class TCOB_API sum {
public:
    sum(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(Aggregate<sum>);

////////////////////////////////////////////////////////////

class TCOB_API no_constraint {
public:
    auto str() const -> utf8_string;
};

template <typename T>
class default_value {
public:
    default_value(T defaultValue);

    T DefaultValue;

    auto str() const -> utf8_string;
};

class TCOB_API unique {
public:
    auto str() const -> utf8_string;
};

class TCOB_API primary_key {
public:
    auto str() const -> utf8_string;
};

class TCOB_API check {
public:
    check(utf8_string check);

    utf8_string Check;

    auto str() const -> utf8_string;
};

////////////////////////////////////////////////////////////

template <typename T = no_constraint>
struct column {
    utf8_string Name;
    type        Type {type::Integer};
    bool        NotNull {false};
    T           Constraint {};

    auto str() const -> utf8_string;
};

////////////////////////////////////////////////////////////

struct distinct {
};

////////////////////////////////////////////////////////////

}

    #include "SqliteColumn.inl"

#endif

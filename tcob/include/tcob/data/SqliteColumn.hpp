// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include "tcob/data/Sqlite.hpp"

    #include <optional>
    #include <variant>

namespace tcob::data::sqlite {
////////////////////////////////////////////////////////////

class TCOB_API avg {
public:
    avg(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(detail::HasStr<avg>);

class TCOB_API count {
public:
    count(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(detail::HasStr<count>);

class TCOB_API max {
public:
    max(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(detail::HasStr<max>);

class TCOB_API min {
public:
    min(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(detail::HasStr<min>);

class TCOB_API sum {
public:
    sum(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};
static_assert(detail::HasStr<sum>);

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
    unique();
    unique(auto&&... columns);

    utf8_string Columns;

    auto str() const -> utf8_string;
};

class TCOB_API primary_key {
public:
    auto str() const -> utf8_string;
};

class TCOB_API foreign_key {
public:
    utf8_string ForeignTable;
    utf8_string ForeignColumn;

    auto str() const -> utf8_string;
};

class TCOB_API table_foreign_key {
public:
    utf8_string Column;

    utf8_string ForeignTable;
    utf8_string ForeignColumn;

    auto str() const -> utf8_string;
};

class TCOB_API check {
public:
    check(utf8_string check);

    utf8_string Check;

    auto str() const -> utf8_string;
};

////////////////////////////////////////////////////////////

enum class order : u8 {
    Ascending,
    Descending
};

template <order Order = order::Ascending>
class TCOB_API ordering {
public:
    std::variant<utf8_string, i32> Column;

    auto str() const -> utf8_string;
};

using asc  = ordering<order::Ascending>;
using desc = ordering<order::Descending>;

////////////////////////////////////////////////////////////

enum class op : u8 {
    Equal,
    NotEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Like
};

template <op Operator>
class TCOB_API conditional {
public:
    utf8_string                                   Column;
    std::optional<std::variant<i32, utf8_string>> Value {};

    auto str() const -> utf8_string;
};

using equal         = conditional<op::Equal>;
using not_equal     = conditional<op::NotEqual>;
using greater       = conditional<op::Greater>;
using greater_equal = conditional<op::GreaterEqual>;
using less          = conditional<op::Less>;
using less_equal    = conditional<op::LessEqual>;
using like          = conditional<op::Like>;

////////////////////////////////////////////////////////////

template <type Type = type::Integer, typename C = no_constraint>
struct column {
    utf8_string Name;
    bool        NotNull {false};
    C           Constraint {};

    auto str() const -> utf8_string;
};

////////////////////////////////////////////////////////////

}

    #include "SqliteColumn.inl"

#endif

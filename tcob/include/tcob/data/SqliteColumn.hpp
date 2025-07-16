// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <variant>
    #include <vector>

    #include "tcob/data/Sqlite.hpp"
    #include "tcob/data/SqliteStatement.hpp"

namespace tcob::db {
////////////////////////////////////////////////////////////

class TCOB_API avg {
public:
    avg(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};

class TCOB_API count {
public:
    count(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};

class TCOB_API max {
public:
    max(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};

class TCOB_API min {
public:
    min(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};

class TCOB_API sum {
public:
    sum(utf8_string column);

    utf8_string Column;

    auto str() const -> utf8_string;
};

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
class ordering {
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
    Like,
    Glob,
    In,
    Between
};

enum class combine_op : u8 {
    And,
    Or
};

template <combine_op Operator, typename C0, typename C1>
class combined_condition {
public:
    combined_condition(C0 const& cond1, C1 const& cond2);

    auto str() const -> utf8_string;
    auto bind() const -> bind_func;

    template <typename C>
    auto operator&&(C const& other) const -> combined_condition<combine_op::And, combined_condition, C>;
    template <typename C>
    auto operator||(C const& other) const -> combined_condition<combine_op::Or, combined_condition, C>;

    auto operator!() const -> combined_condition;

private:
    C0   _cond0;
    C1   _cond1;
    bool _not {false};
};

template <op Operator>
class conditional {
    using params = std::variant<i32, f32, bool, utf8_string>;

public:
    template <typename T>
    conditional(T const& column, auto&&... params);

    auto str() const -> utf8_string;
    auto bind() const -> bind_func;

    template <typename C>
    auto operator&&(C const& other) const -> combined_condition<combine_op::And, conditional, C>;
    template <typename C>
    auto operator||(C const& other) const -> combined_condition<combine_op::Or, conditional, C>;

    auto operator!() const -> conditional;

private:
    utf8_string         _column;
    std::vector<params> _params;
    bool                _not {false};
};

using equal         = conditional<op::Equal>;
using not_equal     = conditional<op::NotEqual>;
using greater       = conditional<op::Greater>;
using greater_equal = conditional<op::GreaterEqual>;
using less          = conditional<op::Less>;
using less_equal    = conditional<op::LessEqual>;
using like          = conditional<op::Like>;
using glob          = conditional<op::Glob>;
using in            = conditional<op::In>;
using between       = conditional<op::Between>;

////////////////////////////////////////////////////////////

template <type Type, typename C>
struct column {
    utf8_string Name;
    bool        NotNull {false};
    C           Constraint {};

    auto str() const -> utf8_string;
};

template <typename C = no_constraint>
using text_column = column<type::Text, C>;
template <typename C = no_constraint>
using numeric_column = column<type::Numeric, C>;
template <typename C = no_constraint>
using int_column = column<type::Integer, C>;
template <typename C = no_constraint>
using real_column = column<type::Real, C>;
template <typename C = no_constraint>
using blob_column = column<type::Blob, C>;

////////////////////////////////////////////////////////////

}

    #include "SqliteColumn.inl"

#endif

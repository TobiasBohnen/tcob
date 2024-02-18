// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

namespace tcob::data::sqlite {

template <typename T>
inline default_value<T>::default_value(T defaultValue)
    : DefaultValue {defaultValue}
{
}

template <typename T>
inline auto default_value<T>::str() const -> string
{
    return " DEFAULT " + std::to_string(DefaultValue);
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto column<T>::str() const -> string
{
    string ss;

    // name
    ss = "\"" + Name + "\"";

    // type
    switch (Type) {
    case type::Text:
        ss += " TEXT";
        break;
    case type::Numeric:
        ss += " NUMERIC";
        break;
    case type::Integer:
        ss += " INTEGER";
        break;
    case type::Real:
        ss += " REAL";
        break;
    case type::Blob:
        ss += " BLOB";
        break;
    case type::Null:
        break;
    }

    // not null
    if (NotNull) { ss += " NOT NULL"; }

    // constraint
    ss += " " + Constraint.str();

    return ss;
}

////////////////////////////////////////////////////////////

namespace detail {
    template <typename T>
    inline auto column_builder::operator()(type t, bool notNull, T constraint) const -> column<T>
    {
        return {Name, t, notNull, constraint};
    }
}

////////////////////////////////////////////////////////////

}

#endif

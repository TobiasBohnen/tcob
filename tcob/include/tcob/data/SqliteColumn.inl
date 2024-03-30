// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "SqliteColumn.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <format>
    #include <unordered_set>

    #include "tcob/core/StringUtils.hpp"

namespace tcob::data::sqlite {

template <typename T>
inline default_value<T>::default_value(T defaultValue)
    : DefaultValue {defaultValue}
{
}

template <typename T>
inline auto default_value<T>::str() const -> utf8_string
{
    return std::format(" DEFAULT {}", DefaultValue);
}

////////////////////////////////////////////////////////////

inline unique::unique(auto&&... columns)
    : Columns {helper::join(std::unordered_set<utf8_string> {("\"" + utf8_string {columns} + "\"")...}, ",")}
{
}

////////////////////////////////////////////////////////////

template <typename T>
inline auto column<T>::str() const -> utf8_string
{
    utf8_string type;
    // type
    switch (Type) {
    case type::Text:
        type = "TEXT";
        break;
    case type::Numeric:
        type = "NUMERIC";
        break;
    case type::Integer:
        type = "INTEGER";
        break;
    case type::Real:
        type = "REAL";
        break;
    case type::Blob:
        type = "BLOB";
        break;
    case type::Null:
        break;
    }

    return std::format("{} {} {} {}", quote_string(Name), type, NotNull ? "NOT NULL" : "", Constraint.str());
}

////////////////////////////////////////////////////////////

}

#endif

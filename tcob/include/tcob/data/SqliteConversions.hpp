// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// IWYU pragma: always_keep

#pragma once
#include "tcob/tcob_config.hpp"

#if defined(TCOB_ENABLE_ADDON_DATA_SQLITE)

    #include <optional>
    #include <tuple>

    #include "tcob/core/Color.hpp"
    #include "tcob/core/Concepts.hpp"
    #include "tcob/core/Point.hpp"
    #include "tcob/core/Rect.hpp"
    #include "tcob/core/Size.hpp"
    #include "tcob/data/Sqlite.hpp"

namespace tcob::data::sqlite {

////basic/////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <>
struct converter<char const*> {
    auto static To(statement_view stmt, i32& idx, char const* value) -> bool
    {
        return stmt.bind(idx++, value);
    }
};

template <usize N>
struct converter<char[N]> { // NOLINT
    auto static To(statement_view stmt, i32& idx, char const* value) -> bool
    {
        return stmt.bind(idx++, value);
    }
};

template <>
struct converter<utf8_string> {
    auto static From(statement_view stmt, i32 col, utf8_string& value) -> bool
    {
        value = stmt.column_text(col);
        return true;
    }

    auto static To(statement_view stmt, i32& idx, utf8_string const& value) -> bool
    {
        return stmt.bind(idx++, value);
    }
};

template <>
struct converter<bool> {
    auto static From(statement_view stmt, i32 col, bool& value) -> bool
    {
        value = stmt.column_int(col) != 0;
        return true;
    }

    auto static To(statement_view stmt, i32& idx, bool value) -> bool
    {
        return stmt.bind(idx++, value ? 1 : 0);
    }
};

template <Integral T>
struct converter<T> {
    auto static From(statement_view stmt, i32 col, T& value) -> bool
    {
        value = static_cast<T>(stmt.column_int64(col));
        return true;
    }

    auto static To(statement_view stmt, i32& idx, T const& value) -> bool
    {
        return stmt.bind(idx++, value);
    }
};

template <FloatingPoint T>
struct converter<T> {
    auto static From(statement_view stmt, i32 col, T& value) -> bool
    {
        value = static_cast<T>(stmt.column_double(col));
        return true;
    }

    auto static To(statement_view stmt, i32& idx, T const& value) -> bool
    {
        return stmt.bind(idx++, value);
    }
};

template <>
struct converter<std::nullptr_t> {
    auto static To(statement_view stmt, i32& idx, std::nullptr_t const&) -> bool
    {
        return stmt.bind_null(idx++);
    }
};

////STL///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct converter<std::optional<T>> {
    auto static From(statement_view stmt, i32 col, std::optional<T>& value) -> bool
    {
        if (stmt.get_column_type(col) == type::Null) {
            value = std::nullopt;
        } else {
            T val {};
            converter<T>::From(stmt, col, val);
            value = val;
        }

        return true;
    }

    auto static To(statement_view stmt, i32& idx, std::optional<T> const& value) -> bool
    {
        if (!value.has_value()) {
            return stmt.bind_null(idx++);
        }

        return converter<T>::To(stmt, idx, *value);
    }
};

template <>
struct converter<std::nullopt_t> {
    auto static To(statement_view stmt, i32& idx, std::nullopt_t const&) -> bool
    {
        return stmt.bind_null(idx++);
    }
};

template <typename... T>
struct converter<std::tuple<T...>> {
    auto static From(statement_view stmt, i32 col, std::tuple<T...>& value) -> bool
    {
        std::apply(
            [&](auto&&... item) {
                ((converter<std::remove_cvref_t<decltype(item)>>::From(stmt, col++, item)), ...);
            },
            value);

        return true;
    }

    auto static To(statement_view stmt, i32& idx, std::tuple<T...> const& value) -> bool
    {
        std::apply(
            [&](auto&&... item) {
                (converter<std::remove_cvref_t<decltype(item)>>::To(stmt, idx, item), ...);
            },
            value);
        return true;
    }
};

template <Container T>
struct converter<T> {
    using value_type = typename T::value_type;

    auto static From(statement_view stmt, i32 col, T& value) -> bool
    {
        while (stmt.step() == step_status::Row) {
            value_type val {};
            converter<value_type>::From(stmt, col, val);
            value.push_back(val);
        }
        return true;
    }

    auto static To(statement_view stmt, i32& idx, T const& value) -> bool
    {
        for (usize i {0}; i < value.size(); ++i) {
            converter<value_type>::To(stmt, idx, value[i]);
        }
        return true;
    }
};

template <Set T>
struct converter<T> {
    using key_type = typename T::key_type;

    auto static From(statement_view stmt, i32 col, T& value) -> bool
    {
        while (stmt.step() == step_status::Row) {
            key_type val;
            converter<key_type>::From(stmt, col, val);
            value.insert(val);
        }
        return true;
    }

    auto static To(statement_view stmt, i32& idx, T const& value) -> bool
    {
        for (auto const& item : value) {
            converter<key_type>::To(stmt, idx, item);
        }
        return true;
    }
};

////tcob//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

template <typename T>
struct blob_converter {
    auto static From(statement_view stmt, i32 col, T& value) -> bool
    {
        value = *static_cast<T const*>(stmt.column_blob(col));
        return true;
    }

    auto static To(statement_view stmt, i32& idx, T const& value) -> bool
    {
        return stmt.bind(idx++, &value, sizeof(T));
    }
};

template <>
struct converter<color> : blob_converter<color> { };

template <Arithmetic T>
struct converter<point<T>> : blob_converter<point<T>> { };

template <Arithmetic T>
struct converter<size<T>> : blob_converter<size<T>> { };

template <Arithmetic T>
struct converter<rect<T>> : blob_converter<rect<T>> { };

}

#endif

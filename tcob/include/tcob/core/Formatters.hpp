// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <format>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"

template <>
struct std::formatter<tcob::color> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::color val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(r:{},g:{},b:{},a:{})", val.R, val.G, val.B, val.A);
    }
};

template <tcob::Arithmetic T>
struct std::formatter<tcob::point<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::point<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(x:{},y:{})", val.X, val.Y);
    }
};

template <tcob::Arithmetic T>
struct std::formatter<tcob::rect<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::rect<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(x:{},y:{},w:{},h:{})", val.left(), val.top(), val.width(), val.height());
    }
};

template <tcob::Arithmetic T>
struct std::formatter<tcob::size<T>> {
    constexpr auto parse(format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(tcob::size<T> val, format_context& ctx) const
    {
        return format_to(ctx.out(), "(w:{},h:{})", val.Width, val.Height);
    }
};

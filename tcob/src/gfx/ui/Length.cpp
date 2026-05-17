// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ui/Length.hpp"

#include <algorithm>
#include <cassert>

#include "tcob/core/Common.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ui {

////////////////////////////////////////////////////////////

length::length(f32 val, type t)
    : Value {val}
    , Type {t}
{
}

auto length::calc(f32 min, f32 refSize) const -> f32
{
    return std::max(min, calc(refSize));
}

auto length::calc(f32 refSize) const -> f32
{
    switch (Type) {
    case type::Relative: return Value * refSize;
    case type::Absolute: return Value;
    }

    return 0.0f;
}

auto length::operator-() const -> length
{
    return length {-Value, Type};
}

////////////////////////////////////////////////////////////

thickness::thickness(length l, length r, length t, length b)
    : Left {l}
    , Right {r}
    , Top {t}
    , Bottom {b}
{
}

thickness::thickness(length lr, length tb)
    : Left {lr}
    , Right {lr}
    , Top {tb}
    , Bottom {tb}
{
}

thickness::thickness(length all)
    : Left {all}
    , Right {all}
    , Top {all}
    , Bottom {all}
{
}

////////////////////////////////////////////////////////////

auto length::Lerp(length const& from, length const& to, f64 step) -> length
{
    assert(from.Type == to.Type);
    length retValue;
    retValue.Type  = from.Type;
    retValue.Value = helper::lerp(from.Value, to.Value, step);
    return retValue;
}

auto thickness::Lerp(thickness const& from, thickness const& to, f64 step) -> thickness
{
    thickness retValue;
    retValue.Bottom = length::Lerp(from.Bottom, to.Bottom, step);
    retValue.Top    = length::Lerp(from.Top, to.Top, step);
    retValue.Left   = length::Lerp(from.Left, to.Left, step);
    retValue.Right  = length::Lerp(from.Right, to.Right, step);
    return retValue;
}

auto dimensions::calc(size_f refSize) const -> size_f
{
    return {Width.calc(refSize.Width), Height.calc(refSize.Height)};
}

auto dimensions::Lerp(dimensions const& from, dimensions const& to, f64 step) -> dimensions
{
    dimensions retValue;
    retValue.Width  = length::Lerp(from.Width, to.Width, step);
    retValue.Height = length::Lerp(from.Height, to.Height, step);
    return retValue;
}

}

// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ColorGradient.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <initializer_list>
#include <span>

#include "tcob/core/Color.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

color_stop::color_stop(f32 pos, color c)
    : Position {pos}
    , Value {c}
{
}

////////////////////////////////////////////////////////////

color_gradient::color_gradient()
    : color_gradient {colors::White, colors::White}
{
}

color_gradient::color_gradient(color startColor, color endColor, bool preMulAlpha)
    : _premulAlpha {preMulAlpha}
{
    if (startColor == endColor) {
        _colorStops[0] = startColor;
    } else {
        _colorStops[0]        = startColor;
        _colorStops[Size - 1] = endColor;
    }
}

color_gradient::color_gradient(std::initializer_list<color_stop const> colorStops, bool preMulAlpha)
    : color_gradient {std::span<color_stop const> {colorStops}, preMulAlpha}
{
}

color_gradient::color_gradient(std::span<color_stop const> colorStops, bool preMulAlpha)
    : _premulAlpha {preMulAlpha}
{
    for (auto const& cs : colorStops) {
        u32 pos {static_cast<u32>(cs.Position * (Size - 1))};
        if (_colorStops.contains(pos)) {
            pos++;
        }
        _colorStops[std::min(pos, Size - 1)] = cs.Value;
    }
}

auto color_gradient::colors() const -> std::array<color, Size>
{
    std::array<color, Size> retValue {};

    if (_colorStops.size() == 1) {
        retValue.fill(_colorStops.begin()->second);
    } else {
        auto it2 {_colorStops.begin()};
        auto it1 {it2++};
        while (it2 != _colorStops.end()) {
            auto const [k1, col1] {*it1};
            auto const [k2, col2] {*it2};

            u32 const start {k1};
            u32 const size {k2 - start};

            for (u32 i {0}; i <= size; ++i) {
                color col {color::Lerp(col1, col2, static_cast<f32>(i) / size)};
                if (_premulAlpha) { col = col.as_alpha_premultiplied(); }

                retValue[i + start] = col;
            }

            it1 = it2;
            ++it2;
        }
    }

    return retValue;
}

auto color_gradient::is_single_color() const -> bool
{
    return _colorStops.size() == 1;
}

auto color_gradient::first_color() const -> color
{
    return _colorStops.begin()->second;
}

auto color_gradient::Lerp(color_gradient const& left, color_gradient const& right, f64 step) -> color_gradient
{
    assert(left._colorStops.size() == right._colorStops.size());
    color_gradient retValue;
    for (auto const& [k, v] : left._colorStops) {
        assert(right._colorStops.contains(k));
        retValue._colorStops[k] = color::Lerp(v, right._colorStops.at(k), step);
    }

    return retValue;
}

} // namespace gfx

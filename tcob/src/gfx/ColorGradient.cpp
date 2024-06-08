// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ColorGradient.hpp"

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

color_gradient::color_gradient(std::span<color_stop> colorStops, bool preMulAlpha)
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

auto color_gradient::get_colors() const -> std::array<color, Size>
{
    std::array<vec4, Size> const colors {get_color_array(1)};

    std::array<color, Size> retValue;
    for (u32 idx {0}; idx < Size; ++idx) {
        retValue[idx].R = static_cast<u8>(colors[idx][0] * 255.0f);
        retValue[idx].G = static_cast<u8>(colors[idx][1] * 255.0f);
        retValue[idx].B = static_cast<u8>(colors[idx][2] * 255.0f);
        retValue[idx].A = static_cast<u8>(colors[idx][3] * 255.0f);
    }
    return retValue;
}

auto color_gradient::get_color_array(f32 multAlpha) const -> std::array<vec4, Size>
{
    std::array<vec4, Size> retValue {};

    if (_colorStops.size() == 1) {
        color const c {_colorStops.cbegin()->second};
        retValue.fill({c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, c.A / 255.0f * multAlpha});
    } else {
        auto it2 {_colorStops.cbegin()};
        auto it1 {it2++};
        while (it2 != _colorStops.cend()) {
            auto const [k1, col1] {*it1};
            auto const [k2, col2] {*it2};

            u32 const start {k1};
            u32 const size {k2 - start};

            for (u32 i {0}; i <= size; ++i) {
                color c {color::Lerp(col1, col2, static_cast<f32>(i) / size)};
                if (_premulAlpha) {
                    c = c.as_alpha_premultiplied();
                }

                retValue[i + start][0] = c.R / 255.0f;
                retValue[i + start][1] = c.G / 255.0f;
                retValue[i + start][2] = c.B / 255.0f;
                retValue[i + start][3] = c.A / 255.0f * multAlpha;
            }

            it1 = it2;
            ++it2;
        }
    }

    return retValue;
}

auto operator==(color_gradient const& left, color_gradient const& right) -> bool
{
    return left._premulAlpha == right._premulAlpha && right._colorStops == left._colorStops;
}

} // namespace gfx

// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/ColorGradient.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <set>
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

color_gradient::color_gradient(std::initializer_list<color const> colors)
{
    if (colors.size() == 1) {
        _colorStops[0] = *colors.begin();
        return;
    }

    f32 const step {1.0f / (colors.size() - 1)};
    f32       current {0.0f};
    for (auto const& color : colors) {
        _colorStops[static_cast<u32>(current * (Size - 1))] = color;
        current += step;
    }
}

color_gradient::color_gradient(std::initializer_list<color_stop const> colorStops)
    : color_gradient {std::span<color_stop const> {colorStops}}
{
}

color_gradient::color_gradient(std::span<color_stop const> colorStops)
{
    for (auto const& cs : colorStops) {
        u32 pos {static_cast<u32>(cs.Position * (Size - 1))};
        if (_colorStops.contains(pos)) { pos++; }
        _colorStops[std::min(pos, Size - 1)] = cs.Value;
    }
}

auto color_gradient::colors(bool preMulAlpha) const -> std::array<color, Size>
{
    std::array<color, Size> retValue {};

    if (_colorStops.size() == 1) {
        color const col {_colorStops.begin()->second};
        retValue.fill(preMulAlpha ? col.as_alpha_premultiplied() : col);
    } else {
        auto curr {_colorStops.begin()};
        for (auto next {std::next(curr)}; next != _colorStops.end(); ++curr, ++next) {
            auto const [currStop, currColor] {*curr};
            auto const [nextStop, nextColor] {*next};

            u32 const distance {nextStop - currStop};
            for (u32 i {0}; i <= distance; ++i) {
                color const col {color::Lerp(currColor, nextColor, static_cast<f32>(i) / distance)};
                retValue[i + currStop] = preMulAlpha ? col.as_alpha_premultiplied() : col;
            }
        }
    }

    return retValue;
}

auto color_gradient::solid_color() const -> std::optional<color>
{
    return _colorStops.size() == 1
        ? std::optional<color> {_colorStops.begin()->second}
        : std::nullopt;
}

auto color_gradient::get_color_at(u32 key) const -> color
{
    if (_colorStops.empty()) { return {}; }

    auto it {_colorStops.lower_bound(key)};
    if (it == _colorStops.begin()) { return it->second; }
    if (it == _colorStops.end()) { return std::prev(it)->second; }

    auto upper {it};
    auto lower {std::prev(it)};

    u64 const range {upper->first - lower->first};
    f64 const t {(key - lower->first) / static_cast<f64>(range)};
    return color::Lerp(lower->second, upper->second, t);
}

auto color_gradient::Lerp(color_gradient const& from, color_gradient const& to, f64 step) -> color_gradient
{
    color_gradient retValue;

    std::set<u32> allKeys;
    for (auto const& [k, _] : from._colorStops) { allKeys.insert(k); }
    for (auto const& [k, _] : to._colorStops) { allKeys.insert(k); }

    for (auto k : allKeys) {
        color const leftColor {from._colorStops.contains(k) ? from._colorStops.at(k) : from.get_color_at(k)};
        color const rightColor {to._colorStops.contains(k) ? to._colorStops.at(k) : to.get_color_at(k)};

        retValue._colorStops[k] = color::Lerp(leftColor, rightColor, step);
    }

    return retValue;
}

} // namespace gfx

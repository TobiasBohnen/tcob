// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/core/data/Color.hpp>

namespace tcob {
auto Color::interpolate(const Color& other, f64 step) const -> Color
{
    const u8 nr { static_cast<u8>(R + ((other.R - R) * step)) };
    const u8 ng { static_cast<u8>(G + ((other.G - G) * step)) };
    const u8 nb { static_cast<u8>(B + ((other.B - B) * step)) };
    const u8 na { static_cast<u8>(A + ((other.A - A) * step)) };
    return { nr, ng, nb, na };
}

auto Color::premultiply_alpha() const -> Color
{
    const f32 factor { A / 255.f };
    const u8 nr { static_cast<u8>(R * factor) };
    const u8 ng { static_cast<u8>(G * factor) };
    const u8 nb { static_cast<u8>(B * factor) };
    return { nr, ng, nb, A };
}
}
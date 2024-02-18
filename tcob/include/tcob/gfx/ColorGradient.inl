// Copyright (c) 2023 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "ColorGradient.hpp"

namespace tcob::gfx {

inline void color_stop::Serialize(color_stop const& v, auto&& s)
{
    s["pos"]   = v.Position;
    s["value"] = v.Value;
}

inline auto color_stop::Deserialize(color_stop& v, auto&& s) -> bool
{
    return s.try_get(v.Position, "pos") && s.try_get(v.Value, "value");
}

}

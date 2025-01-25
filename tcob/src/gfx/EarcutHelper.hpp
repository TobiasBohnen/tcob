// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

// IWYU pragma: always_keep

#include "tcob/tcob_config.hpp"

#include "earcut/earcut.hpp"

#include "tcob/core/Point.hpp"

namespace mapbox::util {

template <>
struct nth<0, tcob::point_f> {
    static auto get(tcob::point_f const& t)
    {
        return t.X;
    }
};
template <>
struct nth<1, tcob::point_f> {
    static auto get(tcob::point_f const& t)
    {
        return t.Y;
    }
};

}

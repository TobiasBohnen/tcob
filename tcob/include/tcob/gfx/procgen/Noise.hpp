// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

class TCOB_API perlin_noise final {
public:
    explicit perlin_noise(i32 gridSize, f32 scale = 1.0f, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32;

private:
    auto random_gradient(point_i i) const -> point_f;

    auto dot_grid_gradient(point_i i, point_f f) const -> f32;

    f32 _scale;
    u64 _seed;
    i32 _gridSize;
};

////////////////////////////////////////////////////////////

class TCOB_API cellular_noise final {
public:
    explicit cellular_noise(i32 points, f32 scale = 1.0f, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32;

private:
    void generate_points();

    f32                               _scale;
    i32                               _pointCount;
    i32                               _gridSize;
    std::vector<std::vector<point_f>> _grid;
    rng                               _rand;
};

////////////////////////////////////////////////////////////

class TCOB_API value_noise final {
public:
    value_noise(i32 gridSize, f32 scale = 1.0f, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32;

private:
    void generate_grid(i32 gridSize);

    f32       _scale;
    grid<f32> _grid;
    rng       _rand;
};

}

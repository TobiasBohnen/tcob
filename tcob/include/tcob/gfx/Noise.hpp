// Copyright (c) 2024 Tobias Bohnen
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

class TCOB_API noise_base {
public:
    explicit noise_base(u64 seed);
    virtual ~noise_base();

    auto virtual operator()(point_f p) const -> f32 = 0;

protected:
    auto interpolate(f32 a0, f32 a1, f32 w) const -> f32;
    auto rand(f32 min, f32 max) -> f32;

private:
    rng _rand;
};

////////////////////////////////////////////////////////////

class TCOB_API perlin_noise final : public noise_base {
public:
    explicit perlin_noise(f32 scale = 1.0f, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32 override;

private:
    auto random_gradient(point_i i) const -> point_f;

    auto dot_grid_gradient(point_i i, point_f f) const -> f32;

    f32 _scale;
    u64 _seed;
};

////////////////////////////////////////////////////////////

class TCOB_API cellular_noise final : public noise_base {
public:
    explicit cellular_noise(i32 points, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32 override;

private:
    void generate_points();

    i32                  _pointCount;
    std::vector<point_f> _points;
};

////////////////////////////////////////////////////////////

class TCOB_API value_noise final : public noise_base {
public:
    value_noise(i32 gridSize, u64 seed = static_cast<u64>(clock::now().time_since_epoch().count()));

    auto operator()(point_f p) const -> f32 override;

private:
    void generate_grid(i32 gridSize);

    grid<f32> _grid;
};

}

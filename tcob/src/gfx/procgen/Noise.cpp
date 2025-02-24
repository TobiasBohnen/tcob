// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/procgen/Noise.hpp"

#include <cmath>
#include <limits>

namespace tcob::gfx {

////////////////////////////////////////////////////////////

noise_base::noise_base(u64 seed)
    : _rand {seed}
{
}

noise_base::~noise_base() = default;

auto noise_base::interpolate(f32 a0, f32 a1, f32 w) const -> f32
{
    f32 const e {w * w * w * (w * (w * 6.f - 15.f) + 10.f)};
    return (a1 - a0) * e + a0;
}

auto noise_base::rand(f32 min, f32 max) -> f32
{
    return _rand(min, max);
}

////////////////////////////////////////////////////////////

perlin_noise::perlin_noise(f32 scale, u64 seed)
    : noise_base {seed}
    , _scale {scale}
    , _seed {seed}
{
}

auto perlin_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    i32 const x0 {static_cast<i32>(std::floor(ps.X))};
    i32 const x1 {x0 + 1};
    i32 const y0 {static_cast<i32>(std::floor(ps.Y))};
    i32 const y1 {y0 + 1};

    f32 const sx {ps.X - static_cast<f32>(x0)};
    f32 const sy {ps.Y - static_cast<f32>(y0)};

    f32 const n0 {dot_grid_gradient({x0, y0}, ps)};
    f32 const n1 {dot_grid_gradient({x1, y0}, ps)};
    f32 const ix0 {interpolate(n0, n1, sx)};

    f32 const n2 {dot_grid_gradient({x0, y1}, ps)};
    f32 const n3 {dot_grid_gradient({x1, y1}, ps)};
    f32 const ix1 {interpolate(n2, n3, sx)};

    f32 const result {interpolate(ix0, ix1, sy) * std::sqrt(2.0f)};
    return (result + 1.0f) * 0.5f;
}

auto perlin_noise::random_gradient(point_i i) const -> point_f
{
    rng       rand {i.X * 73856093 ^ i.Y * 19349663 ^ _seed};
    f32 const random {rand(0.f, TAU_F)};
    return {std::cos(random), std::sin(random)};
}

auto perlin_noise::dot_grid_gradient(point_i i, point_f f) const -> f32
{
    point_f const grad {random_gradient(i)};
    f32 const     dx {f.X - static_cast<f32>(i.X)};
    f32 const     dy {f.Y - static_cast<f32>(i.Y)};
    return dx * grad.X + dy * grad.Y;
}

////////////////////////////////////////////////////////////

cellular_noise::cellular_noise(i32 points, u64 seed)
    : noise_base {seed}
    , _pointCount {points}
{
    generate_points();
}

auto cellular_noise::operator()(point_f p) const -> f32
{
    f64 minDist {std::numeric_limits<f64>::max()};

    for (auto const& point : _points) {
        f64 const dist {p.distance_to(point)};
        if (dist < minDist) {
            minDist = dist;
        }
    }

    return static_cast<f32>(minDist);
}

void cellular_noise::generate_points()
{
    for (i32 i {0}; i < _pointCount; ++i) {
        point_f new_point {rand(0.f, 1.0f), rand(0.f, 1.0f)};
        _points.push_back(new_point);
    }
}

////////////////////////////////////////////////////////////

value_noise::value_noise(i32 gridSize, u64 seed)
    : noise_base {seed}
    , _grid {{gridSize, gridSize}}
{
    generate_grid(gridSize);
}

auto value_noise::operator()(point_f p) const -> f32
{
    auto const gridSize {_grid.width()};
    i32 const  x0 {static_cast<i32>(std::floor(p.X * gridSize)) % gridSize};
    i32 const  x1 {(x0 + 1) % gridSize};
    i32 const  y0 {static_cast<i32>(std::floor(p.Y * gridSize)) % gridSize};
    i32 const  y1 {(y0 + 1) % gridSize};

    f32 const sx {(p.X * gridSize) - std::floor(p.X * gridSize)};
    f32 const sy {(p.Y * gridSize) - std::floor(p.Y * gridSize)};

    f32 const n0 {interpolate(_grid[{x0, y0}], _grid[{x1, y0}], sx)};
    f32 const n1 {interpolate(_grid[{x0, y1}], _grid[{x1, y1}], sx)};

    return interpolate(n0, n1, sy);
}

void value_noise::generate_grid(i32 gridSize)
{
    for (i32 x {0}; x < gridSize; ++x) {
        for (i32 y {0}; y < gridSize; ++y) {
            _grid[{x, y}] = rand(0.0f, 1.0f);
        }
    }
}

} // namespace gfx

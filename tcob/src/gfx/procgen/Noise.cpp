// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/procgen/Noise.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "tcob/core/Point.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

static auto Interpolate(f32 a0, f32 a1, f32 w) -> f32
{
    f32 const e {w * w * w * ((w * ((w * 6.0f) - 15.0f)) + 10.0f)};
    return ((a1 - a0) * e) + a0;
}

////////////////////////////////////////////////////////////

perlin_noise::perlin_noise(i32 gridSize, f32 scale, u64 seed)
    : _scale {scale}
    , _seed {seed}
    , _gridSize {gridSize}
{
}

auto perlin_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const x {ps.X - std::floor(ps.X)};
    f32 const y {ps.Y - std::floor(ps.Y)};

    i32 const x0 {static_cast<i32>(x * _gridSize)};
    i32 const y0 {static_cast<i32>(y * _gridSize)};

    i32 const x1 {(x0 + 1) % _gridSize};
    i32 const y1 {(y0 + 1) % _gridSize};

    f32 const sx {(x * _gridSize) - std::floor(x * _gridSize)};
    f32 const sy {(y * _gridSize) - std::floor(y * _gridSize)};

    f32 const n0 {dot_grid_gradient({x0, y0}, {sx, sy})};
    f32 const n1 {dot_grid_gradient({x1, y0}, {sx - 1.0f, sy})};
    f32 const ix0 {Interpolate(n0, n1, sx)};

    f32 const n2 {dot_grid_gradient({x0, y1}, {sx, sy - 1.0f})};
    f32 const n3 {dot_grid_gradient({x1, y1}, {sx - 1.0f, sy - 1.0f})};
    f32 const ix1 {Interpolate(n2, n3, sx)};

    f32 const result {Interpolate(ix0, ix1, sy) * std::numbers::sqrt2_v<f32>};
    return (result + 1.0f) * 0.5f;
}

auto perlin_noise::random_gradient(point_i i) const -> point_f
{
    rng       rng {(i.X * 73856093) ^ (i.Y * 19349663) ^ _seed};
    f32 const random {rng(0.0f, TAU_F)};
    return {std::cos(random), std::sin(random)};
}

auto perlin_noise::dot_grid_gradient(point_i i, point_f f) const -> f32
{
    point_f const grad {random_gradient(i)};
    return (f.X * grad.X) + (f.Y * grad.Y);
}

////////////////////////////////////////////////////////////

cellular_noise::cellular_noise(i32 points, f32 scale, u64 seed)
    : _scale {scale}
    , _pointCount {points}
    , _gridSize {static_cast<i32>(std::sqrt(points))}
{
    _grid.resize(_gridSize * _gridSize);
    generate_points(seed);
}

auto cellular_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const x {ps.X - std::floor(ps.X)};
    f32 const y {ps.Y - std::floor(ps.Y)};
    i32 const cellX {static_cast<i32>(x * static_cast<f32>(_gridSize))};
    i32 const cellY {static_cast<i32>(y * static_cast<f32>(_gridSize))};

    f64 minDist {std::numeric_limits<f64>::max()};

    for (i32 dy {-1}; dy <= 1; ++dy) {
        for (i32 dx {-1}; dx <= 1; ++dx) {
            i32 const nx {(cellX + dx + _gridSize) % _gridSize};
            i32 const ny {(cellY + dy + _gridSize) % _gridSize};
            i32 const idx {(ny * _gridSize) + nx};

            f32 const offsetX {(cellX + dx < 0) ? -1.0f : ((cellX + dx >= _gridSize) ? 1.0f : 0.0f)};
            f32 const offsetY {(cellY + dy < 0) ? -1.0f : ((cellY + dy >= _gridSize) ? 1.0f : 0.0f)};

            for (auto const& point : _grid[idx]) {
                f32 const px {point.X + offsetX};
                f32 const py {point.Y + offsetY};
                f64 const dist {std::hypot(x - px, y - py)};
                minDist = std::min(dist, minDist);
            }
        }
    }

    return static_cast<f32>(minDist);
}

void cellular_noise::generate_points(u64 seed)
{
    rng rng {seed};
    for (i32 i {0}; i < _pointCount; ++i) {
        point_f   p {rng(0.0f, 1.0f), rng(0.0f, 1.0f)};
        i32 const cellX {static_cast<i32>(p.X * static_cast<f32>(_gridSize))};
        i32 const cellY {static_cast<i32>(p.Y * static_cast<f32>(_gridSize))};
        i32 const idx {(cellY * _gridSize) + cellX};
        _grid[idx].push_back(p);
    }
}

////////////////////////////////////////////////////////////

value_noise::value_noise(i32 gridSize, f32 scale, u64 seed)
    : _scale {scale}
    , _grid {{gridSize, gridSize}}
{
    generate_grid(gridSize, seed);
}

auto value_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const x {ps.X - std::floor(ps.X)};
    f32 const y {ps.Y - std::floor(ps.Y)};

    auto const gridSize {_grid.width()};

    i32 const x0 {static_cast<i32>(x * gridSize)};
    i32 const x1 {(x0 + 1) % gridSize};
    i32 const y0 {static_cast<i32>(y * gridSize)};
    i32 const y1 {(y0 + 1) % gridSize};

    f32 const sx {(x * gridSize) - std::floor(x * gridSize)};
    f32 const sy {(y * gridSize) - std::floor(y * gridSize)};

    f32 const n0 {Interpolate(_grid[x0, y0], _grid[x1, y0], sx)};
    f32 const n1 {Interpolate(_grid[x0, y1], _grid[x1, y1], sx)};

    return Interpolate(n0, n1, sy);
}

void value_noise::generate_grid(i32 gridSize, u64 seed)
{
    rng rng {seed};
    for (i32 x {0}; x < gridSize; ++x) {
        for (i32 y {0}; y < gridSize; ++y) {
            _grid[x, y] = rng(0.0f, 1.0f);
        }
    }
}

}

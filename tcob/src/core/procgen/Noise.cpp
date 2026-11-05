// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/core/procgen/Noise.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/random/Random.hpp"

namespace tcob::procgen {

////////////////////////////////////////////////////////////

static auto Smootherstep(f32 a0, f32 a1, f32 w) -> f32
{
    f32 const e {w * w * w * ((w * ((w * 6.0f) - 15.0f)) + 10.0f)};
    return ((a1 - a0) * e) + a0;
}

static auto Fract(f32 v) -> f32
{
    f32 const r {v - std::floor(v)};
    return r >= 1.0f ? 0.0f : r;
}

static auto HashCoords(i32 x, i32 y, u64 seed) -> u64
{
    u64 h {(static_cast<u64>(x) * 73856093) ^ (static_cast<u64>(y) * 19349663) ^ seed};
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return h;
}

////////////////////////////////////////////////////////////

perlin_noise::perlin_noise(size_i gridSize, f32 scale, u64 seed)
    : _scale {scale}
    , _seed {seed}
    , _gridSize {gridSize}
{
}

auto perlin_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const fx {Fract(ps.X) * _gridSize.Width};
    f32 const fy {Fract(ps.Y) * _gridSize.Height};

    i32 const x0 {static_cast<i32>(fx) % _gridSize.Width};
    i32 const y0 {static_cast<i32>(fy) % _gridSize.Height};

    i32 const x1 {(x0 + 1) % _gridSize.Width};
    i32 const y1 {(y0 + 1) % _gridSize.Height};

    f32 const sx {fx - std::floor(fx)};
    f32 const sy {fy - std::floor(fy)};

    f32 const n0 {dot_grid_gradient({x0, y0}, {sx, sy})};
    f32 const n1 {dot_grid_gradient({x1, y0}, {sx - 1.0f, sy})};
    f32 const ix0 {Smootherstep(n0, n1, sx)};

    f32 const n2 {dot_grid_gradient({x0, y1}, {sx, sy - 1.0f})};
    f32 const n3 {dot_grid_gradient({x1, y1}, {sx - 1.0f, sy - 1.0f})};
    f32 const ix1 {Smootherstep(n2, n3, sx)};

    f32 const result {Smootherstep(ix0, ix1, sy) * std::numbers::sqrt2_v<f32>};
    return (result + 1.0f) * 0.5f;
}

auto perlin_noise::random_gradient(point_i i) const -> point_f
{
    u64 const h {HashCoords(i.X, i.Y, _seed)};
    f32 const random {static_cast<f32>(h >> 40) * (TAU_F / 16777216.0f)};
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
    , _gridSize {std::max(1, static_cast<i32>(std::sqrt(static_cast<f32>(std::max(points, 1)))))}
{
    _grid.resize(_gridSize * _gridSize);
    generate_points(seed);
}

auto cellular_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const x {Fract(ps.X)};
    f32 const y {Fract(ps.Y)};
    f32 const gs {static_cast<f32>(_gridSize)};
    i32 const cellX {static_cast<i32>(x * gs) % _gridSize};
    i32 const cellY {static_cast<i32>(y * gs) % _gridSize};

    f32 minDist {std::numeric_limits<f32>::max()};

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
                f32 const dist {std::hypot(x - px, y - py)};
                minDist = std::min(dist, minDist);
            }
        }
    }

    return std::min(minDist * gs / std::numbers::sqrt2_v<f32>, 1.0f);
}

void cellular_noise::generate_points(u64 seed)
{
    rng       rng {seed};
    f32 const gs {static_cast<f32>(_gridSize)};
    for (i32 cy {0}; cy < _gridSize; ++cy) {
        for (i32 cx {0}; cx < _gridSize; ++cx) {
            point_f const p {(static_cast<f32>(cx) + rng(0.0f, 1.0f)) / gs,
                             (static_cast<f32>(cy) + rng(0.0f, 1.0f)) / gs};
            _grid[(cy * _gridSize) + cx].push_back(p);
        }
    }
}

////////////////////////////////////////////////////////////

value_noise::value_noise(size_i gridSize, f32 scale, u64 seed)
    : _scale {scale}
    , _grid {gridSize}
    , _gridSize {gridSize}
{
    generate_grid(seed);
}

auto value_noise::operator()(point_f p) const -> f32
{
    point_f const ps {p * _scale};

    f32 const fx {Fract(ps.X) * _gridSize.Width};
    f32 const fy {Fract(ps.Y) * _gridSize.Height};

    i32 const x0 {static_cast<i32>(fx) % _gridSize.Width};
    i32 const x1 {(x0 + 1) % _gridSize.Width};
    i32 const y0 {static_cast<i32>(fy) % _gridSize.Height};
    i32 const y1 {(y0 + 1) % _gridSize.Height};

    f32 const sx {fx - std::floor(fx)};
    f32 const sy {fy - std::floor(fy)};

    f32 const n0 {Smootherstep(_grid[x0, y0], _grid[x1, y0], sx)};
    f32 const n1 {Smootherstep(_grid[x0, y1], _grid[x1, y1], sx)};

    return Smootherstep(n0, n1, sy);
}

void value_noise::generate_grid(u64 seed)
{
    rng rng {seed};
    for (i32 x {0}; x < _gridSize.Width; ++x) {
        for (i32 y {0}; y < _gridSize.Height; ++y) {
            _grid[x, y] = rng(0.0f, 1.0f);
        }
    }
}

}

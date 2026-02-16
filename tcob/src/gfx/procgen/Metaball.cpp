// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/procgen/Metaball.hpp"

#include <algorithm>
#include <vector>

#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Image.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

metaball::metaball(size_i gridSize)
    : _gridSize {gridSize}
{
}

auto metaball::operator()(point_f p) const -> f32
{
    return calculate_field(p.X, p.Y);
}

void metaball::on_update(milliseconds deltaTime)
{
    if (!Balls->empty()) {
        f32 const dt {static_cast<f32>(deltaTime.count() / 1000.0)};

        Balls.mutate([&](std::vector<ball>& balls) {
            for (auto& b : balls) {
                if (b.Velocity == point_f::Zero) { continue; }

                _dirty = true;
                b.Position += b.Velocity * dt;

                // Bounce off bounds
                if (b.Position.X - b.Radius < 0 || b.Position.X + b.Radius >= static_cast<f32>(_gridSize.Width)) {
                    b.Velocity.X *= -1.0f;
                    b.Position.X = std::clamp(b.Position.X, b.Radius, static_cast<f32>(_gridSize.Width) - b.Radius);
                }
                if (b.Position.Y - b.Radius < 0 || b.Position.Y + b.Radius >= static_cast<f32>(_gridSize.Height)) {
                    b.Velocity.Y *= -1.0f;
                    b.Position.Y = std::clamp(b.Position.Y, b.Radius, static_cast<f32>(_gridSize.Height) - b.Radius);
                }
            }
        });
    }
}

auto metaball::calculate_field(f32 x, f32 y) const -> f32
{
    f32 sum {0.0f};

    for (auto const& ball : *Balls) {
        f32 const dx {x - ball.Position.X};
        f32 const dy {y - ball.Position.Y};
        f32 const distSq {(dx * dx) + (dy * dy)};

        if (distSq > 0.0f) {
            sum += (ball.Radius * ball.Radius) / distSq;
        }
    }

    return sum;
}

auto metaball::is_dirty() const -> bool { return _dirty; }
void metaball::mark_clean() { _dirty = false; }

////////////////////////////////////////////////////////////

metaball_image::metaball_image(size_i gridSize, color_gradient const& gradient)
    : metaball {gridSize}
    , _colors {gradient.colors(false)}
    , _image {image::CreateEmpty(gridSize, image::format::RGBA)}
{
}

auto metaball_image::image() -> gfx::image const&
{
    if (is_dirty()) { update_image(); }
    return _image;
}

void metaball_image::update_image()
{
    mark_clean();

    auto const& imgInfo {_image.info()};
    i32 const   width {imgInfo.Size.Width};
    i32 const   height {imgInfo.Size.Height};

    _image.clear();

    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            for (isize pixIdx {ctx.Start}; pixIdx < ctx.End; ++pixIdx) {
                isize const x {pixIdx % width};
                isize const y {pixIdx / width};
                f32 const   fieldValue {calculate_field(static_cast<f32>(x), static_cast<f32>(y))};

                if (fieldValue >= 1) {
                    f32 const   t {std::clamp(1.0f - (1.0f / fieldValue), 0.0f, 1.0f)};
                    u32 const   gradientIdx {static_cast<u32>(t * (color_gradient::Size - 1))};
                    color const pixelColor {_colors[gradientIdx]};
                    _image.set_pixel(pixIdx, pixelColor);
                }
            }
        },
        width * height);
}

}
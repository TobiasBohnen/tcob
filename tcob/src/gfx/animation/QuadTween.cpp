// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/QuadTween.hpp"

#include <algorithm>

namespace tcob::gfx {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

namespace detail {
    quad_tween_base::quad_tween_base(milliseconds duration)
        : tween_base {duration}
    {
    }

    void quad_tween_base::add_quad(std::reference_wrapper<quad> q)
    {
        _dstQuads.emplace_back(q);
        _srcQuads.push_back(q);
    }

    void quad_tween_base::clear_quads()
    {
        _dstQuads.clear();
        _srcQuads.clear();
    }

    auto quad_tween_base::source_quads() const -> std::vector<quad> const&
    {
        return _srcQuads;
    }

    void quad_tween_base::copy_to_dest(std::span<quad> quads)
    {
        for (usize i {0}; i < quads.size(); ++i) {
            _dstQuads[i].get() = quads[i];
        }
    }
}

////////////////////////////////////////////////////////////

void quad_tweens::start_all(playback_mode mode)
{
    for (auto& [_, effect] : _effects) {
        effect->start(mode);
    }
}

void quad_tweens::stop_all()
{
    for (auto& [_, effect] : _effects) {
        effect->stop();
    }
}

void quad_tweens::add_quad(u8 id, std::reference_wrapper<quad> q) const
{
    _effects.at(id)->add_quad(q);
}

auto quad_tweens::has(u8 id) const -> bool
{
    return _effects.contains(id);
}

void quad_tweens::clear_quads()
{
    for (auto& [_, effect] : _effects) {
        effect->clear_quads();
    }
}

void quad_tweens::on_update(milliseconds deltaTime)
{
    for (auto& [_, effect] : _effects) {
        effect->update(deltaTime);
    }
}

////////////////////////////////////////////////////////////

namespace effect {

    void typing::operator()(f64 t, std::span<quad> quads) const
    {
        usize const size {quads.size()};
        usize const fadeidx {static_cast<usize>(t * size)};
        for (usize idx {0}; idx < size; ++idx) {
            quad& dst {quads[idx]};
            if (idx <= fadeidx) {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 255;
            } else {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 0;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_in::operator()(f64 t, std::span<quad> quads) const
    {
        usize const totalQuads {quads.size()};
        usize const width {static_cast<usize>(Width)};
        usize const fadeIdx {static_cast<usize>(t * (totalQuads + width))};

        for (usize idx {0}; idx < totalQuads; ++idx) {
            quad& dst {quads[idx]};

            if (idx + width - 1 < fadeIdx) {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 255;
            } else if (idx > fadeIdx) {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 0;
            } else {
                f64 const val {((t * (totalQuads + width)) - static_cast<f64>(idx)) / static_cast<f64>(width)};
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = static_cast<u8>(val * 255.);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_out::operator()(f64 t, std::span<quad> quads) const
    {
        usize const totalQuads {quads.size()};
        usize const width {static_cast<usize>(Width)};
        usize const fadeIdx {static_cast<usize>(t * (totalQuads + width))};

        for (usize idx {0}; idx < totalQuads; ++idx) {
            quad& dst {quads[idx]};

            if (idx + width - 1 < fadeIdx) {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 0;
            } else if (idx > fadeIdx) {
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = 255;
            } else {
                f64 const val {1.0 - (((t * (totalQuads + width) - static_cast<f64>(idx))) / static_cast<f64>(width))};
                dst[0].Color.A = dst[1].Color.A = dst[2].Color.A = dst[3].Color.A = static_cast<u8>(val * 255.);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void blink::operator()(f64 t, std::span<quad> quads)
    {
        easing::square_wave<bool> wave {.Frequency = Frequency};
        bool const                flip {wave(t)};
        for (auto& q : quads) {
            geometry::set_color(q, flip ? Color0 : Color1);
        }
    }

    ////////////////////////////////////////////////////////////

    void gradient::operator()(f64 t, std::span<quad> quads) const
    {
        for (auto& q : quads) {
            geometry::set_color(q, Gradient[static_cast<u8>(255 * t)]);
        }
    }

    ////////////////////////////////////////////////////////////

    void shake::operator()(f64 /* t */, std::span<quad> quads)
    {
        for (auto& q : quads) {
            f32 const r {RNG(-Intensity, Intensity)};
            switch (RNG(0, 1)) {
            case 0:
                for (u32 i {0}; i < 4; ++i) {
                    q[i].Position.X += r;
                    q[i].Position.Y += r;
                }
                break;
            case 1:
                for (u32 i {0}; i < 4; ++i) {
                    q[i].Position.X += r;
                    q[i].Position.Y -= r;
                }
                break;
            default:
                break;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void wave::operator()(f64 t, std::span<quad> quads) const
    {
        for (usize idx {0}; idx < quads.size(); ++idx) {
            quad& dst {quads[idx]};

            easing::sine_wave<f64> wave {.Min = 0, .Max = 1, .Phase = static_cast<f64>(idx) / quads.size() * Amplitude};
            f64 const              val {wave(t) * Height};

            for (u32 i {0}; i < 4; ++i) {
                dst[i].Position.Y = static_cast<f32>(dst[i].Position.Y + val);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void size::operator()(f64 t, std::span<quad> quads) const
    {
        size_f maxSize {size_f::Zero};
        for (auto const& q : quads) {
            f32 const width {q[1].Position.X - q[3].Position.X};
            maxSize.Width = std::max(width, maxSize.Width);
            f32 const height {q[1].Position.Y - q[3].Position.Y};
            maxSize.Height = std::max(height, maxSize.Height);
        }

        f32 const wDiff {WidthEnd - WidthStart};
        f32 const wFac {wDiff > 0 ? static_cast<f32>(t * wDiff)
                                  : static_cast<f32>(1.0 - (t * -wDiff))};
        f32 const hDiff {HeightEnd - HeightStart};
        f32 const hFac {hDiff > 0 ? static_cast<f32>(t * hDiff)
                                  : static_cast<f32>(1.0 - (t * -hDiff))};

        for (auto& q : quads) {
            rect_f const rect {rect_f::FromLTRB(q[3].Position.X, q[3].Position.Y, q[1].Position.X, q[1].Position.Y)};
            transform    xform;
            size_f const scale {wFac, hFac};
            point_f      center {point_f::Zero};

            switch (Anchor.Horizontal) {
            case horizontal_alignment::Left:
                center.X = rect.left() - maxSize.Width + rect.width();
                break;
            case horizontal_alignment::Centered:
                center.X = rect.center().X - (maxSize.Width - rect.width()) / 2;
                break;
            case horizontal_alignment::Right:
                center.X = rect.right();
                break;
            }
            switch (Anchor.Vertical) {
            case vertical_alignment::Top:
                center.Y = rect.top() - maxSize.Height + rect.height();
                break;
            case vertical_alignment::Middle:
                center.Y = rect.center().Y - (maxSize.Height - rect.height()) / 2;
                break;
            case vertical_alignment::Bottom:
                center.Y = rect.bottom();
                break;
            }

            xform.scale_at(scale, center);
            for (u32 i {0}; i < 4; ++i) {
                point_f const pos {xform * point_f {q[i].Position.X, q[i].Position.Y}};
                q[i].Position.X = pos.X;
                q[i].Position.Y = pos.Y;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void rotate::operator()(f64 t, std::span<quad> quads) const
    {
        for (auto& q : quads) {
            rect_f const rect {rect_f::FromLTRB(q[3].Position.X, q[3].Position.Y, q[1].Position.X, q[1].Position.Y)};
            transform    rot;
            rot.rotate_at(degree_f {static_cast<f32>(360 * t * Speed)}, rect.center());

            for (u32 i {0}; i < 4; ++i) {
                point_f const pos {rot * point_f {q[i].Position.X, q[i].Position.Y}};
                q[i].Position.X = pos.X;
                q[i].Position.Y = pos.Y;
            }
        }
    }

}
}

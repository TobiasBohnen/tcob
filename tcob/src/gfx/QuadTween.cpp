// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/QuadTween.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

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

auto quad_tween_base::get_source_quads() const -> std::vector<quad> const&
{
    return _srcQuads;
}

void quad_tween_base::set_quads(std::span<quad> quads)
{
    for (usize i {0}; i < quads.size(); ++i) {
        _dstQuads[i].get() = quads[i];
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
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 255;
            } else {
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 0;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_in::operator()(f64 t, std::span<quad> quads) const
    {
        usize const totalQuads {quads.size()};
        usize const fadeIdx {static_cast<usize>(t * (totalQuads + Width))};

        for (usize idx {0}; idx < totalQuads; ++idx) {
            quad& dst {quads[idx]};

            if (idx + Width - 1 < fadeIdx) {
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 255;
            } else if (idx > fadeIdx) {
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 0;
            } else {
                f64 const val {((t * (totalQuads + Width)) - static_cast<f64>(idx)) / static_cast<f64>(Width)};
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = static_cast<u8>(val * 255.);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_out::operator()(f64 t, std::span<quad> quads) const
    {
        usize const totalQuads {quads.size()};
        usize const fadeIdx {static_cast<usize>(t * (totalQuads + Width))};

        for (usize idx {0}; idx < totalQuads; ++idx) {
            quad& dst {quads[idx]};

            if (idx + Width - 1 < fadeIdx) {
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 0;
            } else if (idx > fadeIdx) {
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = 255;
            } else {
                f64 const val {1.0 - (((t * (totalQuads + Width) - static_cast<f64>(idx))) / static_cast<f64>(Width))};
                dst[0].Color[3] = dst[1].Color[3] = dst[2].Color[3] = dst[3].Color[3] = static_cast<u8>(val * 255.);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void blink::operator()(f64 t, std::span<quad> quads)
    {
        tweening::func::square_wave<bool> wave {.Frequency = Frequency};
        bool const                        flip {wave(t)};
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
                for (i32 i {0}; i < 4; ++i) {
                    q[i].Position[0] += r;
                    q[i].Position[1] += r;
                }
                break;
            case 1:
                for (i32 i {0}; i < 4; ++i) {
                    q[i].Position[0] += r;
                    q[i].Position[1] -= r;
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

            tweening::func::sine_wave<f64> wave {.MinValue = 0, .MaxValue = 1, .Phase = static_cast<f64>(idx) / quads.size() * Amplitude};
            f64 const                      val {wave(t) * Height};

            for (i32 i {0}; i < 4; ++i) {
                dst[i].Position[1] = static_cast<f32>(dst[i].Position[1] + val);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void size::operator()(f64 t, std::span<quad> quads) const
    {
        size_f maxSize {size_f::Zero};
        for (auto const& q : quads) {
            f32 const width {q[1].Position[0] - q[3].Position[0]};
            if (width > maxSize.Width) { maxSize.Width = width; }
            f32 const height {q[1].Position[1] - q[3].Position[1]};
            if (height > maxSize.Height) { maxSize.Height = height; }
        }

        f32 const wDiff {WidthEnd - WidthStart};
        f32 const wFac {wDiff > 0 ? static_cast<f32>(t * wDiff)
                                  : static_cast<f32>(1.0 - (t * -wDiff))};
        f32 const hDiff {HeightEnd - HeightStart};
        f32 const hFac {hDiff > 0 ? static_cast<f32>(t * hDiff)
                                  : static_cast<f32>(1.0 - (t * -hDiff))};

        for (auto& q : quads) {
            rect_f const rect {rect_f::FromLTRB(q[3].Position[0], q[3].Position[1], q[1].Position[0], q[1].Position[1])};
            transform    xform;
            size_f const scale {wFac, hFac};
            point_f      center {point_f::Zero};

            switch (Anchor.Horizontal) {
            case horizontal_alignment::Left:
                center.X = rect.left() - maxSize.Width + rect.Width;
                break;
            case horizontal_alignment::Centered:
                center.X = rect.get_center().X - (maxSize.Width - rect.Width) / 2;
                break;
            case horizontal_alignment::Right:
                center.X = rect.right();
                break;
            }
            switch (Anchor.Vertical) {
            case vertical_alignment::Top:
                center.Y = rect.top() - maxSize.Height + rect.Height;
                break;
            case vertical_alignment::Middle:
                center.Y = rect.get_center().Y - (maxSize.Height - rect.Height) / 2;
                break;
            case vertical_alignment::Bottom:
                center.Y = rect.bottom();
                break;
            }

            xform.scale_at(scale, center);
            for (i32 i {0}; i < 4; ++i) {
                point_f const pos {xform * point_f {q[i].Position[0], q[i].Position[1]}};
                q[i].Position[0] = pos.X;
                q[i].Position[1] = pos.Y;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void rotate::operator()(f64 t, std::span<quad> quads) const
    {
        for (auto& q : quads) {
            rect_f const rect {rect_f::FromLTRB(q[3].Position[0], q[3].Position[1], q[1].Position[0], q[1].Position[1])};
            transform    rot;
            rot.rotate_at(degree_f {static_cast<f32>(360 * t * Speed)}, rect.get_center());

            for (i32 i {0}; i < 4; ++i) {
                point_f const pos {rot * point_f {q[i].Position[0], q[i].Position[1]}};
                q[i].Position[0] = pos.X;
                q[i].Position[1] = pos.Y;
            }
        }
    }

}
}

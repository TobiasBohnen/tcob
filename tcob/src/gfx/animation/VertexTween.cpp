// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/animation/VertexTween.hpp"

#include <algorithm>
#include <limits>
#include <span>
#include <vector>

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/core/tweening/TweenFunc.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"

namespace tcob::gfx {
using namespace std::chrono_literals;

////////////////////////////////////////////////////////////

namespace detail {

    vertex_tween_base::vertex_tween_base(milliseconds duration)
        : tween_base {duration}
    {
    }

    void vertex_tween_base::add_group(std::span<vertex> verts)
    {
        static auto computeRect {[](auto&& g) -> rect_f {
            f32 minX {std::numeric_limits<f32>::max()}, maxX {std::numeric_limits<f32>::lowest()};
            f32 minY {std::numeric_limits<f32>::max()}, maxY {std::numeric_limits<f32>::lowest()};
            for (auto const& v : g) {
                minX = std::min(minX, v.Position.X);
                maxX = std::max(maxX, v.Position.X);
                minY = std::min(minY, v.Position.Y);
                maxY = std::max(maxY, v.Position.Y);
            }
            return rect_f::FromLTRB(minX, minY, maxX, maxY);
        }};

        _srcGroups.push_back(vertex_tween_group {.Verts = {verts.begin(), verts.end()}, .BBox = computeRect(verts)});
        _dstGroups.push_back(verts);
    }

    void vertex_tween_base::clear_groups()
    {
        _srcGroups.clear();
        _dstGroups.clear();
    }

    auto vertex_tween_base::source_groups() const -> std::vector<vertex_tween_group> const&
    {
        return _srcGroups;
    }

    void vertex_tween_base::copy_to_dest(std::span<vertex_tween_group const> groups)
    {
        for (usize i {0}; i < groups.size(); ++i) {
            auto&       dstGroup {_dstGroups[i]};
            auto const& srcGroup {groups[i]};
            for (usize j {0}; j < srcGroup.Verts.size(); ++j) {
                dstGroup[j] = srcGroup.Verts[j];
            }
        }
    }

}

////////////////////////////////////////////////////////////

namespace effect {

    void typing::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        usize const size {groups.size()};
        usize const fadeIdx {static_cast<usize>(t * size)};
        for (usize idx {0}; idx < size; ++idx) {
            u8 const a {idx <= fadeIdx ? u8 {255} : u8 {0}};
            for (auto& v : groups[idx].Verts) { v.Color.A = a; }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_in::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        usize const total {groups.size()};
        usize const width {static_cast<usize>(Width)};
        usize const fadeIdx {static_cast<usize>(t * (total + width))};

        for (usize idx {0}; idx < total; ++idx) {
            u8 a {};
            if (idx + width - 1 < fadeIdx) {
                a = 255;
            } else if (idx > fadeIdx) {
                a = 0;
            } else {
                f64 const val {((t * (total + width)) - static_cast<f64>(idx)) / static_cast<f64>(width)};
                a = static_cast<u8>(val * 255.0);
            }
            for (auto& v : groups[idx].Verts) { v.Color.A = a; }
        }
    }

    ////////////////////////////////////////////////////////////

    void fade_out::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        usize const total {groups.size()};
        usize const width {static_cast<usize>(Width)};
        usize const fadeIdx {static_cast<usize>(t * (total + width))};

        for (usize idx {0}; idx < total; ++idx) {
            u8 a {};
            if (idx + width - 1 < fadeIdx) {
                a = 0;
            } else if (idx > fadeIdx) {
                a = 255;
            } else {
                f64 const val {1.0 - (((t * (total + width)) - static_cast<f64>(idx)) / static_cast<f64>(width))};
                a = static_cast<u8>(val * 255.0);
            }
            for (auto& v : groups[idx].Verts) { v.Color.A = a; }
        }
    }

    ////////////////////////////////////////////////////////////

    void blink::operator()(f64 t, std::span<vertex_tween_group> groups)
    {
        tween_func::square_wave<bool> wave {.Frequency = Frequency};
        bool const                    flip {wave(t)};
        color const                   c {flip ? Color0 : Color1};
        for (auto& g : groups) {
            for (auto& v : g.Verts) { v.Color = c; }
        }
    }

    ////////////////////////////////////////////////////////////

    void gradient::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        color const c {Gradient[static_cast<u8>(255 * t)]};
        for (auto& g : groups) {
            for (auto& v : g.Verts) { v.Color = c; }
        }
    }

    ////////////////////////////////////////////////////////////

    void shake::operator()(f64, std::span<vertex_tween_group> groups)
    {
        for (auto& g : groups) {
            f32 const r {RNG(-Intensity, Intensity)};
            switch (RNG(0, 1)) {
            case 0:
                for (auto& v : g.Verts) {
                    v.Position.X += r;
                    v.Position.Y += r;
                }
                break;
            case 1:
                for (auto& v : g.Verts) {
                    v.Position.X += r;
                    v.Position.Y -= r;
                }
                break;
            default:
                break;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void jitter::operator()(f64, std::span<vertex_tween_group> groups)
    {
        for (auto& g : groups) {
            for (auto& v : g.Verts) {
                v.Position.X += RNG(-Intensity, Intensity);
                v.Position.Y += RNG(-Intensity, Intensity);
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void wave::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        tween_func::sine_wave<f64> w {.Min = 0, .Max = 1};
        for (usize idx {0}; idx < groups.size(); ++idx) {
            w.Phase = static_cast<f64>(idx) / static_cast<f64>(groups.size()) * Amplitude;
            f32 const val {static_cast<f32>(w(t) * Height)};
            for (auto& v : groups[idx].Verts) { v.Position.Y += val; }
        }
    }

    ////////////////////////////////////////////////////////////

    void bounce::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        tween_func::bounce<f64> w {.Start = 0, .End = 1};
        for (usize idx {0}; idx < groups.size(); ++idx) {
            f32 const val {static_cast<f32>(w(t) * Height)};
            for (auto& v : groups[idx].Verts) { v.Position.Y += val; }
        }
    }

    ////////////////////////////////////////////////////////////

    void elastic::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        tween_func::elastic<f64> w {.Start = 0, .End = 1, .Amplitude = Amplitude, .Period = Period};
        for (usize idx {0}; idx < groups.size(); ++idx) {
            f32 const val {static_cast<f32>(w(t) * Height)};
            for (auto& v : groups[idx].Verts) { v.Position.Y += val; }
        }
    }

    ////////////////////////////////////////////////////////////

    void back::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        tween_func::back<f64> w {.Start = 0, .End = 1, .Overshoot = Overshoot};
        for (usize idx {0}; idx < groups.size(); ++idx) {
            f32 const val {static_cast<f32>(w(t) * Height)};
            for (auto& v : groups[idx].Verts) { v.Position.Y += val; }
        }
    }

    ////////////////////////////////////////////////////////////

    void size::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        rect_f totalRect {groups[0].BBox};
        for (usize i {1}; i < groups.size(); ++i) {
            totalRect = totalRect.as_union_with(groups[i].BBox);
        }

        f32 const wDiff {WidthEnd - WidthStart};
        f32 const wFac {wDiff > 0 ? static_cast<f32>(t * wDiff)
                                  : static_cast<f32>(1.0 - (t * -wDiff))};
        f32 const hDiff {HeightEnd - HeightStart};
        f32 const hFac {hDiff > 0 ? static_cast<f32>(t * hDiff)
                                  : static_cast<f32>(1.0 - (t * -hDiff))};

        for (usize i {0}; i < groups.size(); ++i) {
            point_f center {point_f::Zero};

            switch (Anchor.Horizontal) {
            case horizontal_alignment::Left:   center.X = totalRect.left(); break;
            case horizontal_alignment::Center: center.X = totalRect.center().X; break;
            case horizontal_alignment::Right:  center.X = totalRect.right(); break;
            }
            switch (Anchor.Vertical) {
            case vertical_alignment::Top:    center.Y = totalRect.top(); break;
            case vertical_alignment::Middle: center.Y = totalRect.center().Y; break;
            case vertical_alignment::Bottom: center.Y = totalRect.bottom(); break;
            }

            transform xform;
            xform.scale_at({wFac, hFac}, center);
            for (auto& v : groups[i].Verts) {
                v.Position = xform * v.Position;
            }
        }
    }

    ////////////////////////////////////////////////////////////

    void rotate::operator()(f64 t, std::span<vertex_tween_group> groups) const
    {
        for (auto& g : groups) {
            auto const rect {g.BBox};
            transform  rot;
            rot.rotate_at(degree_f {static_cast<f32>(360.0 * t * Speed)}, rect.center());
            for (auto& v : g.Verts) {
                v.Position = rot * v.Position;
            }
        }
    }

}
}

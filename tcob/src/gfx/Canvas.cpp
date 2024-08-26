// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Canvas.hpp"

#include <cmath>
#include <vector>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/tweening/Tween.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/TextFormatter.hpp"

namespace tcob::gfx {
using namespace detail;
using namespace tcob::tweening;

auto constexpr NVG_KAPPA90 = 0.5522847493f; // Length proportional to radius of a cubic bezier handle for 90deg arcs.;

enum commands {
    MoveTo   = 0,
    LineTo   = 1,
    BezierTo = 2,
    Close    = 3,
    Winding  = 4,
};

enum point_flags {
    Corner     = 0x01,
    Left       = 0x02,
    Bevel      = 0x04,
    InnerBevel = 0x08,
};

auto static constexpr signf(f32 a) -> f32 { return a >= 0.0f ? 1.0f : -1.0f; }

auto static constexpr cross(f32 dx0, f32 dy0, f32 dx1, f32 dy1) -> f32 { return dx1 * dy0 - dx0 * dy1; }

auto static Normalize(f32& x, f32& y) -> f32
{
    f32 const d {std::sqrt(x * x + y * y)};
    if (d > 1e-6f) {
        f32 const id {1.0f / d};
        x *= id;
        y *= id;
    }
    return d;
}

auto static constexpr CompositeOperationState(composite_operation op) -> blend_funcs
{
    blend_func sfactor {}, dfactor {};

    switch (op) {
    case composite_operation::SourceOver:
        sfactor = blend_func::One;
        dfactor = blend_func::OneMinusSrcAlpha;
        break;
    case composite_operation::SourceIn:
        sfactor = blend_func::DstAlpha;
        dfactor = blend_func::Zero;
        break;
    case composite_operation::SourceOut:
        sfactor = blend_func::OneMinusDstAlpha;
        dfactor = blend_func::Zero;
        break;
    case composite_operation::Atop:
        sfactor = blend_func::DstAlpha;
        dfactor = blend_func::OneMinusSrcAlpha;
        break;
    case composite_operation::DestinationOver:
        sfactor = blend_func::OneMinusDstAlpha;
        dfactor = blend_func::One;
        break;
    case composite_operation::DestinationIn:
        sfactor = blend_func::Zero;
        dfactor = blend_func::SrcAlpha;
        break;
    case composite_operation::DestinationOut:
        sfactor = blend_func::Zero;
        dfactor = blend_func::OneMinusSrcAlpha;
        break;
    case composite_operation::DestinationAtop:
        sfactor = blend_func::OneMinusDstAlpha;
        dfactor = blend_func::SrcAlpha;
        break;
    case composite_operation::Lighter:
        sfactor = blend_func::One;
        dfactor = blend_func::One;
        break;
    case composite_operation::Copy:
        sfactor = blend_func::One;
        dfactor = blend_func::Zero;
        break;
    case composite_operation::Xor:
        sfactor = blend_func::OneMinusDstAlpha;
        dfactor = blend_func::OneMinusSrcAlpha;
        break;
    }

    return {
        .SourceColorBlendFunc      = sfactor,
        .DestinationColorBlendFunc = dfactor,
        .SourceAlphaBlendFunc      = sfactor,
        .DestinationAlphaBlendFunc = dfactor};
}

auto static constexpr PointEquals(f32 x1, f32 y1, f32 x2, f32 y2, f32 tol) -> i32
{
    f32 const dx {x2 - x1};
    f32 const dy {y2 - y1};
    return dx * dx + dy * dy < tol * tol;
}

auto static constexpr DistancePointSegment(f32 x, f32 y, f32 px, f32 py, f32 qx, f32 qy) -> f32
{
    f32 const pqx {qx - px};
    f32 const pqy {qy - py};
    f32       dx {x - px};
    f32       dy {y - py};
    f32 const d {pqx * pqx + pqy * pqy};
    f32       t {pqx * dx + pqy * dy};
    if (d > 0) {
        t /= d;
    }
    if (t < 0) {
        t = 0;
    } else if (t > 1) {
        t = 1;
    }
    dx = px + t * pqx - x;
    dy = py + t * pqy - y;
    return dx * dx + dy * dy;
}

auto static GetAverageScale(mat3 const& t) -> f32
{
    f32 const sx {std::sqrt(t[0] * t[0] + t[3] * t[3])};
    f32 const sy {std::sqrt(t[1] * t[1] + t[4] * t[4])};
    return (sx + sy) * 0.5f;
}

auto static constexpr TriArea2(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) -> f32
{
    f32 const abx {bx - ax};
    f32 const aby {by - ay};
    f32 const acx {cx - ax};
    f32 const acy {cy - ay};
    return acx * aby - abx * acy;
}

void static inline constexpr SetVertex(vertex* vtx, f32 x, f32 y, f32 u, f32 v, f32 level = 0)
{
    vtx->Position[0] = x;
    vtx->Position[1] = y;

    vtx->TexCoords[0] = u;
    vtx->TexCoords[1] = v;
    vtx->TexCoords[2] = level;
}

auto static CurveDivs(f32 r, f32 arc, f32 tol) -> i32
{
    f32 const da {std::acos(r / (r + tol)) * 2.0f};
    return std::max(2, static_cast<i32>(std::ceil(arc / da)));
}

auto static constexpr PolyArea(std::span<detail::canvas_point> pts) -> f32
{
    f32 area {0};
    for (usize i {2}; i < pts.size(); ++i) {
        detail::canvas_point const a {pts[0]};
        detail::canvas_point const b {pts[i - 1]};
        detail::canvas_point const c {pts[i]};
        area += TriArea2(a.X, a.Y, b.X, b.Y, c.X, c.Y);
    }
    return area * 0.5f;
}

void static constexpr PolyReverse(std::span<detail::canvas_point> pts)
{
    detail::canvas_point tmp;
    usize                i {0};
    usize                j {pts.size() - 1};
    while (i < j) {
        tmp    = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        ++i;
        --j;
    }
}

void static constexpr ChooseBevel(i32 bevel, detail::canvas_point const& p0, detail::canvas_point const& p1, f32 w,
                                  f32& x0, f32& y0, f32& x1, f32& y1)
{
    if (bevel) {
        x0 = p1.X + p0.DY * w;
        y0 = p1.Y - p0.DX * w;
        x1 = p1.X + p1.DY * w;
        y1 = p1.Y - p1.DX * w;
    } else {
        x0 = p1.X + p1.DMX * w;
        y0 = p1.Y + p1.DMY * w;
        x1 = p1.X + p1.DMX * w;
        y1 = p1.Y + p1.DMY * w;
    }
}

auto static RoundJoin(vertex* dst, detail::canvas_point const& p0, detail::canvas_point const& p1,
                      f32 lw, f32 rw, f32 lu, f32 ru, i32 ncap) -> vertex*
{
    f32 const dlx0 {p0.DY};
    f32 const dly0 {-p0.DX};
    f32 const dlx1 {p1.DY};
    f32 const dly1 {-p1.DX};

    if (p1.Flags & Left) {
        f32 lx0 {0}, ly0 {0}, lx1 {0}, ly1 {0}, a0 {0}, a1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, lw, lx0, ly0, lx1, ly1);
        a0 = std::atan2(-dly0, -dlx0);
        a1 = std::atan2(-dly1, -dlx1);
        if (a1 > a0) {
            a1 -= TAU_F;
        }

        SetVertex(dst, lx0, ly0, lu, 1);
        dst++;
        SetVertex(dst, p1.X - dlx0 * rw, p1.Y - dly0 * rw, ru, 1);
        dst++;

        i32 const n {std::clamp(static_cast<i32>(ceilf(((a0 - a1) / (TAU_F / 2)) * ncap)), 2, ncap)};
        for (i32 i {0}; i < n; ++i) {
            f32 const u {i / static_cast<f32>(n - 1)};
            f32 const a {a0 + u * (a1 - a0)};
            f32 const rx {p1.X + std::cos(a) * rw};
            f32 const ry {p1.Y + std::sin(a) * rw};
            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;
            SetVertex(dst, rx, ry, ru, 1);
            dst++;
        }

        SetVertex(dst, lx1, ly1, lu, 1);
        dst++;
        SetVertex(dst, p1.X - dlx1 * rw, p1.Y - dly1 * rw, ru, 1);
        dst++;
    } else {
        f32 rx0 {0}, ry0 {0}, rx1 {0}, ry1 {0}, a0 {0}, a1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, -rw, rx0, ry0, rx1, ry1);
        a0 = std::atan2(dly0, dlx0);
        a1 = std::atan2(dly1, dlx1);
        if (a1 < a0) {
            a1 += TAU_F;
        }

        SetVertex(dst, p1.X + dlx0 * rw, p1.Y + dly0 * rw, lu, 1);
        dst++;
        SetVertex(dst, rx0, ry0, ru, 1);
        dst++;

        i32 const n {std::clamp(static_cast<i32>(std::ceil(((a1 - a0) / (TAU_F / 2)) * ncap)), 2, ncap)};
        for (i32 i {0}; i < n; ++i) {
            f32 const u {i / static_cast<f32>(n - 1)};
            f32 const a {a0 + u * (a1 - a0)};
            f32 const lx {p1.X + std::cos(a) * lw};
            f32 const ly {p1.Y + std::sin(a) * lw};
            SetVertex(dst, lx, ly, lu, 1);
            dst++;
            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;
        }

        SetVertex(dst, p1.X + dlx1 * rw, p1.Y + dly1 * rw, lu, 1);
        dst++;
        SetVertex(dst, rx1, ry1, ru, 1);
        dst++;
    }
    return dst;
}

auto static BevelJoin(vertex* dst, detail::canvas_point const& p0, detail::canvas_point const& p1,
                      f32 lw, f32 rw, f32 lu, f32 ru) -> vertex*
{
    f32 const dlx0 {p0.DY};
    f32 const dly0 {-p0.DX};
    f32 const dlx1 {p1.DY};
    f32 const dly1 {-p1.DX};

    if (p1.Flags & Left) {
        f32 lx0 {0}, ly0 {0}, lx1 {0}, ly1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, lw, lx0, ly0, lx1, ly1);

        SetVertex(dst, lx0, ly0, lu, 1);
        dst++;
        SetVertex(dst, p1.X - dlx0 * rw, p1.Y - dly0 * rw, ru, 1);
        dst++;

        if (p1.Flags & Bevel) {
            SetVertex(dst, lx0, ly0, lu, 1);
            dst++;
            SetVertex(dst, p1.X - dlx0 * rw, p1.Y - dly0 * rw, ru, 1);
            dst++;

            SetVertex(dst, lx1, ly1, lu, 1);
            dst++;
            SetVertex(dst, p1.X - dlx1 * rw, p1.Y - dly1 * rw, ru, 1);
            dst++;
        } else {
            f32 const rx0 {p1.X - p1.DMX * rw};
            f32 const ry0 {p1.Y - p1.DMY * rw};

            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;
            SetVertex(dst, p1.X - dlx0 * rw, p1.Y - dly0 * rw, ru, 1);
            dst++;

            SetVertex(dst, rx0, ry0, ru, 1);
            dst++;
            SetVertex(dst, rx0, ry0, ru, 1);
            dst++;

            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;
            SetVertex(dst, p1.X - dlx1 * rw, p1.Y - dly1 * rw, ru, 1);
            dst++;
        }

        SetVertex(dst, lx1, ly1, lu, 1);
        dst++;
        SetVertex(dst, p1.X - dlx1 * rw, p1.Y - dly1 * rw, ru, 1);
        dst++;
    } else {
        f32 rx0 {0}, ry0 {0}, rx1 {0}, ry1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, -rw, rx0, ry0, rx1, ry1);

        SetVertex(dst, p1.X + dlx0 * lw, p1.Y + dly0 * lw, lu, 1);
        dst++;
        SetVertex(dst, rx0, ry0, ru, 1);
        dst++;

        if (p1.Flags & Bevel) {
            SetVertex(dst, p1.X + dlx0 * lw, p1.Y + dly0 * lw, lu, 1);
            dst++;
            SetVertex(dst, rx0, ry0, ru, 1);
            dst++;

            SetVertex(dst, p1.X + dlx1 * lw, p1.Y + dly1 * lw, lu, 1);
            dst++;
            SetVertex(dst, rx1, ry1, ru, 1);
            dst++;
        } else {
            f32 const lx0 {p1.X + p1.DMX * lw};
            f32 const ly0 {p1.Y + p1.DMY * lw};

            SetVertex(dst, p1.X + dlx0 * lw, p1.Y + dly0 * lw, lu, 1);
            dst++;
            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;

            SetVertex(dst, lx0, ly0, lu, 1);
            dst++;
            SetVertex(dst, lx0, ly0, lu, 1);
            dst++;

            SetVertex(dst, p1.X + dlx1 * lw, p1.Y + dly1 * lw, lu, 1);
            dst++;
            SetVertex(dst, p1.X, p1.Y, 0.5f, 1);
            dst++;
        }

        SetVertex(dst, p1.X + dlx1 * lw, p1.Y + dly1 * lw, lu, 1);
        dst++;
        SetVertex(dst, rx1, ry1, ru, 1);
        dst++;
    }

    return dst;
}

auto static ButtCapStart(vertex* dst, detail::canvas_point const& p,
                         f32 dx, f32 dy, f32 w, f32 d,
                         f32 aa, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X - dx * d};
    f32 const py {p.Y - dy * d};
    f32 const dlx {dy};
    f32 const dly {-dx};
    SetVertex(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);
    dst++;
    SetVertex(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);
    dst++;
    SetVertex(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    SetVertex(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    return dst;
}

auto static ButtCapEnd(vertex* dst, detail::canvas_point const& p,
                       f32 dx, f32 dy, f32 w, f32 d,
                       f32 aa, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X + dx * d};
    f32 const py {p.Y + dy * d};
    f32 const dlx {dy};
    f32 const dly {-dx};
    SetVertex(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    SetVertex(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    SetVertex(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);
    dst++;
    SetVertex(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);
    dst++;
    return dst;
}

auto static RoundCapStart(vertex* dst, detail::canvas_point const& p,
                          f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X};
    f32 const py {p.Y};
    f32 const dlx {dy};
    f32 const dly {-dx};

    for (i32 i {0}; i < ncap; ++i) {
        f32 const a {i / static_cast<f32>(ncap - 1) * (TAU_F / 2)};
        f32 const ax {std::cos(a) * w}, ay {std::sin(a) * w};
        SetVertex(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);
        dst++;
        SetVertex(dst, px, py, 0.5f, 1);
        dst++;
    }
    SetVertex(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    SetVertex(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    return dst;
}

auto static RoundCapEnd(vertex* dst, detail::canvas_point const& p,
                        f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X};
    f32 const py {p.Y};
    f32 const dlx {dy};
    f32 const dly {-dx};

    SetVertex(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    SetVertex(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    for (i32 i {0}; i < ncap; ++i) {
        f32 const a {i / static_cast<f32>(ncap - 1) * (TAU_F / 2)};
        f32 const ax {std::cos(a) * w}, ay {std::sin(a) * w};
        SetVertex(dst, px, py, 0.5f, 1);
        dst++;
        SetVertex(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
        dst++;
    }
    return dst;
}

auto static Quantize(f32 a, f32 d) -> f32
{
    return (static_cast<i32>(a / d + 0.5f)) * d;
}

////////////////////////////////////////////////////////////

canvas::canvas()
    : _impl {locate_service<render_system>().create_canvas()}
{
    save();
    reset();

    set_device_pixel_ratio(1.0f);
}

auto canvas::get_texture(i32 level) -> assets::asset_ptr<texture>
{
    return _rtt[level];
}

void canvas::begin_frame(size_i windowSize, f32 devicePixelRatio, i32 rtt)
{
    _activeRtt  = rtt;
    _windowSize = windowSize;

    _rtt[_activeRtt]->Size = windowSize;
    _rtt[_activeRtt]->prepare_render();
    _rtt[_activeRtt]->clear({0, 0, 0, 0});

    _states = {};

    save();
    reset();

    set_device_pixel_ratio(devicePixelRatio);

    _impl->set_size(size_f {windowSize});
}

void canvas::end_frame()
{
    _impl->flush();
    _rtt[_activeRtt]->finalize_render();
}

void canvas::cancel_frame()
{
    _impl->cancel();
}

void canvas::save()
{
    if (_states.empty()) {
        _states.emplace();
    } else {
        _states.push(_states.top());
    }
}

void canvas::restore()
{
    _states.pop();
}

void canvas::reset()
{
    state& s {get_state()};

    set_paint_color(s.Fill, colors::White);
    set_paint_color(s.Stroke, colors::Black);
    s.CompositeOperation = CompositeOperationState(composite_operation::SourceOver);
    s.ShapeAntiAlias     = true;
    s.StrokeWidth        = 1.0f;
    s.MiterLimit         = 10.0f;
    s.LineCap            = line_cap::Butt;
    s.LineJoin           = line_join::Miter;
    s.Alpha              = 1.0f;
    s.XForm              = transform::Identity;

    s.Scissor.Extent[0] = -1.0f;
    s.Scissor.Extent[1] = -1.0f;

    s.TextAlign = {};
}

auto canvas::create_guard() -> state_guard
{
    return state_guard {this};
}

////////////////////////////////////////////////////////////

void canvas::begin_path()
{
    _commands.clear();
    clear_path_cache();
}

void canvas::close_path()
{
    append_commands(std::vector<f32> {Close});
}

void canvas::set_path_winding(winding dir)
{
    append_commands(std::vector<f32> {Winding, static_cast<f32>(dir)});
}

void canvas::set_path_winding(solidity s)
{
    append_commands(std::vector<f32> {Winding, static_cast<f32>(s)});
}

////////////////////////////////////////////////////////////

void canvas::move_to(point_f pos)
{
    append_commands(std::vector<f32> {MoveTo, pos.X, pos.Y});
}

void canvas::line_to(point_f pos)
{
    append_commands(std::vector<f32> {LineTo, pos.X, pos.Y});
}

void canvas::fill_lines(std::span<point_f const> points)
{
    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        line_to(points[i]);
    }

    fill();
}

void canvas::stroke_line(point_f from, point_f to)
{
    begin_path();

    move_to(from);
    line_to(to);

    stroke();
}

void canvas::stroke_lines(std::span<point_f const> points)
{
    if (points.size() == 1) {
        return;
    }

    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        line_to(points[i]);
    }

    stroke();
}

void canvas::stroke_dashed_cubic_bezier(point_f start, point_f cp0, point_f cp1, point_f end, i32 numDashes)
{
    begin_path();

    funcs::cubic_bezier_curve func {start, cp0, cp1, end};
    f32 const                 inc {1.0f / (numDashes * 2)};

    for (f32 t {0}; t <= 1.0f; t += inc * 2) {
        move_to(func(t));
        line_to(func(t + inc));
    }

    stroke();
}

void canvas::stroke_dashed_quad_bezier(point_f start, point_f cp, point_f end, i32 numDashes)
{
    begin_path();

    funcs::quad_bezier_curve func {start, cp, end};
    f32 const                inc {1.0f / (numDashes * 2)};

    for (f32 t {0}; t <= 1.0f; t += inc * 2) {
        move_to(func(t));
        line_to(func(t + inc));
    }

    stroke();
}

void canvas::stroke_dashed_line(point_f from, point_f to, i32 numDashes)
{
    begin_path();

    funcs::linear<point_f> func {from, to};
    f32 const              inc {1.0f / (numDashes * 2)};

    for (f32 t {0}; t <= 1.0f; t += inc * 2) {
        move_to(func(t));
        line_to(func(t + inc));
    }

    stroke();
}

void canvas::stroke_dashed_circle(point_f center, f32 r, i32 numDashes)
{
    begin_path();

    funcs::circular func {degree_f {0}, degree_f {360}};
    f32 const       inc {1.0f / (numDashes * 2)};

    for (f32 t {0}; t <= 1.0f; t += inc * 2) {
        move_to(func(t) * r + center);
        line_to(func(t + inc) * r + center);
    }

    stroke();
}

void canvas::dotted_cubic_bezier(point_f start, point_f cp0, point_f cp1, point_f end, f32 r, i32 numDots)
{
    funcs::cubic_bezier_curve func {start, cp0, cp1, end};
    f32 const                 inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), r);
    }
}

void canvas::dotted_quad_bezier(point_f start, point_f cp, point_f end, f32 r, i32 numDots)
{
    funcs::quad_bezier_curve func {start, cp, end};
    f32 const                inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), r);
    }
}

void canvas::dotted_line(point_f from, point_f to, f32 r, i32 numDots)
{
    funcs::linear<point_f> func {from, to};
    f32 const              inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), r);
    }
}

void canvas::dotted_circle(point_f center, f32 rcircle, f32 rdots, i32 numDots)
{
    funcs::circular func {degree_f {0}, degree_f {360}};
    f32 const       inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t) * rcircle + center, rdots);
    }
}

void canvas::wavy_line(point_f from, point_f to, f32 amp, f32 freq, f32 phase)
{
    f32 const xMin {std::min(from.X, to.X)};
    f32 const xDiff {std::abs(from.X - to.X)};

    for (f32 f {0}; f < xDiff; ++f) {
        f32 const x {f + xMin};
        f32 const y {amp * std::sin(freq * x + phase) + (from.Y + (to.Y - from.Y) * (x - from.X) / (to.X - from.X))};
        line_to({x, y});
    }
}

void canvas::regular_polygon(point_f pos, size_f size, i32 n)
{
    auto const [x, y] {pos};
    move_to({x, y - size.Height});
    for (i32 i {1}; i < n; ++i) {
        f32 const angle {TAU_F / n * i};
        f32 const dx {std::sin(angle) * size.Width};
        f32 const dy {-std::cos(angle) * size.Height};
        line_to({x + dx, y + dy});
    }
    line_to({x, y - size.Height});
}

void canvas::star(point_f pos, f32 outerR, f32 innerR, i32 n)
{
    auto const [x, y] {pos};
    move_to({x, y - outerR});
    for (i32 i {1}; i < n * 2; ++i) {
        f32 const angle {(TAU_F / 2) / n * i};
        f32 const r {(i % 2 == 0) ? outerR : innerR};
        f32 const dx {std::sin(angle) * r};
        f32 const dy {-std::cos(angle) * r};
        line_to({x + dx, y + dy});
    }
    line_to({x, y - outerR});
}

void canvas::triangle(point_f a, point_f b, point_f c)
{
    move_to(a);
    line_to(b);
    line_to(c);
    line_to(a);
}

auto canvas::path_2d(path2d const& path) -> void
{
    for (auto const& command : path.Commands) {
        command(*this);
    }
}

void canvas::path_arc_to(f32 x1, f32 y1, std::vector<f32> const& args, bool rel)
{
    auto static nsvg_xformPoint {[](f32* dx, f32* dy, f32 x, f32 y, std::array<f32, 6> const& t) {
        *dx = x * t[0] + y * t[2] + t[4];
        *dy = x * t[1] + y * t[3] + t[5];
    }};
    auto static nsvg_xformVec {[](f32* dx, f32* dy, f32 x, f32 y, std::array<f32, 6> const& t) {
        *dx = x * t[0] + y * t[2];
        *dy = x * t[1] + y * t[3];
    }};
    auto static nsvg_sqr {[](f32 x) -> f32 { return x * x; }};
    auto static nsvg_vecang {[](f32 ux, f32 uy, f32 vx, f32 vy) -> f32 {
        f32 const r {(ux * vx + uy * vy) / (std::sqrt(ux * ux + uy * uy) * std::sqrt(vx * vx + vy * vy))};
        return ((ux * vy < uy * vx) ? -1.0f : 1.0f) * std::acos(std::clamp(r, -1.0f, 1.0f));
    }};

    // based on https://github.com/memononen/nanosvg

    f32       rx {std::fabs(args[0])};                // y radius
    f32       ry {std::fabs(args[1])};                // x radius
    f32 const rotx {args[2] / 360.0f * TAU_F};        // x rotation angle
    i32 const fa {std::fabs(args[3]) > 1e-6 ? 1 : 0}; // Large arc
    i32 const fs {std::fabs(args[4]) > 1e-6 ? 1 : 0}; // Sweep direction

    f32 x2 {0}, y2 {0};
    if (rel) {                                        // end point
        x2 = x1 + args[5];
        y2 = y1 + args[6];
    } else {
        x2 = args[5];
        y2 = args[6];
    }

    f32 dx {x1 - x2};
    f32 dy {y1 - y2};
    f32 d {std::sqrt(dx * dx + dy * dy)};
    if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
        line_to({x2, y2});
        return;
    }

    f32 const sinrx {std::sin(rotx)};
    f32 const cosrx {std::cos(rotx)};

    // Convert to center point parameterization.
    // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
    // 1) Compute x1', y1'
    f32 const x1p {cosrx * dx / 2.0f + sinrx * dy / 2.0f};
    f32 const y1p {-sinrx * dx / 2.0f + cosrx * dy / 2.0f};
    d = nsvg_sqr(x1p) / nsvg_sqr(rx) + nsvg_sqr(y1p) / nsvg_sqr(ry);
    if (d > 1) {
        d = std::sqrt(d);
        rx *= d;
        ry *= d;
    }
    // 2) Compute cx', cy'
    f32       s {0.0f};
    f32       sa {nsvg_sqr(rx) * nsvg_sqr(ry) - nsvg_sqr(rx) * nsvg_sqr(y1p) - nsvg_sqr(ry) * nsvg_sqr(x1p)};
    f32 const sb {nsvg_sqr(rx) * nsvg_sqr(y1p) + nsvg_sqr(ry) * nsvg_sqr(x1p)};
    if (sa < 0.0f) { sa = 0.0f; }
    if (sb > 0.0f) { s = std::sqrt(sa / sb); }
    if (fa == fs) { s = -s; }
    f32 const cxp {s * rx * y1p / ry};
    f32 const cyp {s * -ry * x1p / rx};

    // 3) Compute cx,cy from cx',cy'
    f32 const cx {(x1 + x2) / 2.0f + cosrx * cxp - sinrx * cyp};
    f32 const cy {(y1 + y2) / 2.0f + sinrx * cxp + cosrx * cyp};

    // 4) Calculate theta1, and delta theta.
    f32 const ux {(x1p - cxp) / rx};
    f32 const uy {(y1p - cyp) / ry};
    f32 const vx {(-x1p - cxp) / rx};
    f32 const vy {(-y1p - cyp) / ry};
    f32 const a1 {nsvg_vecang(1.0f, 0.0f, ux, uy)}; // Initial angle
    f32       da {nsvg_vecang(ux, uy, vx, vy)};     // Delta angle

    if (fs == 0 && da > 0) {
        da -= TAU_F;
    } else if (fs == 1 && da < 0) {
        da += TAU_F;
    }

    // Approximate the arc using cubic spline segments.
    std::array<f32, 6> const t {cosrx, sinrx, -sinrx, cosrx, cx, cy};

    // Split arc into max 90 degree segments.
    // The loop assumes an iteration per end point (including start and end), this +1.
    i32 const ndivs {static_cast<i32>(std::fabs(da) / (TAU_F / 4) + 1.0f)};
    f32       hda {(da / static_cast<f32>(ndivs)) / 2.0f};
    // Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
    if ((hda < 1e-3f) && (hda > -1e-3f)) {
        hda *= 0.5f;
    } else {
        hda = (1.0f - std::cos(hda)) / std::sin(hda);
    }
    f32 kappa {std::fabs(4.0f / 3.0f * hda)};
    if (da < 0.0f) { kappa = -kappa; }

    f32 x {0}, y {0};
    f32 tanx {0}, tany {0};
    f32 px {0}, py {0};
    f32 ptanx {0}, ptany {0};
    for (i32 i {0}; i <= ndivs; ++i) {
        f32 const a {a1 + da * (static_cast<f32>(i) / static_cast<f32>(ndivs))};
        dx = std::cos(a);
        dy = std::sin(a);
        nsvg_xformPoint(&x, &y, dx * rx, dy * ry, t);                      // position
        nsvg_xformVec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t); // tangent
        if (i > 0) { cubic_bezier_to({px + ptanx, py + ptany}, {x - tanx, y - tany}, {x, y}); }
        px    = x;
        py    = y;
        ptanx = tanx;
        ptany = tany;
    }
}

////////////////////////////////////////////////////////////

void canvas::cubic_bezier_to(point_f cp0, point_f cp1, point_f end)
{
    append_commands(std::vector<f32> {BezierTo, cp0.X, cp0.Y, cp1.X, cp1.Y, end.X, end.Y});
}

void canvas::quad_bezier_to(point_f cp, point_f end)
{
    auto const [x0, y0] {_commandPoint};

    append_commands(std::vector<f32> {
        BezierTo,
        x0 + 2.0f / 3.0f * (cp.X - x0), y0 + 2.0f / 3.0f * (cp.Y - y0),
        end.X + 2.0f / 3.0f * (cp.X - end.X), end.Y + 2.0f / 3.0f * (cp.Y - end.Y),
        end.X, end.Y});
}

////////////////////////////////////////////////////////////

void canvas::arc(point_f c, f32 r, radian_f startAngle, radian_f endAngle, winding dir)
{
    i32 const move {!_commands.empty() ? LineTo : MoveTo};
    f32 const a0 {startAngle.Value};
    f32 const a1 {endAngle.Value};

    // Clamp angles
    f32 da {a1 - a0};
    if (dir == winding::CW) {
        if (std::abs(da) >= TAU_F) {
            da = TAU_F;
        } else {
            while (da < 0.0f) {
                da += TAU_F;
            }
        }
    } else {
        if (std::abs(da) >= TAU_F) {
            da = -TAU_F;
        } else {
            while (da > 0.0f) {
                da -= TAU_F;
            }
        }
    }

    // Split arc into max 90 degree segments.
    i32 const ndivs {std::max(1, std::min(static_cast<i32>(std::abs(da) / (TAU_F * 0.25f) + 0.5f), 5))};
    f32 const hda {(da / static_cast<f32>(ndivs)) / 2.0f};
    f32       kappa {std::abs(4.0f / 3.0f * (1.0f - std::cos(hda)) / std::sin(hda))};
    if (dir == winding::CCW) {
        kappa = -kappa;
    }

    f32 px {0}, py {0}, ptanx {0}, ptany {0};

    std::vector<f32> vals;
    vals.reserve(138);
    for (i32 i {0}; i <= ndivs; ++i) {
        f32 const a {a0 + da * (i / static_cast<f32>(ndivs))};
        f32 const dx {std::cos(a)};
        f32 const dy {std::sin(a)};
        f32 const x {c.X + dx * r};
        f32 const y {c.Y + dy * r};
        f32 const tanx {-dy * r * kappa};
        f32 const tany {dx * r * kappa};

        if (i == 0) {
            vals.push_back(static_cast<f32>(move));
            vals.push_back(x);
            vals.push_back(y);
        } else {
            vals.push_back(BezierTo);
            vals.push_back(px + ptanx);
            vals.push_back(py + ptany);
            vals.push_back(x - tanx);
            vals.push_back(y - tany);
            vals.push_back(x);
            vals.push_back(y);
        }
        px    = x;
        py    = y;
        ptanx = tanx;
        ptany = tany;
    }

    append_commands(vals);
}

void canvas::arc_to(point_f pos1, point_f pos2, f32 radius)
{
    if (_commands.empty()) {
        return;
    }

    winding dir {};

    // Handle degenerate cases.
    if (_commandPoint.equals(pos1, _distTol)
        || pos1.equals(pos2, _distTol)
        || DistancePointSegment(pos1.X, pos1.Y, _commandPoint.X, _commandPoint.Y, pos2.X, pos2.Y) < _distTol * _distTol
        || radius < _distTol) {
        line_to(pos1);
        return;
    }

    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    f32 dx0 {_commandPoint.X - pos1.X};
    f32 dy0 {_commandPoint.Y - pos1.Y};
    f32 dx1 {pos2.X - pos1.X};
    f32 dy1 {pos2.Y - pos1.Y};
    Normalize(dx0, dy0);
    Normalize(dx1, dy1);
    f32 const a {std::acos(dx0 * dx1 + dy0 * dy1)};
    f32 const d {radius / std::tan(a / 2.0f)};

    if (d > 10000.0f) {
        line_to(pos1);
        return;
    }

    f32      cx {0}, cy {0};
    radian_f a0 {0}, a1 {0};
    if (cross(dx0, dy0, dx1, dy1) > 0.0f) {
        cx  = pos1.X + dx0 * d + dy0 * radius;
        cy  = pos1.Y + dy0 * d + -dx0 * radius;
        a0  = std::atan2(dx0, -dy0);
        a1  = std::atan2(-dx1, dy1);
        dir = winding::CW;
    } else {
        cx  = pos1.X + dx0 * d + -dy0 * radius;
        cy  = pos1.Y + dy0 * d + dx0 * radius;
        a0  = std::atan2(-dx0, dy0);
        a1  = std::atan2(dx1, -dy1);
        dir = winding::CCW;
    }

    arc({cx, cy}, radius, a0, a1, dir);
}

////////////////////////////////////////////////////////////

void canvas::rect(rect_f const& rec)
{
    auto const [x, y, w, h] {rec};
    append_commands(std::vector<f32> {
        MoveTo, x, y,
        LineTo, x, y + h,
        LineTo, x + w, y + h,
        LineTo, x + w, y,
        Close});
}

void canvas::rounded_rect(rect_f const& r, f32 rad)
{
    rounded_rect_varying(r, rad, rad, rad, rad);
}

void canvas::rounded_rect_varying(rect_f const& rec, f32 radTL, f32 radTR, f32 radBR, f32 radBL)
{
    auto const [x, y, w, h] {rec};
    if (radTL < 0.1f && radTR < 0.1f && radBR < 0.1f && radBL < 0.1f) {
        rect(rec);
        return;
    }

    f32 const halfw {std::abs(w) * 0.5f};
    f32 const halfh {std::abs(h) * 0.5f};
    f32 const rxBL {std::min(radBL, halfw) * signf(w)}, ryBL {std::min(radBL, halfh) * signf(h)};
    f32 const rxBR {std::min(radBR, halfw) * signf(w)}, ryBR {std::min(radBR, halfh) * signf(h)};
    f32 const rxTR {std::min(radTR, halfw) * signf(w)}, ryTR {std::min(radTR, halfh) * signf(h)};
    f32 const rxTL {std::min(radTL, halfw) * signf(w)}, ryTL {std::min(radTL, halfh) * signf(h)};
    append_commands(std::vector<f32> {
        MoveTo, x, y + ryTL,
        LineTo, x, y + h - ryBL,
        BezierTo, x, y + h - ryBL * (1 - NVG_KAPPA90), x + rxBL * (1 - NVG_KAPPA90), y + h, x + rxBL, y + h,
        LineTo, x + w - rxBR, y + h,
        BezierTo, x + w - rxBR * (1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR * (1 - NVG_KAPPA90), x + w, y + h - ryBR,
        LineTo, x + w, y + ryTR,
        BezierTo, x + w, y + ryTR * (1 - NVG_KAPPA90), x + w - rxTR * (1 - NVG_KAPPA90), y, x + w - rxTR, y,
        LineTo, x + rxTL, y,
        BezierTo, x + rxTL * (1 - NVG_KAPPA90), y, x, y + ryTL * (1 - NVG_KAPPA90), x, y + ryTL,
        Close});
}

////////////////////////////////////////////////////////////

void canvas::ellipse(point_f c, f32 rx, f32 ry)
{
    auto const [cx, cy] {c};
    append_commands(std::vector<f32> {
        MoveTo, cx - rx, cy,
        BezierTo, cx - rx, cy + ry * NVG_KAPPA90, cx - rx * NVG_KAPPA90, cy + ry, cx, cy + ry,
        BezierTo, cx + rx * NVG_KAPPA90, cy + ry, cx + rx, cy + ry * NVG_KAPPA90, cx + rx, cy,
        BezierTo, cx + rx, cy - ry * NVG_KAPPA90, cx + rx * NVG_KAPPA90, cy - ry, cx, cy - ry,
        BezierTo, cx - rx * NVG_KAPPA90, cy - ry, cx - rx, cy - ry * NVG_KAPPA90, cx - rx, cy,
        Close});
}

void canvas::circle(point_f c, f32 r)
{
    ellipse(c, r, r);
}

////////////////////////////////////////////////////////////

void canvas::set_fill_style(color c)
{
    set_paint_color(get_state().Fill, c);
}

void canvas::set_fill_style(canvas_paint const& paint)
{
    state& s {get_state()};
    s.Fill       = paint;
    s.Fill.XForm = s.XForm * s.Fill.XForm;
}

void canvas::set_stroke_style(color c)
{
    set_paint_color(get_state().Stroke, c);
}

void canvas::set_stroke_style(canvas_paint const& paint)
{
    state& s {get_state()};
    s.Stroke       = paint;
    s.Stroke.XForm = s.XForm * s.Stroke.XForm;
}

void canvas::set_shape_antialias(bool enabled)
{
    get_state().ShapeAntiAlias = enabled;
}

void canvas::set_miter_limit(f32 limit)
{
    get_state().MiterLimit = limit;
}

void canvas::set_stroke_width(f32 size)
{
    get_state().StrokeWidth = size;
}

void canvas::set_edge_antialias(bool enabled)
{
    _edgeAntiAlias = enabled;
}

void canvas::set_line_cap(line_cap cap)
{
    get_state().LineCap = cap;
}

void canvas::set_line_join(line_join join)
{
    get_state().LineJoin = join;
}

void canvas::set_global_alpha(f32 alpha)
{
    get_state().Alpha = alpha;
}

void static multiply_alpha_paint(paint_color& c, f32 alpha)
{
    if (auto* arg0 {std::get_if<color>(&c)}) {
        arg0->A = static_cast<u8>(arg0->A * alpha);
    } else if (auto* arg1 {std::get_if<paint_gradient>(&c)}) {
        arg1->first *= alpha;
    }
}

void canvas::fill()
{
    state&       s {get_state()};
    canvas_paint fillPaint {s.Fill};

    flatten_paths();
    if (_edgeAntiAlias && s.ShapeAntiAlias) {
        expand_fill(_fringeWidth, line_join::Miter, 2.4f);
    } else {
        expand_fill(0.0f, line_join::Miter, 2.4f);
    }

    // Apply global alpha
    multiply_alpha_paint(fillPaint.Color, s.Alpha);

    _impl->render_fill(fillPaint, s.CompositeOperation, s.Scissor, _fringeWidth, _cache.bounds, _cache.paths);
}

void canvas::stroke()
{
    state&       s {get_state()};
    f32 const    scale {GetAverageScale(s.XForm.Matrix)};
    f32          strokeWidth {std::clamp(s.StrokeWidth * scale, 0.0f, 200.0f)};
    canvas_paint strokePaint {s.Stroke};

    if (strokeWidth < _fringeWidth) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        // Since coverage is area, scale by alpha*alpha.
        f32 const alpha {std::clamp(strokeWidth / _fringeWidth, 0.0f, 1.0f)};

        multiply_alpha_paint(strokePaint.Color, alpha * alpha);
        strokeWidth = _fringeWidth;
    }

    // Apply global alpha
    multiply_alpha_paint(strokePaint.Color, s.Alpha);

    flatten_paths();

    if (_edgeAntiAlias && s.ShapeAntiAlias) {
        expand_stroke(strokeWidth * 0.5f, _fringeWidth, s.LineCap, s.LineJoin, s.MiterLimit);
    } else {
        expand_stroke(strokeWidth * 0.5f, 0.0f, s.LineCap, s.LineJoin, s.MiterLimit);
    }

    _impl->render_stroke(strokePaint, s.CompositeOperation, s.Scissor, _fringeWidth, strokeWidth, _cache.paths);
}

////////////////////////////////////////////////////////////

auto canvas::create_linear_gradient(point_f s, point_f e, color_gradient const& gradient) -> canvas_paint
{
    f32 const large {1e5};

    // Calculate transform aligned to the line
    f32       dx {e.X - s.X};
    f32       dy {e.Y - s.Y};
    f32 const d {std::sqrt(dx * dx + dy * dy)};
    if (d > 0.0001f) {
        dx /= d;
        dy /= d;
    } else {
        dx = 0;
        dy = 1;
    }

    return {
        .XForm   = {dy, dx, s.X - dx * large,
                    -dx, dy, s.Y - dy * large,
                    0, 0, 1},
        .Extent  = {large, large + d * 0.5f},
        .Radius  = 0.0f,
        .Feather = std::max(1.0f, d),
        .Color   = paint_gradient {1.0f, gradient}};
}

auto canvas::create_box_gradient(rect_f const& rect, f32 r, f32 f, color_gradient const& gradient) -> canvas_paint
{
    auto const& c {rect.get_center()};
    return {
        .XForm   = {1.0f, 0.0f, c.X,
                    0.0f, 1.0f, c.Y,
                    0.0f, 0.0f, 1.0f},
        .Extent  = {rect.Width * 0.5f, rect.Height * 0.5f},
        .Radius  = r,
        .Feather = std::max(1.0f, f),
        .Color   = paint_gradient {1.0f, gradient}};
}

auto canvas::create_radial_gradient(point_f c, f32 inr, f32 outr, color_gradient const& gradient) -> canvas_paint
{
    return create_radial_gradient(c, inr, outr, size_f::One, gradient);
}

auto canvas::create_radial_gradient(point_f c, f32 inr, f32 outr, size_f scale, color_gradient const& gradient) -> canvas_paint
{
    f32 const r {(inr + outr) * 0.5f};
    f32 const f {(outr - inr)};

    return {
        .XForm   = {scale.Width, 0.0f, c.X,
                    0.0f, scale.Height, c.Y,
                    0.0f, 0.0f, 1.0f},
        .Extent  = {r, r},
        .Radius  = r,
        .Feather = std::max(1.0f, f),
        .Color   = paint_gradient {1.0f, gradient}};
}

auto canvas::create_image_pattern(point_f c, size_f e, degree_f angle, texture* image, f32 alpha) -> canvas_paint
{
    canvas_paint p {.Extent = {e.Width, e.Height},
                    .Color  = color {255, 255, 255, static_cast<u8>(255 * alpha)},
                    .Image  = image};

    p.XForm.rotate(angle);
    p.XForm.translate(c);

    return p;
}

////////////////////////////////////////////////////////////

void canvas::set_global_composite_operation(composite_operation op)
{
    get_state().CompositeOperation = CompositeOperationState(op);
}

void canvas::set_global_composite_blendfunc(blend_func sfactor, blend_func dfactor)
{
    set_global_composite_blendfunc_separate(sfactor, dfactor, sfactor, dfactor);
}

void canvas::set_global_composite_blendfunc_separate(blend_func srcRGB, blend_func dstRGB, blend_func srcAlpha, blend_func dstAlpha)
{
    blend_funcs op {
        .SourceColorBlendFunc      = srcRGB,
        .DestinationColorBlendFunc = dstRGB,
        .SourceAlphaBlendFunc      = srcAlpha,
        .DestinationAlphaBlendFunc = dstAlpha};

    get_state().CompositeOperation = op;
}

void canvas::set_global_enforce_path_winding(bool force)
{
    _enforceWinding = force;
}

////////////////////////////////////////////////////////////

void canvas::translate(point_f c)
{
    get_state().XForm.translate(c);
}

void canvas::rotate(degree_f angle)
{
    get_state().XForm.rotate(angle);
}

void canvas::rotate_at(degree_f angle, point_f p)
{
    translate(p);
    rotate(angle);
    translate(-p);
}

void canvas::scale(size_f scale)
{
    get_state().XForm.scale(scale);
}

void canvas::scale_at(size_f sc, point_f p)
{
    translate(p);
    scale(sc);
    translate(-p);
}

void canvas::skew(degree_f angleX, degree_f angleY)
{
    get_state().XForm.skew({angleX, angleY});
}

void canvas::skew_at(degree_f angleX, degree_f angleY, point_f p)
{
    translate(p);
    skew(angleX, angleY);
    translate(-p);
}

void canvas::set_transform(transform xform)
{
    get_state().XForm = xform;
}

void canvas::reset_transform()
{
    get_state().XForm = transform::Identity;
}

////////////////////////////////////////////////////////////

void canvas::set_scissor(rect_f const& rect, bool transform)
{
    state& s {get_state()};

    auto [x, y, w, h] {rect};
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    s.Scissor.XForm = transform::Identity;

    s.Scissor.XForm.translate({x + w * 0.5f, y + h * 0.5f});
    if (transform) { s.Scissor.XForm = s.XForm * s.Scissor.XForm; }

    s.Scissor.Extent[0] = w * 0.5f;
    s.Scissor.Extent[1] = h * 0.5f;
}

void canvas::reset_scissor()
{
    state& s {get_state()};
    s.Scissor.XForm     = transform {0, 0, 0, 0, 0, 0, 0, 0, 0};
    s.Scissor.Extent[0] = -1.0f;
    s.Scissor.Extent[1] = -1.0f;
}

void canvas::set_font(font* font)
{
    state& s {get_state()};
    s.Font = font;
}

////////////////////////////////////////////////////////////

void canvas::draw_textbox(rect_f const& rect, utf8_string_view text)
{
    draw_textbox(rect.get_position(), format_text(rect.get_size(), text));
}

void canvas::draw_textbox(point_f offset, text_formatter::result const& formatResult)
{
    state& s {get_state()};
    if (!s.Font) { return; }

    f32 const scale {get_font_scale() * _devicePxRatio};
    f32 const invscale {1.0f / scale};
    auto*     verts {alloc_temp_verts(formatResult.QuadCount * 6)};
    usize     nverts {0};

    f32 const x {std::floor(offset.X + 0.5f)};
    f32 const y {std::floor(offset.Y + 0.5f)};

    bool const isTranslation {s.XForm.is_translate_only()};

    for (auto const& token : formatResult.Tokens) {
        for (usize i {0}; i < token.Quads.size(); ++i) {
            auto const& quad {token.Quads[i]};
            auto const& posRect {quad.Rect};

            auto const& uvRect {quad.TexRegion.UVRect};
            f32 const   uvLeft {uvRect.X};
            f32 const   uvRight {uvRect.right()};
            f32 const   uvTop {uvRect.Y};
            f32 const   uvBottom {uvRect.bottom()};

            f32 const level {static_cast<f32>(quad.TexRegion.Level)};

            point_f topLeft {}, topRight {}, bottomLeft {}, bottomRight {};

            if (isTranslation) {
                auto const tl {s.XForm * point_f {posRect.left() * invscale + x, posRect.top() * invscale + y}};
                auto const br {s.XForm * point_f {posRect.right() * invscale + x, posRect.bottom() * invscale + y}};

                topLeft.X = bottomLeft.X = std::floor(tl.X + 0.5f);
                topRight.X = bottomRight.X = std::floor(br.X + 0.5f);
                topLeft.Y = topRight.Y = std::floor(tl.Y + 0.5f);
                bottomLeft.Y = bottomRight.Y = std::floor(br.Y + 0.5f);
            } else {
                topLeft       = {s.XForm * point_f {posRect.left() * invscale + x, posRect.top() * invscale + y}};
                topLeft.X     = std::floor(topLeft.X + 0.5f);
                topLeft.Y     = std::floor(topLeft.Y + 0.5f);
                topRight      = {s.XForm * point_f {posRect.right() * invscale + x, posRect.top() * invscale + y}};
                topRight.X    = std::floor(topRight.X + 0.5f);
                topRight.Y    = std::floor(topRight.Y + 0.5f);
                bottomRight   = {s.XForm * point_f {posRect.right() * invscale + x, posRect.bottom() * invscale + y}};
                bottomRight.X = std::floor(bottomRight.X + 0.5f);
                bottomRight.Y = std::floor(bottomRight.Y + 0.5f);
                bottomLeft    = {s.XForm * point_f {posRect.left() * invscale + x, posRect.bottom() * invscale + y}};
                bottomLeft.X  = std::floor(bottomLeft.X + 0.5f);
                bottomLeft.Y  = std::floor(bottomLeft.Y + 0.5f);
            }

            SetVertex(&verts[nverts++], topLeft.X, topLeft.Y, uvLeft, uvTop, level);
            SetVertex(&verts[nverts++], bottomRight.X, bottomRight.Y, uvRight, uvBottom, level);
            SetVertex(&verts[nverts++], topRight.X, topRight.Y, uvRight, uvTop, level);
            SetVertex(&verts[nverts++], topLeft.X, topLeft.Y, uvLeft, uvTop, level);
            SetVertex(&verts[nverts++], bottomLeft.X, bottomLeft.Y, uvLeft, uvBottom, level);
            SetVertex(&verts[nverts++], bottomRight.X, bottomRight.Y, uvRight, uvBottom, level);
        }
    }

    render_text(s.Font, {verts, nverts});
}

auto canvas::format_text(size_f const& size, utf8_string_view text, f32 scale) -> text_formatter::result
{
    state& s {get_state()};
    if (!s.Font) { return {}; }
    return text_formatter::format(text, *s.Font, s.TextAlign, size, scale, true);
}

auto canvas::measure_text(f32 height, utf8_string_view text) -> size_f
{
    state& s {get_state()};
    if (!s.Font) { return {}; }
    return text_formatter::measure(text, *s.Font, height, true);
}

void canvas::set_text_halign(horizontal_alignment align)
{
    get_state().TextAlign.Horizontal = align;
}

void canvas::set_text_valign(vertical_alignment align)
{
    get_state().TextAlign.Vertical = align;
}

////////////////////////////////////////////////////////////

void canvas::draw_image(texture* image, string const& region, rect_f const& rect)
{
    state&       s {get_state()};
    canvas_paint paint {s.Fill};

    // Render triangles.
    paint.Image = image;

    // Apply global alpha
    multiply_alpha_paint(paint.Color, s.Alpha);

    texture_region texRegion {paint.Image->get_region(region)};

    quad quad {};
    geometry::set_position(quad, rect, s.XForm);
    geometry::set_color(quad, colors::White);
    geometry::set_texcoords(quad, texRegion);

    for (auto& vert : quad) {
        vert.Position[0] = std::floor(vert.Position[0] + 0.5f);
        vert.Position[1] = std::floor(vert.Position[1] + 0.5f);
    }

    std::array<vertex, 6> const verts {quad[3], quad[1], quad[0], quad[3], quad[2], quad[1]};
    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, verts, _fringeWidth);
}

void canvas::draw_nine_patch(texture* image, string const& region, rect_f const& rect, point_f offsetCenterLT, point_f offsetCenterRB, rect_f const& localCenterUV)
{
    point_f const posTopLeft {offsetCenterLT};
    point_f const posBottomRight {offsetCenterRB};

    f32 const leftCenter {rect.left() + posTopLeft.X};
    f32 const rightCenter {rect.right() - posBottomRight.X};
    f32 const topCenter {rect.top() + posTopLeft.Y};
    f32 const bottomCenter {rect.bottom() - posBottomRight.Y};

    draw_nine_patch(image, region, rect, rect_f::FromLTRB(leftCenter, topCenter, rightCenter, bottomCenter), localCenterUV);
}

void canvas::draw_nine_patch(texture* image, string const& region, rect_f const& rect, rect_f const& center, rect_f const& localCenterUV)
{
    state&       s {get_state()};
    canvas_paint paint {s.Fill};

    // Render triangles.
    paint.Image = image;

    // Apply global alpha
    multiply_alpha_paint(paint.Color, s.Alpha);

    f32 const left {rect.left()};
    f32 const leftCenter {center.left()};
    f32 const rightCenter {center.right()};
    f32 const right {rect.right()};

    f32 const top {rect.top()};
    f32 const topCenter {center.top()};
    f32 const bottomCenter {center.bottom()};
    f32 const bottom {rect.bottom()};

    texture_region texRegion {paint.Image->get_region(region)};
    rect_f const&  uvRect {texRegion.UVRect};
    u32 const      level {texRegion.Level};

    f32 const uv_left {uvRect.left()};
    f32 const uv_leftCenter {uvRect.left() + localCenterUV.left()};
    f32 const uv_rightCenter {uvRect.left() + localCenterUV.right()};
    f32 const uv_right {uvRect.right()};

    f32 const uv_top {uvRect.top()};
    f32 const uv_topCenter {uvRect.top() + localCenterUV.top()};
    f32 const uv_bottomCenter {uvRect.top() + localCenterUV.bottom()};
    f32 const uv_bottom {uvRect.bottom()};

    auto const setQuad {[&s, level](quad& q, rect_f const& pos, rect_f const& uv) {
        geometry::set_position(q, pos, s.XForm);
        geometry::set_color(q, colors::White);
        geometry::set_texcoords(q, {uv, level});

        for (auto& vert : q) {
            vert.Position[0] = std::floor(vert.Position[0] + 0.5f);
            vert.Position[1] = std::floor(vert.Position[1] + 0.5f);
        }
    }};

    std::array<quad, 9> quads {};

    setQuad(quads[0], rect_f::FromLTRB(left, top, leftCenter, topCenter), rect_f::FromLTRB(uv_left, uv_top, uv_leftCenter, uv_topCenter));                                 // top-left
    setQuad(quads[1], rect_f::FromLTRB(leftCenter, top, rightCenter, topCenter), rect_f::FromLTRB(uv_leftCenter, uv_top, uv_rightCenter, uv_topCenter));                   // top
    setQuad(quads[2], rect_f::FromLTRB(rightCenter, top, right, topCenter), rect_f::FromLTRB(uv_rightCenter, uv_top, uv_right, uv_topCenter));                             // top-right
    setQuad(quads[3], rect_f::FromLTRB(left, topCenter, leftCenter, bottomCenter), rect_f::FromLTRB(uv_left, uv_topCenter, uv_leftCenter, uv_bottomCenter));               // left
    setQuad(quads[4], rect_f::FromLTRB(leftCenter, topCenter, rightCenter, bottomCenter), rect_f::FromLTRB(uv_leftCenter, uv_topCenter, uv_rightCenter, uv_bottomCenter)); // center
    setQuad(quads[5], rect_f::FromLTRB(rightCenter, topCenter, right, bottomCenter), rect_f::FromLTRB(uv_rightCenter, uv_topCenter, uv_right, uv_bottomCenter));           // right
    setQuad(quads[6], rect_f::FromLTRB(left, bottomCenter, leftCenter, bottom), rect_f::FromLTRB(uv_left, uv_bottomCenter, uv_leftCenter, uv_bottom));                     // bottom-left
    setQuad(quads[7], rect_f::FromLTRB(leftCenter, bottomCenter, rightCenter, bottom), rect_f::FromLTRB(uv_leftCenter, uv_bottomCenter, uv_rightCenter, uv_bottom));       // bottom
    setQuad(quads[8], rect_f::FromLTRB(rightCenter, bottomCenter, right, bottom), rect_f::FromLTRB(uv_rightCenter, uv_bottomCenter, uv_right, uv_bottom));                 // bottom-right

    std::array<vertex, 9 * 6> const verts {quads[0][3], quads[0][1], quads[0][0], quads[0][3], quads[0][2], quads[0][1],
                                           quads[1][3], quads[1][1], quads[1][0], quads[1][3], quads[1][2], quads[1][1],
                                           quads[2][3], quads[2][1], quads[2][0], quads[2][3], quads[2][2], quads[2][1],
                                           quads[3][3], quads[3][1], quads[3][0], quads[3][3], quads[3][2], quads[3][1],
                                           quads[4][3], quads[4][1], quads[4][0], quads[4][3], quads[4][2], quads[4][1],
                                           quads[5][3], quads[5][1], quads[5][0], quads[5][3], quads[5][2], quads[5][1],
                                           quads[6][3], quads[6][1], quads[6][0], quads[6][3], quads[6][2], quads[6][1],
                                           quads[7][3], quads[7][1], quads[7][0], quads[7][3], quads[7][2], quads[7][1],
                                           quads[8][3], quads[8][1], quads[8][0], quads[8][3], quads[8][2], quads[8][1]};

    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, verts, _fringeWidth);
}

////////////////////////////////////////////////////////////

auto canvas::get_size() const -> size_i
{
    return _windowSize;
}

auto canvas::get_impl() const -> render_backend::canvas_base*
{
    return _impl.get();
}

void canvas::set_device_pixel_ratio(f32 ratio)
{
    _tessTol       = 0.25f / ratio;
    _distTol       = 0.01f / ratio;
    _fringeWidth   = 1.0f / ratio;
    _devicePxRatio = ratio;
}

////////////////////////////////////////////////////////////

auto canvas::get_state() -> state&
{
    return _states.top();
}

void canvas::set_paint_color(canvas_paint& p, color c)
{
    p.XForm   = transform::Identity;
    p.Extent  = {};
    p.Radius  = 0.0f;
    p.Feather = 1.0f;
    p.Color   = c;
    p.Image   = nullptr;
}

auto canvas::get_font_scale() -> f32
{
    return std::min(Quantize(GetAverageScale(get_state().XForm.Matrix), 0.01f), 4.0f);
}

void canvas::append_commands(std::span<f32 const> vals)
{
    state&      s {get_state()};
    usize const nvals {vals.size()};

    if (static_cast<i32>(vals[0]) != Close && static_cast<i32>(vals[0]) != Winding) {
        _commandPoint = {vals[nvals - 2], vals[nvals - 1]};
    }
    _commands.reserve(_commands.size() + nvals);

    // transform commands
    usize   i {0};
    point_f p;
    while (i < nvals) {
        i32 cmd {static_cast<i32>(vals[i])};
        _commands.push_back(static_cast<f32>(cmd));
        switch (cmd) {
        case MoveTo:
        case LineTo:
            p = s.XForm * point_f {vals[i + 1], vals[i + 2]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            i += 3;
            break;
        case BezierTo:
            p = s.XForm * point_f {vals[i + 1], vals[i + 2]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            p = s.XForm * point_f {vals[i + 3], vals[i + 4]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            p = s.XForm * point_f {vals[i + 5], vals[i + 6]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            i += 7;
            break;
        case Close:
            ++i;
            break;
        case Winding:
            _commands.push_back(vals[i + 1]);
            i += 2;
            break;
        default:
            ++i;
        }
    }
}

void canvas::clear_path_cache()
{
    _cache.points.clear();
    _cache.paths.clear();
}

auto canvas::get_last_path() -> canvas_path&
{
    if (_cache.paths.empty()) {
        _cache.paths.emplace_back();
    }

    return _cache.paths.back();
}

void canvas::add_path()
{
    canvas_path path;
    path.First = static_cast<i32>(_cache.points.size());
    _cache.paths.push_back(path);
}

auto canvas::get_last_point() -> detail::canvas_point&
{
    return _cache.points.back();
}

void canvas::add_point(f32 x, f32 y, i32 flags)
{
    canvas_path& path {get_last_path()};

    if (path.Count > 0 && !_cache.points.empty()) {
        auto& pt {get_last_point()};
        if (PointEquals(pt.X, pt.Y, x, y, _distTol)) {
            pt.Flags |= flags;
            return;
        }
    }

    detail::canvas_point pt;
    pt.X     = x;
    pt.Y     = y;
    pt.Flags = static_cast<ubyte>(flags);

    _cache.points.push_back(pt);
    path.Count++;
}

void canvas::close_last_path()
{
    canvas_path& path {get_last_path()};
    path.Closed = true;
}

auto canvas::alloc_temp_verts(usize nverts) -> vertex*
{
    if (nverts > _cache.verts.capacity()) {
        _cache.verts.reserve((nverts + 0xff) & ~0xff);
    }

    return _cache.verts.data();
}

void canvas::tesselate_bezier(f32 x1, f32 y1, f32 x2, f32 y2,
                              f32 x3, f32 y3, f32 x4, f32 y4,
                              i32 level, i32 type)
{
    if (level > 10) {
        return;
    }

    f32 const dx {x4 - x1};
    f32 const dy {y4 - y1};
    f32 const d2 {std::abs(((x2 - x4) * dy - (y2 - y4) * dx))};
    f32 const d3 {std::abs(((x3 - x4) * dy - (y3 - y4) * dx))};

    if ((d2 + d3) * (d2 + d3) < _tessTol * (dx * dx + dy * dy)) {
        add_point(x4, y4, type);
        return;
    }

    f32 const x12 {(x1 + x2) * 0.5f};
    f32 const y12 {(y1 + y2) * 0.5f};
    f32 const x23 {(x2 + x3) * 0.5f};
    f32 const y23 {(y2 + y3) * 0.5f};
    f32 const x34 {(x3 + x4) * 0.5f};
    f32 const y34 {(y3 + y4) * 0.5f};
    f32 const x123 {(x12 + x23) * 0.5f};
    f32 const y123 {(y12 + y23) * 0.5f};
    f32 const x234 {(x23 + x34) * 0.5f};
    f32 const y234 {(y23 + y34) * 0.5f};
    f32 const x1234 {(x123 + x234) * 0.5f};
    f32 const y1234 {(y123 + y234) * 0.5f};

    tesselate_bezier(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
    tesselate_bezier(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);
}

void canvas::flatten_paths()
{
    if (!_cache.paths.empty()) {
        return;
    }

    // Flatten
    for (usize i {0}; i < _commands.size();) {
        i32 const cmd {static_cast<i32>(_commands[i])};
        switch (cmd) {
        case MoveTo: {
            add_path();
            f32 const* p {&_commands[i + 1]};
            add_point(p[0], p[1], Corner);
            i += 3;
        } break;
        case LineTo: {
            f32 const* p {&_commands[i + 1]};
            add_point(p[0], p[1], Corner);
            i += 3;
        } break;
        case BezierTo: {
            auto const& last {get_last_point()};
            f32 const*  cp1 {&_commands[i + 1]};
            f32 const*  cp2 {&_commands[i + 3]};
            f32 const*  p {&_commands[i + 5]};
            tesselate_bezier(last.X, last.Y, cp1[0], cp1[1], cp2[0], cp2[1], p[0], p[1], 0, Corner);
            i += 7;
        } break;
        case Close: {
            close_last_path();
            ++i;
        } break;
        case Winding: {
            canvas_path& path {get_last_path()};
            path.Winding = static_cast<winding>(_commands[i + 1]);
            i += 2;
        } break;
        default:
            ++i;
        }
    }

    _cache.bounds[0] = _cache.bounds[1] = 1e6f;
    _cache.bounds[2] = _cache.bounds[3] = -1e6f;

    // Calculate the direction and length of line segments.
    for (auto& path : _cache.paths) {
        canvas_point* pts {&_cache.points[path.First]};

        // If the first and last points are the same, remove the last, mark as closed path.
        canvas_point* p0 {&pts[path.Count - 1]};
        canvas_point* p1 {&pts[0]};

        if (PointEquals(p0->X, p0->Y, p1->X, p1->Y, _distTol)) {
            path.Count--;
            p0          = &pts[path.Count - 1];
            path.Closed = true;
        }

        // Enforce winding.
        if (_enforceWinding && path.Count > 2) {
            f32 const area {PolyArea({pts, path.Count})};
            if (path.Winding == winding::CCW && area < 0.0f) {
                PolyReverse({pts, path.Count});
            }
            if (path.Winding == winding::CW && area > 0.0f) {
                PolyReverse({pts, path.Count});
            }
        }

        for (usize i {0}; i < path.Count; ++i) {
            // Calculate segment direction and length
            p0->DX           = p1->X - p0->X;
            p0->DY           = p1->Y - p0->Y;
            p0->Length       = Normalize(p0->DX, p0->DY);
            // Update bounds
            _cache.bounds[0] = std::min(_cache.bounds[0], p0->X);
            _cache.bounds[1] = std::min(_cache.bounds[1], p0->Y);
            _cache.bounds[2] = std::max(_cache.bounds[2], p0->X);
            _cache.bounds[3] = std::max(_cache.bounds[3], p0->Y);
            // Advance
            p0               = p1++;
        }
    }
}

void canvas::calculate_joins(f32 w, line_join lineJoin, f32 miterLimit)
{
    f32 iw {0.0f};

    if (w > 0.0f) {
        iw = 1.0f / w;
    }

    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (auto& path : _cache.paths) {
        canvas_point* pts {&_cache.points[path.First]};
        canvas_point* p0 {&pts[path.Count - 1]};
        canvas_point* p1 {&pts[0]};
        usize         nleft {0};

        path.BevelCount = 0;

        for (usize j {0}; j < path.Count; j++) {
            f32 const dlx0 {p0->DY};
            f32 const dly0 {-p0->DX};
            f32 const dlx1 {p1->DY};
            f32 const dly1 {-p1->DX};
            // Calculate extrusions
            p1->DMX = (dlx0 + dlx1) * 0.5f;
            p1->DMY = (dly0 + dly1) * 0.5f;
            f32 const dmr2 {p1->DMX * p1->DMX + p1->DMY * p1->DMY};
            if (dmr2 > 0.000001f) {
                f32 scale {1.0f / dmr2};
                if (scale > 600.0f) {
                    scale = 600.0f;
                }
                p1->DMX *= scale;
                p1->DMY *= scale;
            }

            // Clear flags, but keep the corner.
            p1->Flags = (p1->Flags & Corner) ? Corner : 0;

            // Keep track of left turns.
            f32 const cross {p1->DX * p0->DY - p0->DX * p1->DY};
            if (cross > 0.0f) {
                nleft++;
                p1->Flags |= Left;
            }

            // Calculate if we should use bevel or miter for inner join.
            f32 const limit {std::max(1.01f, std::min(p0->Length, p1->Length) * iw)};
            if ((dmr2 * limit * limit) < 1.0f) {
                p1->Flags |= InnerBevel;
            }

            // Check to see if the corner needs to be beveled.
            if (p1->Flags & Corner) {
                if ((dmr2 * miterLimit * miterLimit) < 1.0f || lineJoin == line_join::Bevel || lineJoin == line_join::Round) {
                    p1->Flags |= Bevel;
                }
            }

            if ((p1->Flags & (Bevel | InnerBevel)) != 0) {
                path.BevelCount++;
            }

            p0 = p1++;
        }

        path.Convex = (nleft == path.Count);
    }
}

void canvas::expand_stroke(f32 w, f32 fringe, line_cap lineCap, line_join lineJoin, f32 miterLimit)
{
    f32 const aa {fringe};                              // fringeWidth;
    f32 const u0 {aa == 0.0f ? 0.5f : 0.0f};
    f32 const u1 {aa == 0.0f ? 0.5f : 1.0f};
    i32 const ncap {CurveDivs(w, TAU_F / 2, _tessTol)}; // Calculate divisions per half circle.

    w += aa * 0.5f;

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    usize cverts {0};
    for (auto& path : _cache.paths) {
        if (lineJoin == line_join::Round) {
            cverts += (path.Count + path.BevelCount * (ncap + 2) + 1) * 2; // plus one for loop}
        } else {
            cverts += (path.Count + path.BevelCount * 5 + 1) * 2;          // plus one for loop}
        }
        if (!path.Closed) {
            // space for caps
            if (lineCap == line_cap::Round) {
                cverts += (ncap * 2 + 2) * 2;
            } else {
                cverts += (3 + 3) * 2;
            }
        }
    }

    vertex* verts {alloc_temp_verts(cverts)};
    if (verts == nullptr) {
        return;
    }

    vertex* dst {nullptr};
    for (auto& path : _cache.paths) {
        canvas_point* pts {&_cache.points[path.First]};
        canvas_point* p0 {nullptr};
        canvas_point* p1 {nullptr};
        usize         s {0}, e {0};

        path.Fill      = nullptr;
        path.FillCount = 0;

        // Calculate fringe or stroke
        dst         = verts;
        path.Stroke = dst;

        if (path.Closed) {
            // Looping
            p0 = &pts[path.Count - 1];
            p1 = &pts[0];
            s  = 0;
            e  = path.Count;
        } else {
            // Add cap
            p0 = &pts[0];
            p1 = &pts[1];
            s  = 1;
            e  = path.Count - 1;
        }

        if (!path.Closed) {
            // Add cap
            f32 dx {p1->X - p0->X};
            f32 dy {p1->Y - p0->Y};
            Normalize(dx, dy);
            if (lineCap == line_cap::Butt) {
                dst = ButtCapStart(dst, *p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            } else if (lineCap == line_cap::Square) {
                dst = ButtCapStart(dst, *p0, dx, dy, w, w - aa, aa, u0, u1);
            } else if (lineCap == line_cap::Round) {
                dst = RoundCapStart(dst, *p0, dx, dy, w, ncap, u0, u1);
            }
        }

        for (usize j {s}; j < e; ++j) {
            if ((p1->Flags & (Bevel | InnerBevel)) != 0) {
                if (lineJoin == line_join::Round) {
                    dst = RoundJoin(dst, *p0, *p1, w, w, u0, u1, ncap);
                } else {
                    dst = BevelJoin(dst, *p0, *p1, w, w, u0, u1);
                }
            } else {
                SetVertex(dst, p1->X + (p1->DMX * w), p1->Y + (p1->DMY * w), u0, 1);
                dst++;
                SetVertex(dst, p1->X - (p1->DMX * w), p1->Y - (p1->DMY * w), u1, 1);
                dst++;
            }
            p0 = p1++;
        }

        if (path.Closed) {
            // Loop it
            SetVertex(dst, verts[0].Position[0], verts[0].Position[1], u0, 1);
            dst++;
            SetVertex(dst, verts[1].Position[0], verts[1].Position[1], u1, 1);
            dst++;
        } else {
            // Add cap
            f32 dx {p1->X - p0->X};
            f32 dy {p1->Y - p0->Y};
            Normalize(dx, dy);
            if (lineCap == line_cap::Butt) {
                dst = ButtCapEnd(dst, *p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            } else if (lineCap == line_cap::Square) {
                dst = ButtCapEnd(dst, *p1, dx, dy, w, w - aa, aa, u0, u1);
            } else if (lineCap == line_cap::Round) {
                dst = RoundCapEnd(dst, *p1, dx, dy, w, ncap, u0, u1);
            }
        }

        path.StrokeCount = dst - verts;

        verts = dst;
    }
}

void canvas::expand_fill(f32 w, line_join lineJoin, f32 miterLimit)
{
    vertex*    dst {nullptr};
    f32 const  aa {_fringeWidth};
    bool const fringe {w > 0.0f};
    f32 const  woff {0.5f * aa};

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    usize cverts {0};
    for (auto& path : _cache.paths) {
        cverts += path.Count + path.BevelCount + 1;
        if (fringe) {
            cverts += (path.Count + path.BevelCount * 5 + 1) * 2; // plus one for loop
        }
    }

    vertex* verts {alloc_temp_verts(cverts)};
    if (verts == nullptr) {
        return;
    }

    bool const convex {_cache.paths.size() == 1 && _cache.paths[0].Convex};

    for (auto& path : _cache.paths) {
        canvas_point* pts {&_cache.points[path.First]};
        canvas_point* p0 {nullptr};
        canvas_point* p1 {nullptr};

        // Calculate shape vertices.
        dst       = verts;
        path.Fill = dst;

        if (fringe) {
            // Looping
            p0 = &pts[path.Count - 1];
            p1 = &pts[0];
            for (u32 j {0}; j < path.Count; ++j) { //|here
                if (p1->Flags & Bevel) {
                    f32 const dlx0 {p0->DY};
                    f32 const dly0 {-p0->DX};
                    f32 const dlx1 {p1->DY};
                    f32 const dly1 {-p1->DX};
                    if (p1->Flags & Left) {
                        f32 const lx {p1->X + p1->DMX * woff};
                        f32 const ly {p1->Y + p1->DMY * woff};
                        SetVertex(dst, lx, ly, 0.5f, 1);
                        dst++;
                    } else {
                        f32 const lx0 {p1->X + dlx0 * woff};
                        f32 const ly0 {p1->Y + dly0 * woff};
                        f32 const lx1 {p1->X + dlx1 * woff};
                        f32 const ly1 {p1->Y + dly1 * woff};
                        SetVertex(dst, lx0, ly0, 0.5f, 1);
                        dst++;
                        SetVertex(dst, lx1, ly1, 0.5f, 1);
                        dst++;
                    }
                } else {
                    SetVertex(dst, p1->X + (p1->DMX * woff), p1->Y + (p1->DMY * woff), 0.5f, 1);
                    dst++;
                }
                p0 = p1++;
            }
        } else {
            for (u32 j {0}; j < path.Count; ++j) {
                SetVertex(dst, pts[j].X, pts[j].Y, 0.5f, 1);
                dst++;
            }
        }

        path.FillCount = dst - verts;
        verts          = dst;

        // Calculate fringe
        if (fringe) {
            f32 const lw {convex ? woff : w + woff};
            f32 const rw {w - woff};
            f32 const lu {convex ? 0.5f : 0};
            f32 const ru {1};
            dst         = verts;
            path.Stroke = dst;

            // Looping
            p0 = &pts[path.Count - 1];
            p1 = &pts[0];

            for (u32 j {0}; j < path.Count; ++j) {
                if ((p1->Flags & (Bevel | InnerBevel)) != 0) {
                    dst = BevelJoin(dst, *p0, *p1, lw, rw, lu, ru);
                } else {
                    SetVertex(dst, p1->X + (p1->DMX * lw), p1->Y + (p1->DMY * lw), lu, 1);
                    dst++;
                    SetVertex(dst, p1->X - (p1->DMX * rw), p1->Y - (p1->DMY * rw), ru, 1);
                    dst++;
                }
                p0 = p1++;
            }

            // Loop it
            SetVertex(dst, verts[0].Position[0], verts[0].Position[1], lu, 1);
            dst++;
            SetVertex(dst, verts[1].Position[0], verts[1].Position[1], ru, 1);
            dst++;

            path.StrokeCount = dst - verts;
            verts            = dst;
        } else {
            path.Stroke      = nullptr;
            path.StrokeCount = 0;
        }
    }
}

void canvas::render_text(font* font, std::span<vertex const> verts)
{
    state&       s {get_state()};
    canvas_paint paint {s.Fill};

    // Render triangles.
    paint.Image = font->get_texture().get_obj();

    // Apply global alpha
    multiply_alpha_paint(paint.Color, s.Alpha);

    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, verts, _fringeWidth);
}

canvas::state_guard::state_guard(canvas* c)
    : _canvas {c}
{
    c->save();
}

canvas::state_guard::state_guard::~state_guard()
{
    _canvas->restore();
}

////////////////////////////////////////////////////////////

auto canvas::path2d::Parse(string_view path) -> std::optional<path2d>
{
    auto static const getPoint {[](std::optional<std::vector<f32>> const& values) -> point_f { return {(*values)[0], (*values)[1]}; }};

    auto const commands {GetCommands(path)};
    if (!commands) { return std::nullopt; }

    path2d retValue;

    point_f lastPoint;
    point_f lastQuadControlPoint;
    point_f lastCubicControlPoint;
    retValue.Commands.emplace_back([](canvas& canvas) { canvas.begin_path(); });

    char command {0};
    for (usize idx {0}; idx < commands->size();) {
        auto const getValues {[&idx, &commands](usize size) -> std::optional<std::vector<f32>> {
            std::vector<f32> values;
            values.resize(size);
            for (usize i {0}; i < size; ++i) {
                if (idx >= commands->size()) { return std::nullopt; }

                if (auto const* c {std::get_if<f32>(&commands->at(idx++))}) {
                    values[i] = *c;
                } else {
                    return std::nullopt;
                }
            }

            return values;
        }};

        if (auto const* c {std::get_if<char>(&commands->at(idx))}) {
            command = *c;
            idx++;
        }
        bool const isAbs {std::isupper(command) != 0};

        switch (command) {
        case 'm':
        case 'M': {
            auto const p {getValues(2)};
            if (!p) { return std::nullopt; }
            lastPoint = isAbs ? getPoint(p) : getPoint(p) + lastPoint;
            retValue.Commands.emplace_back([lastPoint](canvas& canvas) { canvas.move_to(lastPoint); });
            command = isAbs ? 'L' : 'l';
        } break;
        case 'l':
        case 'L': {
            auto const p {getValues(2)};
            if (!p) { return std::nullopt; }
            lastPoint = isAbs ? getPoint(p) : getPoint(p) + lastPoint;
            retValue.Commands.emplace_back([lastPoint](canvas& canvas) { canvas.line_to(lastPoint); });
        } break;
        case 'h':
        case 'H': {
            auto const p {getValues(1)};
            if (!p) { return std::nullopt; }
            lastPoint = isAbs ? point_f {(*p)[0], lastPoint.Y} : point_f {(*p)[0] + lastPoint.X, lastPoint.Y};
            retValue.Commands.emplace_back([lastPoint](canvas& canvas) { canvas.line_to(lastPoint); });
        } break;
        case 'v':
        case 'V': {
            auto const p {getValues(1)};
            if (!p) { return std::nullopt; }
            lastPoint = isAbs ? point_f {lastPoint.X, (*p)[0]} : point_f {lastPoint.X, (*p)[0] + lastPoint.Y};
            retValue.Commands.emplace_back([lastPoint](canvas& canvas) { canvas.line_to(lastPoint); });
        } break;
        case 'q':
        case 'Q': {
            auto const cpv {getValues(2)};
            auto const endv {getValues(2)};
            if (!cpv || !endv) { return std::nullopt; }
            point_f const cp {isAbs ? getPoint(cpv) : getPoint(cpv) + lastPoint};
            point_f const end {isAbs ? getPoint(endv) : getPoint(endv) + lastPoint};
            retValue.Commands.emplace_back([cp, end](canvas& canvas) { canvas.quad_bezier_to(cp, end); });
            lastQuadControlPoint = cp;
            lastPoint            = end;
        } break;
        case 't':
        case 'T': {
            auto const endv {getValues(2)};
            if (!endv) { return std::nullopt; }
            point_f const end {isAbs ? getPoint(endv) : getPoint(endv) + lastPoint};
            point_f const cp {2 * lastPoint.X - lastQuadControlPoint.X, 2 * lastPoint.Y - lastQuadControlPoint.Y};
            retValue.Commands.emplace_back([cp, end](canvas& canvas) { canvas.quad_bezier_to(cp, end); });
            lastQuadControlPoint = cp;
            lastPoint            = end;
        } break;
        case 'c':
        case 'C': {
            auto const cp0v {getValues(2)};
            auto const cp1v {getValues(2)};
            auto const endv {getValues(2)};
            if (!cp0v || !cp1v || !endv) { return std::nullopt; }
            auto const cp0 {isAbs ? getPoint(cp0v) : getPoint(cp0v) + lastPoint};
            auto const cp1 {isAbs ? getPoint(cp1v) : getPoint(cp1v) + lastPoint};
            auto const end {isAbs ? getPoint(endv) : getPoint(endv) + lastPoint};
            retValue.Commands.emplace_back([cp0, cp1, end](canvas& canvas) { canvas.cubic_bezier_to(cp0, cp1, end); });
            lastCubicControlPoint = cp1;
            lastPoint             = end;
        } break;
        case 's':
        case 'S': {
            auto const cp1v {getValues(2)};
            auto const endv {getValues(2)};
            if (!cp1v || !endv) { return std::nullopt; }
            point_f const cp0 {2 * lastPoint.X - lastCubicControlPoint.X, 2 * lastPoint.Y - lastCubicControlPoint.Y};
            auto const    cp1 {isAbs ? getPoint(cp1v) : getPoint(cp1v) + lastPoint};
            auto const    end {isAbs ? getPoint(endv) : getPoint(endv) + lastPoint};
            retValue.Commands.emplace_back([cp0, cp1, end](canvas& canvas) { canvas.cubic_bezier_to(cp0, cp1, end); });
            lastCubicControlPoint = cp1;
            lastPoint             = end;
        } break;
        case 'a':
        case 'A': {
            auto const valv {getValues(7)};
            if (!valv) { return std::nullopt; }
            auto const& val {*valv};
            retValue.Commands.emplace_back([lastPoint, val, isAbs](canvas& canvas) { canvas.path_arc_to(lastPoint.X, lastPoint.Y, val, !isAbs); });
            lastPoint = isAbs ? point_f {val[5], val[6]} : point_f {val[5], val[6]} + lastPoint;
        } break;
        case 'z':
        case 'Z':
            retValue.Commands.emplace_back([](canvas& canvas) { canvas.close_path(); });
            break;
        default:
            return std::nullopt;
        }
    }

    return retValue;
}

auto canvas::path2d::GetCommands(string_view path) -> std::optional<std::vector<std::variant<char, f32>>>
{
    std::vector<std::variant<char, f32>> commands;
    string                               valStr;

    auto const getFloat {[&valStr, &commands]() -> bool {
        if (!valStr.empty()) {
            if (auto value {helper::to_number<f32>(valStr)}) {
                commands.emplace_back(*value);
            } else {
                return false;
            }
            valStr = "";
        }
        return true;
    }};

    for (auto const c : path) {
        if (c == ' ' || c == ',') {
            if (!getFloat()) { return std::nullopt; }
        } else if (std::isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'e') {
            valStr += c;
        } else {
            commands.emplace_back(c);
        }
    }
    if (!getFloat()) { return std::nullopt; }

    return commands;
}
}

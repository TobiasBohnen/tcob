// Copyright (c) 2021 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <tcob/gfx/Canvas.hpp>

#include "gl/nanovg/GLNVGcontext.hpp"

#include <tcob/core/data/Transform.hpp>
#include <tcob/core/io/FileStream.hpp>
#include <tcob/gfx/TextFormatter.hpp>

namespace tcob {
using namespace detail;

constexpr auto NVG_KAPPA90 = 0.5522847493f; // Length proportional to radius of a cubic bezier handle for 90deg arcs.;

enum NVGcommands {
    NVG_MOVETO = 0,
    NVG_LINETO = 1,
    NVG_BEZIERTO = 2,
    NVG_CLOSE = 3,
    NVG_WINDING = 4,
};

enum NVGpointFlags {
    NVG_PT_CORNER = 0x01,
    NVG_PT_LEFT = 0x02,
    NVG_PT_BEVEL = 0x04,
    NVG_PR_INNERBEVEL = 0x08,
};

static constexpr auto nvg__signf(f32 a) -> f32 { return a >= 0.0f ? 1.0f : -1.0f; }

static constexpr auto nvg__cross(f32 dx0, f32 dy0, f32 dx1, f32 dy1) -> f32 { return dx1 * dy0 - dx0 * dy1; }

static auto nvg__normalize(f32& x, f32& y) -> f32
{
    f32 d { std::sqrt(x * x + y * y) };
    if (d > 1e-6f) {
        f32 id { 1.0f / d };
        x *= id;
        y *= id;
    }
    return d;
}

static constexpr auto nvg__compositeOperationState(CompositeOperation op) -> gl::BlendFuncs
{
    gl::BlendFunc sfactor, dfactor;

    switch (op) {
    case CompositeOperation::SourceOver:
        sfactor = gl::BlendFunc::One;
        dfactor = gl::BlendFunc::OneMinusSrcAlpha;
        break;
    case CompositeOperation::SourceIn:
        sfactor = gl::BlendFunc::DstAlpha;
        dfactor = gl::BlendFunc::Zero;
        break;
    case CompositeOperation::SourceOut:
        sfactor = gl::BlendFunc::OneMinusDstAlpha;
        dfactor = gl::BlendFunc::Zero;
        break;
    case CompositeOperation::Atop:
        sfactor = gl::BlendFunc::DstAlpha;
        dfactor = gl::BlendFunc::OneMinusSrcAlpha;
        break;
    case CompositeOperation::DestinationOver:
        sfactor = gl::BlendFunc::OneMinusDstAlpha;
        dfactor = gl::BlendFunc::One;
        break;
    case CompositeOperation::DestinationIn:
        sfactor = gl::BlendFunc::Zero;
        dfactor = gl::BlendFunc::SrcAlpha;
        break;
    case CompositeOperation::DestinationOut:
        sfactor = gl::BlendFunc::Zero;
        dfactor = gl::BlendFunc::OneMinusSrcAlpha;
        break;
    case CompositeOperation::DestinationAtop:
        sfactor = gl::BlendFunc::OneMinusDstAlpha;
        dfactor = gl::BlendFunc::SrcAlpha;
        break;
    case CompositeOperation::Lighter:
        sfactor = gl::BlendFunc::One;
        dfactor = gl::BlendFunc::One;
        break;
    case CompositeOperation::Copy:
        sfactor = gl::BlendFunc::One;
        dfactor = gl::BlendFunc::Zero;
        break;
    case CompositeOperation::Xor:
        sfactor = gl::BlendFunc::OneMinusDstAlpha;
        dfactor = gl::BlendFunc::OneMinusSrcAlpha;
        break;
    default:
        sfactor = gl::BlendFunc::One;
        dfactor = gl::BlendFunc::Zero;
        break;
    }

    return {
        .SourceColorBlendFunc = sfactor,
        .DestinationColorBlendFunc = dfactor,
        .SourceAlphaBlendFunc = sfactor,
        .DestinationAlphaBlendFunc = dfactor
    };
}

static constexpr auto nvg__ptEquals(f32 x1, f32 y1, f32 x2, f32 y2, f32 tol) -> i32
{
    f32 dx { x2 - x1 };
    f32 dy { y2 - y1 };
    return dx * dx + dy * dy < tol * tol;
}

static constexpr auto nvg__distPtSeg(f32 x, f32 y, f32 px, f32 py, f32 qx, f32 qy) -> f32
{
    f32 pqx { qx - px };
    f32 pqy { qy - py };
    f32 dx { x - px };
    f32 dy { y - py };
    f32 d { pqx * pqx + pqy * pqy };
    f32 t { pqx * dx + pqy * dy };
    if (d > 0)
        t /= d;
    if (t < 0)
        t = 0;
    else if (t > 1)
        t = 1;
    dx = px + t * pqx - x;
    dy = py + t * pqy - y;
    return dx * dx + dy * dy;
}

static auto nvg__getAverageScale(const mat3& t) -> f32
{
    const f32 sx { std::sqrt(t[0] * t[0] + t[3] * t[3]) };
    const f32 sy { std::sqrt(t[1] * t[1] + t[4] * t[4]) };
    return (sx + sy) * 0.5f;
}

static constexpr auto nvg__triarea2(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) -> f32
{
    const f32 abx { bx - ax };
    const f32 aby { by - ay };
    const f32 acx { cx - ax };
    const f32 acy { cy - ay };
    return acx * aby - abx * acy;
}

static constexpr auto nvg__polyArea(const NVGpoint* pts, isize npts) -> f32
{
    f32 area { 0 };
    for (isize i { 2 }; i < npts; ++i) {
        const NVGpoint* a { &pts[0] };
        const NVGpoint* b { &pts[i - 1] };
        const NVGpoint* c { &pts[i] };
        area += nvg__triarea2(a->x, a->y, b->x, b->y, c->x, c->y);
    }
    return area * 0.5f;
}

static void nvg__polyReverse(NVGpoint* pts, isize npts)
{
    NVGpoint tmp;
    isize i { 0 }, j { npts - 1 };
    while (i < j) {
        tmp = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        ++i;
        --j;
    }
}

static void nvg__vset(Vertex* vtx, f32 x, f32 y, f32 u, f32 v)
{
    vtx->Position = { x, y };
    vtx->TexCoords = { u, v, 0 };
}

static auto nvg__curveDivs(f32 r, f32 arc, f32 tol) -> i32
{
    const f32 da { std::acos(r / (r + tol)) * 2.0f };
    return std::max(2, static_cast<i32>(std::ceil(arc / da)));
}

static constexpr void nvg__chooseBevel(i32 bevel, const NVGpoint& p0, const NVGpoint& p1, f32 w,
    f32& x0, f32& y0, f32& x1, f32& y1)
{
    if (bevel) {
        x0 = p1.x + p0.dy * w;
        y0 = p1.y - p0.dx * w;
        x1 = p1.x + p1.dy * w;
        y1 = p1.y - p1.dx * w;
    } else {
        x0 = p1.x + p1.dmx * w;
        y0 = p1.y + p1.dmy * w;
        x1 = p1.x + p1.dmx * w;
        y1 = p1.y + p1.dmy * w;
    }
}

static auto nvg__roundJoin(Vertex* dst, const NVGpoint& p0, const NVGpoint& p1,
    f32 lw, f32 rw, f32 lu, f32 ru, i32 ncap) -> Vertex*
{
    f32 dlx0 { p0.dy };
    f32 dly0 { -p0.dx };
    f32 dlx1 { p1.dy };
    f32 dly1 { -p1.dx };

    if (p1.flags & NVG_PT_LEFT) {
        f32 lx0 { 0 }, ly0 { 0 }, lx1 { 0 }, ly1 { 0 }, a0 { 0 }, a1 { 0 };
        nvg__chooseBevel(p1.flags & NVG_PR_INNERBEVEL, p0, p1, lw, lx0, ly0, lx1, ly1);
        a0 = std::atan2(-dly0, -dlx0);
        a1 = std::atan2(-dly1, -dlx1);
        if (a1 > a0)
            a1 -= TAU_F;

        nvg__vset(dst, lx0, ly0, lu, 1);
        dst++;
        nvg__vset(dst, p1.x - dlx0 * rw, p1.y - dly0 * rw, ru, 1);
        dst++;

        i32 n { std::clamp(static_cast<i32>(ceilf(((a0 - a1) / (TAU_F / 2)) * ncap)), 2, ncap) };
        for (i32 i { 0 }; i < n; ++i) {
            f32 u = i / (f32)(n - 1);
            f32 a = a0 + u * (a1 - a0);
            f32 rx = p1.x + std::cos(a) * rw;
            f32 ry = p1.y + std::sin(a) * rw;
            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;
            nvg__vset(dst, rx, ry, ru, 1);
            dst++;
        }

        nvg__vset(dst, lx1, ly1, lu, 1);
        dst++;
        nvg__vset(dst, p1.x - dlx1 * rw, p1.y - dly1 * rw, ru, 1);
        dst++;
    } else {
        f32 rx0 { 0 }, ry0 { 0 }, rx1 { 0 }, ry1 { 0 }, a0 { 0 }, a1 { 0 };
        nvg__chooseBevel(p1.flags & NVG_PR_INNERBEVEL, p0, p1, -rw, rx0, ry0, rx1, ry1);
        a0 = std::atan2(dly0, dlx0);
        a1 = std::atan2(dly1, dlx1);
        if (a1 < a0)
            a1 += TAU_F;

        nvg__vset(dst, p1.x + dlx0 * rw, p1.y + dly0 * rw, lu, 1);
        dst++;
        nvg__vset(dst, rx0, ry0, ru, 1);
        dst++;

        i32 n { std::clamp((int)std::ceil(((a1 - a0) / (TAU_F / 2)) * ncap), 2, ncap) };
        for (i32 i { 0 }; i < n; ++i) {
            f32 u { i / (f32)(n - 1) };
            f32 a { a0 + u * (a1 - a0) };
            f32 lx { p1.x + std::cos(a) * lw };
            f32 ly { p1.y + std::sin(a) * lw };
            nvg__vset(dst, lx, ly, lu, 1);
            dst++;
            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;
        }

        nvg__vset(dst, p1.x + dlx1 * rw, p1.y + dly1 * rw, lu, 1);
        dst++;
        nvg__vset(dst, rx1, ry1, ru, 1);
        dst++;
    }
    return dst;
}

static auto nvg__bevelJoin(Vertex* dst, const NVGpoint& p0, const NVGpoint& p1,
    f32 lw, f32 rw, f32 lu, f32 ru) -> Vertex*
{
    f32 dlx0 { p0.dy };
    f32 dly0 { -p0.dx };
    f32 dlx1 { p1.dy };
    f32 dly1 { -p1.dx };

    if (p1.flags & NVG_PT_LEFT) {
        f32 lx0 { 0 }, ly0 { 0 }, lx1 { 0 }, ly1 { 0 };
        nvg__chooseBevel(p1.flags & NVG_PR_INNERBEVEL, p0, p1, lw, lx0, ly0, lx1, ly1);

        nvg__vset(dst, lx0, ly0, lu, 1);
        dst++;
        nvg__vset(dst, p1.x - dlx0 * rw, p1.y - dly0 * rw, ru, 1);
        dst++;

        if (p1.flags & NVG_PT_BEVEL) {
            nvg__vset(dst, lx0, ly0, lu, 1);
            dst++;
            nvg__vset(dst, p1.x - dlx0 * rw, p1.y - dly0 * rw, ru, 1);
            dst++;

            nvg__vset(dst, lx1, ly1, lu, 1);
            dst++;
            nvg__vset(dst, p1.x - dlx1 * rw, p1.y - dly1 * rw, ru, 1);
            dst++;
        } else {
            f32 rx0 { p1.x - p1.dmx * rw };
            f32 ry0 { p1.y - p1.dmy * rw };

            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;
            nvg__vset(dst, p1.x - dlx0 * rw, p1.y - dly0 * rw, ru, 1);
            dst++;

            nvg__vset(dst, rx0, ry0, ru, 1);
            dst++;
            nvg__vset(dst, rx0, ry0, ru, 1);
            dst++;

            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;
            nvg__vset(dst, p1.x - dlx1 * rw, p1.y - dly1 * rw, ru, 1);
            dst++;
        }

        nvg__vset(dst, lx1, ly1, lu, 1);
        dst++;
        nvg__vset(dst, p1.x - dlx1 * rw, p1.y - dly1 * rw, ru, 1);
        dst++;
    } else {
        f32 rx0 { 0 }, ry0 { 0 }, rx1 { 0 }, ry1 { 0 };
        nvg__chooseBevel(p1.flags & NVG_PR_INNERBEVEL, p0, p1, -rw, rx0, ry0, rx1, ry1);

        nvg__vset(dst, p1.x + dlx0 * lw, p1.y + dly0 * lw, lu, 1);
        dst++;
        nvg__vset(dst, rx0, ry0, ru, 1);
        dst++;

        if (p1.flags & NVG_PT_BEVEL) {
            nvg__vset(dst, p1.x + dlx0 * lw, p1.y + dly0 * lw, lu, 1);
            dst++;
            nvg__vset(dst, rx0, ry0, ru, 1);
            dst++;

            nvg__vset(dst, p1.x + dlx1 * lw, p1.y + dly1 * lw, lu, 1);
            dst++;
            nvg__vset(dst, rx1, ry1, ru, 1);
            dst++;
        } else {
            f32 lx0 { p1.x + p1.dmx * lw };
            f32 ly0 { p1.y + p1.dmy * lw };

            nvg__vset(dst, p1.x + dlx0 * lw, p1.y + dly0 * lw, lu, 1);
            dst++;
            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;

            nvg__vset(dst, lx0, ly0, lu, 1);
            dst++;
            nvg__vset(dst, lx0, ly0, lu, 1);
            dst++;

            nvg__vset(dst, p1.x + dlx1 * lw, p1.y + dly1 * lw, lu, 1);
            dst++;
            nvg__vset(dst, p1.x, p1.y, 0.5f, 1);
            dst++;
        }

        nvg__vset(dst, p1.x + dlx1 * lw, p1.y + dly1 * lw, lu, 1);
        dst++;
        nvg__vset(dst, rx1, ry1, ru, 1);
        dst++;
    }

    return dst;
}

static auto nvg__buttCapStart(Vertex* dst, const NVGpoint& p,
    f32 dx, f32 dy, f32 w, f32 d,
    f32 aa, f32 u0, f32 u1) -> Vertex*
{
    f32 px { p.x - dx * d };
    f32 py { p.y - dy * d };
    f32 dlx { dy };
    f32 dly { -dx };
    nvg__vset(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);
    dst++;
    nvg__vset(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);
    dst++;
    nvg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    nvg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    return dst;
}

static auto nvg__buttCapEnd(Vertex* dst, const NVGpoint& p,
    f32 dx, f32 dy, f32 w, f32 d,
    f32 aa, f32 u0, f32 u1) -> Vertex*
{
    f32 px { p.x + dx * d };
    f32 py { p.y + dy * d };
    f32 dlx { dy };
    f32 dly { -dx };
    nvg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    nvg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    nvg__vset(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);
    dst++;
    nvg__vset(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);
    dst++;
    return dst;
}

static auto nvg__roundCapStart(Vertex* dst, const NVGpoint& p,
    f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> Vertex*
{
    f32 px { p.x };
    f32 py { p.y };
    f32 dlx { dy };
    f32 dly { -dx };

    for (i32 i { 0 }; i < ncap; ++i) {
        f32 a { i / (f32)(ncap - 1) * (TAU_F / 2) };
        f32 ax { std::cos(a) * w }, ay { std::sin(a) * w };
        nvg__vset(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);
        dst++;
        nvg__vset(dst, px, py, 0.5f, 1);
        dst++;
    }
    nvg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    nvg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    return dst;
}

static auto nvg__roundCapEnd(Vertex* dst, const NVGpoint& p,
    f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> Vertex*
{
    f32 px { p.x };
    f32 py { p.y };
    f32 dlx { dy };
    f32 dly { -dx };

    nvg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
    dst++;
    nvg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
    dst++;
    for (i32 i { 0 }; i < ncap; ++i) {
        f32 a { i / (f32)(ncap - 1) * (TAU_F / 2) };
        f32 ax { std::cos(a) * w }, ay { std::sin(a) * w };
        nvg__vset(dst, px, py, 0.5f, 1);
        dst++;
        nvg__vset(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
        dst++;
    }
    return dst;
}

static auto nvg__quantize(f32 a, f32 d) -> f32
{
    return (static_cast<i32>(a / d + 0.5f)) * d;
}

static auto nvg__getFontScale(NVGstate& state) -> f32
{
    return std::min(nvg__quantize(nvg__getAverageScale(state.xform.matrix3()), 0.01f), 4.0f);
}

////////////////////////////////////////////////////////////

Canvas::Canvas()
{
    _fonts.reserve(5);

    save();
    reset();

    device_pixel_ratio(1.0f);

    _glc = std::make_unique<detail::GLNVGcontext>();
}

Canvas::~Canvas() = default;

void Canvas::begin_frame(const SizeU& windowSize, f32 devicePixelRatio)
{
    _windowSize = static_cast<SizeF>(windowSize);
    /*	printf("Tris: draws:%d  fill:%d  stroke:%d  text:%d  TOT:%d\n",
		drawCallCount, fillTriCount, strokeTriCount, textTriCount,
		fillTriCount+strokeTriCount+textTriCount);*/

    _states = std::stack<NVGstate>();

    save();
    reset();

    device_pixel_ratio(devicePixelRatio);

    _glc->set_viewport(_windowSize);

    _drawCallCount = 0;
    _fillTriCount = 0;
    _strokeTriCount = 0;
    _textTriCount = 0;
}

void Canvas::end_frame()
{
    _glc->flush();
}

void Canvas::cancel_frame()
{
    _glc->cancel();
}

void Canvas::save()
{
    if (_states.empty()) {
        _states.emplace();
    } else {
        _states.push(_states.top());
    }
}

void Canvas::restore()
{
    _states.pop();
}

void Canvas::reset()
{
    NVGstate& s { state() };

    set_paint_color(s.fill, Colors::White);
    set_paint_color(s.stroke, Colors::Black);
    s.compositeOperation = nvg__compositeOperationState(CompositeOperation::SourceOver);
    s.shapeAntiAlias = true;
    s.strokeWidth = 1.0f;
    s.miterLimit = 10.0f;
    s.lineCap = LineCap::Butt;
    s.lineJoin = LineJoin::Miter;
    s.alpha = 1.0f;
    s.xform = Transform::Identity;

    s.scissor.extent[0] = -1.0f;
    s.scissor.extent[1] = -1.0f;

    s.textAlign = TextAlignment::Left;
    s.fontId = 0;
}

////////////////////////////////////////////////////////////

void Canvas::begin_path()
{
    _commands.clear();
    clear_path_cache();
}

void Canvas::close_path()
{
    append_commands({ NVG_CLOSE });
}

void Canvas::path_winding(Winding dir)
{
    append_commands({ NVG_WINDING, static_cast<f32>(dir) });
}

////////////////////////////////////////////////////////////

void Canvas::move_to(const PointF& pos)
{
    append_commands({ NVG_MOVETO, pos.X, pos.Y });
}

void Canvas::line_to(const PointF& pos)
{
    append_commands({ NVG_LINETO, pos.X, pos.Y });
}

void Canvas::fill_lines(std::span<PointF> points)
{
    begin_path();

    move_to(points[0]);
    for (u32 i { 1 }; i < points.size(); ++i) {
        line_to(points[i]);
    }

    fill();
}

void Canvas::stroke_lines(std::span<PointF> points)
{
    begin_path();

    move_to(points[0]);
    for (u32 i { 1 }; i < points.size(); ++i) {
        line_to(points[i]);
    }

    stroke();
}

////////////////////////////////////////////////////////////

void Canvas::cubic_bezier_to(const PointF& c1, const PointF& c2, const PointF& pos)
{
    append_commands({ NVG_BEZIERTO, c1.X, c1.Y, c2.X, c2.Y, pos.X, pos.Y });
}

void Canvas::quad_bezier_to(const PointF& c, const PointF& pos)
{
    f32 x0 { _commandx };
    f32 y0 { _commandy };

    append_commands(
        { NVG_BEZIERTO,
            x0 + 2.0f / 3.0f * (c.X - x0), y0 + 2.0f / 3.0f * (c.Y - y0),
            pos.X + 2.0f / 3.0f * (c.X - pos.X), pos.Y + 2.0f / 3.0f * (c.Y - pos.Y),
            pos.X, pos.Y });
}

////////////////////////////////////////////////////////////

void Canvas::arc(const PointF& c, f32 r, f32 a0, f32 a1, Winding dir)
{
    i32 move { !_commands.empty() ? NVG_LINETO : NVG_MOVETO };

    // Clamp angles
    f32 da { a1 - a0 };
    if (dir == Winding::CW) {
        if (std::abs(da) >= TAU_F) {
            da = TAU_F;
        } else {
            while (da < 0.0f)
                da += TAU_F;
        }
    } else {
        if (std::abs(da) >= TAU_F) {
            da = -TAU_F;
        } else {
            while (da > 0.0f)
                da -= TAU_F;
        }
    }

    // Split arc into max 90 degree segments.
    const i32 ndivs { std::max(1, std::min(static_cast<i32>(std::abs(da) / (TAU_F * 0.25f) + 0.5f), 5)) };
    const f32 hda { (da / static_cast<f32>(ndivs)) / 2.0f };
    f32 kappa { std::abs(4.0f / 3.0f * (1.0f - std::cos(hda)) / std::sin(hda)) };

    if (dir == Winding::CCW)
        kappa = -kappa;

    f32 px { 0 }, py { 0 }, ptanx { 0 }, ptany { 0 };

    std::vector<f32> vals;
    vals.reserve(3 + 5 * 7 + 100);
    for (i32 i { 0 }; i <= ndivs; ++i) {
        const f32 a { a0 + da * (i / static_cast<f32>(ndivs)) };
        const f32 dx { std::cos(a) };
        const f32 dy { std::sin(a) };
        const f32 x { c.X + dx * r };
        const f32 y { c.Y + dy * r };
        const f32 tanx { -dy * r * kappa };
        const f32 tany { dx * r * kappa };

        if (i == 0) {
            vals.push_back(static_cast<f32>(move));
            vals.push_back(x);
            vals.push_back(y);
        } else {
            vals.push_back(NVG_BEZIERTO);
            vals.push_back(px + ptanx);
            vals.push_back(py + ptany);
            vals.push_back(x - tanx);
            vals.push_back(y - tany);
            vals.push_back(x);
            vals.push_back(y);
        }
        px = x;
        py = y;
        ptanx = tanx;
        ptany = tany;
    }

    append_commands(std::forward<std::vector<f32>>(vals));
}

void Canvas::fill_arc(const PointF& center, f32 r, f32 a0, f32 a1, Winding wind)
{
    begin_path();
    arc(center, r, deg_to_rad(a0), deg_to_rad(a1), wind);
    fill();
}

void Canvas::stroke_arc(const PointF& center, f32 r, f32 a0, f32 a1, Winding wind)
{
    begin_path();
    arc(center, r, deg_to_rad(a0), deg_to_rad(a1), wind);
    stroke();
}

void Canvas::arc_to(const PointF& pos1, const PointF& pos2, f32 radius)
{
    f32 x0 { _commandx };
    f32 y0 { _commandy };
    Winding dir;

    if (_commands.empty()) {
        return;
    }

    // Handle degenerate cases.
    if (nvg__ptEquals(x0, y0, pos1.X, pos1.Y, _distTol)
        || nvg__ptEquals(pos1.X, pos1.Y, pos2.X, pos2.Y, _distTol)
        || nvg__distPtSeg(pos1.X, pos1.Y, x0, y0, pos2.X, pos2.Y) < _distTol * _distTol
        || radius < _distTol) {
        line_to(pos1);
        return;
    }

    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    f32 dx0 { x0 - pos1.X };
    f32 dy0 { y0 - pos1.Y };
    f32 dx1 { pos2.X - pos1.X };
    f32 dy1 { pos2.Y - pos1.Y };
    nvg__normalize(dx0, dy0);
    nvg__normalize(dx1, dy1);
    f32 a = std::acos(dx0 * dx1 + dy0 * dy1);
    f32 d = radius / std::tan(a / 2.0f);

    //	printf("a=%f° d=%f\n", a/NVG_PI*180.0f, d);

    if (d > 10000.0f) {
        line_to(pos1);
        return;
    }

    f32 cx { 0 }, cy { 0 }, a0 { 0 }, a1 { 0 };
    if (nvg__cross(dx0, dy0, dx1, dy1) > 0.0f) {
        cx = pos1.X + dx0 * d + dy0 * radius;
        cy = pos1.Y + dy0 * d + -dx0 * radius;
        a0 = std::atan2(dx0, -dy0);
        a1 = std::atan2(-dx1, dy1);
        dir = Winding::CW;
        //		printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy, a0/NVG_PI*180.0f, a1/NVG_PI*180.0f);
    } else {
        cx = pos1.X + dx0 * d + -dy0 * radius;
        cy = pos1.Y + dy0 * d + dx0 * radius;
        a0 = std::atan2(-dx0, dy0);
        a1 = std::atan2(dx1, -dy1);
        dir = Winding::CCW;
        //		printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy, a0/NVG_PI*180.0f, a1/NVG_PI*180.0f);
    }

    arc({ cx, cy }, radius, a0, a1, dir);
}

////////////////////////////////////////////////////////////

void Canvas::rect(const RectF& rec)
{
    auto [x, y, w, h] { rec };
    append_commands(
        { NVG_MOVETO, x, y,
            NVG_LINETO, x, y + h,
            NVG_LINETO, x + w, y + h,
            NVG_LINETO, x + w, y,
            NVG_CLOSE });
}

void Canvas::fill_rect(const RectF& r)
{
    begin_path();
    rect(r);
    fill();
}

void Canvas::stroke_rect(const RectF& r)
{
    begin_path();
    rect(r);
    stroke();
}

void Canvas::rounded_rect(const RectF& rec, f32 r)
{
    rounded_rect_varying(rec, r, r, r, r);
}

void Canvas::fill_rounded_rect(const RectF& r, f32 rad)
{
    begin_path();
    rounded_rect(r, rad);
    fill();
}

void Canvas::stroke_rounded_rect(const RectF& r, f32 rad)
{
    begin_path();
    rounded_rect(r, rad);
    stroke();
}

void Canvas::rounded_rect_varying(const RectF& rec, f32 radTopLeft, f32 radTopRight, f32 radBottomRight, f32 radBottomLeft)
{
    auto [x, y, w, h] { rec };
    if (radTopLeft < 0.1f && radTopRight < 0.1f && radBottomRight < 0.1f && radBottomLeft < 0.1f) {
        rect(rec);
        return;
    } else {
        f32 halfw { std::abs(w) * 0.5f };
        f32 halfh { std::abs(h) * 0.5f };
        f32 rxBL { std::min(radBottomLeft, halfw) * nvg__signf(w) }, ryBL { std::min(radBottomLeft, halfh) * nvg__signf(h) };
        f32 rxBR { std::min(radBottomRight, halfw) * nvg__signf(w) }, ryBR { std::min(radBottomRight, halfh) * nvg__signf(h) };
        f32 rxTR { std::min(radTopRight, halfw) * nvg__signf(w) }, ryTR { std::min(radTopRight, halfh) * nvg__signf(h) };
        f32 rxTL { std::min(radTopLeft, halfw) * nvg__signf(w) }, ryTL { std::min(radTopLeft, halfh) * nvg__signf(h) };
        std::vector<f32> vals {
            NVG_MOVETO, x, y + ryTL,
            NVG_LINETO, x, y + h - ryBL,
            NVG_BEZIERTO, x, y + h - ryBL * (1 - NVG_KAPPA90), x + rxBL * (1 - NVG_KAPPA90), y + h, x + rxBL, y + h,
            NVG_LINETO, x + w - rxBR, y + h,
            NVG_BEZIERTO, x + w - rxBR * (1 - NVG_KAPPA90), y + h, x + w, y + h - ryBR * (1 - NVG_KAPPA90), x + w, y + h - ryBR,
            NVG_LINETO, x + w, y + ryTR,
            NVG_BEZIERTO, x + w, y + ryTR * (1 - NVG_KAPPA90), x + w - rxTR * (1 - NVG_KAPPA90), y, x + w - rxTR, y,
            NVG_LINETO, x + rxTL, y,
            NVG_BEZIERTO, x + rxTL * (1 - NVG_KAPPA90), y, x, y + ryTL * (1 - NVG_KAPPA90), x, y + ryTL,
            NVG_CLOSE
        };
        append_commands(std::forward<std::vector<f32>>(vals));
    }
}

void Canvas::fill_rounded_rect_varying(const RectF& r, f32 rtl, f32 rtr, f32 rbr, f32 rbl)
{
    begin_path();
    rounded_rect_varying(r, rtl, rtr, rbr, rbl);
    fill();
}

void Canvas::stroke_rounded_rect_varying(const RectF& r, f32 rtl, f32 rtr, f32 rbr, f32 rbl)
{
    begin_path();
    rounded_rect_varying(r, rtl, rtr, rbr, rbl);
    stroke();
}

////////////////////////////////////////////////////////////

void Canvas::ellipse(const PointF& c, f32 rx, f32 ry)
{
    auto [cx, cy] { c };
    append_commands(
        { NVG_MOVETO, cx - rx, cy,
            NVG_BEZIERTO, cx - rx, cy + ry * NVG_KAPPA90, cx - rx * NVG_KAPPA90, cy + ry, cx, cy + ry,
            NVG_BEZIERTO, cx + rx * NVG_KAPPA90, cy + ry, cx + rx, cy + ry * NVG_KAPPA90, cx + rx, cy,
            NVG_BEZIERTO, cx + rx, cy - ry * NVG_KAPPA90, cx + rx * NVG_KAPPA90, cy - ry, cx, cy - ry,
            NVG_BEZIERTO, cx - rx * NVG_KAPPA90, cy - ry, cx - rx, cy - ry * NVG_KAPPA90, cx - rx, cy,
            NVG_CLOSE });
}

void Canvas::fill_ellipse(const PointF& center, f32 hr, f32 vr)
{
    begin_path();
    ellipse(center, hr, vr);
    fill();
}

void Canvas::stroke_ellipse(const PointF& center, f32 hr, f32 vr)
{
    begin_path();
    ellipse(center, hr, vr);
    stroke();
}

void Canvas::circle(const PointF& c, f32 r)
{
    ellipse(c, r, r);
}

void Canvas::fill_circle(const PointF& center, f32 r)
{
    begin_path();
    circle(center, r);
    fill();
}

void Canvas::stroke_circle(const PointF& center, f32 r)
{
    begin_path();
    circle(center, r);
    stroke();
}

////////////////////////////////////////////////////////////

void Canvas::fill_color(const Color& color)
{
    NVGstate& s { state() };
    set_paint_color(s.fill, color);
}

void Canvas::fill_paint(const CanvasPaint& paint)
{
    NVGstate& s { state() };
    s.fill = paint;
    s.fill.xform *= s.xform;
}

void Canvas::stroke_color(const Color& color)
{
    NVGstate& s { state() };
    set_paint_color(s.stroke, color);
}

void Canvas::stroke_paint(const CanvasPaint& paint)
{
    NVGstate& s { state() };
    s.stroke = paint;
    s.stroke.xform *= s.xform;
}

void Canvas::shape_antialias(bool enabled)
{
    NVGstate& s { state() };
    s.shapeAntiAlias = enabled;
}

void Canvas::miter_limit(f32 limit)
{
    NVGstate& s { state() };
    s.miterLimit = limit;
}

void Canvas::stroke_width(f32 size)
{
    NVGstate& s { state() };
    s.strokeWidth = size;
}

void Canvas::line_cap(LineCap cap)
{
    NVGstate& s { state() };
    s.lineCap = cap;
}

void Canvas::line_join(LineJoin join)
{
    NVGstate& s { state() };
    s.lineJoin = join;
}

void Canvas::global_alpha(f32 alpha)
{
    NVGstate& s { state() };
    s.alpha = alpha;
}

void Canvas::fill()
{
    NVGstate& s { state() };
    CanvasPaint& fillPaint { s.fill };

    flatten_paths();
    if (_edgeAntiAlias && s.shapeAntiAlias)
        expand_fill(_fringeWidth, LineJoin::Miter, 2.4f);
    else
        expand_fill(0.0f, LineJoin::Miter, 2.4f);

    // Apply global alpha
    fillPaint.gradient.multiply_alpha(s.alpha);

    _glc->render_fill(fillPaint, s.compositeOperation, s.scissor, _fringeWidth,
        _cache.bounds, _cache.paths);

    // Count triangles
    for (const auto& path : _cache.paths) {
        _fillTriCount += path.nfill - 2;
        _fillTriCount += path.nstroke - 2;
        _drawCallCount += 2;
    }
}

void Canvas::stroke()
{
    NVGstate& s { state() };
    const f32 scale { nvg__getAverageScale(s.xform.matrix3()) };
    f32 strokeWidth { std::clamp(s.strokeWidth * scale, 0.0f, 200.0f) };
    CanvasPaint& strokePaint { s.stroke };

    if (strokeWidth < _fringeWidth) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        // Since coverage is area, scale by alpha*alpha.
        f32 alpha = std::clamp(strokeWidth / _fringeWidth, 0.0f, 1.0f);
        strokePaint.gradient.multiply_alpha(alpha * alpha);
        strokeWidth = _fringeWidth;
    }

    // Apply global alpha
    strokePaint.gradient.multiply_alpha(s.alpha);

    flatten_paths();

    if (_edgeAntiAlias && s.shapeAntiAlias)
        expand_stroke(strokeWidth * 0.5f, _fringeWidth, s.lineCap, s.lineJoin, s.miterLimit);
    else
        expand_stroke(strokeWidth * 0.5f, 0.0f, s.lineCap, s.lineJoin, s.miterLimit);

    _glc->render_stroke(strokePaint, s.compositeOperation, s.scissor, _fringeWidth,
        strokeWidth, _cache.paths);

    // Count triangles
    for (const auto& path : _cache.paths) {
        _strokeTriCount += path.nstroke - 2;
        _drawCallCount++;
    }
}

////////////////////////////////////////////////////////////

auto Canvas::create_linear_gradient(const PointF& s, const PointF& e, const ColorGradient<256>& gradient) -> CanvasPaint
{
    CanvasPaint p;
    const f32 large { 1e5 };

    // Calculate transform aligned to the line
    f32 dx { e.X - s.X };
    f32 dy { e.Y - s.Y };
    f32 d { std::sqrt(dx * dx + dy * dy) };
    if (d > 0.0001f) {
        dx /= d;
        dy /= d;
    } else {
        dx = 0;
        dy = 1;
    }

    p.xform = {
        dy, dx, s.X - dx * large,
        -dx, dy, s.Y - dy * large,
        0, 0, 1
    };

    p.extent[0] = large;
    p.extent[1] = large + d * 0.5f;

    p.radius = 0.0f;

    p.feather = std::max(1.0f, d);

    p.gradient = gradient;

    return p;
}

auto Canvas::create_box_gradient(const RectF& rect, f32 r, f32 f, const ColorGradient<256>& gradient) -> CanvasPaint
{
    CanvasPaint p;

    p.xform = {
        1.f, 0.f, rect.Left + rect.Width * 0.5f,
        0.f, 1.f, rect.Top + rect.Height * 0.5f,
        0.f, 0.f, 1.f
    };

    p.extent[0] = rect.Width * 0.5f;
    p.extent[1] = rect.Height * 0.5f;

    p.radius = r;

    p.feather = std::max(1.0f, f);

    p.gradient = gradient;

    return p;
}

auto Canvas::create_radial_gradient(const PointF& c, f32 inr, f32 outr, const ColorGradient<256>& gradient) -> CanvasPaint
{
    CanvasPaint p;
    f32 r { (inr + outr) * 0.5f };
    f32 f { (outr - inr) };

    p.xform = {
        1.f, 0.f, c.X,
        0.f, 1.f, c.Y,
        0.f, 0.f, 1.f
    };

    p.extent[0] = r;
    p.extent[1] = r;

    p.radius = r;

    p.feather = std::max(1.0f, f);

    p.gradient = gradient;

    return p;
}

auto Canvas::create_image_pattern(const PointF& c, const SizeF& e, f32 angle, i32 image, f32 alpha) -> CanvasPaint
{
    CanvasPaint p;

    p.xform = Transform::Identity;
    p.xform.rotate(angle);
    p.xform.translate(c);

    p.extent[0] = e.Width;
    p.extent[1] = e.Height;

    p.image = _images[image].get();

    p.gradient = { Color { 255, 255, 255, static_cast<u8>(255 * alpha) }, Color { 255, 255, 255, static_cast<u8>(255 * alpha) } };

    return p;
}

auto Canvas::add_image(const std::string& imageName) -> i32
{
    Image image { Image::Load(imageName) };
    auto imgSize { image.info().SizeInPixels };
    if (imgSize != SizeU::Zero) {
        auto tex { std::make_unique<gl::Texture2D>() };
        tex->create(imgSize, gl::TextureFormat::RGBA8);
        tex->update(PointU::Zero, imgSize, image.buffer());
        _images.push_back(std::move(tex));
        return static_cast<i32>(_images.size() - 1);
    }
    return -1;
}

////////////////////////////////////////////////////////////

void Canvas::global_composite_operation(CompositeOperation op)
{
    NVGstate& s { state() };
    s.compositeOperation = nvg__compositeOperationState(op);
}

void Canvas::global_composite_blendfunc(gl::BlendFunc sfactor, gl::BlendFunc dfactor)
{
    global_composite_blendfunc_separate(sfactor, dfactor, sfactor, dfactor);
}

void Canvas::global_composite_blendfunc_separate(gl::BlendFunc srcRGB, gl::BlendFunc dstRGB, gl::BlendFunc srcAlpha, gl::BlendFunc dstAlpha)
{
    gl::BlendFuncs op {
        .SourceColorBlendFunc = srcRGB,
        .DestinationColorBlendFunc = dstRGB,
        .SourceAlphaBlendFunc = srcAlpha,
        .DestinationAlphaBlendFunc = dstAlpha
    };

    NVGstate& s { state() };
    s.compositeOperation = op;
}

////////////////////////////////////////////////////////////

void Canvas::translate(const PointF& c)
{
    NVGstate& s { state() };
    s.xform.translate(c);
}

void Canvas::rotate(f32 angle)
{
    NVGstate& s { state() };
    s.xform.rotate(angle);
}

void Canvas::rotate_at(f32 angle, const PointF& p)
{
    translate(p);
    rotate(angle);
    translate(-p);
}

void Canvas::scale(const SizeF& scale)
{
    NVGstate& s { state() };
    s.xform.scale(scale);
}

void Canvas::scale_at(const SizeF& sc, const PointF& p)
{
    translate(p);
    scale(sc);
    translate(-p);
}

void Canvas::skew_x(f32 angle)
{
    NVGstate& s { state() };
    s.xform.skew({ angle, 0 });
}

void Canvas::skew_x_at(f32 angle, const PointF& p)
{
    translate(p);
    skew_x(angle);
    translate(-p);
}

void Canvas::skew_y(f32 angle)
{
    NVGstate& s { state() };
    s.xform.skew({ 0, angle });
}

void Canvas::skew_y_at(f32 angle, const PointF& p)
{
    translate(p);
    skew_y(angle);
    translate(-p);
}

void Canvas::reset_transform()
{
    NVGstate& s { state() };
    s.xform = Transform::Identity;
}

////////////////////////////////////////////////////////////

void Canvas::scissor(const RectF& rect)
{
    NVGstate& s { state() };

    auto [x, y, w, h] { rect };
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    s.scissor.xform = Transform::Identity;
    s.scissor.xform.translate({ x + w * 0.5f, y + h * 0.5f });
    s.scissor.xform *= s.xform;

    s.scissor.extent[0] = w * 0.5f;
    s.scissor.extent[1] = h * 0.5f;
}

void Canvas::reset_scissor()
{
    NVGstate& s { state() };
    s.scissor.xform = Transform { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    s.scissor.extent[0] = -1.0f;
    s.scissor.extent[1] = -1.0f;
}

////////////////////////////////////////////////////////////

auto Canvas::add_font(ResourcePtr<Font> font) -> isize
{
    _fonts.push_back(std::move(font));
    return _fonts.size() - 1;
}

void Canvas::font_face_ID(isize id)
{
    if (id >= _fonts.size())
        return;

    NVGstate& s = state();
    s.fontId = id;
}

void Canvas::draw_textbox(const PointF& pos, const SizeF& size, const std::string& text)
{
    auto [x, y] { pos };
    NVGstate& s { state() };
    auto font { _fonts[s.fontId] };
    auto formatResult { TextFormatter::format(text, font, s.textAlign, size) };

    f32 scale { nvg__getFontScale(s) * _devicePxRatio };
    f32 invscale { 1.0f / scale };
    auto verts = alloc_temp_verts(static_cast<i32>(formatResult.GlyphCount) * 4);
    i32 nverts { 0 };
    for (const auto& token : formatResult.Tokens) {
        for (isize i { 0 }; i < token.Quads.size(); ++i) {
            auto& posRect { token.Quads[i].Rect };
            auto& uvRect { token.Quads[i].TexRegion.UVRect };

            auto topLeft { s.xform * PointF { posRect.Left * invscale + x, posRect.Top * invscale + y } };
            auto topRight { s.xform * PointF { posRect.right() * invscale + x, posRect.Top * invscale + y } };
            auto bottomRight { s.xform * PointF { posRect.right() * invscale + x, posRect.bottom() * invscale + y } };
            auto bottomLeft { s.xform * PointF { posRect.Left * invscale + x, posRect.bottom() * invscale + y } };

            verts[nverts].Position = { topLeft.X, topLeft.Y };
            verts[nverts].TexCoords = { uvRect.Left, uvRect.Top };
            nverts++;
            verts[nverts].Position = { bottomRight.X, bottomRight.Y };
            verts[nverts].TexCoords = { uvRect.right(), uvRect.bottom() };
            nverts++;
            verts[nverts].Position = { topRight.X, topRight.Y };
            verts[nverts].TexCoords = { uvRect.right(), uvRect.Top };
            nverts++;
            verts[nverts].Position = { topLeft.X, topLeft.Y };
            verts[nverts].TexCoords = { uvRect.Left, uvRect.Top };
            nverts++;
            verts[nverts].Position = { bottomLeft.X, bottomLeft.Y };
            verts[nverts].TexCoords = { uvRect.Left, uvRect.bottom() };
            nverts++;
            verts[nverts].Position = { bottomRight.X, bottomRight.Y };
            verts[nverts].TexCoords = { uvRect.right(), uvRect.bottom() };
            nverts++;
        }
    }

    render_text(verts, nverts);
}

void Canvas::text_align(TextAlignment align)
{
    NVGstate& s { state() };
    s.textAlign = align;
}

void Canvas::text_outline_color(const Color& color)
{
    NVGstate& s { state() };
    s.fill.text_outline_color = color;
}

void Canvas::text_outline_thickness(f32 thickness)
{
    NVGstate& s { state() };
    s.fill.text_outline_thickness = thickness;
}

////////////////////////////////////////////////////////////

void Canvas::draw_image(i32 handle, const RectF& rect)
{
    const CanvasPaint img { create_image_pattern(rect.position(), rect.size(), 0, handle, 1) };

    fill_paint(img);
    fill_rect(rect);
}

void Canvas::draw_image_clipped(i32 handle, const RectF& srect, const RectF& rect)
{
    const SizeI i { _images[handle]->size() };

    const f32 ax { rect.Width / srect.Width };
    const f32 ay { rect.Height / srect.Height };

    const CanvasPaint img { create_image_pattern({ rect.Left - srect.Left * ax, rect.Top - srect.Top * ay }, { i.Width * ax, i.Height * ay }, 0, handle, 1) };

    fill_paint(img);
    fill_rect(rect);
}

////////////////////////////////////////////////////////////

auto Canvas::window_size() const -> SizeU
{
    return static_cast<SizeU>(_windowSize);
}

void Canvas::device_pixel_ratio(f32 ratio)
{
    _tessTol = 0.25f / ratio;
    _distTol = 0.01f / ratio;
    _fringeWidth = 1.0f / ratio;
    _devicePxRatio = ratio;
}

////////////////////////////////////////////////////////////

auto Canvas::state() -> NVGstate&
{
    return _states.top();
}

void Canvas::set_paint_color(CanvasPaint& p, const Color& color)
{
    p.xform = Transform::Identity;
    p.radius = 0.0f;
    p.feather = 1.0f;
    p.gradient = { color, color };
    p.image = nullptr;
}

void Canvas::append_commands(std::vector<f32>&& vals)
{
    NVGstate& s { state() };
    isize nvals { vals.size() };

    if (static_cast<i32>(vals[0]) != NVG_CLOSE && static_cast<i32>(vals[0]) != NVG_WINDING) {
        _commandx = vals[nvals - 2];
        _commandy = vals[nvals - 1];
    }

    // transform commands
    isize i { 0 };
    PointF p;
    while (i < nvals) {
        i32 cmd { static_cast<i32>(vals[i]) };
        switch (cmd) {
        case NVG_MOVETO:
            p = s.xform * PointF { vals[i + 1], vals[i + 2] };
            vals[i + 1] = p.X;
            vals[i + 2] = p.Y;
            i += 3;
            break;
        case NVG_LINETO:
            p = s.xform * PointF { vals[i + 1], vals[i + 2] };
            vals[i + 1] = p.X;
            vals[i + 2] = p.Y;
            i += 3;
            break;
        case NVG_BEZIERTO:
            p = s.xform * PointF { vals[i + 1], vals[i + 2] };
            vals[i + 1] = p.X;
            vals[i + 2] = p.Y;
            p = s.xform * PointF { vals[i + 3], vals[i + 4] };
            vals[i + 3] = p.X;
            vals[i + 4] = p.Y;
            p = s.xform * PointF { vals[i + 5], vals[i + 6] };
            vals[i + 5] = p.X;
            vals[i + 6] = p.Y;
            i += 7;
            break;
        case NVG_CLOSE:
            ++i;
            break;
        case NVG_WINDING:
            i += 2;
            break;
        default:
            ++i;
        }
    }

    _commands.reserve(_commands.size() + vals.size());
    _commands.insert(_commands.end(), vals.begin(), vals.end());
}

void Canvas::clear_path_cache()
{
    _cache.points.clear();
    _cache.paths.clear();
}

auto Canvas::get_last_path() -> NVGpath&
{
    return _cache.paths.back();
}

void Canvas::add_path()
{
    NVGpath path;
    path.first = static_cast<i32>(_cache.points.size());
    _cache.paths.push_back(path);
}

auto Canvas::get_last_point() -> NVGpoint&
{
    return _cache.points.back();
}

void Canvas::add_point(f32 x, f32 y, i32 flags)
{
    NVGpath& path { get_last_path() };

    if (path.count > 0 && !_cache.points.empty()) {
        NVGpoint& pt { get_last_point() };
        if (nvg__ptEquals(pt.x, pt.y, x, y, _distTol)) {
            pt.flags |= flags;
            return;
        }
    }

    NVGpoint pt;
    pt.x = x;
    pt.y = y;
    pt.flags = (unsigned char)flags;

    _cache.points.push_back(pt);
    path.count++;
}

void Canvas::close_last_path()
{
    NVGpath& path { get_last_path() };
    path.closed = true;
}

void Canvas::set_path_winding(Winding winding)
{
    NVGpath& path { get_last_path() };
    path.winding = winding;
}

auto Canvas::alloc_temp_verts(isize nverts) -> Vertex*
{
    if (nverts > _cache.verts.capacity()) {
        _cache.verts.reserve((nverts + 0xff) & ~0xff);
    }

    return _cache.verts.data();
}

void Canvas::tesselate_bezier(
    f32 x1, f32 y1, f32 x2, f32 y2,
    f32 x3, f32 y3, f32 x4, f32 y4,
    i32 level, i32 type)
{
    if (level > 10)
        return;

    f32 x12 { (x1 + x2) * 0.5f };
    f32 y12 { (y1 + y2) * 0.5f };
    f32 x23 { (x2 + x3) * 0.5f };
    f32 y23 { (y2 + y3) * 0.5f };
    f32 x34 { (x3 + x4) * 0.5f };
    f32 y34 { (y3 + y4) * 0.5f };
    f32 x123 { (x12 + x23) * 0.5f };
    f32 y123 { (y12 + y23) * 0.5f };

    f32 dx { x4 - x1 };
    f32 dy { y4 - y1 };
    f32 d2 { std::abs(((x2 - x4) * dy - (y2 - y4) * dx)) };
    f32 d3 { std::abs(((x3 - x4) * dy - (y3 - y4) * dx)) };

    if ((d2 + d3) * (d2 + d3) < _tessTol * (dx * dx + dy * dy)) {
        add_point(x4, y4, type);
        return;
    }

    f32 x234 { (x23 + x34) * 0.5f };
    f32 y234 { (y23 + y34) * 0.5f };
    f32 x1234 { (x123 + x234) * 0.5f };
    f32 y1234 { (y123 + y234) * 0.5f };

    tesselate_bezier(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1, 0);
    tesselate_bezier(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1, type);
}

void Canvas::flatten_paths()
{
    if (!_cache.paths.empty())
        return;

    // Flatten
    isize i { 0 };
    while (i < _commands.size()) {
        i32 cmd { static_cast<i32>(_commands[i]) };
        switch (cmd) {
        case NVG_MOVETO: {
            add_path();
            f32* p { &_commands[i + 1] };
            add_point(p[0], p[1], NVG_PT_CORNER);
            i += 3;
        } break;
        case NVG_LINETO: {
            f32* p { &_commands[i + 1] };
            add_point(p[0], p[1], NVG_PT_CORNER);
            i += 3;
            break;
        }
        case NVG_BEZIERTO: {
            NVGpoint& last { get_last_point() };
            f32* cp1 { &_commands[i + 1] };
            f32* cp2 { &_commands[i + 3] };
            f32* p { &_commands[i + 5] };
            tesselate_bezier(last.x, last.y, cp1[0], cp1[1], cp2[0], cp2[1], p[0], p[1], 0, NVG_PT_CORNER);
            i += 7;
        } break;
        case NVG_CLOSE:
            close_last_path();
            ++i;
            break;
        case NVG_WINDING:
            set_path_winding(static_cast<Winding>(_commands[i + 1]));
            i += 2;
            break;
        default:
            ++i;
        }
    }

    _cache.bounds[0] = _cache.bounds[1] = 1e6f;
    _cache.bounds[2] = _cache.bounds[3] = -1e6f;

    // Calculate the direction and length of line segments.
    for (auto& path : _cache.paths) {
        NVGpoint* pts { &_cache.points[path.first] };

        // If the first and last points are the same, remove the last, mark as closed path.
        NVGpoint* p0 { &pts[path.count - 1] };
        NVGpoint* p1 { &pts[0] };

        if (nvg__ptEquals(p0->x, p0->y, p1->x, p1->y, _distTol)) {
            path.count--;
            p0 = &pts[path.count - 1];
            path.closed = true;
        }

        // Enforce winding.
        if (path.count > 2) {
            f32 area { nvg__polyArea(pts, path.count) };
            if (path.winding == Winding::CCW && area < 0.0f)
                nvg__polyReverse(pts, path.count);
            if (path.winding == Winding::CW && area > 0.0f)
                nvg__polyReverse(pts, path.count);
        }

        for (i = 0; i < path.count; ++i) {
            // Calculate segment direction and length
            p0->dx = p1->x - p0->x;
            p0->dy = p1->y - p0->y;
            p0->len = nvg__normalize(p0->dx, p0->dy);
            // Update bounds
            _cache.bounds[0] = std::min(_cache.bounds[0], p0->x);
            _cache.bounds[1] = std::min(_cache.bounds[1], p0->y);
            _cache.bounds[2] = std::max(_cache.bounds[2], p0->x);
            _cache.bounds[3] = std::max(_cache.bounds[3], p0->y);
            // Advance
            p0 = p1++;
        }
    }
}

void Canvas::calculate_joins(f32 w, LineJoin lineJoin, f32 miterLimit)
{
    f32 iw { 0.0f };

    if (w > 0.0f)
        iw = 1.0f / w;

    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (auto& path : _cache.paths) {
        NVGpoint* pts { &_cache.points[path.first] };
        NVGpoint* p0 { &pts[path.count - 1] };
        NVGpoint* p1 { &pts[0] };
        isize nleft { 0 };

        path.nbevel = 0;

        for (isize j { 0 }; j < path.count; j++) {
            f32 dlx0 { p0->dy };
            f32 dly0 { -p0->dx };
            f32 dlx1 { p1->dy };
            f32 dly1 { -p1->dx };
            // Calculate extrusions
            p1->dmx = (dlx0 + dlx1) * 0.5f;
            p1->dmy = (dly0 + dly1) * 0.5f;
            f32 dmr2 { p1->dmx * p1->dmx + p1->dmy * p1->dmy };
            if (dmr2 > 0.000001f) {
                f32 scale { 1.0f / dmr2 };
                if (scale > 600.0f) {
                    scale = 600.0f;
                }
                p1->dmx *= scale;
                p1->dmy *= scale;
            }

            // Clear flags, but keep the corner.
            p1->flags = (p1->flags & NVG_PT_CORNER) ? NVG_PT_CORNER : 0;

            // Keep track of left turns.
            f32 cross { p1->dx * p0->dy - p0->dx * p1->dy };
            if (cross > 0.0f) {
                nleft++;
                p1->flags |= NVG_PT_LEFT;
            }

            // Calculate if we should use bevel or miter for inner join.
            f32 limit { std::max(1.01f, std::min(p0->len, p1->len) * iw) };
            if ((dmr2 * limit * limit) < 1.0f)
                p1->flags |= NVG_PR_INNERBEVEL;

            // Check to see if the corner needs to be beveled.
            if (p1->flags & NVG_PT_CORNER) {
                if ((dmr2 * miterLimit * miterLimit) < 1.0f || lineJoin == LineJoin::Bevel || lineJoin == LineJoin::Round) {
                    p1->flags |= NVG_PT_BEVEL;
                }
            }

            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0)
                path.nbevel++;

            p0 = p1++;
        }

        path.convex = (nleft == path.count);
    }
}

void Canvas::expand_stroke(f32 w, f32 fringe, LineCap lineCap, LineJoin lineJoin, f32 miterLimit)
{
    f32 aa { fringe }; //fringeWidth;
    f32 u0 { 0.0f }, u1 { 1.0f };
    i32 ncap { nvg__curveDivs(w, TAU_F / 2, _tessTol) }; // Calculate divisions per half circle.

    w += aa * 0.5f;

    // Disable the gradient used for antialiasing when antialiasing is not used.
    if (aa == 0.0f) {
        u0 = 0.5f;
        u1 = 0.5f;
    }

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    isize cverts { 0 };
    for (auto& path : _cache.paths) {
        bool loop { path.closed };
        if (lineJoin == LineJoin::Round)
            cverts += (path.count + path.nbevel * (ncap + 2) + 1) * 2; // plus one for loop
        else
            cverts += (path.count + path.nbevel * 5 + 1) * 2; // plus one for loop
        if (!loop) {
            // space for caps
            if (lineCap == LineCap::Round) {
                cverts += (ncap * 2 + 2) * 2;
            } else {
                cverts += (3 + 3) * 2;
            }
        }
    }

    Vertex* verts { alloc_temp_verts(cverts) };
    if (verts == nullptr)
        return;

    Vertex* dst { nullptr };
    for (auto& path : _cache.paths) {
        NVGpoint* pts { &_cache.points[path.first] };
        NVGpoint* p0 { nullptr };
        NVGpoint* p1 { nullptr };
        isize s { 0 }, e { 0 };

        path.fill = nullptr;
        path.nfill = 0;

        // Calculate fringe or stroke
        const bool loop { path.closed };
        dst = verts;
        path.stroke = dst;

        if (loop) {
            // Looping
            p0 = &pts[path.count - 1];
            p1 = &pts[0];
            s = 0;
            e = path.count;
        } else {
            // Add cap
            p0 = &pts[0];
            p1 = &pts[1];
            s = 1;
            e = path.count - 1;
        }

        if (!loop) {
            // Add cap
            f32 dx { p1->x - p0->x };
            f32 dy { p1->y - p0->y };
            nvg__normalize(dx, dy);
            if (lineCap == LineCap::Butt)
                dst = nvg__buttCapStart(dst, *p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (lineCap == LineCap::Butt || lineCap == LineCap::Square)
                dst = nvg__buttCapStart(dst, *p0, dx, dy, w, w - aa, aa, u0, u1);
            else if (lineCap == LineCap::Round)
                dst = nvg__roundCapStart(dst, *p0, dx, dy, w, ncap, u0, u1);
        }

        for (isize j { s }; j < e; ++j) {
            if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0) {
                if (lineJoin == LineJoin::Round) {
                    dst = nvg__roundJoin(dst, *p0, *p1, w, w, u0, u1, ncap);
                } else {
                    dst = nvg__bevelJoin(dst, *p0, *p1, w, w, u0, u1);
                }
            } else {
                nvg__vset(dst, p1->x + (p1->dmx * w), p1->y + (p1->dmy * w), u0, 1);
                dst++;
                nvg__vset(dst, p1->x - (p1->dmx * w), p1->y - (p1->dmy * w), u1, 1);
                dst++;
            }
            p0 = p1++;
        }

        if (loop) {
            // Loop it
            nvg__vset(dst, verts[0].Position[0], verts[0].Position[1], u0, 1);
            dst++;
            nvg__vset(dst, verts[1].Position[0], verts[1].Position[1], u1, 1);
            dst++;
        } else {
            // Add cap
            f32 dx { p1->x - p0->x };
            f32 dy { p1->y - p0->y };
            nvg__normalize(dx, dy);
            if (lineCap == LineCap::Butt)
                dst = nvg__buttCapEnd(dst, *p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
            else if (lineCap == LineCap::Butt || lineCap == LineCap::Square)
                dst = nvg__buttCapEnd(dst, *p1, dx, dy, w, w - aa, aa, u0, u1);
            else if (lineCap == LineCap::Round)
                dst = nvg__roundCapEnd(dst, *p1, dx, dy, w, ncap, u0, u1);
        }

        path.nstroke = static_cast<i32>(dst - verts);

        verts = dst;
    }
}

void Canvas::expand_fill(f32 w, LineJoin lineJoin, f32 miterLimit)
{
    Vertex* dst { nullptr };
    f32 aa { _fringeWidth };
    bool fringe { w > 0.0f };

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    isize cverts { 0 };
    for (auto& path : _cache.paths) {
        cverts += path.count + path.nbevel + 1;
        if (fringe)
            cverts += (path.count + path.nbevel * 5 + 1) * 2; // plus one for loop
    }

    Vertex* verts { alloc_temp_verts(cverts) };
    if (verts == nullptr)
        return;

    bool convex { _cache.paths.size() == 1 && _cache.paths[0].convex };

    for (auto& path : _cache.paths) {
        NVGpoint* pts { &_cache.points[path.first] };
        NVGpoint* p0 { nullptr };
        NVGpoint* p1 { nullptr };

        // Calculate shape vertices.
        f32 woff { 0.5f * aa };
        dst = verts;
        path.fill = dst;

        if (fringe) {
            // Looping
            p0 = &pts[path.count - 1];
            p1 = &pts[0];
            for (u32 j { 0 }; j < path.count; ++j) { //|here
                if (p1->flags & NVG_PT_BEVEL) {
                    f32 dlx0 { p0->dy };
                    f32 dly0 { -p0->dx };
                    f32 dlx1 { p1->dy };
                    f32 dly1 { -p1->dx };
                    if (p1->flags & NVG_PT_LEFT) {
                        f32 lx { p1->x + p1->dmx * woff };
                        f32 ly { p1->y + p1->dmy * woff };
                        nvg__vset(dst, lx, ly, 0.5f, 1);
                        dst++;
                    } else {
                        f32 lx0 { p1->x + dlx0 * woff };
                        f32 ly0 { p1->y + dly0 * woff };
                        f32 lx1 { p1->x + dlx1 * woff };
                        f32 ly1 { p1->y + dly1 * woff };
                        nvg__vset(dst, lx0, ly0, 0.5f, 1);
                        dst++;
                        nvg__vset(dst, lx1, ly1, 0.5f, 1);
                        dst++;
                    }
                } else {
                    nvg__vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff), 0.5f, 1);
                    dst++;
                }
                p0 = p1++;
            }
        } else {
            for (u32 j { 0 }; j < path.count; ++j) {
                nvg__vset(dst, pts[j].x, pts[j].y, 0.5f, 1);
                dst++;
            }
        }

        path.nfill = static_cast<i32>(dst - verts);
        verts = dst;

        // Calculate fringe
        if (fringe) {
            f32 lw { w + woff };
            f32 rw { w - woff };
            f32 lu { 0 };
            f32 ru { 1 };
            dst = verts;
            path.stroke = dst;

            // Create only half a fringe for convex shapes so that
            // the shape can be rendered without stenciling.
            if (convex) {
                lw = woff; // This should generate the same vertex as fill inset above.
                lu = 0.5f; // Set outline fade at middle.
            }

            // Looping
            p0 = &pts[path.count - 1];
            p1 = &pts[0];

            for (u32 j { 0 }; j < path.count; ++j) {
                if ((p1->flags & (NVG_PT_BEVEL | NVG_PR_INNERBEVEL)) != 0) {
                    dst = nvg__bevelJoin(dst, *p0, *p1, lw, rw, lu, ru);
                } else {
                    nvg__vset(dst, p1->x + (p1->dmx * lw), p1->y + (p1->dmy * lw), lu, 1);
                    dst++;
                    nvg__vset(dst, p1->x - (p1->dmx * rw), p1->y - (p1->dmy * rw), ru, 1);
                    dst++;
                }
                p0 = p1++;
            }

            // Loop it
            nvg__vset(dst, verts[0].Position[0], verts[0].Position[1], lu, 1);
            dst++;
            nvg__vset(dst, verts[1].Position[0], verts[1].Position[1], ru, 1);
            dst++;

            path.nstroke = static_cast<i32>(dst - verts);
            verts = dst;
        } else {
            path.stroke = nullptr;
            path.nstroke = 0;
        }
    }
}

void Canvas::render_text(Vertex* verts, i32 nverts)
{
    NVGstate& s { state() };
    CanvasPaint& paint { s.fill };

    // Render triangles.
    paint.image = _fonts[s.fontId]->texture();

    // Apply global alpha
    paint.gradient.multiply_alpha(s.alpha);
    paint.text_outline_color.A = static_cast<u8>(paint.text_outline_color.A * s.alpha);

    static gl::BlendFuncs comp {
        .SourceColorBlendFunc = gl::BlendFunc::SrcAlpha,
        .DestinationColorBlendFunc = gl::BlendFunc::OneMinusSrcAlpha,
        .SourceAlphaBlendFunc = gl::BlendFunc::SrcAlpha,
        .DestinationAlphaBlendFunc = gl::BlendFunc::OneMinusSrcAlpha
    };

    _glc->render_triangles(paint, comp, s.scissor, verts, nverts);

    _drawCallCount++;
    _textTriCount += nverts / 3;
}

auto Canvas::deg_to_rad(f32 deg) -> f32
{
    return deg / 360.0f * TAU_F;
}
}
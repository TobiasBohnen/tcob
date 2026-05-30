// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Canvas.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <span>
#include <variant>
#include <vector>

#include "Canvas_private.hpp"

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Color.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/Transform.hpp"
#include "tcob/core/assets/Asset.hpp"
#include "tcob/gfx/ColorGradient.hpp"
#include "tcob/gfx/Font.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Path2d.hpp"
#include "tcob/gfx/Polygon.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/TextFormatter.hpp"
#include "tcob/gfx/Texture.hpp"

namespace tcob::gfx {
using namespace detail;

constexpr f32 KAPPA90 {0.5522847493f}; // Length proportional to radius of a cubic bezier handle for 90deg arcs.;
constexpr f32 RAD90 {TAU_F / 4};

static auto SignF(f32 a) -> f32 { return a >= 0.0f ? 1.0f : -1.0f; }

static auto CompositeOperationState(composite_operation op) -> blend_funcs
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

static auto GetAverageScale(mat3 const& t) -> f32
{
    f32 const sx {std::sqrt((t[0] * t[0]) + (t[3] * t[3]))};
    f32 const sy {std::sqrt((t[1] * t[1]) + (t[4] * t[4]))};
    return (sx + sy) * 0.5f;
}

static void MultiplyAlphaPaint(paint_color& c, f32 alpha)
{
    if (auto* arg0 {std::get_if<color>(&c)}) {
        arg0->A = static_cast<u8>(arg0->A * alpha);
    } else if (auto* arg1 {std::get_if<paint_gradient>(&c)}) {
        arg1->first *= alpha;
    }
}

////////////////////////////////////////////////////////////

canvas::canvas()
    : _impl {locate_service<render_system>().create_canvas()}
    , _states {std::make_unique<states>()}
    , _cache {std::make_unique<path_cache>()}
{
    save();
    reset();

    set_device_pixel_ratio(1.0f);
}

canvas::~canvas() = default;

auto canvas::get_texture(i32 layer) -> asset_ptr<texture>
{
    return _rtt[layer];
}

void canvas::begin_frame(size_i windowSize, f32 devicePixelRatio, i32 layer, bool clear)
{
    _activeRtt  = layer;
    _windowSize = windowSize;

    auto& artt {_rtt[_activeRtt]};
    artt->Size = windowSize;
    artt->prepare_render({.ViewMatrix  = transform::Identity.as_matrix4(),
                          .ModelMatrix = transform::Identity.as_matrix4(),
                          .Viewport    = rect_i {point_i::Zero, windowSize}});
    if (clear) {
        artt->clear({0, 0, 0, 0});
    }

    _states->reset();

    save();
    reset();

    set_device_pixel_ratio(devicePixelRatio);
}

void canvas::end_frame()
{
    _impl->flush(size_f {_windowSize});
    _rtt[_activeRtt]->finalize_render();
}

void canvas::cancel_frame()
{
    _impl->cancel();
}

void canvas::save()
{
    _states->save();
}

void canvas::restore()
{
    _states->restore();
}

void canvas::reset()
{
    state& s {_states->get()};

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

    s.Scissor.Extent = {-1.0f, -1.0f};

    s.TextAlign = {};
    s.Dash      = {};

    s.Font = nullptr;
}

////////////////////////////////////////////////////////////

void canvas::begin_path()
{
    _cache->clear();
}

void canvas::close_path()
{
    _cache->append_commands(std::vector<f32> {Close}, _states->get().XForm);
}

void canvas::set_path_winding(winding dir)
{
    _cache->append_commands(std::vector<f32> {Winding, static_cast<f32>(dir)}, _states->get().XForm);
}

void canvas::set_path_winding(solidity s)
{
    _cache->append_commands(std::vector<f32> {Winding, static_cast<f32>(s)}, _states->get().XForm);
}

////////////////////////////////////////////////////////////

void canvas::move_to(point_f pos)
{
    _cache->append_commands(path2d::CommandsMoveTo(pos), _states->get().XForm);
}

void canvas::line_to(point_f pos)
{
    _cache->append_commands(path2d::CommandsLineTo(pos), _states->get().XForm);
}

////////////////////////////////////////////////////////////

void canvas::rect(rect_f const& rect)
{
    auto const [x, y] {rect.Position};
    auto const [w, h] {rect.Size};

    state& s {_states->get()};

    _cache->append_commands(std::vector<f32> {
                                MoveTo, x, y,
                                LineTo, x, y + h,
                                LineTo, x + w, y + h,
                                LineTo, x + w, y,
                                Close},
                            s.XForm);
}

void canvas::rounded_rect(rect_f const& r, f32 rad)
{
    rounded_rect_varying(r, rad, rad, rad, rad);
}

void canvas::rounded_rect_varying(rect_f const& rect, f32 radTL, f32 radTR, f32 radBR, f32 radBL)
{
    if (radTL < 0.1f && radTR < 0.1f && radBR < 0.1f && radBL < 0.1f) {
        this->rect(rect);
        return;
    }

    auto const [x, y] {rect.Position};
    auto const [w, h] {rect.Size};

    f32 const halfw {std::abs(w) * 0.5f};
    f32 const halfh {std::abs(h) * 0.5f};
    f32 const rxBL {std::min(radBL, halfw) * SignF(w)}, ryBL {std::min(radBL, halfh) * SignF(h)};
    f32 const rxBR {std::min(radBR, halfw) * SignF(w)}, ryBR {std::min(radBR, halfh) * SignF(h)};
    f32 const rxTR {std::min(radTR, halfw) * SignF(w)}, ryTR {std::min(radTR, halfh) * SignF(h)};
    f32 const rxTL {std::min(radTL, halfw) * SignF(w)}, ryTL {std::min(radTL, halfh) * SignF(h)};

    _cache->append_commands(std::vector<f32> {
                                MoveTo, x, y + ryTL,
                                LineTo, x, y + h - ryBL,
                                BezierTo, x, y + h - (ryBL * (1 - KAPPA90)), x + (rxBL * (1 - KAPPA90)), y + h, x + rxBL, y + h,
                                LineTo, x + w - rxBR, y + h,
                                BezierTo, x + w - (rxBR * (1 - KAPPA90)), y + h, x + w, y + h - (ryBR * (1 - KAPPA90)), x + w, y + h - ryBR,
                                LineTo, x + w, y + ryTR,
                                BezierTo, x + w, y + (ryTR * (1 - KAPPA90)), x + w - (rxTR * (1 - KAPPA90)), y, x + w - rxTR, y,
                                LineTo, x + rxTL, y,
                                BezierTo, x + (rxTL * (1 - KAPPA90)), y, x, y + (ryTL * (1 - KAPPA90)), x, y + ryTL,
                                Close},
                            _states->get().XForm);
}

void canvas::border_rect(rect_f const& outer,
                         f32 top, f32 right, f32 bottom, f32 left,
                         f32 radTL, f32 radTR, f32 radBR, f32 radBL)
{
    if (top <= 0 && right <= 0 && bottom <= 0 && left <= 0) { return; }

    auto const [ox, oy] {outer.Position};
    auto const [ow, oh] {outer.Size};

    bool const hasTop {top > 0};
    bool const hasRight {right > 0};
    bool const hasBottom {bottom > 0};
    bool const hasLeft {left > 0};

    auto static const clampRad {[](f32 const r, f32 const limit) -> f32 {
        return std::clamp(r, 0.f, limit);
    }};

    f32 const iTL {std::max(0.f, radTL - std::max(top, left))};    // NOLINT(readability-suspicious-call-argument)
    f32 const iTR {std::max(0.f, radTR - std::max(top, right))};
    f32 const iBR {std::max(0.f, radBR - std::max(bottom, right))};
    f32 const iBL {std::max(0.f, radBL - std::max(bottom, left))}; // NOLINT(readability-suspicious-call-argument)

    rect_f const inner {outer.left() + left, outer.top() + top,
                        outer.width() - left - right, outer.height() - top - bottom};

    auto const [ix, iy] {inner.Position};
    auto const [iw, ih] {inner.Size};

    f32 const halfw {std::abs(iw) * 0.5f};
    f32 const halfh {std::abs(ih) * 0.5f};

    f32 const rxoTL {clampRad(radTL, std::abs(ow) * 0.5f)};
    f32 const ryoTL {clampRad(radTL, std::abs(oh) * 0.5f)};
    f32 const rxoTR {clampRad(radTR, std::abs(ow) * 0.5f)};
    f32 const ryoTR {clampRad(radTR, std::abs(oh) * 0.5f)};
    f32 const rxoBR {clampRad(radBR, std::abs(ow) * 0.5f)};
    f32 const ryoBR {clampRad(radBR, std::abs(oh) * 0.5f)};
    f32 const rxoBL {clampRad(radBL, std::abs(ow) * 0.5f)};
    f32 const ryoBL {clampRad(radBL, std::abs(oh) * 0.5f)};

    f32 const rxiTL {clampRad(iTL, halfw)};
    f32 const ryiTL {clampRad(iTL, halfh)};
    f32 const rxiTR {clampRad(iTR, halfw)};
    f32 const ryiTR {clampRad(iTR, halfh)};
    f32 const rxiBR {clampRad(iBR, halfw)};
    f32 const ryiBR {clampRad(iBR, halfh)};
    f32 const rxiBL {clampRad(iBL, halfw)};
    f32 const ryiBL {clampRad(iBL, halfh)};

    // full border
    if (hasTop && hasRight && hasBottom && hasLeft) {
        _cache->append_commands(
            std::vector<f32> {
                MoveTo, ox, oy + ryoTL,
                Winding, static_cast<f32>(winding::CCW),
                LineTo, ox, oy + oh - ryoBL,
                BezierTo, ox, oy + oh - (ryoBL * (1 - KAPPA90)), ox + (rxoBL * (1 - KAPPA90)), oy + oh, ox + rxoBL, oy + oh,
                LineTo, ox + ow - rxoBR, oy + oh,
                BezierTo, ox + ow - (rxoBR * (1 - KAPPA90)), oy + oh, ox + ow, oy + oh - (ryoBR * (1 - KAPPA90)), ox + ow, oy + oh - ryoBR,
                LineTo, ox + ow, oy + ryoTR,
                BezierTo, ox + ow, oy + (ryoTR * (1 - KAPPA90)), ox + ow - (rxoTR * (1 - KAPPA90)), oy, ox + ow - rxoTR, oy,
                LineTo, ox + rxoTL, oy,
                BezierTo, ox + (rxoTL * (1 - KAPPA90)), oy, ox, oy + (ryoTL * (1 - KAPPA90)), ox, oy + ryoTL,
                Close,

                MoveTo, ix + rxiTL, iy,
                Winding, static_cast<f32>(winding::CW),
                LineTo, ix + iw - rxiTR, iy,
                BezierTo, ix + iw - (rxiTR * (1 - KAPPA90)), iy, ix + iw, iy + (ryiTR * (1 - KAPPA90)), ix + iw, iy + ryiTR,
                LineTo, ix + iw, iy + ih - ryiBR,
                BezierTo, ix + iw, iy + ih - (ryiBR * (1 - KAPPA90)), ix + iw - (rxiBR * (1 - KAPPA90)), iy + ih, ix + iw - rxiBR, iy + ih,
                LineTo, ix + rxiBL, iy + ih,
                BezierTo, ix + (rxiBL * (1 - KAPPA90)), iy + ih, ix, iy + ih - (ryiBL * (1 - KAPPA90)), ix, iy + ih - ryiBL,
                LineTo, ix, iy + ryiTL,
                BezierTo, ix, iy + (ryiTL * (1 - KAPPA90)), ix + (rxiTL * (1 - KAPPA90)), iy, ix + rxiTL, iy,
                Close},
            _states->get().XForm);

        return;
    }

    // partial border
    std::vector<f32> cmds;
    cmds.reserve(128);

    static std::array<i32, 4> const CCW_ORDER {3, 2, 1, 0}; // Left, Bottom, Right, Top
    std::array<bool, 4> const       has {hasTop, hasRight, hasBottom, hasLeft};

    std::array<i32, 4> chainStarts {};
    i32                numChains {0};

    for (i32 i {0}; i < 4; ++i) {
        i32 const curr {CCW_ORDER[i]};
        i32 const prev {CCW_ORDER[(i + 3) % 4]};
        if (has[curr] && !has[prev]) {
            chainStarts[numChains++] = i;
        }
    }

    auto const drawOutCorner {[&](i32 c) {
        switch (c) {
        case 3: cmds.insert(cmds.end(), {BezierTo, ox + (rxoTL * (1 - KAPPA90)), oy, ox, oy + (ryoTL * (1 - KAPPA90)), ox, oy + ryoTL}); break;
        case 2: cmds.insert(cmds.end(), {BezierTo, ox, oy + oh - (ryoBL * (1 - KAPPA90)), ox + (rxoBL * (1 - KAPPA90)), oy + oh, ox + rxoBL, oy + oh}); break;
        case 1: cmds.insert(cmds.end(), {BezierTo, ox + ow - (rxoBR * (1 - KAPPA90)), oy + oh, ox + ow, oy + oh - (ryoBR * (1 - KAPPA90)), ox + ow, oy + oh - ryoBR}); break;
        case 0: cmds.insert(cmds.end(), {BezierTo, ox + ow, oy + (ryoTR * (1 - KAPPA90)), ox + ow - (rxoTR * (1 - KAPPA90)), oy, ox + ow - rxoTR, oy}); break;
        }
    }};

    auto const drawOutSide {[&](i32 s) {
        switch (s) {
        case 3: cmds.insert(cmds.end(), {LineTo, ox, oy + oh - ryoBL}); break;
        case 2: cmds.insert(cmds.end(), {LineTo, ox + ow - rxoBR, oy + oh}); break;
        case 1: cmds.insert(cmds.end(), {LineTo, ox + ow, oy + ryoTR}); break;
        case 0: cmds.insert(cmds.end(), {LineTo, ox + rxoTL, oy}); break;
        }
    }};

    auto const drawInSide {[&](i32 s) {
        switch (s) {
        case 0: cmds.insert(cmds.end(), {LineTo, ix + iw - rxiTR, iy}); break;
        case 1: cmds.insert(cmds.end(), {LineTo, ix + iw, iy + ih - ryiBR}); break;
        case 2: cmds.insert(cmds.end(), {LineTo, ix + rxiBL, iy + ih}); break;
        case 3: cmds.insert(cmds.end(), {LineTo, ix, iy + ryiTL}); break;
        }
    }};

    auto const drawInCorner {[&](i32 c) {
        switch (c) {
        case 0: cmds.insert(cmds.end(), {BezierTo, ix + iw - (rxiTR * (1 - KAPPA90)), iy, ix + iw, iy + (ryiTR * (1 - KAPPA90)), ix + iw, iy + ryiTR}); break;
        case 1: cmds.insert(cmds.end(), {BezierTo, ix + iw, iy + ih - (ryiBR * (1 - KAPPA90)), ix + iw - (rxiBR * (1 - KAPPA90)), iy + ih, ix + iw - rxiBR, iy + ih}); break;
        case 2: cmds.insert(cmds.end(), {BezierTo, ix + (rxiBL * (1 - KAPPA90)), iy + ih, ix, iy + ih - (ryiBL * (1 - KAPPA90)), ix, iy + ih - ryiBL}); break;
        case 3: cmds.insert(cmds.end(), {BezierTo, ix, iy + (ryiTL * (1 - KAPPA90)), ix + (rxiTL * (1 - KAPPA90)), iy, ix + rxiTL, iy}); break;
        }
    }};

    for (i32 c {0}; c < numChains; ++c) {
        i32 const          startIdx {chainStarts[c]};
        std::array<i32, 4> ccwChain {};
        i32                chainLen {0};

        for (i32 k {0}; k < 4; ++k) {
            i32 side {CCW_ORDER[(startIdx + k) % 4]};
            if (!has[side]) { break; }
            ccwChain[chainLen++] = side;
        }

        // Outer Path
        i32 const firstOutCorner {ccwChain[0]};
        f32       sx {0}, sy {0};
        switch (firstOutCorner) {
        case 3:
            sx = ox + rxoTL;
            sy = oy;
            break;
        case 2:
            sx = ox;
            sy = oy + oh - ryoBL;
            break;
        case 1:
            sx = ox + ow - rxoBR;
            sy = oy + oh;
            break;
        case 0:
            sx = ox + ow;
            sy = oy + ryoTR;
            break;
        }
        cmds.insert(cmds.end(), {MoveTo, sx, sy});
        drawOutCorner(firstOutCorner);

        for (i32 i {0}; i < chainLen; ++i) {
            i32 side {ccwChain[i]};
            drawOutSide(side);
            drawOutCorner((side + 3) % 4);
        }

        // Inner Path (Cap directly to inner starting point and trace backward)
        i32 const lastSide {ccwChain[chainLen - 1]};
        i32 const lastCorner {(lastSide + 3) % 4};
        f32       capX {0}, capY {0};
        switch (lastCorner) {
        case 0:
            capX = ix + iw - rxiTR;
            capY = iy;
            break;
        case 1:
            capX = ix + iw;
            capY = iy + ih - ryiBR;
            break;
        case 2:
            capX = ix + rxiBL;
            capY = iy + ih;
            break;
        case 3:
            capX = ix;
            capY = iy + ryiTL;
            break;
        }
        cmds.insert(cmds.end(), {LineTo, capX, capY});

        for (i32 i {chainLen - 1}; i >= 0; --i) {
            i32 const s {ccwChain[i]};
            if (i == chainLen - 1) {
                drawInCorner((s + 3) % 4);
            }
            drawInSide(s);
            drawInCorner(s);
        }
        cmds.push_back(Close);
    }

    _cache->append_commands(cmds, _states->get().XForm);
}

////////////////////////////////////////////////////////////

void canvas::ellipse(point_f c, f32 rx, f32 ry)
{
    auto const [cx, cy] {c};

    _cache->append_commands(std::vector<f32> {
                                MoveTo, cx - rx, cy,
                                BezierTo, cx - rx, cy + (ry * KAPPA90), cx - (rx * KAPPA90), cy + ry, cx, cy + ry,
                                BezierTo, cx + (rx * KAPPA90), cy + ry, cx + rx, cy + (ry * KAPPA90), cx + rx, cy,
                                BezierTo, cx + rx, cy - (ry * KAPPA90), cx + (rx * KAPPA90), cy - ry, cx, cy - ry,
                                BezierTo, cx - (rx * KAPPA90), cy - ry, cx - rx, cy - (ry * KAPPA90), cx - rx, cy,
                                Close},
                            _states->get().XForm);
}

void canvas::circle(point_f c, f32 r)
{
    ellipse(c, r, r);
}

////////////////////////////////////////////////////////////

void canvas::cubic_bezier_to(point_f cp0, point_f cp1, point_f end)
{
    _cache->append_commands(path2d::CommandsCubicTo(cp0, cp1, end), _states->get().XForm);
}

void canvas::quad_bezier_to(point_f cp, point_f end)
{
    _cache->append_commands(path2d::CommandsQuadTo(_cache->command_point(), cp, end), _states->get().XForm);
}

////////////////////////////////////////////////////////////

void canvas::arc(point_f const c, f32 const r, radian_f const startAngle, radian_f const endAngle, winding const dir)
{
    i32 const move {_cache->has_commands() ? LineTo : MoveTo};
    f32 const a0 {startAngle.Value - RAD90};
    f32 const a1 {endAngle.Value - RAD90};

    // Normalize angles.
    f32 da {a1 - a0};
    if (dir == winding::CW) {
        if (std::abs(da) >= TAU_F) {
            da = TAU_F;
        } else {
            while (da < 0.0f) { da += TAU_F; }
        }
    } else { // CCW
        if (std::abs(da) >= TAU_F) {
            da = -TAU_F;
        } else {
            while (da > 0.0f) { da -= TAU_F; }
        }
    }

    std::vector<f32> vals;
    vals.reserve(138);
    i32 const ndivs {std::max(1, std::min(static_cast<i32>((std::abs(da) / (TAU_F * 0.25f)) + 0.5f), 5))};
    f32 const hda {(da / static_cast<f32>(ndivs)) / 2.0f};
    f32       kappa {std::abs(4.0f / 3.0f * (1.0f - std::cos(hda)) / std::sin(hda))};
    if (dir == winding::CCW) { kappa = -kappa; }

    f32 px {0.0f}, py {0.0f}, ptanx {0.0f}, ptany {0.0f};
    for (i32 i {0}; i <= ndivs; ++i) {
        f32 const a {a0 + (da * (i / static_cast<f32>(ndivs)))};
        f32 const dx {std::cos(a)};
        f32 const dy {std::sin(a)};
        f32 const x {c.X + (dx * r)};
        f32 const y {c.Y + (dy * r)};
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

    _cache->append_commands(vals, _states->get().XForm);
}

void canvas::arc_to(point_f pos1, point_f pos2, f32 radius)
{
    if (!_cache->has_commands()) { return; }

    winding dir {};

    // Handle degenerate cases.
    if (_cache->is_degenerate_arc(pos1, pos2, radius)) {
        line_to(pos1);
        return;
    }

    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    f32 dx0 {_cache->command_point().X - pos1.X};
    f32 dy0 {_cache->command_point().Y - pos1.Y};
    f32 dx1 {pos2.X - pos1.X};
    f32 dy1 {pos2.Y - pos1.Y};
    Normalize(dx0, dy0);
    Normalize(dx1, dy1);
    f32 const a {std::acos((dx0 * dx1) + (dy0 * dy1))};
    f32 const d {radius / std::tan(a / 2.0f)};

    if (d > 10000.0f) {
        line_to(pos1);
        return;
    }

    f32      cx {0}, cy {0};
    radian_f a0 {0}, a1 {0};

    static auto const cross {[](f32 x0, f32 y0, f32 x1, f32 y1) -> f32 { return (x1 * y0) - (x0 * y1); }};

    if (cross(dx0, dy0, dx1, dy1) > 0.0f) {
        cx  = pos1.X + (dx0 * d) + (dy0 * radius);
        cy  = pos1.Y + (dy0 * d) + (-dx0 * radius);
        a0  = radian_f {std::atan2(dx0, -dy0) + RAD90};
        a1  = radian_f {std::atan2(-dx1, dy1) + RAD90};
        dir = winding::CW;
    } else {
        cx  = pos1.X + (dx0 * d) + (-dy0 * radius);
        cy  = pos1.Y + (dy0 * d) + (dx0 * radius);
        a0  = radian_f {std::atan2(-dx0, dy0) + RAD90};
        a1  = radian_f {std::atan2(dx1, -dy1) + RAD90};
        dir = winding::CCW;
    }

    arc({cx, cy}, radius, a0, a1, dir);
}

////////////////////////////////////////////////////////////

void canvas::set_line_dash(std::span<f32 const> dashPattern)
{
    _states->get().Dash = {dashPattern.begin(), dashPattern.end()};
}

void canvas::set_dash_offset(f32 offset)
{
    _states->get().DashOffset = offset;
}

////////////////////////////////////////////////////////////

void canvas::fill_polyline(polyline_span points)
{
    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        _cache->append_commands(path2d::CommandsLineTo(points[i]), _states->get().XForm);
    }

    fill();
}

void canvas::stroke_polyline(polyline_span points)
{
    if (points.size() == 1) { return; }

    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        line_to(points[i]);
    }

    stroke();
}

void canvas::stroke_line(point_f from, point_f to)
{
    begin_path();

    move_to(from);
    line_to(to);

    stroke();
}

void canvas::wavy_line_to(point_f to, f32 amp, f32 freq, f32 phase)
{
    state const& s {_states->get()};

    point_f const from {_cache->command_point()};

    point_f const d {to - from};
    f32 const     l {static_cast<f32>(d.length())};
    if (l < EPSILON) { return; }

    point_f const unit {d.as_normalized()};
    point_f const perp {unit.as_perpendicular()};

    i32 const segCount {std::max(2, static_cast<i32>(l))};
    f32 const step {l / static_cast<f32>(segCount)};

    _cache->append_commands(path2d::CommandsMoveTo(from), s.XForm);

    for (i32 i {1}; i <= segCount; ++i) {
        f32 const sl {i * step}; // Arc length along the line.
        f32 const t {sl / l};    // Normalized parameter [0,1].

        point_f const base {from.X + (d.X * t), from.Y + (d.Y * t)};
        f32 const     offset {amp * std::sin((freq * sl) + phase)};
        point_f const finalPt {base.X + (offset * perp.X), base.Y + (offset * perp.Y)};

        _cache->append_commands(path2d::CommandsLineTo(finalPt), s.XForm);
    }
}

void canvas::regular_polygon(point_f pos, size_f size, i32 n)
{
    state const& s {_states->get()};

    auto const [x, y] {pos};
    _cache->append_commands(path2d::CommandsMoveTo({x, y - size.Height}), s.XForm);
    for (i32 i {1}; i < n; ++i) {
        f32 const angle {TAU_F / n * i};
        f32 const dx {std::sin(angle) * size.Width};
        f32 const dy {-std::cos(angle) * size.Height};
        _cache->append_commands(path2d::CommandsLineTo({x + dx, y + dy}), s.XForm);
    }
    _cache->append_commands(path2d::CommandsLineTo({x, y - size.Height}), s.XForm);
}

void canvas::star(point_f pos, f32 outerR, f32 innerR, i32 n)
{
    state const& s {_states->get()};

    auto const [x, y] {pos};
    _cache->append_commands(path2d::CommandsMoveTo({x, y - outerR}), s.XForm);
    for (i32 i {1}; i < n * 2; ++i) {
        f32 const angle {(TAU_F / 2) / n * i};
        f32 const r {(i % 2 == 0) ? outerR : innerR};
        f32 const dx {std::sin(angle) * r};
        f32 const dy {-std::cos(angle) * r};
        _cache->append_commands(path2d::CommandsLineTo({x + dx, y + dy}), s.XForm);
    }
    _cache->append_commands(path2d::CommandsLineTo({x, y - outerR}), s.XForm);
}

void canvas::triangle(point_f a, point_f b, point_f c)
{
    state const& s {_states->get()};

    _cache->append_commands(path2d::CommandsMoveTo(a), s.XForm);
    _cache->append_commands(path2d::CommandsLineTo(b), s.XForm);
    _cache->append_commands(path2d::CommandsLineTo(c), s.XForm);
    _cache->append_commands(path2d::CommandsLineTo(a), s.XForm);
}

auto canvas::path_2d(path2d const& path) -> void
{
    begin_path();
    _cache->append_commands(path.Commands, _states->get().XForm);
}

////////////////////////////////////////////////////////////

void canvas::set_fill_style(color c)
{
    set_paint_color(_states->get().Fill, c);
}

void canvas::set_fill_style(paint const& paint)
{
    state& s {_states->get()};
    s.Fill       = paint;
    s.Fill.XForm = s.XForm * s.Fill.XForm;
}

void canvas::set_stroke_style(color c)
{
    set_paint_color(_states->get().Stroke, c);
}

void canvas::set_stroke_style(paint const& paint)
{
    state& s {_states->get()};
    s.Stroke       = paint;
    s.Stroke.XForm = s.XForm * s.Stroke.XForm;
}

void canvas::set_shape_antialias(bool enabled)
{
    _states->get().ShapeAntiAlias = enabled;
}

void canvas::set_miter_limit(f32 limit)
{
    _states->get().MiterLimit = limit;
}

void canvas::set_stroke_width(f32 size)
{
    _states->get().StrokeWidth = size;
}

void canvas::set_edge_antialias(bool enabled)
{
    _edgeAntiAlias = enabled;
}

void canvas::set_line_cap(line_cap cap)
{
    _states->get().LineCap = cap;
}

void canvas::set_line_join(line_join join)
{
    _states->get().LineJoin = join;
}

void canvas::set_global_alpha(f32 alpha)
{
    _states->get().Alpha = alpha;
}

void canvas::fill(bool enforceWinding)
{
    state const& s {_states->get()};
    paint        fillPaint {s.Fill}; // copy

    _cache->fill(s, enforceWinding, _edgeAntiAlias, _fringeWidth);

    // Apply global alpha
    MultiplyAlphaPaint(fillPaint.Color, s.Alpha);

    _impl->render_fill(fillPaint, s.CompositeOperation, s.Scissor, _fringeWidth, _cache->bounds(), _cache->paths());
}

void canvas::stroke()
{
    state const& s {_states->get()};
    f32 const    scale {GetAverageScale(s.XForm.Matrix)};
    f32          strokeWidth {std::clamp(s.StrokeWidth * scale, 0.0f, 200.0f)};
    paint        strokePaint {s.Stroke}; // copy

    if (strokeWidth < _fringeWidth) {
        // If the stroke width is less than pixel size, use alpha to emulate coverage.
        // Since coverage is area, scale by alpha*alpha.
        f32 const alpha {std::clamp(strokeWidth / _fringeWidth, 0.0f, 1.0f)};

        MultiplyAlphaPaint(strokePaint.Color, alpha * alpha);
        strokeWidth = _fringeWidth;
    }

    _cache->stroke(s, true, _edgeAntiAlias, strokeWidth, _fringeWidth);

    // Apply global alpha
    MultiplyAlphaPaint(strokePaint.Color, s.Alpha);

    _impl->render_stroke(strokePaint, s.CompositeOperation, s.Scissor, _fringeWidth, strokeWidth, _cache->paths());
}

void canvas::clip(bool enforceWinding)
{
    state const& s {_states->get()};
    _cache->clip(enforceWinding, _fringeWidth);

    _impl->render_clip(s.Scissor, _fringeWidth, _cache->paths());
}

void canvas::reset_clip()
{
    _impl->render_clip({}, 0, {});
}

void canvas::clear(color color)
{
    state&     s {_states->get()};
    auto const oldCO {s.CompositeOperation};
    auto const oldFill {s.Fill};

    set_global_composite_blendfunc(blend_func::One, blend_func::Zero);
    set_fill_style(color);
    fill();

    s.CompositeOperation = oldCO;
    s.Fill               = oldFill;
}

////////////////////////////////////////////////////////////

auto canvas::create_linear_gradient(point_f s, point_f e, color_gradient const& gradient) -> paint
{
    f32 const large {1e5};

    // Calculate transform aligned to the line
    f32       dx {e.X - s.X};
    f32       dy {e.Y - s.Y};
    f32 const d {std::sqrt((dx * dx) + (dy * dy))};
    if (d > 0.0001f) {
        dx /= d;
        dy /= d;
    } else {
        dx = 0;
        dy = 1;
    }

    return {
        .XForm   = transform::FromRows(dy, dx, s.X - (dx * large),
                                       -dx, dy, s.Y - (dy * large),
                                       0, 0, 1),
        .Extent  = {large, large + (d * 0.5f)},
        .Radius  = 0.0f,
        .Feather = std::max(1.0f, d),
        .Color   = create_gradient(gradient)};
}

auto canvas::create_box_gradient(rect_f const& rect, f32 r, f32 f, color_gradient const& gradient) -> paint
{
    auto const& c {rect.center()};
    return {
        .XForm   = transform::FromRows(1.0f, 0.0f, c.X,
                                       0.0f, 1.0f, c.Y,
                                       0.0f, 0.0f, 1.0f),
        .Extent  = {rect.width() * 0.5f, rect.height() * 0.5f},
        .Radius  = r,
        .Feather = std::max(1.0f, f),
        .Color   = create_gradient(gradient)};
}

auto canvas::create_radial_gradient(point_f c, f32 inr, f32 outr, color_gradient const& gradient) -> paint
{
    return create_radial_gradient(c, inr, outr, size_f::One, gradient);
}

auto canvas::create_radial_gradient(point_f c, f32 inr, f32 outr, size_f scale, color_gradient const& gradient) -> paint
{
    f32 const r {(inr + outr) * 0.5f};
    f32 const f {(outr - inr)};

    return {
        .XForm   = transform::FromRows(scale.Width, 0.0f, c.X,
                                       0.0f, scale.Height, c.Y,
                                       0.0f, 0.0f, 1.0f),
        .Extent  = {r, r},
        .Radius  = r,
        .Feather = std::max(1.0f, f),
        .Color   = create_gradient(gradient)};
}

auto canvas::create_conic_gradient(point_f center, color_gradient const& gradient) -> paint
{
    return {
        .XForm     = transform::FromRows(1.0f, 0.0f, center.X,
                                         0.0f, 1.0f, center.Y,
                                         0.0f, 0.0f, 1.0f),
        .Extent    = {1.0f, 1.0f},
        .Radius    = 0.0f,
        .Feather   = 1.0f,
        .Color     = create_gradient(gradient),
        .IsConical = true};
}

auto canvas::create_gradient(color_gradient const& gradient) -> paint_color
{
    if (auto color {gradient.solid_color()}) {
        return *color;
    }

    for (usize i {0}; i < _gradients.size(); ++i) {
        if (_gradients[i] == gradient) { return paint_gradient {1.0f, static_cast<i32>(i)}; }
    }

    i32 const newIdx {static_cast<i32>(_gradients.size())};
    _gradients.push_back(gradient);
    _impl->add_gradient(newIdx, gradient);

    return paint_gradient {1.0f, newIdx};
}

auto canvas::create_image_pattern(point_f c, size_f e, degree_f angle, texture* image, f32 alpha) -> paint
{
    paint p {.Extent = {e.Width, e.Height},
             .Color  = color {255, 255, 255, static_cast<u8>(255 * alpha)},
             .Image  = image};

    p.XForm.rotate(angle);
    p.XForm.translate(c);

    return p;
}

////////////////////////////////////////////////////////////

void canvas::set_global_composite_operation(composite_operation op)
{
    _states->get().CompositeOperation = CompositeOperationState(op);
}

void canvas::set_global_composite_blendfunc(blend_func sfactor, blend_func dfactor)
{
    set_global_composite_blendfunc_separate(sfactor, dfactor, sfactor, dfactor);
}

void canvas::set_global_composite_blendfunc_separate(blend_func srcRGB, blend_func dstRGB, blend_func srcAlpha, blend_func dstAlpha)
{
    _states->get().CompositeOperation = {.SourceColorBlendFunc      = srcRGB,
                                         .DestinationColorBlendFunc = dstRGB,
                                         .SourceAlphaBlendFunc      = srcAlpha,
                                         .DestinationAlphaBlendFunc = dstAlpha};
}

////////////////////////////////////////////////////////////

void canvas::translate(point_f c)
{
    _states->get().XForm.translate(c);
}

void canvas::rotate(degree_f angle)
{
    _states->get().XForm.rotate(angle);
}

void canvas::rotate_at(degree_f angle, point_f p)
{
    translate(p);
    rotate(angle);
    translate(-p);
}

void canvas::scale(size_f scale)
{
    _states->get().XForm.scale(scale);
}

void canvas::scale_at(size_f sc, point_f p)
{
    translate(p);
    scale(sc);
    translate(-p);
}

void canvas::skew(degree_f angleX, degree_f angleY)
{
    _states->get().XForm.skew({angleX, angleY});
}

void canvas::skew_at(degree_f angleX, degree_f angleY, point_f p)
{
    translate(p);
    skew(angleX, angleY);
    translate(-p);
}

void canvas::set_transform(transform const& xform)
{
    _states->get().XForm = xform;
}

void canvas::reset_transform()
{
    _states->get().XForm = transform::Identity;
}

////////////////////////////////////////////////////////////

void canvas::set_scissor(rect_f const& rect, bool transform)
{
    state& s {_states->get()};

    auto [x, y] {rect.Position};
    auto [w, h] {rect.Size};
    w = std::max(0.0f, w);
    h = std::max(0.0f, h);

    s.Scissor.XForm = transform::Identity;

    s.Scissor.XForm.translate({x + (w * 0.5f), y + (h * 0.5f)});
    if (transform) { s.Scissor.XForm = s.XForm * s.Scissor.XForm; }

    s.Scissor.Extent = {w * 0.5f, h * 0.5f};
}

void canvas::reset_scissor()
{
    state& s {_states->get()};
    s.Scissor.XForm  = transform::FromRows(0, 0, 0, 0, 0, 0, 0, 0, 0);
    s.Scissor.Extent = {-1.0f, -1.0f};
}

void canvas::set_font(font* font)
{
    _states->get().Font = font;
}

////////////////////////////////////////////////////////////

void canvas::draw_text(rect_f const& rect, utf8_string_view text)
{
    draw_text(rect.Position, format_text(rect.Size, text));
}

void canvas::draw_text(point_f offset, text_formatter::result const& formatResult)
{
    state const& s {_states->get()};
    if (!s.Font) { return; }

    f32 const           scale {get_font_scale() * _devicePxRatio};
    f32 const           invscale {1.0f / scale};
    std::vector<vertex> verts(formatResult.QuadCount * 6);
    usize               nverts {0};

    auto const [x, y] {offset};
    for (auto const& token : formatResult.Tokens) {
        for (usize i {0}; i < token.Quads.size(); ++i) {
            auto const& quad {token.Quads[i]};
            auto const& posRect {quad.Rect};

            auto const& uvRect {quad.TextureRegion.UVRect};
            f32 const   uvLeft {uvRect.left()};
            f32 const   uvRight {uvRect.right()};
            f32 const   uvTop {uvRect.top()};
            f32 const   uvBottom {uvRect.bottom()};

            f32 const level {static_cast<f32>(quad.TextureRegion.Level)};

            point_f const posTL {s.XForm * point_f {(posRect.left() * invscale) + x, (posRect.top() * invscale) + y}};
            point_f const posTR {s.XForm * point_f {(posRect.right() * invscale) + x, (posRect.top() * invscale) + y}};
            point_f const posBR {s.XForm * point_f {(posRect.right() * invscale) + x, (posRect.bottom() * invscale) + y}};
            point_f const posBL {s.XForm * point_f {(posRect.left() * invscale) + x, (posRect.bottom() * invscale) + y}};

            point_f const topLeft {std::round(posTL.X), std::round(posTL.Y)};
            point_f const topRight {std::round(posTR.X), std::round(posTR.Y)};
            point_f const bottomRight {std::round(posBR.X), std::round(posBR.Y)};
            point_f const bottomLeft {std::round(posBL.X), std::round(posBL.Y)};

            verts[nverts++] = vertex {.Position = topLeft, .TexCoords = {.U = uvLeft, .V = uvTop, .Level = level}};
            verts[nverts++] = vertex {.Position = bottomRight, .TexCoords = {.U = uvRight, .V = uvBottom, .Level = level}};
            verts[nverts++] = vertex {.Position = topRight, .TexCoords = {.U = uvRight, .V = uvTop, .Level = level}};
            verts[nverts++] = vertex {.Position = topLeft, .TexCoords = {.U = uvLeft, .V = uvTop, .Level = level}};
            verts[nverts++] = vertex {.Position = bottomLeft, .TexCoords = {.U = uvLeft, .V = uvBottom, .Level = level}};
            verts[nverts++] = vertex {.Position = bottomRight, .TexCoords = {.U = uvRight, .V = uvBottom, .Level = level}};
        }
    }
    render_text(s.Font, {verts.data(), nverts});
}

void canvas::fill_text(rect_f const& rect, utf8_string_view text)
{
    f32 const oldFringe {_fringeWidth};
    _fringeWidth = -1;

    decompose_text(text, rect);
    fill(false);

    _fringeWidth = oldFringe;
}

void canvas::stroke_text(rect_f const& rect, utf8_string_view text)
{
    decompose_text(text, rect);
    stroke();
}

void canvas::decompose_text(utf8_string_view text, rect_f const& rect)
{
    begin_path();

    state const& s {_states->get()};
    if (!s.Font) { return; }

    decompose_callbacks cb {};
    cb.MoveTo  = [this, &cb](point_f p) { move_to(p + cb.Offset); };
    cb.LineTo  = [this, &cb](point_f p) { line_to(p + cb.Offset); };
    cb.ConicTo = [this, &cb](point_f p0, point_f p1) { quad_bezier_to(p0 + cb.Offset, p1 + cb.Offset); };
    cb.CubicTo = [this, &cb](point_f p0, point_f p1, point_f p2) { cubic_bezier_to(p0 + cb.Offset, p1 + cb.Offset, p2 + cb.Offset); };

    point_f     offset {rect.Position};
    auto const& align {s.TextAlign};
    if (align.Horizontal != horizontal_alignment::Left || align.Vertical != vertical_alignment::Top) {
        auto const textSize {measure_text(rect.height(), text) + size_f {s.StrokeWidth, s.StrokeWidth}};
        switch (align.Horizontal) {
        case horizontal_alignment::Left:   break;
        case horizontal_alignment::Right:  offset.X = rect.right() - textSize.Width; break;
        case horizontal_alignment::Center: offset.X = rect.left() + ((rect.width() - textSize.Width) / 2); break;
        }
        switch (align.Vertical) {
        case vertical_alignment::Top:    break;
        case vertical_alignment::Bottom: offset.Y = rect.bottom() - textSize.Height; break;
        case vertical_alignment::Middle: offset.Y = rect.top() + ((rect.height() - textSize.Height) / 2); break;
        }
    }

    cb.Offset = offset;

    s.Font->decompose_text(text, true, cb);
}

auto canvas::format_text(size_f const& size, utf8_string_view text, f32 scale) -> text_formatter::result
{
    state const& s {_states->get()};
    if (!s.Font) { return {}; }
    return text_formatter::format(text, *s.Font, s.TextAlign, size, scale, true);
}

auto canvas::measure_text(f32 height, utf8_string_view text) -> size_f
{
    state const& s {_states->get()};
    if (!s.Font) { return {}; }
    return text_formatter::measure(text, *s.Font, height, true);
}

void canvas::set_text_halign(horizontal_alignment align)
{
    _states->get().TextAlign.Horizontal = align;
}

void canvas::set_text_valign(vertical_alignment align)
{
    _states->get().TextAlign.Vertical = align;
}

////////////////////////////////////////////////////////////

void canvas::draw_image(texture* image, string const& region, rect_f const& rect)
{
    state const& s {_states->get()};
    paint        paint {s.Fill}; // copy

    // Render triangles.
    paint.Image = image;

    // Apply global alpha
    MultiplyAlphaPaint(paint.Color, s.Alpha);

    texture_region texRegion {paint.Image->regions()[region]};

    quad quad {};
    geometry::set_position(quad, rect, s.XForm);
    geometry::set_color(quad, colors::White);
    geometry::set_texcoords(quad, texRegion);

    for (auto& vert : quad) {
        vert.Position.X = std::floor(vert.Position.X + 0.5f);
        vert.Position.Y = std::floor(vert.Position.Y + 0.5f);
    }

    std::array<vertex, 6> const verts {quad[3], quad[1], quad[0], quad[3], quad[2], quad[1]};
    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, _fringeWidth, verts);
}

void canvas::draw_nine_patch(texture* image, string const& region, rect_f const& rect, rect_f const& center, rect_f const& localCenterUV)
{
    state const& s {_states->get()};
    auto         paint {s.Fill}; // copy

    // Render triangles.
    paint.Image = image;

    // Apply global alpha
    MultiplyAlphaPaint(paint.Color, s.Alpha);

    f32 const left {rect.left()};
    f32 const leftCenter {center.left()};
    f32 const rightCenter {center.right()};
    f32 const right {rect.right()};

    f32 const top {rect.top()};
    f32 const topCenter {center.top()};
    f32 const bottomCenter {center.bottom()};
    f32 const bottom {rect.bottom()};

    texture_region texRegion {paint.Image->regions()[region]};
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

    std::array<vertex, 9 * 6> verts;
    usize                     i {0};
    quad                      q;

    auto const emitQuad {[&](rect_f const& pos, rect_f const& uv) {
        geometry::set_position(q, pos, s.XForm);
        for (auto& vert : q) { vert.Position = {std::floor(vert.Position.X + 0.5f), std::floor(vert.Position.Y + 0.5f)}; }
        geometry::set_color(q, colors::White);
        geometry::set_texcoords(q, {.UVRect = uv, .Level = level});

        for (usize j {0}; j < 6; ++j) { verts[i++] = q[QuadIndicies[j]]; }
    }};

    emitQuad(rect_f::FromLTRB(left, top, leftCenter, topCenter), rect_f::FromLTRB(uv_left, uv_top, uv_leftCenter, uv_topCenter));
    emitQuad(rect_f::FromLTRB(leftCenter, top, rightCenter, topCenter), rect_f::FromLTRB(uv_leftCenter, uv_top, uv_rightCenter, uv_topCenter));
    emitQuad(rect_f::FromLTRB(rightCenter, top, right, topCenter), rect_f::FromLTRB(uv_rightCenter, uv_top, uv_right, uv_topCenter));

    emitQuad(rect_f::FromLTRB(left, topCenter, leftCenter, bottomCenter), rect_f::FromLTRB(uv_left, uv_topCenter, uv_leftCenter, uv_bottomCenter));
    emitQuad(rect_f::FromLTRB(leftCenter, topCenter, rightCenter, bottomCenter), rect_f::FromLTRB(uv_leftCenter, uv_topCenter, uv_rightCenter, uv_bottomCenter));
    emitQuad(rect_f::FromLTRB(rightCenter, topCenter, right, bottomCenter), rect_f::FromLTRB(uv_rightCenter, uv_topCenter, uv_right, uv_bottomCenter));

    emitQuad(rect_f::FromLTRB(left, bottomCenter, leftCenter, bottom), rect_f::FromLTRB(uv_left, uv_bottomCenter, uv_leftCenter, uv_bottom));
    emitQuad(rect_f::FromLTRB(leftCenter, bottomCenter, rightCenter, bottom), rect_f::FromLTRB(uv_leftCenter, uv_bottomCenter, uv_rightCenter, uv_bottom));
    emitQuad(rect_f::FromLTRB(rightCenter, bottomCenter, right, bottom), rect_f::FromLTRB(uv_rightCenter, uv_bottomCenter, uv_right, uv_bottom));

    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, _fringeWidth, verts);
}

////////////////////////////////////////////////////////////

auto canvas::get_impl() const -> render_backend::canvas_base*
{
    return _impl.get();
}

void canvas::set_device_pixel_ratio(f32 ratio)
{
    _cache->set_tolerances(0.01f / ratio, 0.25f / ratio);

    _fringeWidth   = 1.0f / ratio;
    _devicePxRatio = ratio;
}

////////////////////////////////////////////////////////////

void canvas::set_paint_color(paint& p, color c)
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
    static auto Quantize {[](f32 a, f32 d) -> f32 {
        return (static_cast<f32>(static_cast<i32>((a / d) + 0.5f))) * d;
    }};

    return std::min(Quantize(GetAverageScale(_states->get().XForm.Matrix), 0.01f), 4.0f);
}

void canvas::render_text(font* font, std::span<vertex const> verts)
{
    state const& s {_states->get()};
    paint        paint {s.Fill};

    // Render triangles
    paint.Image = font->texture().ptr();

    // Apply global alpha
    MultiplyAlphaPaint(paint.Color, s.Alpha);

    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, _fringeWidth, verts);
}

////////////////////////////////////////////////////////////

scoped_canvas_state::scoped_canvas_state(canvas& c)
    : _canvas {c}
{
    c.save();
}

scoped_canvas_state::~scoped_canvas_state()
{
    _canvas.restore();
}

////////////////////////////////////////////////////////////
}

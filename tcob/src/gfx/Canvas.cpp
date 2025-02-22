// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Canvas.hpp"

#include <algorithm>
#include <cmath>

#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/Size.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/easing/Easing.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/RenderSystem.hpp"
#include "tcob/gfx/RenderSystemImpl.hpp"
#include "tcob/gfx/TextFormatter.hpp"

#include "Canvas_types.hpp"

namespace tcob::gfx {
using namespace detail;

auto constexpr NVG_KAPPA90 {0.5522847493f}; // Length proportional to radius of a cubic bezier handle for 90deg arcs.;

auto static signf(f32 a) -> f32 { return a >= 0.0f ? 1.0f : -1.0f; }

auto static CompositeOperationState(composite_operation op) -> blend_funcs
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

auto static DistancePointSegment(f32 x, f32 y, f32 px, f32 py, f32 qx, f32 qy) -> f32
{
    f32 const pqx {qx - px};
    f32 const pqy {qy - py};
    f32       dx {x - px};
    f32       dy {y - py};
    f32 const d {(pqx * pqx) + (pqy * pqy)};
    f32       t {(pqx * dx) + (pqy * dy)};
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
    return (dx * dx) + (dy * dy);
}

auto static GetAverageScale(mat3 const& t) -> f32
{
    f32 const sx {std::sqrt((t[0] * t[0]) + (t[3] * t[3]))};
    f32 const sy {std::sqrt((t[1] * t[1]) + (t[4] * t[4]))};
    return (sx + sy) * 0.5f;
}

auto static Rect(f32 x, f32 y, f32 w, f32 h, f32 t) -> point_f
{
    f32 const perimeter {2 * (w + h)};
    f32       dist {t * perimeter};

    // Top edge
    if (dist < w) { return {x + dist, y}; }
    dist -= w;

    // Right edge
    if (dist < h) { return {x + w, y + dist}; }
    dist -= h;

    // Bottom edge
    if (dist < w) { return {x + w - dist, y + h}; }
    dist -= w;

    // Left edge
    return {x, y + h - dist};
}

auto static QuadBezierLength(point_f p0, point_f p1, point_f p2) -> f32
{
    constexpr i32 numSamples {10};
    f32           length {0.0f};
    point_f       prev {p0};

    for (i32 i {1}; i <= numSamples; ++i) {
        f32 const     sampleT {static_cast<f32>(i) / numSamples};
        point_f const current {(1 - sampleT) * (1 - sampleT) * p0.X + 2 * (1 - sampleT) * sampleT * p1.X + sampleT * sampleT * p2.X,
                               (1 - sampleT) * (1 - sampleT) * p0.Y + 2 * (1 - sampleT) * sampleT * p1.Y + sampleT * sampleT * p2.Y};
        length += std::hypot(current.X - prev.X, current.Y - prev.Y);
        prev = current;
    }
    return length;
}

static auto DashPattern(std::vector<f32> const& src, std::vector<f32>& dst, f32 total) -> bool
{
    auto const size {src.size()};
    dst.resize(size);
    f32 sumDash {0.0f};

    for (usize i {0}; i < size; ++i) {
        sumDash += dst[i] = std::max(0.0f, src[i]);
    }

    if (sumDash <= EPSILON) { return false; }

    usize const reps {std::max(usize {1}, static_cast<usize>(std::round(total / sumDash)))};
    f32 const   scale {total / (reps * sumDash)};
    for (f32& d : dst) { d *= scale; }

    return true;
}

auto static Quantize(f32 a, f32 d) -> f32
{
    return (static_cast<i32>((a / d) + 0.5f)) * d;
}

void static MultiplyAlphaPaint(paint_color& c, f32 alpha)
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

auto canvas::get_texture(i32 level) -> assets::asset_ptr<texture>
{
    return _rtt[level];
}

void canvas::begin_frame(size_i windowSize, f32 devicePixelRatio, i32 rtt)
{
    _activeRtt  = rtt;
    _windowSize = windowSize;

    auto& artt {_rtt[_activeRtt]};
    artt->Size = windowSize;
    artt->prepare_render();
    artt->clear({0, 0, 0, 0});

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

auto canvas::create_guard() -> state_guard
{
    return state_guard {this};
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
    if (!do_dash()) {
        _cache->append_commands(path2d::CommandsLineTo(pos), _states->get().XForm);
        return;
    }

    easing::linear<point_f> func {.Start = _cache->CommandPoint, .End = pos};

    f32 const totalLength {static_cast<f32>((pos - _cache->CommandPoint).length())};
    f32       currentLength {0.0f};
    bool      drawing {true};

    state&           s {_states->get()};
    std::vector<f32> dashPattern;
    if (!DashPattern(s.Dash, dashPattern, totalLength)) { return; }

    for (usize dashIndex {0};; ++dashIndex) {
        f32 const dashLength {dashPattern[dashIndex % dashPattern.size()]};
        if (currentLength + dashLength > totalLength) { break; }

        if (dashLength <= EPSILON) {
            drawing = !drawing;
            continue;
        }

        if (drawing) {
            f32 const start {currentLength / totalLength};
            f32 const end {(currentLength + dashLength) / totalLength};

            _cache->append_commands(path2d::CommandsMoveTo(func(start)), s.XForm);
            _cache->append_commands(path2d::CommandsLineTo(func(end)), s.XForm);
        }

        drawing = !drawing;
        currentLength += dashLength;
    }
}

////////////////////////////////////////////////////////////

void canvas::rect(rect_f const& rect)
{
    auto const [x, y] {rect.Position};
    auto const [w, h] {rect.Size};

    state& s {_states->get()};

    if (!do_dash()) {
        _cache->append_commands(std::vector<f32> {
                                    MoveTo, x, y,
                                    LineTo, x, y + h,
                                    LineTo, x + w, y + h,
                                    LineTo, x + w, y,
                                    Close},
                                s.XForm);
        return;
    }

    f32 const totalLength {2 * (w + h)};

    std::vector<f32> dashPattern;
    if (!DashPattern(s.Dash, dashPattern, totalLength)) { return; }

    std::array<f32, 4> const     cornerPositions {{0.0f, w, w + h, 2 * w + h}};
    std::array<point_f, 4> const corners {{rect.top_left(),
                                           rect.top_right(),
                                           rect.bottom_right(),
                                           rect.bottom_left()}};

    std::vector<f32> cmds {};
    cmds.reserve(64);

    point_f prevPoint {corners[0]};
    cmds.push_back(static_cast<f32>(MoveTo));
    cmds.push_back(prevPoint.X);
    cmds.push_back(prevPoint.Y);

    f32  currentLength {0.0f};
    bool drawing {true};

    for (usize dashIndex {0};; ++dashIndex) {
        f32 const dashLength {dashPattern[dashIndex % s.Dash.size()]};

        if (dashLength <= EPSILON) {
            drawing = !drawing;
            continue;
        }

        f32 const segEnd {std::min(totalLength, currentLength + dashLength)};
        if (segEnd >= totalLength) { break; }

        if (drawing) {
            for (f32 cp : cornerPositions) {
                if (cp > currentLength && cp < segEnd) {
                    point_f const cornerPt {corners[std::distance(cornerPositions.begin(), std::find(cornerPositions.begin(), cornerPositions.end(), cp))]};
                    cmds.push_back(static_cast<f32>(LineTo));
                    cmds.push_back(cornerPt.X);
                    cmds.push_back(cornerPt.Y);
                    prevPoint = cornerPt;
                }
            }

            point_f const dashEndPt {Rect(x, y, w, h, segEnd / totalLength)};
            if (!prevPoint.equals(dashEndPt, EPSILON)) {
                cmds.push_back(static_cast<f32>(LineTo));
                cmds.push_back(dashEndPt.X);
                cmds.push_back(dashEndPt.Y);
                prevPoint = dashEndPt;
            }
        } else {
            point_f const gapEndPt {Rect(x, y, w, h, segEnd / totalLength)};
            cmds.push_back(static_cast<f32>(MoveTo));
            cmds.push_back(gapEndPt.X);
            cmds.push_back(gapEndPt.Y);
            prevPoint = gapEndPt;
        }

        currentLength = segEnd;
        drawing       = !drawing;
    }

    _cache->append_commands(cmds, s.XForm);
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
    f32 const rxBL {std::min(radBL, halfw) * signf(w)}, ryBL {std::min(radBL, halfh) * signf(h)};
    f32 const rxBR {std::min(radBR, halfw) * signf(w)}, ryBR {std::min(radBR, halfh) * signf(h)};
    f32 const rxTR {std::min(radTR, halfw) * signf(w)}, ryTR {std::min(radTR, halfh) * signf(h)};
    f32 const rxTL {std::min(radTL, halfw) * signf(w)}, ryTL {std::min(radTL, halfh) * signf(h)};

    if (!do_dash()) {
        _cache->append_commands(std::vector<f32> {
                                    MoveTo, x, y + ryTL,
                                    LineTo, x, y + h - ryBL,
                                    BezierTo, x, y + h - (ryBL * (1 - NVG_KAPPA90)), x + (rxBL * (1 - NVG_KAPPA90)), y + h, x + rxBL, y + h,
                                    LineTo, x + w - rxBR, y + h,
                                    BezierTo, x + w - (rxBR * (1 - NVG_KAPPA90)), y + h, x + w, y + h - (ryBR * (1 - NVG_KAPPA90)), x + w, y + h - ryBR,
                                    LineTo, x + w, y + ryTR,
                                    BezierTo, x + w, y + (ryTR * (1 - NVG_KAPPA90)), x + w - (rxTR * (1 - NVG_KAPPA90)), y, x + w - rxTR, y,
                                    LineTo, x + rxTL, y,
                                    BezierTo, x + (rxTL * (1 - NVG_KAPPA90)), y, x, y + (ryTL * (1 - NVG_KAPPA90)), x, y + ryTL,
                                    Close},
                                _states->get().XForm);
        return;
    }

    f32 const perimTop {w - (rxTL + rxTR)};
    f32 const perimRight {h - (ryTR + ryBR)};
    f32 const perimBottom {w - (rxBL + rxBR)};
    f32 const perimLeft {h - (ryBL + ryTL)};

    f32 const arcTopRightLen {QuadBezierLength({x + w - rxTR, y}, {x + w, y}, {x + w, y + ryTR})};
    f32 const arcBottomRightLen {QuadBezierLength({x + w, y + h - ryBR}, {x + w, y + h}, {x + w - rxBR, y + h})};
    f32 const arcBottomLeftLen {QuadBezierLength({x + rxBL, y + h}, {x, y + h}, {x, y + h - ryBL})};
    f32 const arcTopLeftLen {QuadBezierLength({x, y + ryTL}, {x, y}, {x + rxTL, y})};
    f32 const totalLength {perimTop + perimRight + perimBottom + perimLeft + arcTopRightLen + arcBottomRightLen + arcBottomLeftLen + arcTopLeftLen};

    auto const func {[&](f64 t) -> point_f {
        auto static quad_bezier {[](point_f p0, point_f p1, point_f p2, f32 t) -> point_f {
            f32 const oneMinusT {1.0f - t};
            f32 const exp0 {oneMinusT * oneMinusT};
            f32 const exp1 {2.0f * t * oneMinusT};
            f32 const exp2 {t * t};
            return {exp0 * p0.X + exp1 * p1.X + exp2 * p2.X, exp0 * p0.Y + exp1 * p1.Y + exp2 * p2.Y};
        }};

        f64 scaledT {t * totalLength};

        if (scaledT < perimTop) { return {x + rxTL + static_cast<f32>(scaledT), y}; }
        scaledT -= perimTop;

        if (scaledT < arcTopRightLen) { return quad_bezier({x + w - rxTR, y}, {x + w, y}, {x + w, y + ryTR}, static_cast<f32>(scaledT / arcTopRightLen)); }
        scaledT -= arcTopRightLen;

        if (scaledT < perimRight) { return {x + w, y + ryTR + static_cast<f32>(scaledT)}; }
        scaledT -= perimRight;

        if (scaledT < arcBottomRightLen) { return quad_bezier({x + w, y + h - ryBR}, {x + w, y + h}, {x + w - rxBR, y + h}, static_cast<f32>(scaledT / arcBottomRightLen)); }
        scaledT -= arcBottomRightLen;

        if (scaledT < perimBottom) { return {x + w - rxBR - static_cast<f32>(scaledT), y + h}; }
        scaledT -= perimBottom;

        if (scaledT < arcBottomLeftLen) { return quad_bezier({x + rxBL, y + h}, {x, y + h}, {x, y + h - ryBL}, static_cast<f32>(scaledT / arcBottomLeftLen)); }
        scaledT -= arcBottomLeftLen;

        if (scaledT < perimLeft) { return {x, y + h - ryBL - static_cast<f32>(scaledT)}; }
        scaledT -= perimLeft;

        return quad_bezier({x, y + ryTL}, {x, y}, {x + rxTL, y}, static_cast<f32>(scaledT / arcTopLeftLen));
    }};

    dashed_bezier_path(func);
}

////////////////////////////////////////////////////////////

void canvas::ellipse(point_f c, f32 rx, f32 ry)
{
    auto const [cx, cy] {c};

    if (!do_dash()) {
        _cache->append_commands(std::vector<f32> {
                                    MoveTo, cx - rx, cy,
                                    BezierTo, cx - rx, cy + (ry * NVG_KAPPA90), cx - (rx * NVG_KAPPA90), cy + ry, cx, cy + ry,
                                    BezierTo, cx + (rx * NVG_KAPPA90), cy + ry, cx + rx, cy + (ry * NVG_KAPPA90), cx + rx, cy,
                                    BezierTo, cx + rx, cy - (ry * NVG_KAPPA90), cx + (rx * NVG_KAPPA90), cy - ry, cx, cy - ry,
                                    BezierTo, cx - (rx * NVG_KAPPA90), cy - ry, cx - rx, cy - (ry * NVG_KAPPA90), cx - rx, cy,
                                    Close},
                                _states->get().XForm);
        return;
    }

    auto const ellipse_func {[=](f32 t) -> point_f {
        f32 const angle {(TAU_F * t) - (TAU_F / 4)};
        return point_f {cx + rx * std::cos(angle), cy + ry * std::sin(angle)};
    }};

    dashed_bezier_path(ellipse_func);
}

void canvas::circle(point_f c, f32 r)
{
    ellipse(c, r, r);
}

////////////////////////////////////////////////////////////

void canvas::cubic_bezier_to(point_f cp0, point_f cp1, point_f end)
{
    if (!do_dash()) {
        _cache->append_commands(path2d::CommandsCubicTo(cp0, cp1, end), _states->get().XForm);
        return;
    }

    easing::cubic_bezier_curve func {.StartPoint = _cache->CommandPoint, .ControlPoint0 = cp0, .ControlPoint1 = cp1, .EndPoint = end};
    dashed_bezier_path(func);
}

void canvas::quad_bezier_to(point_f cp, point_f end)
{
    if (!do_dash()) {
        _cache->append_commands(path2d::CommandsQuadTo(_cache->CommandPoint, cp, end), _states->get().XForm);
        return;
    }

    easing::quad_bezier_curve func {.StartPoint = _cache->CommandPoint, .ControlPoint = cp, .EndPoint = end};
    dashed_bezier_path(func);
}

////////////////////////////////////////////////////////////

void canvas::arc(point_f const c, f32 const r, radian_f const startAngle, radian_f const endAngle, winding const dir)
{
    static f32 const rad90 {TAU_F / 4};

    i32 const move {!_cache->Commands.empty() ? LineTo : MoveTo};
    f32 const a0 {startAngle.Value - rad90};
    f32 const a1 {endAngle.Value - rad90};

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

    if (!do_dash()) {
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

        _cache->append_commands(vals, _states->get().XForm);
        return;
    }

    auto const arc_func {[=](f32 t) -> point_f {
        f32 const angle {a0 + da * t};
        return point_f {c.X + r * std::cos(angle), c.Y + r * std::sin(angle)};
    }};

    dashed_bezier_path(arc_func);
}

void canvas::arc_to(point_f pos1, point_f pos2, f32 radius)
{
    if (_cache->Commands.empty()) { return; }

    winding dir {};

    // Handle degenerate cases.
    if (_cache->CommandPoint.equals(pos1, _distTol)
        || pos1.equals(pos2, _distTol)
        || DistancePointSegment(pos1.X, pos1.Y, _cache->CommandPoint.X, _cache->CommandPoint.Y, pos2.X, pos2.Y) < _distTol * _distTol
        || radius < _distTol) {
        line_to(pos1);
        return;
    }

    // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
    f32 dx0 {_cache->CommandPoint.X - pos1.X};
    f32 dy0 {_cache->CommandPoint.Y - pos1.Y};
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

    auto static cross {[](f32 dx0, f32 dy0, f32 dx1, f32 dy1) -> f32 { return (dx1 * dy0) - (dx0 * dy1); }};

    if (cross(dx0, dy0, dx1, dy1) > 0.0f) {
        cx  = pos1.X + dx0 * d + dy0 * radius;
        cy  = pos1.Y + dy0 * d + -dx0 * radius;
        a0  = radian_f {std::atan2(dx0, -dy0)};
        a1  = radian_f {std::atan2(-dx1, dy1)};
        dir = winding::CW;
    } else {
        cx  = pos1.X + dx0 * d + -dy0 * radius;
        cy  = pos1.Y + dy0 * d + dx0 * radius;
        a0  = radian_f {std::atan2(-dx0, dy0)};
        a1  = radian_f {std::atan2(dx1, -dy1)};
        dir = winding::CCW;
    }

    arc({cx, cy}, radius, a0, a1, dir);
}

////////////////////////////////////////////////////////////

void canvas::set_line_dash(std::span<f32 const> dashPattern)
{
    _states->get().Dash = {dashPattern.begin(), dashPattern.end()};
}

auto canvas::do_dash() const -> bool
{
    return !_states->get().Dash.empty();
}

auto canvas::dashed_bezier_path(auto&& func) -> bool
{
    constexpr i32                   numSamples {50};
    std::array<f32, numSamples + 1> arcLengths {};
    std::array<f32, numSamples + 1> tValues {};
    point_f                         prevPoint {func(0.0f)};

    for (i32 i {1}; i <= numSamples; ++i) {
        f32 const  t {static_cast<f32>(i) / numSamples};
        auto const currentPoint {func(t)};
        f32 const  segmentLength {std::hypot(currentPoint.X - prevPoint.X, currentPoint.Y - prevPoint.Y)};
        arcLengths[i] = arcLengths[i - 1] + segmentLength;
        tValues[i]    = t;
        prevPoint     = currentPoint;
    }
    f32 const totalLength {arcLengths.back()};

    state const& s {_states->get()};

    std::vector<f32> dashPattern;
    if (!DashPattern(s.Dash, dashPattern, totalLength)) { return false; }

    auto const interpolate_arc = [&](f32 targetLength) -> f32 {
        auto it {std::lower_bound(arcLengths.begin(), arcLengths.end(), targetLength)};
        if (it == arcLengths.end()) { return 1.0f; }
        if (it == arcLengths.begin()) { return 0.0f; }
        auto const idx {std::distance(arcLengths.begin(), it)};
        f32 const  t1 {tValues[idx - 1]};
        f32 const  t2 {tValues[idx]};
        f32 const  s1 {arcLengths[idx - 1]};
        f32 const  s2 {arcLengths[idx]};
        f32 const  fraction {(targetLength - s1) / (s2 - s1)};
        return t1 + fraction * (t2 - t1);
    };

    f32  currentLength {0.0f};
    bool drawing {true};
    f32  t0 {0.0f};

    for (usize dashIndex {0};; ++dashIndex) {
        f32 const dashLength {dashPattern[dashIndex % dashPattern.size()]};
        currentLength += dashLength;
        if (currentLength > totalLength) { break; }
        if (dashLength <= EPSILON) {
            drawing = !drawing;
            continue;
        }

        f32 const t1 {interpolate_arc(currentLength)};
        if (drawing) {
            f32 const     deltaT {t1 - t0};
            constexpr f32 subdivThreshold {0.2f};
            if (deltaT > subdivThreshold) {
                constexpr i32 subSegments {20};
                f32 const     frac {deltaT / subSegments};
                for (i32 j {0}; j < subSegments; ++j) {
                    f32 const subT0 {t0 + j * frac};
                    if (j == 0) {
                        auto const start {func(subT0)};
                        _cache->append_commands(path2d::CommandsMoveTo(start), s.XForm);
                    }

                    f32 const  subT1 {t0 + (j + 1) * frac};
                    auto const end {func(subT1)};

                    f32 const  subDeltaT {subT1 - subT0};
                    auto const cp0 {func(subT0 + (subDeltaT / 3.0f))};
                    auto const cp1 {func(subT1 - (subDeltaT / 3.0f))};
                    _cache->append_commands(path2d::CommandsCubicTo(cp0, cp1, end), s.XForm);
                }
            } else {
                auto const start {func(t0)};
                auto const end {func(t1)};
                auto const cp0 {func(t0 + (deltaT / 3.0f))};
                auto const cp1 {func(t1 - (deltaT / 3.0f))};
                _cache->append_commands(path2d::CommandsMoveTo(start), s.XForm);
                _cache->append_commands(path2d::CommandsCubicTo(cp0, cp1, end), s.XForm);
            }
        }
        t0      = t1;
        drawing = !drawing;
    }

    _cache->append_commands(path2d::CommandsMoveTo(func(1.0f)), s.XForm);
    return true;
}

////////////////////////////////////////////////////////////

void canvas::fill_lines(std::span<point_f const> points)
{
    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        _cache->append_commands(path2d::CommandsLineTo(points[i]), _states->get().XForm);
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
    if (points.size() == 1) { return; }

    begin_path();

    move_to(points[0]);
    for (u32 i {1}; i < points.size(); ++i) {
        line_to(points[i]);
    }

    stroke();
}

void canvas::dotted_line_to(point_f to, f32 r, i32 numDots)
{
    easing::linear<point_f> func {.Start = _cache->CommandPoint, .End = to};
    f32 const               inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), r);
    }
}

void canvas::dotted_circle(point_f center, f32 circleR, f32 dotR, i32 numDots)
{
    easing::circular func {.Start = degree_f {0}, .End = degree_f {360}};
    f32 const        inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t) * circleR + center, dotR);
    }
}

void canvas::dotted_cubic_bezier_to(point_f cp0, point_f cp1, point_f end, f32 dotR, i32 numDots)
{
    easing::cubic_bezier_curve func {.StartPoint = _cache->CommandPoint, .ControlPoint0 = cp0, .ControlPoint1 = cp1, .EndPoint = end};
    f32 const                  inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), dotR);
    }
}

void canvas::dotted_quad_bezier_to(point_f cp, point_f end, f32 dotR, i32 numDots)
{
    easing::quad_bezier_curve func {.StartPoint = _cache->CommandPoint, .ControlPoint = cp, .EndPoint = end};
    f32 const                 inc {1.0f / numDots};

    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), dotR);
    }
}

void canvas::dotted_rounded_rect(rect_f const& rect, f32 r, f32 dotR, i32 numDots)
{
    dotted_rounded_rect_varying(rect, r, r, r, r, dotR, numDots);
}

void canvas::dotted_rounded_rect_varying(rect_f const& rect, f32 radTL, f32 radTR, f32 radBR, f32 radBL, f32 dotR, i32 numDots)
{
    auto const [x, y] {rect.Position};
    auto const [w, h] {rect.Size};

    f32 const halfw {std::abs(w) * 0.5f};
    f32 const halfh {std::abs(h) * 0.5f};
    f32 const rxBL {std::min(radBL, halfw) * signf(w)}, ryBL {std::min(radBL, halfh) * signf(h)};
    f32 const rxBR {std::min(radBR, halfw) * signf(w)}, ryBR {std::min(radBR, halfh) * signf(h)};
    f32 const rxTR {std::min(radTR, halfw) * signf(w)}, ryTR {std::min(radTR, halfh) * signf(h)};
    f32 const rxTL {std::min(radTL, halfw) * signf(w)}, ryTL {std::min(radTL, halfh) * signf(h)};

    f32 const perimTop {w - (rxTL + rxTR)};
    f32 const perimRight {h - (ryTR + ryBR)};
    f32 const perimBottom {w - (rxBL + rxBR)};
    f32 const perimLeft {h - (ryBL + ryTL)};

    f32 const arcTopRightLen {QuadBezierLength({x + w - rxTR, y}, {x + w, y}, {x + w, y + ryTR})};
    f32 const arcBottomRightLen {QuadBezierLength({x + w, y + h - ryBR}, {x + w, y + h}, {x + w - rxBR, y + h})};
    f32 const arcBottomLeftLen {QuadBezierLength({x + rxBL, y + h}, {x, y + h}, {x, y + h - ryBL})};
    f32 const arcTopLeftLen {QuadBezierLength({x, y + ryTL}, {x, y}, {x + rxTL, y})};
    f32 const totalPerimeter {perimTop + perimRight + perimBottom + perimLeft + arcTopRightLen + arcBottomRightLen + arcBottomLeftLen + arcTopLeftLen};

    auto const func {[&](f64 t) -> point_f {
        auto static quad_bezier {[](point_f p0, point_f p1, point_f p2, f32 t) -> point_f {
            f32 const oneMinusT {1.f - t};
            f32 const exp0 {oneMinusT * oneMinusT};
            f32 const exp1 {2.f * t * oneMinusT};
            f32 const exp2 {t * t};
            return {exp0 * p0.X + exp1 * p1.X + exp2 * p2.X, exp0 * p0.Y + exp1 * p1.Y + exp2 * p2.Y};
        }};

        if (radTL < 0.1f && radTR < 0.1f && radBR < 0.1f && radBL < 0.1f) { return Rect(x, y, w, h, t); }

        f64 scaledT {t * totalPerimeter};

        if (scaledT < perimTop) { return {x + rxTL + static_cast<f32>(scaledT), y}; }
        scaledT -= perimTop;

        if (scaledT < arcTopRightLen) { return quad_bezier({x + w - rxTR, y}, {x + w, y}, {x + w, y + ryTR}, static_cast<f32>(scaledT / arcTopRightLen)); }
        scaledT -= arcTopRightLen;

        if (scaledT < perimRight) { return {x + w, y + ryTR + static_cast<f32>(scaledT)}; }
        scaledT -= perimRight;

        if (scaledT < arcBottomRightLen) { return quad_bezier({x + w, y + h - ryBR}, {x + w, y + h}, {x + w - rxBR, y + h}, static_cast<f32>(scaledT / arcBottomRightLen)); }
        scaledT -= arcBottomRightLen;

        if (scaledT < perimBottom) { return {x + w - rxBR - static_cast<f32>(scaledT), y + h}; }
        scaledT -= perimBottom;

        if (scaledT < arcBottomLeftLen) { return quad_bezier({x + rxBL, y + h}, {x, y + h}, {x, y + h - ryBL}, static_cast<f32>(scaledT / arcBottomLeftLen)); }
        scaledT -= arcBottomLeftLen;

        if (scaledT < perimLeft) { return {x, y + h - ryBL - static_cast<f32>(scaledT)}; }
        scaledT -= perimLeft;

        return quad_bezier({x, y + ryTL}, {x, y}, {x + rxTL, y}, static_cast<f32>(scaledT / arcTopLeftLen));
    }};

    f32 const inc {1.0f / numDots};
    for (f32 t {0}; t <= 1.0f; t += inc) {
        circle(func(t), dotR);
    }
}

void canvas::wavy_line_to(point_f to, f32 amp, f32 freq, f32 phase)
{
    // TODO: dash
    state const& s {_states->get()};

    point_f const from {_cache->CommandPoint};

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

        point_f const base {from.X + d.X * t, from.Y + d.Y * t};
        f32 const     offset {amp * std::sin(freq * sl + phase)};
        point_f const finalPt {base.X + offset * perp.X, base.Y + offset * perp.Y};

        _cache->append_commands(path2d::CommandsLineTo(finalPt), s.XForm);
    }
}

void canvas::regular_polygon(point_f pos, size_f size, i32 n)
{
    // TODO: dash
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
    // TODO: dash
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
    // TODO: dash
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

void canvas::fill()
{
    state const& s {_states->get()};
    paint        fillPaint {s.Fill}; // copy

    _cache->flatten_paths(_distTol, _tessTol, _enforceWinding);
    if (_edgeAntiAlias && s.ShapeAntiAlias) {
        _cache->expand_fill(_fringeWidth, line_join::Miter, 2.4f, _fringeWidth);
    } else {
        _cache->expand_fill(0.0f, line_join::Miter, 2.4f, _fringeWidth);
    }

    // Apply global alpha
    MultiplyAlphaPaint(fillPaint.Color, s.Alpha);

    _impl->render_fill(fillPaint, s.CompositeOperation, s.Scissor, _fringeWidth, _cache->Bounds, _cache->Paths);
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

    // Apply global alpha
    MultiplyAlphaPaint(strokePaint.Color, s.Alpha);

    _cache->flatten_paths(_distTol, _tessTol, _enforceWinding);

    if (_edgeAntiAlias && s.ShapeAntiAlias) {
        _cache->expand_stroke(strokeWidth * 0.5f, _fringeWidth, s.LineCap, s.LineJoin, s.MiterLimit, _tessTol);
    } else {
        _cache->expand_stroke(strokeWidth * 0.5f, 0.0f, s.LineCap, s.LineJoin, s.MiterLimit, _tessTol);
    }

    _impl->render_stroke(strokePaint, s.CompositeOperation, s.Scissor, _fringeWidth, strokeWidth, _cache->Paths);
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
        .XForm   = {dy, dx, s.X - (dx * large),
                    -dx, dy, s.Y - (dy * large),
                    0, 0, 1},
        .Extent  = {large, large + (d * 0.5f)},
        .Radius  = 0.0f,
        .Feather = std::max(1.0f, d),
        .Color   = create_gradient(gradient)};
}

auto canvas::create_box_gradient(rect_f const& rect, f32 r, f32 f, color_gradient const& gradient) -> paint
{
    auto const& c {rect.center()};
    return {
        .XForm   = {1.0f, 0.0f, c.X,
                    0.0f, 1.0f, c.Y,
                    0.0f, 0.0f, 1.0f},
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
        .XForm   = {scale.Width, 0.0f, c.X,
                    0.0f, scale.Height, c.Y,
                    0.0f, 0.0f, 1.0f},
        .Extent  = {r, r},
        .Radius  = r,
        .Feather = std::max(1.0f, f),
        .Color   = create_gradient(gradient)};
}

auto canvas::create_gradient(color_gradient const& gradient) -> paint_color
{
    if (gradient.is_single_color()) {
        return gradient.first_color();
    }

    for (usize i {0}; i < _gradients.size(); ++i) {
        if (_gradients[i] == gradient) {
            return paint_gradient {1.0f, static_cast<i32>(i)};
        }
    }

    i32 const retValue {static_cast<i32>(_gradients.size())};
    _gradients.push_back(gradient);
    _impl->add_gradient(retValue, gradient);
    return paint_gradient {1.0f, retValue};
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

void canvas::set_global_enforce_path_winding(bool force)
{
    _enforceWinding = force;
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
    s.Scissor.XForm  = transform {0, 0, 0, 0, 0, 0, 0, 0, 0};
    s.Scissor.Extent = {-1.0f, -1.0f};
}

void canvas::set_font(font* font)
{
    _states->get().Font = font;
}

////////////////////////////////////////////////////////////

void canvas::draw_textbox(rect_f const& rect, utf8_string_view text)
{
    draw_textbox(rect.Position, format_text(rect.Size, text));
}

void canvas::draw_textbox(point_f offset, text_formatter::result const& formatResult)
{
    state const& s {_states->get()};
    if (!s.Font) { return; }

    f32 const scale {get_font_scale() * _devicePxRatio};
    f32 const invscale {1.0f / scale};
    auto*     verts {_cache->alloc_temp_verts(formatResult.QuadCount * 6)};
    usize     nverts {0};

    f32 const x {std::floor(offset.X + 0.5f)};
    f32 const y {std::floor(offset.Y + 0.5f)};

    bool const isTranslation {s.XForm.is_translate_only()};

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

            point_f topLeft {}, topRight {}, bottomLeft {}, bottomRight {};

            if (isTranslation) {
                auto const tl {s.XForm * point_f {(posRect.left() * invscale) + x, (posRect.top() * invscale) + y}};
                auto const br {s.XForm * point_f {(posRect.right() * invscale) + x, (posRect.bottom() * invscale) + y}};

                topLeft.X = bottomLeft.X = std::floor(tl.X + 0.5f);
                topRight.X = bottomRight.X = std::floor(br.X + 0.5f);
                topLeft.Y = topRight.Y = std::floor(tl.Y + 0.5f);
                bottomLeft.Y = bottomRight.Y = std::floor(br.Y + 0.5f);
            } else {
                topLeft       = {s.XForm * point_f {(posRect.left() * invscale) + x, (posRect.top() * invscale) + y}};
                topLeft.X     = std::floor(topLeft.X + 0.5f);
                topLeft.Y     = std::floor(topLeft.Y + 0.5f);
                topRight      = {s.XForm * point_f {(posRect.right() * invscale) + x, (posRect.top() * invscale) + y}};
                topRight.X    = std::floor(topRight.X + 0.5f);
                topRight.Y    = std::floor(topRight.Y + 0.5f);
                bottomRight   = {s.XForm * point_f {(posRect.right() * invscale) + x, (posRect.bottom() * invscale) + y}};
                bottomRight.X = std::floor(bottomRight.X + 0.5f);
                bottomRight.Y = std::floor(bottomRight.Y + 0.5f);
                bottomLeft    = {s.XForm * point_f {(posRect.left() * invscale) + x, (posRect.bottom() * invscale) + y}};
                bottomLeft.X  = std::floor(bottomLeft.X + 0.5f);
                bottomLeft.Y  = std::floor(bottomLeft.Y + 0.5f);
            }

            verts[nverts++] = vertex {.Position = topLeft, .TexCoords = {uvLeft, uvTop, level}};
            verts[nverts++] = vertex {.Position = bottomRight, .TexCoords = {uvRight, uvBottom, level}};
            verts[nverts++] = vertex {.Position = topRight, .TexCoords = {uvRight, uvTop, level}};
            verts[nverts++] = vertex {.Position = topLeft, .TexCoords = {uvLeft, uvTop, level}};
            verts[nverts++] = vertex {.Position = bottomLeft, .TexCoords = {uvLeft, uvBottom, level}};
            verts[nverts++] = vertex {.Position = bottomRight, .TexCoords = {uvRight, uvBottom, level}};
        }
    }

    render_text(s.Font, {verts, nverts});
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

    texture_region texRegion {paint.Image->get_region(region)};

    quad quad {};
    geometry::set_position(quad, rect, s.XForm);
    geometry::set_color(quad, colors::White);
    geometry::set_texcoords(quad, texRegion);

    for (auto& vert : quad) {
        vert.Position.X = std::floor(vert.Position.X + 0.5f);
        vert.Position.Y = std::floor(vert.Position.Y + 0.5f);
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
            vert.Position.X = std::floor(vert.Position.X + 0.5f);
            vert.Position.Y = std::floor(vert.Position.Y + 0.5f);
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

    _impl->render_triangles(paint, s.CompositeOperation, s.Scissor, verts, _fringeWidth);
}

////////////////////////////////////////////////////////////

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
}

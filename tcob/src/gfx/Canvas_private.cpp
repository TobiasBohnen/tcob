// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Canvas_private.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/gfx/Canvas.hpp"
#include "tcob/gfx/Geometry.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Transform.hpp"

namespace tcob::gfx {

////////////////////////////////////////////////////////////

auto states::get() -> state&
{
    return _states.top();
}

auto states::get() const -> state const&
{
    return _states.top();
}

void states::save()
{
    if (_states.empty()) {
        _states.emplace();
    } else {
        _states.push(_states.top());
    }
}

void states::restore()
{
    _states.pop();
}

void states::reset()
{
    _states = {};
}

////////////////////////////////////////////////////////////

auto static hypot(f32 x, f32 y) -> f32
{
    return std::sqrt((x * x) + (y * y));
}

void static SetVertex(vertex* vtx, f32 x, f32 y, f32 u, f32 v)
{
    vtx->Position.X = x;
    vtx->Position.Y = y;

    vtx->TexCoords.U = u;
    vtx->TexCoords.V = v;
}

auto Normalize(f32& x, f32& y) -> f32
{
    f32 const d {std::sqrt((x * x) + (y * y))};
    if (d > EPSILON) {
        f32 const id {1.0f / d};
        x *= id;
        y *= id;
    }
    return d;
}

auto static PointEquals(f32 x1, f32 y1, f32 x2, f32 y2, f32 tol) -> i32
{
    f32 const dx {x2 - x1};
    f32 const dy {y2 - y1};
    return dx * dx + dy * dy < tol * tol;
}

void static PolyReverse(std::span<canvas_point> pts)
{
    canvas_point tmp;
    usize        i {0};
    usize        j {pts.size() - 1};
    while (i < j) {
        tmp    = pts[i];
        pts[i] = pts[j];
        pts[j] = tmp;
        ++i;
        --j;
    }
}

auto static TriArea2(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) -> f32
{
    f32 const abx {bx - ax};
    f32 const aby {by - ay};
    f32 const acx {cx - ax};
    f32 const acy {cy - ay};
    return (acx * aby) - (abx * acy);
}

void static ChooseBevel(bool bevel, canvas_point const& p0, canvas_point const& p1, f32 w, f32& x0, f32& y0, f32& x1, f32& y1)
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

auto static RoundJoin(vertex* dst, canvas_point const& p0, canvas_point const& p1, f32 lw, f32 rw, f32 lu, f32 ru, i32 ncap) -> vertex*
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

        SetVertex(dst++, lx0, ly0, lu, 1);
        SetVertex(dst++, p1.X - (dlx0 * rw), p1.Y - (dly0 * rw), ru, 1);

        i32 const n {std::clamp(static_cast<i32>(ceilf(((a0 - a1) / (TAU_F / 2)) * ncap)), 2, ncap)};
        for (i32 i {0}; i < n; ++i) {
            f32 const u {i / static_cast<f32>(n - 1)};
            f32 const a {a0 + (u * (a1 - a0))};
            f32 const rx {p1.X + (std::cos(a) * rw)};
            f32 const ry {p1.Y + (std::sin(a) * rw)};
            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);
            SetVertex(dst++, rx, ry, ru, 1);
        }

        SetVertex(dst++, lx1, ly1, lu, 1);
        SetVertex(dst++, p1.X - (dlx1 * rw), p1.Y - (dly1 * rw), ru, 1);
    } else {
        f32 rx0 {0}, ry0 {0}, rx1 {0}, ry1 {0}, a0 {0}, a1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, -rw, rx0, ry0, rx1, ry1);
        a0 = std::atan2(dly0, dlx0);
        a1 = std::atan2(dly1, dlx1);
        if (a1 < a0) {
            a1 += TAU_F;
        }

        SetVertex(dst++, p1.X + (dlx0 * rw), p1.Y + (dly0 * rw), lu, 1);
        SetVertex(dst++, rx0, ry0, ru, 1);

        i32 const n {std::clamp(static_cast<i32>(std::ceil(((a1 - a0) / (TAU_F / 2)) * ncap)), 2, ncap)};
        for (i32 i {0}; i < n; ++i) {
            f32 const u {i / static_cast<f32>(n - 1)};
            f32 const a {a0 + (u * (a1 - a0))};
            f32 const lx {p1.X + (std::cos(a) * lw)};
            f32 const ly {p1.Y + (std::sin(a) * lw)};
            SetVertex(dst++, lx, ly, lu, 1);
            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);
        }

        SetVertex(dst++, p1.X + (dlx1 * rw), p1.Y + (dly1 * rw), lu, 1);
        SetVertex(dst++, rx1, ry1, ru, 1);
    }
    return dst;
}

auto static BevelJoin(vertex* dst, canvas_point const& p0, canvas_point const& p1, f32 lw, f32 rw, f32 lu, f32 ru) -> vertex*
{
    f32 const dlx0 {p0.DY};
    f32 const dly0 {-p0.DX};
    f32 const dlx1 {p1.DY};
    f32 const dly1 {-p1.DX};

    if (p1.Flags & Left) {
        f32 lx0 {0}, ly0 {0}, lx1 {0}, ly1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, lw, lx0, ly0, lx1, ly1);

        SetVertex(dst++, lx0, ly0, lu, 1);
        SetVertex(dst++, p1.X - (dlx0 * rw), p1.Y - (dly0 * rw), ru, 1);

        if (p1.Flags & Bevel) {
            SetVertex(dst++, lx0, ly0, lu, 1);
            SetVertex(dst++, p1.X - (dlx0 * rw), p1.Y - (dly0 * rw), ru, 1);

            SetVertex(dst++, lx1, ly1, lu, 1);
            SetVertex(dst++, p1.X - (dlx1 * rw), p1.Y - (dly1 * rw), ru, 1);
        } else {
            f32 const rx0 {p1.X - (p1.DMX * rw)};
            f32 const ry0 {p1.Y - (p1.DMY * rw)};

            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);
            SetVertex(dst++, p1.X - (dlx0 * rw), p1.Y - (dly0 * rw), ru, 1);

            SetVertex(dst++, rx0, ry0, ru, 1);
            SetVertex(dst++, rx0, ry0, ru, 1);

            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);
            SetVertex(dst++, p1.X - (dlx1 * rw), p1.Y - (dly1 * rw), ru, 1);
        }

        SetVertex(dst++, lx1, ly1, lu, 1);
        SetVertex(dst++, p1.X - (dlx1 * rw), p1.Y - (dly1 * rw), ru, 1);
    } else {
        f32 rx0 {0}, ry0 {0}, rx1 {0}, ry1 {0};
        ChooseBevel(p1.Flags & InnerBevel, p0, p1, -rw, rx0, ry0, rx1, ry1);

        SetVertex(dst++, p1.X + (dlx0 * lw), p1.Y + (dly0 * lw), lu, 1);
        SetVertex(dst++, rx0, ry0, ru, 1);

        if (p1.Flags & Bevel) {
            SetVertex(dst++, p1.X + (dlx0 * lw), p1.Y + (dly0 * lw), lu, 1);
            SetVertex(dst++, rx0, ry0, ru, 1);

            SetVertex(dst++, p1.X + (dlx1 * lw), p1.Y + (dly1 * lw), lu, 1);
            SetVertex(dst++, rx1, ry1, ru, 1);
        } else {
            f32 const lx0 {p1.X + (p1.DMX * lw)};
            f32 const ly0 {p1.Y + (p1.DMY * lw)};

            SetVertex(dst++, p1.X + (dlx0 * lw), p1.Y + (dly0 * lw), lu, 1);
            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);

            SetVertex(dst++, lx0, ly0, lu, 1);
            SetVertex(dst++, lx0, ly0, lu, 1);

            SetVertex(dst++, p1.X + (dlx1 * lw), p1.Y + (dly1 * lw), lu, 1);
            SetVertex(dst++, p1.X, p1.Y, 0.5f, 1);
        }

        SetVertex(dst++, p1.X + (dlx1 * lw), p1.Y + (dly1 * lw), lu, 1);
        SetVertex(dst++, rx1, ry1, ru, 1);
    }

    return dst;
}

auto static ButtCapStart(vertex* dst, canvas_point const& p, f32 dx, f32 dy, f32 w, f32 d, f32 aa, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X - (dx * d)};
    f32 const py {p.Y - (dy * d)};
    f32 const dlx {dy};
    f32 const dly {-dx};
    SetVertex(dst++, px + (dlx * w) - (dx * aa), py + (dly * w) - (dy * aa), u0, 0);
    SetVertex(dst++, px - (dlx * w) - (dx * aa), py - (dly * w) - (dy * aa), u1, 0);
    SetVertex(dst++, px + (dlx * w), py + (dly * w), u0, 1);
    SetVertex(dst++, px - (dlx * w), py - (dly * w), u1, 1);
    return dst;
}

auto static ButtCapEnd(vertex* dst, canvas_point const& p, f32 dx, f32 dy, f32 w, f32 d, f32 aa, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X + (dx * d)};
    f32 const py {p.Y + (dy * d)};
    f32 const dlx {dy};
    f32 const dly {-dx};
    SetVertex(dst++, px + (dlx * w), py + (dly * w), u0, 1);
    SetVertex(dst++, px - (dlx * w), py - (dly * w), u1, 1);
    SetVertex(dst++, px + (dlx * w) + (dx * aa), py + (dly * w) + (dy * aa), u0, 0);
    SetVertex(dst++, px - (dlx * w) + (dx * aa), py - (dly * w) + (dy * aa), u1, 0);
    return dst;
}

auto static RoundCapStart(vertex* dst, canvas_point const& p, f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X};
    f32 const py {p.Y};
    f32 const dlx {dy};
    f32 const dly {-dx};

    for (i32 i {0}; i < ncap; ++i) {
        f32 const a {i / static_cast<f32>(ncap - 1) * (TAU_F / 2)};
        f32 const ax {std::cos(a) * w}, ay {std::sin(a) * w};
        SetVertex(dst++, px - (dlx * ax) - (dx * ay), py - (dly * ax) - (dy * ay), u0, 1);
        SetVertex(dst++, px, py, 0.5f, 1);
    }
    SetVertex(dst++, px + (dlx * w), py + (dly * w), u0, 1);
    SetVertex(dst++, px - (dlx * w), py - (dly * w), u1, 1);
    return dst;
}

auto static RoundCapEnd(vertex* dst, canvas_point const& p, f32 dx, f32 dy, f32 w, i32 ncap, f32 u0, f32 u1) -> vertex*
{
    f32 const px {p.X};
    f32 const py {p.Y};
    f32 const dlx {dy};
    f32 const dly {-dx};

    SetVertex(dst++, px + (dlx * w), py + (dly * w), u0, 1);
    SetVertex(dst++, px - (dlx * w), py - (dly * w), u1, 1);
    for (i32 i {0}; i < ncap; ++i) {
        f32 const a {i / static_cast<f32>(ncap - 1) * (TAU_F / 2)};
        f32 const ax {std::cos(a) * w}, ay {std::sin(a) * w};
        SetVertex(dst++, px, py, 0.5f, 1);
        SetVertex(dst++, px - (dlx * ax) + (dx * ay), py - (dly * ax) + (dy * ay), u0, 1);
    }
    return dst;
}

auto static DashPattern(std::vector<f32>& dst, std::span<f32 const> src, f32 total) -> bool
{
    if (total <= EPSILON) { return false; }

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

auto static DashPolyline(std::span<canvas_point const> pts, f32 totalLength, std::span<f32 const> dashPattern, f32 dashOffset) -> std::vector<std::vector<canvas_point>>
{
    // Precompute cumulative distances along the polyline.
    std::vector<f32> accumLengths {};
    accumLengths.reserve(pts.size());
    accumLengths.push_back(0.0f);

    for (usize i {1}; i < pts.size(); ++i) {
        f32 const segLen {hypot(pts[i].X - pts[i - 1].X, pts[i].Y - pts[i - 1].Y)};
        accumLengths.push_back(accumLengths.back() + segLen);
    }

    usize      left {0};
    auto const func {[&](f32 d) -> canvas_point {
        if (d <= accumLengths.front()) { return pts.front(); }
        if (d >= accumLengths.back()) { return pts.back(); }

        if (accumLengths.size() == 2) {
            f32 const d0 {accumLengths[0]};
            f32 const d1 {accumLengths[1]};
            f32 const ratio {(d - d0) / (d1 - d0)};
            return {.X = pts[0].X + (ratio * (pts[1].X - pts[0].X)),
                    .Y = pts[0].Y + (ratio * (pts[1].Y - pts[0].Y))};
        }

        while (left + 1 < accumLengths.size() && accumLengths[left + 1] <= d) { ++left; }
        usize const right {left + 1};

        f32 const d0 {accumLengths[left]};
        f32 const d1 {accumLengths[right]};
        f32 const ratio {(d - d0) / (d1 - d0)};
        return {.X = pts[left].X + (ratio * (pts[right].X - pts[left].X)),
                .Y = pts[left].Y + (ratio * (pts[right].Y - pts[left].Y))};
    }};

    // Compute total dash pattern period.
    f32 const period {std::accumulate(dashPattern.begin(), dashPattern.end(), 0.0f)};

    // Compute effective dash offset.
    f32 effectiveOffset {std::fmod(dashOffset, period)};
    if (effectiveOffset < 0.0f) { effectiveOffset += period; }

    // Start at a negative distance so that our first segment starts at the effective offset.
    f32   currentDistance {-effectiveOffset};
    bool  drawing {true};
    usize dashIndex {0};

    std::vector<std::vector<canvas_point>> dashedPaths {};
    dashedPaths.reserve(16);

    usize polyIdx {1};
    while (currentDistance < totalLength) {
        f32 const segDash {dashPattern[dashIndex++ % dashPattern.size()]};
        f32 const nextDistance {std::min(totalLength, currentDistance + segDash)};

        if (drawing) {
            // Clamp the start of the dash segment to 0 if necessary.
            f32 const drawStart {currentDistance < 0.0f ? 0.0f : currentDistance};
            f32 const drawEnd {nextDistance};

            if (drawEnd > drawStart) {
                std::vector<canvas_point>& dashSegment {dashedPaths.emplace_back()};
                dashSegment.reserve(16);

                // Insert the starting point.
                dashSegment.push_back(func(drawStart));

                // Advance polyIdx: skip any points before the segment.
                while (polyIdx < accumLengths.size() && accumLengths[polyIdx] < drawStart) {
                    ++polyIdx;
                }

                // Insert all intermediate polyline points in [drawStart, drawEnd).
                usize tempIdx {polyIdx};
                while (tempIdx < accumLengths.size() && accumLengths[tempIdx] < drawEnd) {
                    dashSegment.push_back(pts[tempIdx++]);
                }
                polyIdx = tempIdx;

                // Insert the endpoint.
                dashSegment.push_back(func(drawEnd));
                dashSegment.front().Flags = Corner;
                dashSegment.back().Flags  = Corner;
            }
        } else {
            // Even in non-drawing segments, move polyIdx forward.
            while (polyIdx < accumLengths.size() && accumLengths[polyIdx] < nextDistance) {
                ++polyIdx;
            }
        }

        currentDistance = nextDistance;
        drawing         = !drawing;
    }

    return dashedPaths;
}

auto static PolyArea(std::span<canvas_point> pts)
{
    f32 area {0};
    for (usize i {2}; i < pts.size(); ++i) {
        canvas_point const a {pts[0]};
        canvas_point const b {pts[i - 1]};
        canvas_point const c {pts[i]};
        area += TriArea2(a.X, a.Y, b.X, b.Y, c.X, c.Y);
    }
    return area * 0.5f;
}

auto static PolylineLength(std::span<canvas_point const> pts) -> f32
{
    f32 length {0.0f};
    for (usize i {1}; i < pts.size(); ++i) {
        length += hypot(pts[i].X - pts[i - 1].X, pts[i].Y - pts[i - 1].Y);
    }
    return length;
}

////////////////////////////////////////////////////////////

void path_cache::clear()
{
    _commands.clear();
    _points.clear();
    _paths.clear();
}

void path_cache::append_commands(std::span<f32 const> vals, transform const& xform)
{
    usize const size {vals.size()};

    if (static_cast<i32>(vals[0]) != Close && static_cast<i32>(vals[0]) != Winding) {
        _commandPoint = {vals[size - 2], vals[size - 1]};
    }
    _commands.reserve(_commands.size() + size);

    // transform commands
    point_f p;
    for (usize i {0}; i < size;) {
        i32 const cmd {static_cast<i32>(vals[i])};
        _commands.push_back(static_cast<f32>(cmd));
        switch (cmd) {
        case MoveTo:
        case LineTo: {
            p = xform * point_f {vals[i + 1], vals[i + 2]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            i += 3;
        } break;
        case BezierTo: {
            p = xform * point_f {vals[i + 1], vals[i + 2]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            p = xform * point_f {vals[i + 3], vals[i + 4]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            p = xform * point_f {vals[i + 5], vals[i + 6]};
            _commands.push_back(p.X);
            _commands.push_back(p.Y);
            i += 7;
        } break;
        case Close: {
            ++i;
        } break;
        case Winding: {
            _commands.push_back(vals[i + 1]);
            i += 2;
        } break;
        default:
            ++i;
        }
    }
}

void path_cache::fill(state const& s, bool enforceWinding, bool edgeAntiAlias, f32 fringeWidth)
{
    _paths.clear();
    _points.clear();

    flatten_paths(enforceWinding, {}, 0);

    if (edgeAntiAlias && s.ShapeAntiAlias) {
        expand_fill(fringeWidth, line_join::Miter, 2.4f, fringeWidth);
    } else {
        expand_fill(0.0f, line_join::Miter, 2.4f, fringeWidth);
    }
}

void path_cache::stroke(state const& s, bool enforceWinding, bool edgeAntiAlias, f32 strokeWidth, f32 fringeWidth)
{
    if (!s.Dash.empty()) {
        _paths.clear();
        _points.clear();
    }

    if (_paths.empty()) {
        flatten_paths(enforceWinding, s.Dash, s.DashOffset);
    }

    if (edgeAntiAlias && s.ShapeAntiAlias) {
        expand_stroke(strokeWidth * 0.5f, s.LineCap, s.LineJoin, s.MiterLimit, fringeWidth);
    } else {
        expand_stroke(strokeWidth * 0.5f, s.LineCap, s.LineJoin, s.MiterLimit, 0.0f);
    }
}

void path_cache::flatten_paths(bool enforceWinding, std::span<f32 const> dash, f32 dashOffset)
{
    // --- Flatten commands into paths and points (solid geometry) ---
    auto const* ptr {_commands.data()};
    for (usize i {0}; i < _commands.size();) {
        auto const* p {ptr + i};
        i32 const   cmd {static_cast<i32>(p[0])};

        switch (cmd) {
        case MoveTo: {
            add_path();
            add_point(p[1], p[2], Corner);
            i += 3;
        } break;
        case LineTo: {
            add_point(p[1], p[2], Corner);
            i += 3;
        } break;
        case BezierTo: {
            auto const& last {get_last_point()};
            tesselate_bezier(last.X, last.Y, p[1], p[2], p[3], p[4], p[5], p[6], 0, Corner);
            i += 7;
        } break;
        case Close: {
            auto& path {get_last_path()};
            if (path.First < std::ssize(_points)) {
                auto const& pt {_points[path.First]};
                add_point(pt.X, pt.Y, Corner); // loop
                path.Closed = true;
            }
            ++i;
        } break;
        case Winding: {
            get_last_path().Winding = static_cast<winding>(_commands[i + 1]);
            i += 2;
        } break;
        default:
            ++i;
        }
    }

    _bounds[0] = _bounds[1] = 1e6f;
    _bounds[2] = _bounds[3] = -1e6f;

    // --- Apply dash conversion to each flattened path ---
    if (!dash.empty()) {
        std::vector<canvas_point> newPoints;
        std::vector<canvas::path> newPaths;
        for (auto const& p : _paths) {
            // Get the original polyline for this path.
            std::span<canvas_point const> polyline {&_points[p.First], p.Count};
            f32 const                     totalLen {PolylineLength(polyline)};
            std::vector<f32>              dashPattern;
            if (!DashPattern(dashPattern, dash, totalLen)) { continue; }

            auto const dashedPaths {DashPolyline(polyline, totalLen, dashPattern, dashOffset)};
            for (auto const& dp : dashedPaths) {
                canvas::path dashedPath {};
                dashedPath.First = static_cast<i32>(newPoints.size());
                dashedPath.Count = dp.size();
                newPoints.insert(newPoints.end(), dp.begin(), dp.end());
                newPaths.push_back(dashedPath);
            }
        }

        _points = std::move(newPoints);
        _paths  = std::move(newPaths);
    }

    // Update bounds and segment data.
    for (auto& path : _paths) {
        if (path.Count == 0) { continue; }

        auto start {_points.begin() + path.First};

        // If the first and last points coincide, remove the duplicate.
        auto p0It {start + path.Count - 1};
        auto p1It {start};

        if (PointEquals(p0It->X, p0It->Y, p1It->X, p1It->Y, _distTolerance)) {
            --path.Count;
            p0It        = start + path.Count - 1;
            path.Closed = true;
        }

        // Enforce winding if requested.
        if (enforceWinding && path.Count > 2) {
            f32 const area {PolyArea({start, path.Count})};
            if (path.Winding == winding::CCW && area < 0.0f) { PolyReverse({start, path.Count}); }
            if (path.Winding == winding::CW && area > 0.0f) { PolyReverse({start, path.Count}); }
        }

        // Calculate segment direction, length, and update _bounds.
        for (usize i {0}; i < path.Count; ++i) {
            auto&       p0 {*p0It};
            auto const& p1 {*p1It};

            p0.DX      = p1.X - p0.X;
            p0.DY      = p1.Y - p0.Y;
            p0.Length  = Normalize(p0.DX, p0.DY);
            _bounds[0] = std::min(_bounds[0], p0.X);
            _bounds[1] = std::min(_bounds[1], p0.Y);
            _bounds[2] = std::max(_bounds[2], p0.X);
            _bounds[3] = std::max(_bounds[3], p0.Y);
            p0It       = p1It++;
        }
    }

    std::erase_if(_paths, [](auto const& path) { return path.Count == 0; });
}

void path_cache::expand_stroke(f32 w, line_cap lineCap, line_join lineJoin, f32 miterLimit, f32 fringeWidth)
{
    auto static CurveDivs {[](f32 r, f32 arc, f32 tol) {
        f32 const da {std::acos(r / (r + tol)) * 2.0f};
        return std::max(2, static_cast<i32>(std::ceil(arc / da)));
    }};

    f32 const u0 {fringeWidth == 0.0f ? 0.5f : 0.0f};
    f32 const u1 {fringeWidth == 0.0f ? 0.5f : 1.0f};
    i32 const ncap {CurveDivs(w, TAU_F / 2, _tessTolerance)}; // Calculate divisions per half circle.

    w += fringeWidth * 0.5f;

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    usize cverts {0};
    for (auto const& path : _paths) {
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

    vertex* dst {nullptr};
    for (auto& path : _paths) {
        canvas_point* pts {&_points[path.First]};
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
                dst = ButtCapStart(dst, *p0, dx, dy, w, -fringeWidth * 0.5f, fringeWidth, u0, u1);
            } else if (lineCap == line_cap::Square) {
                dst = ButtCapStart(dst, *p0, dx, dy, w, w - fringeWidth, fringeWidth, u0, u1);
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
                SetVertex(dst++, p1->X + (p1->DMX * w), p1->Y + (p1->DMY * w), u0, 1);
                SetVertex(dst++, p1->X - (p1->DMX * w), p1->Y - (p1->DMY * w), u1, 1);
            }
            p0 = p1++;
        }

        if (path.Closed) {
            // Loop it
            SetVertex(dst++, verts[0].Position.X, verts[0].Position.Y, u0, 1);
            SetVertex(dst++, verts[1].Position.X, verts[1].Position.Y, u1, 1);
        } else {
            // Add cap
            f32 dx {p1->X - p0->X};
            f32 dy {p1->Y - p0->Y};
            Normalize(dx, dy);
            if (lineCap == line_cap::Butt) {
                dst = ButtCapEnd(dst, *p1, dx, dy, w, -fringeWidth * 0.5f, fringeWidth, u0, u1);
            } else if (lineCap == line_cap::Square) {
                dst = ButtCapEnd(dst, *p1, dx, dy, w, w - fringeWidth, fringeWidth, u0, u1);
            } else if (lineCap == line_cap::Round) {
                dst = RoundCapEnd(dst, *p1, dx, dy, w, ncap, u0, u1);
            }
        }

        path.StrokeCount = dst - verts;

        verts = dst;
    }
}

void path_cache::expand_fill(f32 w, line_join lineJoin, f32 miterLimit, f32 fringeWidth)
{
    vertex*    dst {nullptr};
    bool const fringe {w > 0.0f};
    f32 const  woff {0.5f * fringeWidth};

    calculate_joins(w, lineJoin, miterLimit);

    // Calculate max vertex usage.
    usize cverts {0};
    for (auto const& path : _paths) {
        cverts += path.Count + path.BevelCount + 1;
        if (fringe) {
            cverts += (path.Count + path.BevelCount * 5 + 1) * 2; // plus one for loop
        }
    }

    vertex* verts {alloc_temp_verts(cverts)};

    bool const convex {_paths.size() == 1 && _paths[0].Convex};

    for (auto& path : _paths) {
        canvas_point* pts {&_points[path.First]};
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
                        f32 const lx {p1->X + (p1->DMX * woff)};
                        f32 const ly {p1->Y + (p1->DMY * woff)};
                        SetVertex(dst++, lx, ly, 0.5f, 1);
                    } else {
                        f32 const lx0 {p1->X + (dlx0 * woff)};
                        f32 const ly0 {p1->Y + (dly0 * woff)};
                        f32 const lx1 {p1->X + (dlx1 * woff)};
                        f32 const ly1 {p1->Y + (dly1 * woff)};
                        SetVertex(dst++, lx0, ly0, 0.5f, 1);
                        SetVertex(dst++, lx1, ly1, 0.5f, 1);
                    }
                } else {
                    SetVertex(dst++, p1->X + (p1->DMX * woff), p1->Y + (p1->DMY * woff), 0.5f, 1);
                }
                p0 = p1++;
            }
        } else {
            for (u32 j {0}; j < path.Count; ++j) {
                SetVertex(dst++, pts[j].X, pts[j].Y, 0.5f, 1);
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
                    SetVertex(dst++, p1->X + (p1->DMX * lw), p1->Y + (p1->DMY * lw), lu, 1);
                    SetVertex(dst++, p1->X - (p1->DMX * rw), p1->Y - (p1->DMY * rw), ru, 1);
                }
                p0 = p1++;
            }

            // Loop it
            SetVertex(dst++, verts[0].Position.X, verts[0].Position.Y, lu, 1);
            SetVertex(dst++, verts[1].Position.X, verts[1].Position.Y, ru, 1);

            path.StrokeCount = dst - verts;
            verts            = dst;
        } else {
            path.Stroke      = nullptr;
            path.StrokeCount = 0;
        }
    }
}

auto path_cache::alloc_temp_verts(usize nverts) -> vertex*
{
    if (nverts > _verts.capacity()) { _verts.reserve((nverts + 0xff) & ~0xff); }
    return _verts.data();
}

auto path_cache::paths() const -> std::vector<canvas::path> const&
{
    return _paths;
}

auto path_cache::has_commands() const -> bool
{
    return !_commands.empty();
}

auto path_cache::command_point() const -> point_f const&
{
    return _commandPoint;
}

auto path_cache::bounds() const -> vec4 const&
{
    return _bounds;
}

void path_cache::set_tolerances(f32 dist, f32 tess)
{
    _distTolerance = dist;
    _tessTolerance = tess;
}

auto path_cache::is_degenerate_arc(point_f pos1, point_f pos2, f32 radius) const -> bool
{
    auto static DistancePointSegment {[](point_f r, point_f p, point_f q) -> f32 {
        f32 const pqx {q.X - p.X};
        f32 const pqy {q.Y - p.Y};
        f32       dx {r.X - p.X};
        f32       dy {r.Y - p.Y};
        f32 const d {(pqx * pqx) + (pqy * pqy)};
        f32       t {(pqx * dx) + (pqy * dy)};
        if (d > 0) { t /= d; }
        t  = std::clamp(t, 0.0f, 1.0f);
        dx = p.X + t * pqx - r.X;
        dy = p.Y + t * pqy - r.Y;
        return (dx * dx) + (dy * dy);
    }};

    return _commandPoint.equals(pos1, _distTolerance)
        || pos1.equals(pos2, _distTolerance)
        || DistancePointSegment(pos1, command_point(), pos2) < _distTolerance * _distTolerance
        || radius < _distTolerance;
}

void path_cache::add_path()
{
    canvas::path path;
    path.First = static_cast<i32>(_points.size());
    _paths.push_back(path);
}

auto path_cache::get_last_path() -> canvas::path&
{
    assert(!_paths.empty());
    return _paths.back();
}

void path_cache::add_point(f32 x, f32 y, i32 flags)
{
    canvas::path& path {get_last_path()};

    if (path.Count > 0 && !_points.empty()) {
        auto& pt {get_last_point()};
        if (PointEquals(pt.X, pt.Y, x, y, _distTolerance)) {
            pt.Flags |= flags;
            return;
        }
    }

    canvas_point& pt {_points.emplace_back()};
    pt.X     = x;
    pt.Y     = y;
    pt.Flags = static_cast<byte>(flags);

    ++path.Count;
    path.Closed = false;
}

auto path_cache::get_last_point() -> canvas_point&
{
    assert(!_points.empty());
    return _points.back();
}

void path_cache::tesselate_bezier(f32 x1, f32 y1, f32 x2, f32 y2, f32 x3, f32 y3, f32 x4, f32 y4, i32 level, i32 type)
{
    if (level > 10) { return; }

    f32 const dx {x4 - x1};
    f32 const dy {y4 - y1};
    f32 const d2 {std::abs((((x2 - x4) * dy) - ((y2 - y4) * dx)))};
    f32 const d3 {std::abs((((x3 - x4) * dy) - ((y3 - y4) * dx)))};

    if ((d2 + d3) * (d2 + d3) < _tessTolerance * (dx * dx + dy * dy)) {
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

void path_cache::calculate_joins(f32 w, line_join lineJoin, f32 miterLimit)
{
    f32 const iw {w > 0.0f ? (1.0f / w) : 0.0f};

    // Calculate which joins needs extra vertices to append, and gather vertex count.
    for (auto& path : _paths) {
        auto  start {_points.begin() + path.First};
        auto  p0It {start + path.Count - 1};
        auto  p1It {start};
        usize nleft {0};

        path.BevelCount = 0;

        for (usize j {0}; j < path.Count; j++) {
            auto const& p0 {*p0It};
            auto&       p1 {*p1It};

            f32 const dlx0 {p0.DY};
            f32 const dly0 {-p0.DX};
            f32 const dlx1 {p1.DY};
            f32 const dly1 {-p1.DX};
            // Calculate extrusions
            p1.DMX = (dlx0 + dlx1) * 0.5f;
            p1.DMY = (dly0 + dly1) * 0.5f;
            f32 const dmr2 {(p1.DMX * p1.DMX) + (p1.DMY * p1.DMY)};
            if (dmr2 > 0.000001f) {
                f32 const scale {std::min(1.0f / dmr2, 600.0f)};
                p1.DMX *= scale;
                p1.DMY *= scale;
            }

            // Clear flags, but keep the corner.
            p1.Flags = (p1.Flags & Corner) ? Corner : 0;

            // Keep track of left turns.
            f32 const cross {(p1.DX * p0.DY) - (p0.DX * p1.DY)};
            if (cross > 0.0f) {
                ++nleft;
                p1.Flags |= Left;
            }

            // Calculate if we should use bevel or miter for inner join.
            f32 const limit {std::max(1.01f, std::min(p0.Length, p1.Length) * iw)};
            if ((dmr2 * limit * limit) < 1.0f) {
                p1.Flags |= InnerBevel;
            }

            // Check to see if the corner needs to be beveled.
            if (p1.Flags & Corner) {
                if ((dmr2 * miterLimit * miterLimit) < 1.0f || lineJoin == line_join::Bevel || lineJoin == line_join::Round) {
                    p1.Flags |= Bevel;
                }
            }

            if ((p1.Flags & (Bevel | InnerBevel)) != 0) {
                ++path.BevelCount;
            }

            p0It = p1It++;
        }

        path.Convex = (nleft == path.Count);
    }
}

}

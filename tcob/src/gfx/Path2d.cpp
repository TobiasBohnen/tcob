// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Path2d.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <variant>
#include <vector>

#include "Canvas_private.hpp"

#include "tcob/core/AngleUnits.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/StringUtils.hpp"
#include "tcob/core/easing/Easing.hpp"
#include "tcob/gfx/Gfx.hpp"
#include "tcob/gfx/Polygon.hpp"

namespace tcob::gfx {

auto path2d::Parse(string_view path) -> std::optional<path2d>
{
    static auto const getPoint {[](std::optional<std::vector<f32>> const& values) -> point_f { return {(*values)[0], (*values)[1]}; }};

    auto const commands {GetCommands(path)};
    if (!commands) { return std::nullopt; }

    path2d retValue;

    char command {0};
    for (usize idx {0}; idx < commands->size();) {
        auto const getValues {[&idx, &commands](usize size) -> std::optional<std::vector<f32>> {
            std::vector<f32> values(size);
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

            retValue.move_to(isAbs ? getPoint(p) : getPoint(p) + retValue._lastPoint);
            command = isAbs ? 'L' : 'l';
        } break;
        case 'l':
        case 'L': {
            auto const p {getValues(2)};
            if (!p) { return std::nullopt; }

            retValue.line_to(isAbs ? getPoint(p) : getPoint(p) + retValue._lastPoint);
        } break;
        case 'h':
        case 'H': {
            auto const p {getValues(1)};
            if (!p) { return std::nullopt; }

            retValue.line_to(isAbs ? point_f {(*p)[0], retValue._lastPoint.Y} : point_f {(*p)[0] + retValue._lastPoint.X, retValue._lastPoint.Y});
        } break;
        case 'v':
        case 'V': {
            auto const p {getValues(1)};
            if (!p) { return std::nullopt; }

            retValue.line_to(isAbs ? point_f {retValue._lastPoint.X, (*p)[0]} : point_f {retValue._lastPoint.X, (*p)[0] + retValue._lastPoint.Y});
        } break;
        case 'q':
        case 'Q': {
            auto const cpv {getValues(2)};
            auto const endv {getValues(2)};
            if (!cpv || !endv) { return std::nullopt; }
            point_f const cp {isAbs ? getPoint(cpv) : getPoint(cpv) + retValue._lastPoint};
            point_f const end {isAbs ? getPoint(endv) : getPoint(endv) + retValue._lastPoint};

            retValue.quad_bezier_to(cp, end);
        } break;
        case 't':
        case 'T': {
            auto const endv {getValues(2)};
            if (!endv) { return std::nullopt; }
            point_f const end {isAbs ? getPoint(endv) : getPoint(endv) + retValue._lastPoint};
            point_f const cp {(2 * retValue._lastPoint.X) - retValue._lastQuadControlPoint.X, (2 * retValue._lastPoint.Y) - retValue._lastQuadControlPoint.Y};

            retValue.quad_bezier_to(cp, end);
        } break;
        case 'c':
        case 'C': {
            auto const cp0v {getValues(2)};
            auto const cp1v {getValues(2)};
            auto const endv {getValues(2)};
            if (!cp0v || !cp1v || !endv) { return std::nullopt; }
            auto const cp0 {isAbs ? getPoint(cp0v) : getPoint(cp0v) + retValue._lastPoint};
            auto const cp1 {isAbs ? getPoint(cp1v) : getPoint(cp1v) + retValue._lastPoint};
            auto const end {isAbs ? getPoint(endv) : getPoint(endv) + retValue._lastPoint};

            retValue.cubic_bezier_to(cp0, cp1, end);
        } break;
        case 's':
        case 'S': {
            auto const cp1v {getValues(2)};
            auto const endv {getValues(2)};
            if (!cp1v || !endv) { return std::nullopt; }
            point_f const cp0 {(2 * retValue._lastPoint.X) - retValue._lastCubicControlPoint.X, (2 * retValue._lastPoint.Y) - retValue._lastCubicControlPoint.Y};
            auto const    cp1 {isAbs ? getPoint(cp1v) : getPoint(cp1v) + retValue._lastPoint};
            auto const    end {isAbs ? getPoint(endv) : getPoint(endv) + retValue._lastPoint};

            retValue.cubic_bezier_to(cp0, cp1, end);
        } break;
        case 'a':
        case 'A': {
            auto const valv {getValues(7)};
            if (!valv) { return std::nullopt; }
            auto const&   val {*valv};
            point_f const end {isAbs ? point_f {val[5], val[6]} : point_f {val[5], val[6]} + retValue._lastPoint};

            retValue.arc_to(val[0], val[1], degree_f {val[2]}, val[3] > 1e-6, val[4] > 1e-6, end);
        } break;
        case 'z':
        case 'Z':
            retValue.close();
            break;
        default:
            return std::nullopt;
        }
    }

    return retValue;
}

void path2d::move_to(point_f pos)
{
    std::vector<f32> const cmd {CommandsMoveTo(pos)};

    Commands.insert(Commands.end(), cmd.begin(), cmd.end());
    _lastPoint = pos;
}

void path2d::line_to(point_f pos)
{
    std::vector<f32> const cmd {CommandsLineTo(pos)};

    Commands.insert(Commands.end(), cmd.begin(), cmd.end());
    _lastPoint = pos;
}

void path2d::cubic_bezier_to(point_f cp0, point_f cp1, point_f end)
{
    std::vector<f32> const cmd {CommandsCubicTo(cp0, cp1, end)};

    Commands.insert(Commands.end(), cmd.begin(), cmd.end());
    _lastPoint             = end;
    _lastCubicControlPoint = cp1;
}

void path2d::quad_bezier_to(point_f cp, point_f end)
{
    std::vector<f32> const cmd {CommandsQuadTo(_lastPoint, cp, end)};

    Commands.insert(Commands.end(), cmd.begin(), cmd.end());
    _lastPoint            = end;
    _lastQuadControlPoint = cp;
}

void path2d::arc_to(f32 radiusX, f32 radiusY, degree_f rotX, bool largeArc, bool sweep, point_f end)
{
    static auto nsvg_xformPoint {[](f32* dx, f32* dy, f32 x, f32 y, std::array<f32, 6> const& t) {
        *dx = x * t[0] + y * t[2] + t[4];
        *dy = x * t[1] + y * t[3] + t[5];
    }};
    static auto nsvg_xformVec {[](f32* dx, f32* dy, f32 x, f32 y, std::array<f32, 6> const& t) {
        *dx = x * t[0] + y * t[2];
        *dy = x * t[1] + y * t[3];
    }};
    static auto nsvg_sqr {[](f32 x) -> f32 { return x * x; }};
    static auto nsvg_vecang {[](f32 ux, f32 uy, f32 vx, f32 vy) -> f32 {
        f32 const r {(ux * vx + uy * vy) / (std::sqrt((ux * ux) + (uy * uy)) * std::sqrt((vx * vx) + (vy * vy)))};
        return ((ux * vy < uy * vx) ? -1.0f : 1.0f) * std::acos(std::clamp(r, -1.0f, 1.0f));
    }};

    // based on https://github.com/memononen/nanosvg

    f32            rx {std::abs(radiusX)}; // y radius
    f32            ry {std::abs(radiusY)}; // x radius
    radian_f const rotx {rotX};            // x rotation angle

    auto const [x0, y0] {_lastPoint};      // start point
    auto const [x1, y1] {end};             // end point

    f32 dx {x0 - x1};
    f32 dy {y0 - y1};
    f32 d {std::sqrt((dx * dx) + (dy * dy))};
    if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
        line_to(end);
        return;
    }

    f32 const sinrx {rotx.sin()};
    f32 const cosrx {rotx.cos()};

    // Convert to center point parameterization.
    // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
    // 1) Compute x1', y1'
    f32 const x1p {(cosrx * dx / 2.0f) + (sinrx * dy / 2.0f)};
    f32 const y1p {(-sinrx * dx / 2.0f) + (cosrx * dy / 2.0f)};
    d = nsvg_sqr(x1p) / nsvg_sqr(rx) + nsvg_sqr(y1p) / nsvg_sqr(ry);
    if (d > 1) {
        d = std::sqrt(d);
        rx *= d;
        ry *= d;
    }
    // 2) Compute cx', cy'
    f32       s {0.0f};
    f32 const sa {std::max((nsvg_sqr(rx) * nsvg_sqr(ry)) - (nsvg_sqr(rx) * nsvg_sqr(y1p)) - (nsvg_sqr(ry) * nsvg_sqr(x1p)), 0.0f)};
    f32 const sb {(nsvg_sqr(rx) * nsvg_sqr(y1p)) + (nsvg_sqr(ry) * nsvg_sqr(x1p))};
    if (sb > 0.0f) { s = std::sqrt(sa / sb); }
    if (largeArc == sweep) { s = -s; }
    f32 const cxp {s * rx * y1p / ry};
    f32 const cyp {s * -ry * x1p / rx};

    // 3) Compute cx,cy from cx',cy'
    f32 const cx {((x0 + x1) / 2.0f) + (cosrx * cxp) - (sinrx * cyp)};
    f32 const cy {((y0 + y1) / 2.0f) + (sinrx * cxp) + (cosrx * cyp)};

    // 4) Calculate theta1, and delta theta.
    f32 const ux {(x1p - cxp) / rx};
    f32 const uy {(y1p - cyp) / ry};
    f32 const vx {(-x1p - cxp) / rx};
    f32 const vy {(-y1p - cyp) / ry};
    f32 const a1 {nsvg_vecang(1.0f, 0.0f, ux, uy)}; // Initial angle
    f32       da {nsvg_vecang(ux, uy, vx, vy)};     // Delta angle

    if (!sweep && da > 0) {
        da -= TAU_F;
    } else if (sweep && da < 0) {
        da += TAU_F;
    }

    // Approximate the arc using cubic spline segments.
    std::array<f32, 6> const t {cosrx, sinrx, -sinrx, cosrx, cx, cy};

    // Split arc into max 90 degree segments.
    // The loop assumes an iteration per end point (including start and end), this +1.
    i32 const ndivs {static_cast<i32>((std::abs(da) / (TAU_F / 4)) + 1.0f)};
    f32       hda {(da / static_cast<f32>(ndivs)) / 2.0f};
    // Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
    if ((hda < 1e-3f) && (hda > -1e-3f)) {
        hda *= 0.5f;
    } else {
        hda = (1.0f - std::cos(hda)) / std::sin(hda);
    }
    f32 kappa {std::abs(4.0f / 3.0f * hda)};
    if (da < 0.0f) { kappa = -kappa; }

    f32 x {0}, y {0};
    f32 tanx {0}, tany {0};
    f32 px {0}, py {0};
    f32 ptanx {0}, ptany {0};
    for (i32 i {0}; i <= ndivs; ++i) {
        f32 const a {a1 + (da * (static_cast<f32>(i) / static_cast<f32>(ndivs)))};
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

    _lastPoint = end;
}

void path2d::close()
{
    Commands.emplace_back(Close);
}

auto path2d::polygonize() -> std::vector<polygon>
{
    std::vector<polygon> retValue;

    f32 const            tolerance {0.05f};
    point_f              curPos;
    std::vector<point_f> points;

    auto const addPoly {[&] {
        if (!points.empty()) {
            auto const winding {polygons::get_winding(points)};
            if (winding == winding::CCW) {
                retValue.emplace_back().Outline = points;
            } else if (!retValue.empty()) {
                retValue.at(retValue.size() - 1).Holes.push_back(points);
            }

            points.clear();
        }
    }};

    usize       idx {0};
    usize const nvals {Commands.size()};
    while (idx < nvals) {
        i32 cmd {static_cast<i32>(Commands[idx])};
        switch (cmd) {
        case MoveTo: {
            curPos = point_f {Commands[idx + 1], Commands[idx + 2]};
            idx += 3;
        } break;
        case LineTo: {
            points.push_back(curPos);
            points.emplace_back(Commands[idx + 1], Commands[idx + 2]);
            curPos = points.back();
            idx += 3;
        } break;
        case BezierTo: {
            easing::quad_bezier_curve func;
            func.StartPoint   = {Commands[idx + 1], Commands[idx + 2]};
            func.ControlPoint = {Commands[idx + 3], Commands[idx + 4]};
            func.EndPoint     = {Commands[idx + 5], Commands[idx + 6]};
            for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
            curPos = func.EndPoint;
            idx += 7;
        } break;
        case Close: {
            easing::linear<point_f> func;
            func.Start = points[0];
            func.End   = curPos;
            for (f32 i {0}; i <= 1.0f; i += tolerance) { points.push_back(func(i)); }
            curPos = func.End;
            ++idx;
            addPoly();
        } break;
        default:
            ++idx;
        }
    }

    return retValue;
}

auto path2d::GetCommands(string_view path) -> std::optional<std::vector<std::variant<char, f32>>>
{
    std::vector<std::variant<char, f32>> commands;
    string                               valStr;

    auto const getFloat {[&valStr, &commands] -> bool {
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

auto path2d::CommandsMoveTo(point_f pos) -> std::vector<f32>
{
    return {MoveTo, pos.X, pos.Y};
}

auto path2d::CommandsLineTo(point_f pos) -> std::vector<f32>
{
    return {LineTo, pos.X, pos.Y};
}

auto path2d::CommandsCubicTo(point_f cp0, point_f cp1, point_f end) -> std::vector<f32>
{
    return {BezierTo, cp0.X, cp0.Y, cp1.X, cp1.Y, end.X, end.Y};
}

auto path2d::CommandsQuadTo(point_f start, point_f cp, point_f end) -> std::vector<f32>
{
    auto const [x0, y0] {start};

    return {
        BezierTo,
        x0 + (2.0f / 3.0f * (cp.X - x0)), y0 + (2.0f / 3.0f * (cp.Y - y0)),
        end.X + (2.0f / 3.0f * (cp.X - end.X)), end.Y + (2.0f / 3.0f * (cp.Y - end.Y)),
        end.X, end.Y};
}

} // namespace gfx

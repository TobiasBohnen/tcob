// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Polygon.hpp"

#include "tcob/gfx/Gfx.hpp"

#include "EarcutHelper.hpp"
#include <clipper2/clipper.h>

namespace tcob::gfx {

static auto get_poly_info(polyline_span points) -> polygon::info
{
    polygon::info retValue;

    point_f max {std::numeric_limits<f32>::denorm_min(), std::numeric_limits<f32>::denorm_min()};
    point_f min {std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max()};

    f32 signedArea {0.0f};
    retValue.Centroid = point_f::Zero;

    usize const n {points.size()};

    for (usize i {0}; i < n; ++i) {
        point_f const& p0 {points[i]};
        max.X = std::max(p0.X, max.X);
        max.Y = std::max(p0.Y, max.Y);
        min.X = std::min(p0.X, min.X);
        min.Y = std::min(p0.Y, min.Y);

        point_f const& p1 {points[(i + 1) % n]};
        f32 const      a {(p0.X * p1.Y) - (p1.X * p0.Y)};
        signedArea += a;
        retValue.Centroid.X += (p0.X + p1.X) * a;
        retValue.Centroid.Y += (p0.Y + p1.Y) * a;
    }

    signedArea *= 0.5f;
    retValue.Centroid /= (6.0f * signedArea);

    retValue.BoundingBox = rect_f::FromLTRB(min.X, min.Y, max.X, max.Y);

    return retValue;
}

auto polygon::get_info() const -> info
{
    std::vector<point_f> points;
    points.insert(points.end(), Outline.begin(), Outline.end());
    return get_poly_info(points);
}

auto polygon::earcut() const -> std::vector<u32>
{
    std::vector<polyline_span> earcut;
    earcut.emplace_back(Outline);
    earcut.insert(earcut.end(), Holes.begin(), Holes.end());
    return mapbox::earcut<u32>(earcut);
}

void polygon::apply_transform(transform const& xform)
{
    for (auto& p : Outline) { p = xform * p; }
    for (auto& hole : Holes) {
        for (auto& p : hole) { p = xform * p; }
    }
}

////////////////////////////////////////////////////////////

auto polygons::get_winding(polyline_span polyline) -> winding
{
    if (polyline.size() < 3) { return {}; }
    f32 signedArea {0.0f};
    for (usize i {0}; i < polyline.size(); ++i) {
        point_f const& current {polyline[i]};
        point_f const& next {polyline[(i + 1) % polyline.size()]};
        signedArea += (next.X - current.X) * (next.Y + current.Y);
    }
    if (signedArea > 0) { return winding::CCW; }
    if (signedArea < 0) { return winding::CW; }
    return {};
}

auto polygons::check_winding(std::span<polygon const> polygons) -> bool
{
    return std::ranges::all_of(polygons, [](auto const& poly) {
        if (get_winding(poly.Outline) != winding::CCW) { return false; }
        return std::ranges::all_of(poly.Holes, [](auto const& hole) {
            return get_winding(hole) == winding::CW;
        });
    });
}

auto polygons::is_point_inside(point_f point, polyline_span polyline) -> bool
{
    f32 minX {polyline[0].X};
    f32 maxX {polyline[0].X};
    f32 minY {polyline[0].Y};
    f32 maxY {polyline[0].Y};
    for (usize i {1}; i < polyline.size(); i++) {
        point_f const& q {polyline[i]};
        minX = std::min(q.X, minX);
        maxX = std::max(q.X, maxX);
        minY = std::min(q.Y, minY);
        maxY = std::max(q.Y, maxY);
    }

    if (point.X < minX || point.X > maxX || point.Y < minY || point.Y > maxY) { return false; }
    bool inside {false};
    for (usize i {0}, j {polyline.size() - 1}; i < polyline.size(); j = i++) {
        if ((polyline[i].Y > point.Y) != (polyline[j].Y > point.Y)
            && point.X < (polyline[j].X - polyline[i].X) * (point.Y - polyline[i].Y) / (polyline[j].Y - polyline[i].Y) + polyline[i].X) {
            inside = !inside;
        }
    }

    return inside;
}

auto polygons::get_info(std::span<polygon const> polygons) -> polygon::info
{
    std::vector<point_f> points;
    for (auto const& polygon : polygons) {
        points.insert(points.end(), polygon.Outline.begin(), polygon.Outline.end());
    }

    return get_poly_info(points);
}

auto polygons::get_info(polyline_span polyline) -> polygon::info
{
    return get_poly_info(polyline);
}

void polygons::move_by(std::span<polygon> polygons, point_f offset)
{
    for (auto& polygon : polygons) {
        for (auto& p : polygon.Outline) { p += offset; }
        for (auto& hole : polygon.Holes) {
            for (auto& p : hole) { p += offset; }
        }
    }
}

void polygons::clip(std::vector<polygon>& polygons, std::span<polygon const> other, clip_mode mode)
{
    using namespace Clipper2Lib;

    // Helper function to create Clipper paths
    auto const createPath {[](std::span<polygon const> polys) -> PathsD {
        auto const addPolygonPath = [](auto const& polyline, PathsD& retValue) {
            std::vector<f32> points;
            for (auto const& p : polyline) {
                points.push_back(p.X);
                points.push_back(p.Y);
            }
            if (points.size() < 2) { return; } // Ignore degenerate polygons
            points.push_back(points[0]);       // Close the path
            points.push_back(points[1]);
            retValue.push_back(MakePathD(points));
        };

        PathsD retValue;
        for (auto const& poly : polys) {
            addPolygonPath(poly.Outline, retValue);
            for (auto const& hole : poly.Holes) {
                addPolygonPath(hole, retValue);
            }
        }

        return retValue;
    }};

    ClipperD clipper;
    clipper.AddSubject(createPath(polygons));
    clipper.AddClip(createPath(other));

    polygons.clear();

    ClipType type {ClipType::None};
    switch (mode) {
    case clip_mode::Intersection: type = ClipType::Intersection; break;
    case clip_mode::Union: type = ClipType::Union; break;
    case clip_mode::Difference: type = ClipType::Difference; break;
    case clip_mode::Xor: type = ClipType::Xor; break;
    }

    PolyPathD                   solution;
    [[maybe_unused]] bool const result {clipper.Execute(type, FillRule::NonZero, solution)};
    assert(result);

    std::function<void(PolyPathD*, polygon*)> const parseSolution {
        [&](PolyPathD* node, polygon* parentPoly) {
            polyline* target {nullptr};
            if (!node->IsHole()) { // If not a hole, it's a new polygon outline
                parentPoly = &polygons.emplace_back();
                target     = &parentPoly->Outline;
            } else {               // It's a hole, add to the parent polygon
                assert(parentPoly);
                target = &parentPoly->Holes.emplace_back();
            }

            for (auto const& pt : node->Polygon()) { target->emplace_back(pt.x, pt.y); }
            if (!node->IsHole()) {
                if (polygons::get_winding(*target) == winding::CW) { std::ranges::reverse(*target); }
            } else {
                if (polygons::get_winding(*target) == winding::CCW) { std::ranges::reverse(*target); }
            }

            for (usize i {0}; i < node->Count(); ++i) {
                parseSolution(node->Child(i), parentPoly);
            }
        }};

    assert(solution.Polygon().empty()); // Ensure no top-level polygons
    for (u32 i {0}; i < solution.Count(); ++i) {
        parseSolution(solution.Child(i), nullptr);
    }
}

void polygons::offset(std::vector<polygon>& polygons, f64 delta, offset_join join)
{
    using namespace Clipper2Lib;

    // Helper function to create Clipper paths
    auto const createPath {[](std::span<polygon const> polys) -> Paths64 {
        auto const addPolygonPath = [](auto const& polyline, Paths64& retValue) {
            std::vector<i64> points;
            for (auto const& p : polyline) {
                points.push_back(static_cast<i64>(p.X));
                points.push_back(static_cast<i64>(p.Y));
            }
            if (points.size() < 2) { return; } // Ignore degenerate polygons
            points.push_back(points[0]);       // Close the path
            points.push_back(points[1]);
            retValue.push_back(MakePath(points));
        };

        Paths64 retValue;
        for (auto const& poly : polys) {
            addPolygonPath(poly.Outline, retValue);
            for (auto const& hole : poly.Holes) {
                addPolygonPath(hole, retValue);
            }
        }

        return retValue;
    }};

    JoinType joinType {JoinType::Round};
    switch (join) {
    case offset_join::Square: joinType = JoinType::Square; break;
    case offset_join::Bevel: joinType = JoinType::Bevel; break;
    case offset_join::Round: joinType = JoinType::Round; break;
    case offset_join::Miter: joinType = JoinType::Miter; break;
    }

    ClipperOffset clipper;
    clipper.AddPaths(createPath(polygons), joinType, EndType::Polygon);

    polygons.clear();

    PolyPath64 solution;
    clipper.Execute(delta, solution);

    std::function<void(PolyPath64*, polygon*)> const parseSolution {
        [&](PolyPath64* node, polygon* parentPoly) {
            polyline* target {nullptr};
            if (!node->IsHole()) { // If not a hole, it's a new polygon outline
                parentPoly = &polygons.emplace_back();
                target     = &parentPoly->Outline;
            } else {               // It's a hole, add to the parent polygon
                assert(parentPoly);
                target = &parentPoly->Holes.emplace_back();
            }

            for (auto const& pt : node->Polygon()) { target->emplace_back(pt.x, pt.y); }
            if (!node->IsHole()) {
                if (polygons::get_winding(*target) == winding::CW) { std::ranges::reverse(*target); }
            } else {
                if (polygons::get_winding(*target) == winding::CCW) { std::ranges::reverse(*target); }
            }

            for (usize i {0}; i < node->Count(); ++i) {
                parseSolution(node->Child(i), parentPoly);
            }
        }};

    assert(solution.Polygon().empty()); // Ensure no top-level polygons
    for (u32 i {0}; i < solution.Count(); ++i) {
        parseSolution(solution.Child(i), nullptr);
    }
}

} // namespace gfx

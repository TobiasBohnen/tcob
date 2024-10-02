// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/Polygon.hpp"

#include "tcob/gfx/Gfx.hpp"

#include "EarcutHelper.hpp"
#include <clipper2/clipper.h>

namespace tcob::gfx {

static auto check_winding_order(polyline_span points) -> winding
{
    if (points.size() < 3) { return {}; }
    f32 signedArea {0.0f};
    for (usize i {0}; i < points.size(); ++i) {
        point_f const& current {points[i]};
        point_f const& next {points[(i + 1) % points.size()]};
        signedArea += (next.X - current.X) * (next.Y + current.Y);
    }
    if (signedArea > 0) { return winding::CCW; }
    if (signedArea < 0) { return winding::CW; }
    return {};
}

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

auto polygon::check_winding() const -> bool
{
    if (check_winding_order(Outline) != winding::CCW) { return false; }
    return std::ranges::all_of(Holes, [](auto const& hole) {
        return check_winding_order(hole) == winding::CW;
    });
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

////////////////////////////////////////////////////////////

polygons::polygons(std::initializer_list<polygon> polygons)
    : _polygons {polygons}
{
}

polygons::polygons(std::span<polygon const> polygons)
{
    _polygons.assign(polygons.begin(), polygons.end());
}

auto polygons::begin() -> iterator
{
    return _polygons.begin();
}

auto polygons::begin() const -> const_iterator
{
    return _polygons.begin();
}

auto polygons::end() -> iterator
{
    return _polygons.end();
}

auto polygons::end() const -> const_iterator
{
    return _polygons.end();
}

auto polygons::add() -> polygon&
{
    return _polygons.emplace_back();
}

auto polygons::size() const -> isize
{
    return std::ssize(_polygons);
}

auto polygons::is_empty() const -> bool
{
    return _polygons.empty();
}

void polygons::clear()
{
    _polygons.clear();
}

auto polygons::check_winding() const -> bool
{
    return std::ranges::all_of(_polygons, [](auto const& poly) { return poly.check_winding(); });
}

auto polygons::get_info() const -> polygon::info
{
    std::vector<point_f> points;
    for (auto const& polygon : _polygons) {
        points.insert(points.end(), polygon.Outline.begin(), polygon.Outline.end());
    }

    return get_poly_info(points);
}

void polygons::move_by(point_f offset)
{
    for (auto& polygon : _polygons) {
        for (auto& p : polygon.Outline) { p += offset; }
        for (auto& hole : polygon.Holes) {
            for (auto& p : hole) { p += offset; }
        }
    }
}

void polygons::clip(polygons const& other, clip_mode mode)
{
    using namespace Clipper2Lib;

    // Helper function to create Clipper paths
    auto const createPath {[](polygons const& polys) -> PathsD {
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
    clipper.AddSubject(createPath(*this));
    clipper.AddClip(createPath(other));

    _polygons.clear();

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
                parentPoly = &add();
                target     = &parentPoly->Outline;
            } else {               // It's a hole, add to the parent polygon
                assert(parentPoly);
                target = &parentPoly->Holes.emplace_back();
            }

            for (auto const& pt : node->Polygon()) { target->emplace_back(pt.x, pt.y); }
            if (!node->IsHole()) {
                if (check_winding_order(*target) == winding::CW) { std::ranges::reverse(*target); }
            } else {
                if (check_winding_order(*target) == winding::CCW) { std::ranges::reverse(*target); }
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

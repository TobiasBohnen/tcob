// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/LightingSystem.hpp"

#include <optional>

namespace tcob::gfx {

lighting_system::lighting_system()
    : _lightSources {}
    , _shadowCasters {}
{
    Bounds.Changed.connect([&](auto const&) { request_redraw(); });

    _renderer.set_material(_material);
}

void lighting_system::remove_light_source(light_source const& emitter)
{
    _lightSources.erase(std::find_if(_lightSources.begin(), _lightSources.end(), [&emitter](auto const& val) {
        if (val.get() == &emitter) {
            val.get()->_parent = nullptr;
            return true;
        }

        return false;
    }));
    request_redraw();
}

void lighting_system::clear_light_sources()
{
    for (auto& ls : _lightSources) { ls->_parent = nullptr; }
    _lightSources.clear();
    request_redraw();
}

void lighting_system::remove_shadow_caster(shadow_caster const& emitter)
{
    _shadowCasters.erase(std::find_if(_shadowCasters.begin(), _shadowCasters.end(), [&emitter](auto const& val) {
        if (val.get() == &emitter) {
            val.get()->_parent = nullptr;
            return true;
        }

        return false;
    }));
    request_redraw();
}

void lighting_system::clear_shadow_casters()
{
    for (auto& sc : _shadowCasters) { sc->_parent = nullptr; }
    _shadowCasters.clear();
    request_redraw();
}

void lighting_system::request_redraw()
{
    _isDirty = true;
}

void lighting_system::set_blend_funcs(blend_funcs funcs)
{
    _material->BlendFuncs = funcs;
}

struct collision_result {
    point_f        Point {};
    f32            Distance {};
    usize          CollisionCount {};
    shadow_caster* Caster {nullptr};
};

struct caster_points {
    std::span<point_f const> Points {};
    shadow_caster*           Caster {nullptr};
};

static auto IsPointInPolygon(point_f p, std::span<point_f const> points) -> bool
{
    f32 minX {points[0].X};
    f32 maxX {points[0].X};
    f32 minY {points[0].Y};
    f32 maxY {points[0].Y};
    for (usize i {1}; i < points.size(); i++) {
        point_f const& q {points[i]};
        minX = std::min(q.X, minX);
        maxX = std::max(q.X, maxX);
        minY = std::min(q.Y, minY);
        maxY = std::max(q.Y, maxY);
    }

    if (p.X < minX || p.X > maxX || p.Y < minY || p.Y > maxY) { return false; }
    bool inside {false};
    for (usize i {0}, j {points.size() - 1}; i < points.size(); j = i++) {
        if ((points[i].Y > p.Y) != (points[j].Y > p.Y) && p.X < (points[j].X - points[i].X) * (p.Y - points[i].Y) / (points[j].Y - points[i].Y) + points[i].X) {
            inside = !inside;
        }
    }

    return inside;
}

void lighting_system::on_update(milliseconds /* deltaTime */)
{
    if (!_isDirty) { return; }

    _updateGeometry = true;
    _isDirty        = false;

    _verts.clear();
    _inds.clear();
    i32 indOffset {0};

    // collect collision points
    std::vector<caster_points> casterPoints;
    casterPoints.reserve(_shadowCasters.size() + 1);
    for (auto const& sc : _shadowCasters) { casterPoints.emplace_back(sc->Points(), sc.get()); }
    std::array<point_f, 4> boundPoints {{Bounds->top_left(), Bounds->bottom_left(), Bounds->bottom_right(), Bounds->top_right()}};
    casterPoints.emplace_back(boundPoints);

    for (auto const& ls : _lightSources) {
        bool const limitAngle {ls->StartAngle() || ls->EndAngle()};
        auto const lightPosition {ls->Position()};
        bool const limitRange {ls->Range()};

        bool lightInsideShadowCaster {false};
        for (usize i {0}; i < casterPoints.size() - 1; ++i) {
            if (casterPoints[i].Points.empty()) { continue; }
            lightInsideShadowCaster = IsPointInPolygon(ls->Position, casterPoints[i].Points);
            if (lightInsideShadowCaster) { break; }
        }

        std::map<f32, collision_result, std::greater<>> collisionResult;
        std::set<f32>                                   angles;

        // collect angles
        for (auto const& cp : casterPoints) {
            for (auto const& scp : cp.Points) {
                std::array<f32, 3> vars {-0.00005f, 0, 0.00005f};
                for (f32 var : vars) {
                    auto const deg {lightPosition.angle_to(scp).as_normalized()};
                    if (limitAngle && (deg < *ls->StartAngle || deg > *ls->EndAngle)) { continue; }
                    angles.insert(degree_f {deg.Value - 90 + var}.as_normalized().Value);
                }
            }
        }
        if (limitAngle) {
            angles.insert(degree_f {ls->StartAngle->Value - 90}.as_normalized().Value);
            angles.insert(degree_f {ls->EndAngle->Value - 90}.as_normalized().Value);
        }

        if (limitRange) {
            if (limitAngle) {
                for (f32 i {ls->StartAngle->Value}; i < ls->EndAngle->Value; ++i) { angles.insert(i - 90); }
            } else {
                for (i32 i {0}; i < 360; ++i) { angles.insert(i - 90); }
            }
        }

        // ray cast
        for (auto angle : angles) {
            collision_result nearestPoint;
            nearestPoint.Distance = std::numeric_limits<f32>::max();

            for (auto const& cp : casterPoints) {
                auto const result {ray_intersects_polygon(lightPosition, angle, cp.Points)};
                if (result.empty()) { continue; }
                for (auto const& p : result) {
                    f32 const dist {lightPosition.distance_to(p)};
                    if (dist >= nearestPoint.Distance) { continue; }

                    nearestPoint.Point          = p;
                    nearestPoint.Distance       = dist;
                    nearestPoint.CollisionCount = result.size();
                    nearestPoint.Caster         = cp.Caster;
                }
            }

            if (nearestPoint.Distance == std::numeric_limits<f32>::max()) { continue; }
            collisionResult[angle] = nearestPoint;
        }

        // discard close and stray(FIXME) points
        // move out-of-range points into range
        std::vector<collision_result> collisionResult1;
        collisionResult1.reserve(collisionResult.size());
        for (auto& [k, v] : collisionResult) {
            if (!lightInsideShadowCaster && (v.CollisionCount == 1 && v.Caster != nullptr)) { continue; }
            if (!collisionResult1.empty() && collisionResult1.back().Point.distance_to(v.Point) < 1) { continue; }
            if (limitRange && v.Distance > *ls->Range) {
                f32 const lightRange {*ls->Range()};
                point_f   direction = (v.Point - lightPosition).as_normalized();
                v.Point             = lightPosition + direction * lightRange;
                v.Distance          = lightRange;
            }

            collisionResult1.push_back(v);
        }

        // prepare geometry
        u32 const n {static_cast<u32>(collisionResult1.size())};
        if (n < 2) { continue; }

        _verts.push_back({.Position  = ls->Position->as_array(),
                          .Color     = ls->Color->as_array(),
                          .TexCoords = {0, 0, 0}});

        for (auto const& p : collisionResult1) {
            auto col {ls->Color()};
            if (limitRange) {
                f32 const lightRange {*ls->Range()};
                f32 const falloff {std::clamp(1.0f - (p.Distance / lightRange), 0.0f, 1.0f)};
                col.R = static_cast<u8>(col.R * falloff);
                col.G = static_cast<u8>(col.G * falloff);
                col.B = static_cast<u8>(col.B * falloff);
                col.A = static_cast<u8>(col.A * falloff);
            }

            _verts.push_back({.Position  = p.Point.as_array(),
                              .Color     = col.as_array(),
                              .TexCoords = {0, 0, 0}});
        }

        for (u32 i {2}; i <= n; ++i) {
            _inds.push_back(0 + indOffset);
            _inds.push_back(i + indOffset);
            _inds.push_back(i - 1 + indOffset);
        }
        if (!limitAngle) {
            _inds.push_back(0 + indOffset);
            _inds.push_back(n + indOffset);
            _inds.push_back(1 + indOffset);
        }
        indOffset += n + 1;
    }
}

auto lighting_system::ray_intersects_polygon(point_f rayOrigin, f32 rayDirection, std::span<point_f const> polygon) const -> std::vector<point_f>
{
    radian_f const rad {degree_f {rayDirection}};
    point_f        rayDir {rad.cos(), rad.sin()};
    rayDir = rayDir.as_normalized();

    auto static const RayIntersectsSegment {
        [](point_f ro, point_f rd, point_f p0, point_f p1) -> std::optional<f32> {
            point_f const seg {p1 - p0};
            point_f const segPerp {seg.Y, -seg.X};

            f32 const     denom {segPerp.dot(rd)};
            constexpr f32 epsilon = 1e-6f;
            if (std::abs(denom) < epsilon) { return std::nullopt; }

            point_f const d {p0 - ro};
            f32 const     retValue {segPerp.dot(d) / denom};
            f32 const     s {point_f {rd.Y, -rd.X}.dot(d) / denom};
            if (retValue >= 0.0f && retValue <= std::numeric_limits<f32>::max() && s >= 0.0f && s <= 1.0f) { return retValue; }
            return std::nullopt;
        }};

    std::vector<point_f> retValue;
    usize const          n {polygon.size()};
    for (usize i {0}; i < n; ++i) {
        if (auto distance {RayIntersectsSegment(rayOrigin, rayDir, polygon[i], polygon[(i + 1) % n])}) {
            retValue.push_back(rayOrigin + rayDir * *distance);
        }
    }

    return retValue;
}

auto lighting_system::can_draw() const -> bool
{
    return !_lightSources.empty() && !_shadowCasters.empty();
}

void lighting_system::on_draw_to(render_target& target)
{
    if (_updateGeometry) {
        geometry_data data;
        data.Indices  = _inds;
        data.Vertices = _verts;
        data.Type     = primitive_type::Triangles;
        _renderer.set_geometry(data);
        _updateGeometry = false;
    }
    _renderer.render_to_target(target);
}

////////////////////////////////////////////////////////////

light_source::light_source(lighting_system* parent)
    : _parent {parent}
{
    Color.Changed.connect([&](auto const&) { request_redraw(); });
    Position.Changed.connect([&](auto const&) { request_redraw(); });
    Range.Changed.connect([&](auto const&) { request_redraw(); });
    StartAngle.Changed.connect([&](auto const&) { request_redraw(); });
    EndAngle.Changed.connect([&](auto const&) { request_redraw(); });
}

void light_source::request_redraw()
{
    if (_parent) {
        _parent->request_redraw();
    }
}

shadow_caster::shadow_caster(lighting_system* parent)
    : _parent {parent}
{
    Points.Changed.connect([&](auto const&) { request_redraw(); });
}

void shadow_caster::request_redraw()
{
    if (_parent) {
        _parent->request_redraw();
    }
}

} // namespace gfx

// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/LightingSystem.hpp"

#include <optional>

#include "tcob/gfx/Ray.hpp"

namespace tcob::gfx {

lighting_system::lighting_system()
    : _lightSources {}
    , _shadowCasters {}
{
    Bounds.Changed.connect([&](auto const&) { request_redraw(); });

    blend_funcs funcs;
    funcs.SourceAlphaBlendFunc = funcs.SourceColorBlendFunc = blend_func::SrcAlpha;
    funcs.DestinationAlphaBlendFunc = funcs.DestinationColorBlendFunc = blend_func::One;
    set_blend_funcs(funcs);
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

struct caster_points {
    polygon_span   Points {};
    shadow_caster* Caster {nullptr};
};

void lighting_system::on_update(milliseconds /* deltaTime */)
{
    if (!_isDirty) { return; }

    _updateGeometry = true;
    _isDirty        = false;

    _verts.clear();
    _inds.clear();
    i32 indOffset {0};

    constexpr f32 minAngle {0.0005f};

    // collect collision points
    bool shadowCasterDirty {false};

    std::vector<caster_points> casterPoints;
    casterPoints.reserve(_shadowCasters.size() + 1);
    for (auto const& sc : _shadowCasters) {
        casterPoints.emplace_back(sc->Polygon(), sc.get());
        shadowCasterDirty = shadowCasterDirty || sc->_isDirty;
        sc->_isDirty      = false;
    }

    std::array<point_f, 4> const boundPoints {{Bounds->top_left(), Bounds->bottom_left(), Bounds->bottom_right(), Bounds->top_right()}};
    casterPoints.emplace_back(boundPoints);

    for (auto const& ls : _lightSources) {
        bool const limitRange {ls->Range()};
        f32 const  lightRange {limitRange ? *ls->Range() : std::numeric_limits<f32>::max()};
        bool const limitAngle {ls->StartAngle() || ls->EndAngle()};

        if (shadowCasterDirty || ls->_isDirty) {
            auto const lightPosition {ls->Position()};
            ls->_isDirty = false;

            // check if light source is inside a shadow caster
            bool lightInsideShadowCaster {false};
            for (usize i {0}; i < casterPoints.size() - 1; ++i) {
                if (casterPoints[i].Points.empty()) { continue; }
                lightInsideShadowCaster = is_point_in_polygon(ls->Position, casterPoints[i].Points);
                if (lightInsideShadowCaster) { break; }
            }

            // collect angles
            std::set<f64> angles0;
            for (auto const& cp : casterPoints) {
                for (auto const& scp : cp.Points) {
                    std::array<f64, 3> constexpr vars {-minAngle, 0, minAngle};
                    for (f64 var : vars) {
                        auto const deg {lightPosition.angle_to(scp).as_normalized()};
                        if (limitAngle && (deg.Value < ls->StartAngle->Value || deg.Value > ls->EndAngle->Value)) { continue; }
                        angles0.insert(degree_d {deg.Value + var}.as_normalized().Value);
                    }
                }
            }

            if (limitRange && !lightInsideShadowCaster) {
                if (limitAngle) {
                    for (f32 i {ls->StartAngle->Value}; i < ls->EndAngle->Value; ++i) { angles0.insert(i); }
                } else {
                    for (i32 i {0}; i < 360; ++i) { angles0.insert(i); }
                }
            } else {
                if (limitAngle) {
                    angles0.insert(degree_f {ls->StartAngle->Value}.as_normalized().Value);
                    angles0.insert(degree_f {ls->EndAngle->Value}.as_normalized().Value);
                }
            }

            // discard angles
            std::vector<f64> angles1;
            angles1.reserve(angles0.size());
            for (f64 const angle : angles0) {
                if (!angles1.empty() && angle - angles1.back() < minAngle) { continue; }
                angles1.push_back(angle);
            }

            // ray cast
            std::map<f64, light_collision, std::greater<>> collisionResult0;
            for (f64 const angle : angles1) {
                light_collision nearestPoint;
                nearestPoint.Distance = std::numeric_limits<f32>::max();
                nearestPoint.Source   = ls.get();

                for (auto const& cp : casterPoints) {
                    ray const  ray {lightPosition, degree_d {angle}};
                    auto const result {ray.intersect_polygon(cp.Points)};
                    for (auto const& p : result) {
                        f64 const dist {lightPosition.distance_to(p)};
                        if (p == lightPosition) { continue; }
                        if (dist >= nearestPoint.Distance) { continue; }

                        if (limitRange && dist > lightRange) {
                            // move out-of-range points into range
                            point_d const direction = (p - lightPosition).as_normalized();
                            nearestPoint.Point      = lightPosition + direction * lightRange;
                            nearestPoint.Distance   = lightRange;
                            nearestPoint.Caster     = nullptr;
                        } else {
                            nearestPoint.Point    = p;
                            nearestPoint.Distance = dist;
                            nearestPoint.Caster   = cp.Caster;
                        }

                        nearestPoint.CollisionCount = result.size();
                    }
                }

                if (nearestPoint.Distance == std::numeric_limits<f32>::max()) { continue; }
                collisionResult0[angle] = nearestPoint;
            }

            // discard close points
            ls->_collisionResult.clear();
            ls->_collisionResult.reserve(collisionResult0.size());
            for (auto const& [k, v] : collisionResult0) {
                if (!ls->_collisionResult.empty() && ls->_collisionResult.back().Point.distance_to(v.Point) < 1) { continue; }
                if (!lightInsideShadowCaster && (v.CollisionCount == 1 && v.Caster != nullptr)) { continue; }

                ls->_collisionResult.push_back(v);
                if (v.Caster) { v.Caster->Hit(v); }
            }
        }

        // prepare geometry
        u32 const n {static_cast<u32>(ls->_collisionResult.size())};
        if (n <= 1) { continue; }

        _verts.push_back({.Position  = ls->Position->as_array(),
                          .Color     = ls->Color->as_array(),
                          .TexCoords = {0, 0, 0}});

        for (auto const& p : ls->_collisionResult) {
            auto col {ls->Color()};
            if (limitRange && ls->Falloff) {
                // FIXME: should be inverse square
                f64 const falloff {std::clamp(1.0 - (p.Distance / lightRange), 0.0, 1.0)};
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

auto lighting_system::is_point_in_polygon(point_f p, polygon_span points) const -> bool
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
    Falloff.Changed.connect([&](auto const&) { request_redraw(); });
    StartAngle.Changed.connect([&](auto const&) { request_redraw(); });
    EndAngle.Changed.connect([&](auto const&) { request_redraw(); });
}

void light_source::request_redraw()
{
    if (_parent) {
        _parent->request_redraw();
    }
    _isDirty = true;
}

shadow_caster::shadow_caster(lighting_system* parent)
    : _parent {parent}
{
    Polygon.Changed.connect([&](auto const&) { request_redraw(); });
}

void shadow_caster::request_redraw()
{
    if (_parent) {
        _parent->request_redraw();
    }
    _isDirty = true;
}

} // namespace gfx

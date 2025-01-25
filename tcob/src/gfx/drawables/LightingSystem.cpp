// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/gfx/drawables/LightingSystem.hpp"

#include <algorithm>
#include <mutex>
#include <optional>

#include "tcob/core/Common.hpp"
#include "tcob/core/ServiceLocator.hpp"
#include "tcob/core/TaskManager.hpp"
#include "tcob/gfx/Ray.hpp"

namespace tcob::gfx {

lighting_system::lighting_system(bool multiThreaded)
    : _lightSources {}
    , _shadowCasters {}
    , _multiThreaded {multiThreaded}
{
    Bounds.Changed.connect([this](auto const&) {
        rebuild_quadtree();
        _isDirty = true;
    });

    blend_funcs funcs;
    funcs.SourceAlphaBlendFunc = funcs.SourceColorBlendFunc = blend_func::SrcAlpha;
    funcs.DestinationAlphaBlendFunc = funcs.DestinationColorBlendFunc = blend_func::One;
    set_blend_funcs(funcs);
    _renderer.set_material(_material.ptr());
}

void lighting_system::remove_light_source(light_source const& light)
{
    helper::erase(_lightSources, [&light](auto const& val) {
        if (val.get() == &light) {
            val.get()->_parent = nullptr;
            return true;
        }

        return false;
    });

    _isDirty = true;
}

void lighting_system::clear_light_sources()
{
    for (auto& ls : _lightSources) { ls->_parent = nullptr; }
    _lightSources.clear();
    _isDirty = true;
}

void lighting_system::notify_light_changed(light_source* /* light */)
{
    _isDirty = true;
}

void lighting_system::remove_shadow_caster(shadow_caster const& shadow)
{
    if (_quadTree && shadow._bounds != rect_f::Zero) {
        _quadTree->remove({.Bounds = shadow._bounds, .Caster = &shadow});
        mark_lights_dirty();
    }

    helper::erase(_shadowCasters, [&shadow](auto const& val) {
        if (val.get() == &shadow) {
            val.get()->_parent = nullptr;
            return true;
        }

        return false;
    });

    _isDirty = true;
}

void lighting_system::clear_shadow_casters()
{
    for (auto& sc : _shadowCasters) { sc->_parent = nullptr; }
    _shadowCasters.clear();
    _isDirty = true;

    if (_quadTree) {
        _quadTree->clear();
        mark_lights_dirty();
    }
}

void lighting_system::notify_shadow_changed(shadow_caster* shadow)
{
    mark_lights_dirty();
    if (shadow->Polygon->empty()) {
        _quadTree->remove({.Bounds = shadow->_bounds, .Caster = shadow});
        shadow->_bounds = rect_f::Zero;
        return;
    }

    rect_f const newBounds {polygons::info(shadow->Polygon()).BoundingBox};
    if (shadow->_bounds != rect_f::Zero) {
        _quadTree->replace({.Bounds = shadow->_bounds, .Caster = shadow}, {.Bounds = newBounds, .Caster = shadow});
    } else {
        _quadTree->add({.Bounds = newBounds, .Caster = shadow});
    }
    shadow->_bounds = newBounds;
}

void lighting_system::set_blend_funcs(blend_funcs funcs)
{
    _material->BlendFuncs = funcs;
}

void lighting_system::on_update(milliseconds /* deltaTime */)
{
    if (!_isDirty) { return; }

    _verts.clear();
    _inds.clear();

    // collect collision points
    if (_quadTree) {
        u32 indOffset {0};
        for (auto const& ls : _lightSources) {
            auto const lightRange {ls->is_range_limited() ? *ls->Range() : std::numeric_limits<f32>::max()};
            cast_ray(*ls, lightRange);
            build_geometry(*ls, lightRange, indOffset);
        }
    }

    _updateGeometry = true;
    _isDirty        = false;
}

constexpr f32 angleTolerance {0.05f}; // 0.0005f

auto static is_in_shadowcaster(light_source& light, auto&& casterPoints) -> bool
{
    bool retValue {false};
    for (usize i {0}; i < casterPoints.size() - 1; ++i) {
        if (casterPoints[i].Points.empty()) { continue; }
        retValue = polygons::is_point_inside(light.Position(), casterPoints[i].Points);
        if (retValue) { break; }
    }
    return retValue;
}

void lighting_system::cast_ray(light_source& light, f32 lightRange)
{
    if (!light._isDirty) { return; }
    light._isDirty = false;

    bool const   limitRange {light.is_range_limited()};
    auto const   lightPosition {light.Position()};
    rect_f const lightBounds {limitRange
                                  ? rect_f {point_f::Zero, {lightRange * 2, lightRange * 2}}
                                        .as_centered_at(lightPosition)
                                        .as_intersection_with(Bounds())
                                  : Bounds()};

    auto                              casters {_quadTree->query(lightBounds)};
    std::vector<shadow_caster_points> casterPoints {};
    casterPoints.reserve(casters.size());
    for (auto const& caster : casters) {
        casterPoints.push_back({.Points = caster.Caster->Polygon(), .Caster = caster.Caster});
    }

    std::array<point_f, 4> const boundPoints {{Bounds->top_left(), Bounds->bottom_left(), Bounds->bottom_right(), Bounds->top_right()}};
    casterPoints.emplace_back(boundPoints, nullptr);

    bool const             lightInsideShadowCaster {is_in_shadowcaster(light, casterPoints)};
    std::vector<f64> const angles {collect_angles(light, lightInsideShadowCaster, casterPoints)};

    std::vector<std::pair<f64, light_collision>> collisionResult;
    collisionResult.reserve(angles.size());
    i32 const angleCount {static_cast<i32>(angles.size())};

    // ray cast
    locate_service<task_manager>().run_parallel(
        [&](par_task const& ctx) {
            std::vector<std::pair<f64, light_collision>> taskCollisionResult;
            taskCollisionResult.reserve(ctx.End - ctx.Start);

            for (isize idx {ctx.Start}; idx < ctx.End; ++idx) {
                f64 const angle {angles[idx]};

                light_collision nearestPoint;
                nearestPoint.Distance = std::numeric_limits<f32>::max();
                nearestPoint.Source   = &light;

                for (auto const& cp : casterPoints) {
                    ray const  ray {lightPosition, degree_d {angle}};
                    auto const result {ray.intersect_polyline(cp.Points)};
                    for (auto const& [point, distance] : result) {
                        if (point == lightPosition) { continue; }
                        if (distance >= nearestPoint.Distance) { continue; }

                        if (limitRange && distance > lightRange) {
                            // move out-of-range points into range
                            point_d const direction {(point - lightPosition).as_normalized()};
                            nearestPoint.Point    = lightPosition + direction * lightRange;
                            nearestPoint.Distance = lightRange;
                            nearestPoint.Caster   = nullptr;
                        } else {
                            nearestPoint.Point    = point;
                            nearestPoint.Distance = distance;
                            nearestPoint.Caster   = cp.Caster;
                        }

                        nearestPoint.CollisionCount = result.size();
                    }
                }

                if (nearestPoint.Distance == std::numeric_limits<f32>::max()) { continue; }
                taskCollisionResult.emplace_back(angle, nearestPoint);
            }
            {
                std::scoped_lock lock {_mutex};
                collisionResult.insert(collisionResult.end(), taskCollisionResult.begin(), taskCollisionResult.end());
            }
        },
        angleCount, _multiThreaded ? 64 : angleCount);

    std::ranges::sort(collisionResult, [](auto const& a, auto const& b) { return a.first > b.first; });

    // discard close points
    light._collisionResult.clear();
    light._collisionResult.reserve(collisionResult.size());
    for (auto const& [k, v] : collisionResult) {
        if (!light._collisionResult.empty() && light._collisionResult.back().Point.distance_to(v.Point) < 1) { continue; }
        if (!lightInsideShadowCaster && (v.CollisionCount == 1 && v.Caster != nullptr)) { continue; }

        light._collisionResult.push_back(v);
        if (v.Caster) { v.Caster->Hit(v); }
    }
}

void lighting_system::build_geometry(light_source& light, f32 lightRange, u32& indOffset)
{
    u32 const n {static_cast<u32>(light._collisionResult.size())};
    if (n <= 1) { return; }

    _verts.push_back({.Position  = light.Position(),
                      .Color     = light.Color(),
                      .TexCoords = {0, 0, 0}});

    bool const limitRange {light.is_range_limited()};

    for (auto const& p : light._collisionResult) {
        auto col {light.Color()};
        if (limitRange && light.Falloff()) {
            // FIXME: should be inverse square
            f64 const falloff {std::clamp(1.0 - (p.Distance / lightRange), 0.0, 1.0)};
            col.R = static_cast<u8>(col.R * falloff);
            col.G = static_cast<u8>(col.G * falloff);
            col.B = static_cast<u8>(col.B * falloff);
            col.A = static_cast<u8>(col.A * falloff);
        }

        _verts.push_back({.Position  = p.Point,
                          .Color     = col,
                          .TexCoords = {0, 0, 0}});
    }

    for (u32 i {2}; i <= n; ++i) {
        _inds.push_back(0 + indOffset);
        _inds.push_back(i + indOffset);
        _inds.push_back(i - 1 + indOffset);
    }
    if (!light.is_angle_limited()) {
        _inds.push_back(0 + indOffset);
        _inds.push_back(n + indOffset);
        _inds.push_back(1 + indOffset);
    }
    indOffset += n + 1;
}

auto lighting_system::collect_angles(light_source& light, bool lightInsideShadowCaster, std::vector<shadow_caster_points> const& casterPoints) const -> std::vector<f64>
{
    auto const lightPosition {light.Position()};
    bool const limitAngle {light.is_angle_limited()};

    std::set<f64> angles;
    std::array<f64, 3> constexpr vars {-angleTolerance, 0, angleTolerance};

    for (auto const& cp : casterPoints) {
        for (auto const& scp : cp.Points) {
            auto const baseAngle {lightPosition.angle_to(scp).as_normalized().Value};

            if (!limitAngle || (baseAngle >= light.StartAngle->Value && baseAngle <= light.EndAngle->Value)) {
                for (f64 var : vars) {
                    angles.insert(degree_d {baseAngle + var}.as_normalized().Value);
                }
            }
        }
    }

    if (light.is_range_limited() && !lightInsideShadowCaster) {
        f64 const start {limitAngle ? light.StartAngle->Value : 0.0};
        f64 const end {limitAngle ? light.EndAngle->Value : 360.0};

        for (f64 i {start}; i < end; ++i) {
            angles.insert(i);
        }
    } else if (limitAngle) {
        angles.insert(light.StartAngle->as_normalized().Value);
        angles.insert(light.EndAngle->as_normalized().Value);
    }

    // filter and store angles with tolerance
    std::vector<f64> retValue;
    retValue.reserve(angles.size());

    for (f64 const angle : angles) {
        if (retValue.empty() || (angle - retValue.back() >= angleTolerance)) {
            retValue.push_back(angle);
        }
    }

    return retValue;
}

void lighting_system::rebuild_quadtree()
{
    if (_quadTree) {
        _quadTree->clear();
        mark_lights_dirty();
        for (auto& sc : _shadowCasters) {
            _quadTree->add(quadtree_node {.Bounds = polygons::info(sc->Polygon()).BoundingBox, .Caster = sc.get()});
        }
    } else {
        _quadTree = std::make_unique<quadtree<quadtree_node>>(Bounds());
    }
}

void lighting_system::mark_lights_dirty()
{
    for (auto& light : _lightSources) {
        light->_isDirty = true;
    }
}

auto lighting_system::can_draw() const -> bool
{
    return !_inds.empty();
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

auto light_source::is_range_limited() const -> bool
{
    return Range().has_value();
}

auto light_source::is_angle_limited() const -> bool
{
    return StartAngle().has_value() || EndAngle().has_value();
}

light_source::light_source(lighting_system* parent)
    : _parent {parent}
{
    Color.Changed.connect([this](auto const&) { notify_parent(); });
    Position.Changed.connect([this](auto const&) { notify_parent(); });
    Range.Changed.connect([this](auto const&) { notify_parent(); });
    Falloff.Changed.connect([this](auto const&) { notify_parent(); });
    StartAngle.Changed.connect([this](auto const&) { notify_parent(); });
    EndAngle.Changed.connect([this](auto const&) { notify_parent(); });
}

void light_source::notify_parent()
{
    if (_parent) {
        _parent->notify_light_changed(this);
    }
    _isDirty = true;
}

shadow_caster::shadow_caster(lighting_system* parent)
    : _parent {parent}
{
    Polygon.Changed.connect([this](auto const&) { notify_parent(); });
}

void shadow_caster::notify_parent()
{
    if (_parent) {
        _parent->notify_shadow_changed(this);
    }
}

} // namespace gfx

// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "KDTree.hpp"

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::kd_tree(bounds_type const& bounds)
    : _bounds {bounds}
    , _root {std::make_unique<node>()}
{
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::add(T const& value)
{
    _root->add(0, _bounds, value);
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::remove(T const& value)
{
    _root->remove(_bounds, value);
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::replace(T const& oldValue, T const& newValue)
{
    if (!_root->replace(_bounds, oldValue, newValue)) {
        // Otherwise, remove the old value and add the new one
        remove(oldValue);
        add(newValue);
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::clear()
{
    _root = std::make_unique<node>();
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::query(bounds_type const& bounds) const -> std::vector<T>
{
    if (!node::Intersects(_bounds, bounds)) { return {}; }

    std::vector<T> retValue {};
    _root->query(_bounds, bounds, retValue);
    return retValue;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::find_nearest(point_type const& target) const -> T
{
    T const* best {nullptr};
    f32      minDistSq {std::numeric_limits<f32>::max()};
    _root->find_nearest(target, _bounds, best, minDistSq);
    return *best;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::find_all_intersections() const -> std::vector<std::pair<T, T>>
{
    std::vector<std::pair<T, T>> retValue {};
    _root->find_all_intersections(retValue);
    return retValue;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::bounds() const -> bounds_type const&
{
    return _bounds;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::contains(bounds_type const& bounds) const -> bool
{
    for (usize i {0}; i < Dimensions; ++i) {
        if (bounds.first[i] < _bounds.first[i] || bounds.second[i] > _bounds.second[i]) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::is_leaf() const -> bool
{
    return _children[0] == nullptr;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::add(usize depth, bounds_type const& bounds, T const& value)
{
    if (!is_leaf()) {
        auto const dims {value.get_dimensions()};
        i32 const  side {GetSide(dims, _axis, _splitPos)};
        auto const childBounds {ComputeChildBounds(bounds, _axis, _splitPos, side)};
        _children[static_cast<usize>(side)]->add(depth + 1, childBounds, value);
    } else {
        _values.push_back(value);
        if (_values.size() > SplitThreshold && depth < MaxDepth) {
            split(bounds, depth % Dimensions);
        }
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::split(bounds_type const& bounds, usize axis)
{
    _axis     = axis;
    _splitPos = (bounds.first[axis] + bounds.second[axis]) * 0.5f;

    _children[0] = std::make_unique<node>();
    _children[1] = std::make_unique<node>();

    for (auto const& value : _values) {
        auto const dims {value.get_dimensions()};
        i32 const  side {GetSide(dims, _axis, _splitPos)};
        _children[static_cast<usize>(side)]->_values.push_back(value);
    }
    _values.clear();
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::remove(bounds_type const& bounds, T const& value) -> bool
{
    if (is_leaf()) {
        if (auto it {std::ranges::find_if(_values, [&value](auto const& rhs) { return value == rhs; })}; it != std::end(_values)) {
            *it = std::move(_values.back());
            _values.pop_back();
            return true;
        }
        return false;
    }

    auto const dims {value.get_dimensions()};
    i32 const  side {GetSide(dims, _axis, _splitPos)};
    auto const childBounds {ComputeChildBounds(bounds, _axis, _splitPos, side)};

    if (_children[static_cast<usize>(side)]->remove(childBounds, value)) {
        return try_merge();
    }
    return false;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::try_merge() -> bool
{
    auto nbValues {_values.size()};
    for (auto const& child : _children) {
        if (!child->is_leaf()) { return false; }
        nbValues += child->_values.size();
    }
    if (nbValues <= SplitThreshold) {
        _values.reserve(nbValues);
        for (auto const& child : _children) {
            for (auto const& value : child->_values) {
                _values.push_back(value);
            }
        }
        for (auto& child : _children) { child.reset(); }
        return true;
    }

    return false;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::replace(bounds_type const& bounds, T const& oldValue, T const& newValue) -> bool
{
    if (is_leaf()) {
        if (auto it {std::ranges::find_if(_values, [&oldValue](auto const& rhs) { return oldValue == rhs; })}; it != std::end(_values)) {
            // Check if the NEW value still fits in this node's physical bounds
            if (PointInBounds(newValue.get_dimensions(), bounds)) {
                *it = newValue;
                return true; // Successfully updated in-place
            }
            // If it doesn't fit, return false so the parent knows to remove/re-add
            return false;
        }
        return false;
    }

    auto const oldDims {oldValue.get_dimensions()};
    i32 const  side {GetSide(oldDims, _axis, _splitPos)};
    auto const childBounds {ComputeChildBounds(bounds, _axis, _splitPos, side)};

    // Recurse into the side where the old value was supposed to be
    return _children[static_cast<usize>(side)]->replace(childBounds, oldValue, newValue);
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::query(bounds_type const& bounds, bounds_type const& queryBounds, std::vector<T>& values) const
{
    for (auto const& value : _values) {
        auto const dims {value.get_dimensions()};
        if (PointInBounds(dims, queryBounds)) {
            values.push_back(value);
        }
    }

    if (!is_leaf()) {
        for (i32 i {0}; i < 2; ++i) {
            auto const childBounds {ComputeChildBounds(bounds, _axis, _splitPos, i)};
            if (Intersects(childBounds, queryBounds)) {
                _children[static_cast<usize>(i)]->query(childBounds, queryBounds, values);
            }
        }
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::find_nearest(point_type const& target, bounds_type const& bounds, T const*& best, f32& minDistSq) const
{
    for (auto const& val : _values) {
        auto const dims {val.get_dimensions()};
        f32        dSq {0};
        for (usize i {0}; i < Dimensions; ++i) {
            f32 diff {target[i] - dims[i]};
            dSq += diff * diff;
        }

        if (dSq < minDistSq) {
            minDistSq = dSq;
            best      = &val;
        }
    }

    if (is_leaf()) { return; }

    i32 const  side {GetSide(target, _axis, _splitPos)};
    auto const nearChildBounds {ComputeChildBounds(bounds, _axis, _splitPos, side)};
    _children[side]->find_nearest(target, nearChildBounds, best, minDistSq);

    f32 const planeDiff {target[_axis] - _splitPos};
    if ((planeDiff * planeDiff) < minDistSq) {
        i32 const  otherSide {1 - side};
        auto const farChildBounds {ComputeChildBounds(bounds, _axis, _splitPos, otherSide)};
        _children[otherSide]->find_nearest(target, farChildBounds, best, minDistSq);
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::find_all_intersections(std::vector<std::pair<T, T>>& intersections) const
{
    for (usize i {0}; i < _values.size(); ++i) {
        for (usize j {i + 1}; j < _values.size(); ++j) {
            intersections.emplace_back(_values[i], _values[j]);
        }
        find_intersections_in_descendants(_values[i], intersections);
    }

    if (!is_leaf()) {
        for (auto const& child : _children) {
            child->find_all_intersections(intersections);
        }
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline void kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::find_intersections_in_descendants(T const& value, std::vector<std::pair<T, T>>& intersections) const
{
    if (!is_leaf()) {
        for (auto const& child : _children) {
            for (auto const& childValue : child->_values) {
                intersections.emplace_back(value, childValue);
            }
            child->find_intersections_in_descendants(value, intersections);
        }
    }
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::GetSide(point_type const& point, usize axis, f32 splitPos) -> i32
{
    return point[axis] < splitPos ? 0 : 1;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::ComputeChildBounds(bounds_type const& bounds, usize axis, f32 splitPos, i32 side) -> bounds_type
{
    bounds_type childBounds {bounds};
    if (side == 0) {
        childBounds.second[axis] = splitPos;
    } else {
        childBounds.first[axis] = splitPos;
    }
    return childBounds;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::Intersects(bounds_type const& a, bounds_type const& b) -> bool
{
    for (usize i {0}; i < Dimensions; ++i) {
        if (a.second[i] < b.first[i] || a.first[i] > b.second[i]) {
            return false;
        }
    }
    return true;
}

template <typename T, usize Dimensions, usize SplitThreshold, usize MaxDepth>
    requires KdTreeValue<T, Dimensions>
inline auto kd_tree<T, Dimensions, SplitThreshold, MaxDepth>::node::PointInBounds(point_type const& point, bounds_type const& bounds) -> bool
{
    for (usize i {0}; i < Dimensions; ++i) {
        if (point[i] < bounds.first[i] || point[i] > bounds.second[i]) {
            return false;
        }
    }
    return true;
}

}
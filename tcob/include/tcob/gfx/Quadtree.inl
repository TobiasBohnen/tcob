// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Quadtree.hpp"

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Rect.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline quadtree<T, SplitThreshold, MaxDepth>::quadtree(rect_f const& rect)
    : _bounds {rect}
    , _root {std::make_unique<node>()}
{
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::add(T const& value)
{
    assert(detail::contains(_bounds, value.get_rect()));

    _root->add(0, _bounds, value);
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::remove(T const& value)
{
    assert(detail::contains(_bounds, value.get_rect()));

    _root->remove(_bounds, value);
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::replace(T const& oldValue, T const& newValue)
{
    assert(detail::contains(_bounds, oldValue.get_rect()));
    assert(detail::contains(_bounds, newValue.get_rect()));

    if (!_root->replace(_bounds, oldValue, newValue)) {
        // Otherwise, remove the old value and add the new one
        remove(oldValue);
        add(newValue);
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::clear()
{
    _root = std::make_unique<node>();
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::query(rect_f const& rect) const -> std::vector<T>
{
    if (!detail::intersects(_bounds, rect)) { return {}; }

    std::vector<T> retValue {};
    _root->query(_bounds, rect, retValue);
    return retValue;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::find_all_intersections() const -> std::vector<std::pair<T, T>>
{
    std::vector<std::pair<T, T>> retValue {};
    _root->find_all_intersections(retValue);
    return retValue;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::bounds() const -> rect_f const&
{
    return _bounds;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::contains(rect_f const& rect) const -> bool
{
    return detail::contains(_bounds, rect);
}

////////////////////////////////////////////////////////////

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::is_leaf() const -> bool
{
    return _children[0] == nullptr;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::node::add(usize depth, rect_f const& rect, T const& value)
{
    assert(detail::contains(rect, value.get_rect()));

    if (is_leaf()) {
        // Insert the value in this node if possible
        if (depth >= MaxDepth || _values.size() < SplitThreshold) {
            _values.push_back(value);
        } else {
            // Otherwise, we split and we try again
            split(rect);
            add(depth, rect, value);
        }
    } else {
        auto const i {GetQuadrant(rect, value.get_rect())};
        if (i != -1) {
            // Add the value in a child if the value is entirely contained in it
            _children[static_cast<usize>(i)]->add(depth + 1, ComputeRect(rect, i), value);
        } else {
            // Otherwise, we add the value in the current node
            _values.push_back(value);
        }
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::node::split(rect_f const& rect)
{
    // Create children
    for (auto& child : _children) {
        child = std::make_unique<node>();
    }
    // Assign values to children
    std::vector<T> newValues {}; // New values for this node
    for (auto const& value : _values) {
        i32 const i {GetQuadrant(rect, value.get_rect())};
        if (i != -1) {
            _children[static_cast<usize>(i)]->_values.push_back(value);
        } else {
            newValues.push_back(value);
        }
    }
    _values = std::move(newValues);
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::remove(rect_f const& rect, T const& value) -> bool
{
    assert(detail::contains(rect, value.get_rect()));

    auto const removeValue {[&] {
        // Find the value in node->values
        auto it {std::ranges::find_if(_values, [&value](auto const& rhs) { return value == rhs; })};
        assert(it != std::end(_values) && "Trying to remove a value that is not present in the node");
        // Swap with the last element and pop back
        *it = std::move(_values.back());
        _values.pop_back();
    }};

    if (is_leaf()) {
        // Remove the value from node
        removeValue();
        return true;
    }
    // Remove the value in a child if the value is entirely contained in it
    i32 const i {GetQuadrant(rect, value.get_rect())};
    if (i != -1) {
        if (_children[static_cast<usize>(i)]->remove(ComputeRect(rect, i), value)) {
            return try_merge();
        }
    } else {
        // Otherwise, we remove the value from the current node
        removeValue();
    }
    return false;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::try_merge() -> bool
{
    auto nbValues {_values.size()};
    for (auto const& child : _children) {
        if (!child->is_leaf()) { return false; }
        nbValues += child->_values.size();
    }
    if (nbValues <= SplitThreshold) {
        _values.reserve(nbValues);
        // Merge the values of all the children
        for (auto const& child : _children) {
            for (auto const& value : child->_values) {
                _values.push_back(value);
            }
        }
        // Remove the children
        for (auto& child : _children) { child.reset(); }
        return true;
    }

    return false;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::replace(rect_f const& rect, T const& oldValue, T const& newValue) -> bool
{
    // Ensure the rect contains both oldValue and newValue
    assert(detail::contains(rect, oldValue.get_rect()));
    assert(detail::contains(rect, newValue.get_rect()));

    if (is_leaf()) {
        // Find and replace the oldValue with newValue in this node
        auto it {std::ranges::find_if(_values, [&oldValue](auto const& rhs) { return oldValue == rhs; })};
        if (it != std::end(_values)) {
            *it = newValue;
            return true;
        }
        return false;
    }

    // If this node is not a leaf, determine in which quadrant oldValue and newValue lie
    i32 const oldIndex {GetQuadrant(rect, oldValue.get_rect())};
    i32 const newIndex {GetQuadrant(rect, newValue.get_rect())};

    // If they are both in the same quadrant
    if (oldIndex == newIndex && oldIndex != -1) {
        return _children[static_cast<usize>(oldIndex)]->replace(ComputeRect(rect, oldIndex), oldValue, newValue);
    }

    return false;
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::node::query(rect_f const& rect, rect_f const& queryRect, std::vector<T>& values) const
{
    assert(detail::intersects(queryRect, rect));

    for (auto const& value : _values) {
        if (detail::intersects(queryRect, value.get_rect())) {
            values.push_back(value);
        }
    }
    if (!is_leaf()) {
        for (usize i {0}; i < _children.size(); ++i) {
            if (auto childRect {ComputeRect(rect, static_cast<i32>(i))}; detail::intersects(queryRect, childRect)) {
                _children[i]->query(childRect, queryRect, values);
            }
        }
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::node::find_all_intersections(std::vector<std::pair<T, T>>& intersections) const
{
    // Find intersections between values stored in this node
    // Make sure to not report the same intersection twice
    for (usize i {0}; i < _values.size(); ++i) {
        for (usize j {0}; j < i; ++j) {
            if (detail::intersects(_values[i].get_rect(), _values[j].get_rect())) {
                intersections.emplace_back(_values[i], _values[j]);
            }
        }
    }
    if (!is_leaf()) {
        // Values in this node can intersect values in descendants
        for (auto const& child : _children) {
            for (auto const& value : _values) {
                child->find_intersections_in_descendants(value, intersections);
            }
        }
        // Find intersections in children
        for (auto const& child : _children) {
            child->find_all_intersections(intersections);
        }
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline void quadtree<T, SplitThreshold, MaxDepth>::node::find_intersections_in_descendants(T const& value, std::vector<std::pair<T, T>>& intersections) const
{
    // Test against the values stored in this node
    for (auto const& other : _values) {
        if (detail::intersects(value.get_rect(), other.get_rect())) {
            intersections.emplace_back(value, other);
        }
    }
    // Test against values stored into descendants of this node
    if (!is_leaf()) {
        for (auto const& child : _children) {
            child->find_intersections_in_descendants(value, intersections);
        }
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::ComputeRect(rect_f const& rect, i32 i) -> rect_f
{
    auto const& origin {rect.Position};
    auto const  childSize {rect.Size / 2.f};
    switch (i) {
    case 0: return {origin, childSize};                                               // North West
    case 1: return {point_f {origin.X + childSize.Width, origin.Y}, childSize};       // Norst East
    case 2: return {point_f {origin.X, origin.Y + childSize.Height}, childSize};      // South West
    case 3: return {origin + point_f {childSize.Width, childSize.Height}, childSize}; // South East
    default:
        assert(false && "Invalid child index");
        return {};
    }
}

template <QuadtreeValue T, usize SplitThreshold, usize MaxDepth>
inline auto quadtree<T, SplitThreshold, MaxDepth>::node::GetQuadrant(rect_f const& nodeRect, rect_f const& valueRect) -> i32
{
    auto const center {nodeRect.center()};

    if (valueRect.right() < center.X) {                  // West
        if (valueRect.bottom() < center.Y) { return 0; } // North West
        if (valueRect.top() >= center.Y) { return 2; }   // South West
        return -1;                                       // Not contained in any quadrant
    }

    if (valueRect.left() >= center.X) {                  // East
        if (valueRect.bottom() < center.Y) { return 1; } // North East
        if (valueRect.top() >= center.Y) { return 3; }   // South East
        return -1;                                       // Not contained in any quadrant
    }
    return -1;                                           // Not contained in any quadrant
}
}

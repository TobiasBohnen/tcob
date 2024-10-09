// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Quadtree.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

template <typename T, auto GetRect>
inline quadtree<T, GetRect>::quadtree(rect_f const& rect)
    : _bounds {rect}
    , _root {std::make_unique<node>()}
{
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::add(T const& value)
{
    if (!_bounds.contains(GetRect(value))) { return; }

    add(_root.get(), 0, _bounds, value);
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::remove(T const& value)
{
    if (!_bounds.contains(GetRect(value))) { return; }

    remove(_root.get(), _bounds, value);
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::clear()
{
    _root = std::make_unique<node>();
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::query(rect_f const& rect) const -> std::vector<T>
{
    if (!_bounds.intersects(rect)) { return {}; }

    std::vector<T> retValue {};
    query(_root.get(), _bounds, rect, retValue);
    return retValue;
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::find_all_intersections() const -> std::vector<std::pair<T, T>>
{
    std::vector<std::pair<T, T>> retValue {};
    find_all_intersections(_root.get(), retValue);
    return retValue;
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::get_bounds() const -> rect_f
{
    return _bounds;
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::is_leaf(node const* node) const -> bool
{
    return node->Children[0] == nullptr;
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::compute_rect(rect_f const& rect, i32 i) const -> rect_f
{
    auto const origin {rect.top_left()};
    auto const childSize {rect.get_size() / 2.f};
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

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::get_quadrant(rect_f const& nodeRect, rect_f const& valueRect) const -> i32
{
    auto const center {nodeRect.get_center()};

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

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::add(node* treeNode, usize depth, rect_f const& rect, T const& value)
{
    assert(treeNode != nullptr);
    assert(rect.contains(GetRect(value)));

    if (is_leaf(treeNode)) {
        // Insert the value in this node if possible
        if (depth >= MaxDepth || treeNode->Values.size() < Threshold) {
            treeNode->Values.push_back(value);
        } else {
            // Otherwise, we split and we try again
            split(treeNode, rect);
            add(treeNode, depth, rect, value);
        }
    } else {
        auto const i {get_quadrant(rect, GetRect(value))};
        if (i != -1) {
            // Add the value in a child if the value is entirely contained in it
            add(treeNode->Children[static_cast<usize>(i)].get(), depth + 1, compute_rect(rect, i), value);
        } else {
            // Otherwise, we add the value in the current node
            treeNode->Values.push_back(value);
        }
    }
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::split(node* treeNode, rect_f const& rect)
{
    assert(treeNode != nullptr);
    assert(is_leaf(treeNode) && "Only leaves can be split");

    // Create children
    for (auto& child : treeNode->Children) {
        child = std::make_unique<node>();
    }
    // Assign values to children
    std::vector<T> newValues {}; // New values for this node
    for (auto const& value : treeNode->Values) {
        i32 const i {get_quadrant(rect, GetRect(value))};
        if (i != -1) {
            treeNode->Children[static_cast<usize>(i)]->Values.push_back(value);
        } else {
            newValues.push_back(value);
        }
    }
    treeNode->Values = std::move(newValues);
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::remove(node* treeNode, rect_f const& rect, T const& value) -> bool
{
    assert(treeNode != nullptr);
    assert(rect.contains(GetRect(value)));

    if (is_leaf(treeNode)) {
        // Remove the value from node
        remove_value(treeNode, value);
        return true;
    }
    // Remove the value in a child if the value is entirely contained in it
    i32 const i {get_quadrant(rect, GetRect(value))};
    if (i != -1) {
        if (remove(treeNode->Children[static_cast<usize>(i)].get(), compute_rect(rect, i), value)) {
            return try_merge(treeNode);
        }
    } else {
        // Otherwise, we remove the value from the current node
        remove_value(treeNode, value);
    }
    return false;
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::remove_value(node* node, T const& value)
{
    // Find the value in node->values
    auto it {std::find_if(std::begin(node->Values), std::end(node->Values),
                          [&value](auto const& rhs) { return value == rhs; })};
    assert(it != std::end(node->Values) && "Trying to remove a value that is not present in the node");
    // Swap with the last element and pop back
    *it = std::move(node->Values.back());
    node->Values.pop_back();
}

template <typename T, auto GetRect>
inline auto quadtree<T, GetRect>::try_merge(node* node) -> bool
{
    assert(node != nullptr);
    assert(!is_leaf(node) && "Only interior nodes can be merged");

    auto nbValues {node->Values.size()};
    for (auto const& child : node->Children) {
        if (!is_leaf(child.get())) { return false; }
        nbValues += child->Values.size();
    }
    if (nbValues <= Threshold) {
        node->Values.reserve(nbValues);
        // Merge the values of all the children
        for (auto const& child : node->Children) {
            for (auto const& value : child->Values) {
                node->Values.push_back(value);
            }
        }
        // Remove the children
        for (auto& child : node->Children) { child.reset(); }
        return true;
    }

    return false;
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::query(node* node, rect_f const& rect, rect_f const& queryRect, std::vector<T>& values) const
{
    assert(node != nullptr);
    assert(queryRect.intersects(rect));

    for (auto const& value : node->Values) {
        if (queryRect.intersects(GetRect(value))) {
            values.push_back(value);
        }
    }
    if (!is_leaf(node)) {
        for (usize i {0}; i < node->Children.size(); ++i) {
            if (auto childRect {compute_rect(rect, static_cast<i32>(i))}; queryRect.intersects(childRect)) {
                query(node->Children[i].get(), childRect, queryRect, values);
            }
        }
    }
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::find_all_intersections(node* node, std::vector<std::pair<T, T>>& intersections) const
{
    // Find intersections between values stored in this node
    // Make sure to not report the same intersection twice
    for (usize i {0}; i < node->Values.size(); ++i) {
        for (usize j {0}; j < i; ++j) {
            if (GetRect(node->Values[i]).intersects(GetRect(node->Values[j]))) {
                intersections.emplace_back(node->Values[i], node->Values[j]);
            }
        }
    }
    if (!is_leaf(node)) {
        // Values in this node can intersect values in descendants
        for (auto const& child : node->Children) {
            for (auto const& value : node->Values) {
                find_intersections_in_descendants(child.get(), value, intersections);
            }
        }
        // Find intersections in children
        for (auto const& child : node->Children) {
            find_all_intersections(child.get(), intersections);
        }
    }
}

template <typename T, auto GetRect>
inline void quadtree<T, GetRect>::find_intersections_in_descendants(node* node, T const& value, std::vector<std::pair<T, T>>& intersections) const
{
    // Test against the values stored in this node
    for (auto const& other : node->Values) {
        if (GetRect(value).intersects(GetRect(other))) {
            intersections.emplace_back(value, other);
        }
    }
    // Test against values stored into descendants of this node
    if (!is_leaf(node)) {
        for (auto const& child : node->Children) {
            find_intersections_in_descendants(child.get(), value, intersections);
        }
    }
}

}

// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// based on: https://github.com/pvigier/Quadtree/blob/master/include/Quadtree.h
// Copyright (c) 2019 Pierre Vigier
// used under the terms of the MIT License

#pragma once
#include "tcob/tcob_config.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "tcob/core/Rect.hpp"

namespace tcob::gfx {
////////////////////////////////////////////////////////////

template <typename T, auto GetRect>
class quadtree {
public:
    quadtree(rect_f const& rect);

    void add(T const& value);

    void remove(T const& value);

    void clear();

    auto query(rect_f const& rect) const -> std::vector<T>;

    auto find_all_intersections() const -> std::vector<std::pair<T, T>>;

    auto get_bounds() const -> rect_f;

private:
    static constexpr usize Threshold {16};
    static constexpr usize MaxDepth {8};

    struct node {
        std::array<std::unique_ptr<node>, 4> Children;
        std::vector<T>                       Values;
    };

    auto is_leaf(node const* node) const -> bool;

    auto compute_rect(rect_f const& rect, i32 i) const -> rect_f;

    auto get_quadrant(rect_f const& nodeRect, rect_f const& valueRect) const -> i32;

    void add(node* treeNode, usize depth, rect_f const& rect, T const& value);

    void split(node* treeNode, rect_f const& rect);

    auto remove(node* treeNode, rect_f const& rect, T const& value) -> bool;

    void remove_value(node* node, T const& value);

    auto try_merge(node* node) -> bool;

    void query(node* node, rect_f const& rect, rect_f const& queryRect, std::vector<T>& values) const;

    void find_all_intersections(node* node, std::vector<std::pair<T, T>>& intersections) const;

    void find_intersections_in_descendants(node* node, T const& value, std::vector<std::pair<T, T>>& intersections) const;

    rect_f                _bounds;
    std::unique_ptr<node> _root;
};

}

#include "Quadtree.inl"

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

template <typename T>
concept QuadtreeValue =
    requires(T& t) {
        { t.get_rect() } -> std::convertible_to<rect_f>;
        { t == t } -> std::same_as<bool>;
    };

////////////////////////////////////////////////////////////

template <QuadtreeValue T, usize SplitThreshold = 16, usize MaxDepth = 8>
class quadtree {
public:
    quadtree(rect_f const& rect);

    auto add(T const& value) -> bool;

    auto remove(T const& value) -> bool;

    auto replace(T const& oldValue, T const& newValue) -> bool;

    void clear();

    auto query(rect_f const& rect) const -> std::vector<T>;

    auto find_all_intersections() const -> std::vector<std::pair<T, T>>;

    auto get_bounds() const -> rect_f const&;

private:
    class node {
    public:
        auto is_leaf() const -> bool;

        void add(usize depth, rect_f const& rect, T const& value);

        void split(rect_f const& rect);

        auto remove(rect_f const& rect, T const& value) -> bool;

        auto try_merge() -> bool;

        auto replace(rect_f const& rect, T const& oldValue, T const& newValue) -> bool;

        void query(rect_f const& rect, rect_f const& queryRect, std::vector<T>& values) const;

        void find_all_intersections(std::vector<std::pair<T, T>>& intersections) const;

        void find_intersections_in_descendants(T const& value, std::vector<std::pair<T, T>>& intersections) const;

        auto static ComputeRect(rect_f const& rect, i32 i) -> rect_f;
        auto static GetQuadrant(rect_f const& nodeRect, rect_f const& valueRect) -> i32;

    private:
        std::array<std::unique_ptr<node>, 4> _children;
        std::vector<T>                       _values;
    };

    rect_f                _bounds;
    std::unique_ptr<node> _root;
};

}

#include "Quadtree.inl"

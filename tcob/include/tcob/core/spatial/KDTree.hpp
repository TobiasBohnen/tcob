// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace tcob {
////////////////////////////////////////////////////////////

template <typename T, usize Dimensions>
concept KdTreeValue =
    requires(T& t) {
        { t.get_dimensions() } -> std::same_as<std::array<f64, Dimensions>>;
        { t == t } -> std::same_as<bool>;
    };

////////////////////////////////////////////////////////////

template <typename T, usize Dimensions, usize SplitThreshold = 16, usize MaxDepth = 8>
    requires KdTreeValue<T, Dimensions>
class kd_tree {
public:
    using point_type  = std::array<f64, Dimensions>;
    using bounds_type = std::pair<point_type, point_type>; // min, max

    explicit kd_tree(bounds_type const& bounds);

    void add(T const& value);

    void remove(T const& value);

    void replace(T const& oldValue, T const& newValue);

    void clear();

    auto query(bounds_type const& bounds) const -> std::vector<T>;

    auto find_nearest(point_type const& target) const -> std::optional<T>;

    auto bounds() const -> bounds_type const&;
    auto contains(bounds_type const& bounds) const -> bool;

private:
    class node {
    public:
        auto is_leaf() const -> bool;

        void add(usize depth, bounds_type const& bounds, T const& value);

        void split(bounds_type const& bounds, usize axis);

        auto remove(bounds_type const& bounds, T const& value) -> bool;

        auto try_merge() -> bool;

        auto replace(bounds_type const& bounds, T const& oldValue, T const& newValue) -> bool;

        void query(bounds_type const& bounds, bounds_type const& queryBounds, std::vector<T>& values) const;

        void find_nearest(point_type const& target, bounds_type const& bounds, T const*& best, f64& minDistSq) const;

        static auto GetSide(point_type const& point, usize axis, f64 splitPos) -> u32;
        static auto ComputeChildBounds(bounds_type const& bounds, usize axis, f64 splitPos, u32 side) -> bounds_type;
        static auto Intersects(bounds_type const& a, bounds_type const& b) -> bool;
        static auto PointInBounds(point_type const& point, bounds_type const& bounds) -> bool;

    private:
        std::array<std::unique_ptr<node>, 2> _children;
        std::vector<T>                       _values;
        f64                                  _splitPos {};
        usize                                _axis {};
    };

    bounds_type           _bounds;
    std::unique_ptr<node> _root;
};

}

#include "KDTree.inl"

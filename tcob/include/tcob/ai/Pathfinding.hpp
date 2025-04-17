// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <limits>
#include <unordered_map>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {
////////////////////////////////////////////////////////////

template <typename T>
concept AStarGrid =
    requires(T& t, point_i p) {
        { t.get_cost(p, p) } -> std::same_as<u64>;
    };

////////////////////////////////////////////////////////////

class TCOB_API astar_pathfinding final {
public:
    enum class heuristic {
        Euclidean,
        Manhattan,
        Chebyshev
    };

    explicit astar_pathfinding(bool allowDiagonal = false, heuristic heuristic = heuristic::Manhattan);

    static constexpr u64 IMPASSABLE_COST = std::numeric_limits<u64>::max();

    auto find_path(AStarGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    class TCOB_API node {
    public:
        point_i pos;
        u64     score {};

        auto operator>(node const& other) const -> bool;
    };

    auto distance(point_i a, point_i b) const -> u64;
    auto neighbors(size_i gridSize, point_i pos) const -> std::vector<point_i>;
    auto reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) const -> std::vector<point_i>;

    bool      _allowDiagonal;
    heuristic _heuristic;
};

}

#include "Pathfinding.inl"

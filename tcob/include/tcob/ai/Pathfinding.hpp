// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "tcob/tcob_config.hpp"

#include <limits>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {
////////////////////////////////////////////////////////////

template <typename T>
concept PathGrid =
    requires(T& t, point_i p) {
        { t.get_cost(p, p) } -> std::same_as<u64>;
    };

////////////////////////////////////////////////////////////

class TCOB_API pathfinding final {
public:
    static constexpr u64 IMPASSABLE_COST {std::numeric_limits<u64>::max()};

    enum class heuristic : u8 {
        Euclidean,
        Manhattan,
        Chebyshev
    };

    static auto distance(heuristic h, point_i a, point_i b) -> u64;
    static auto neighbors(bool allowDiagonal, size_i gridSize, point_i pos) -> std::vector<point_i>;

    static auto reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) -> std::vector<point_i>;
    static auto reconstruct_path(grid<point_i> const& cameFrom, point_i current, point_i sentinel) -> std::vector<point_i>;
};

////////////////////////////////////////////////////////////

class TCOB_API astar_pathfinding final {
public:
    explicit astar_pathfinding(bool allowDiagonal = false);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    struct node {
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator>(node const& other) const -> bool { return F > other.F; }
    };

    bool                   _allowDiagonal;
    pathfinding::heuristic _heuristic;
};

////////////////////////////////////////////////////////////

class TCOB_API bidir_astar_pathfinding final {
public:
    explicit bidir_astar_pathfinding(bool allowDiagonal = false);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    struct node {
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator>(node const& other) const -> bool { return F > other.F; }
    };

    bool                   _allowDiagonal;
    pathfinding::heuristic _heuristic;
};

////////////////////////////////////////////////////////////

class TCOB_API thetastar_pathfinding final {
public:
    explicit thetastar_pathfinding(bool allowDiagonal = true);

    auto find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>;

private:
    struct node {
        point_i Pos;
        u64     G {};
        u64     F {};
        auto    operator>(node const& other) const -> bool { return F > other.F; }
    };

    auto has_line_of_sight(PathGrid auto&& testGrid, size_i gridExtent, point_i a, point_i b) const -> bool;

    bool _allowDiagonal;
};

}

#include "Pathfinding.inl"

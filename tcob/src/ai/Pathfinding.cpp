// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/Pathfinding.hpp"

#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {

astar_pathfinding::astar_pathfinding(bool allowDiagonal, heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto astar_pathfinding::distance(point_i a, point_i b) const -> u64
{
    switch (_heuristic) {
    case heuristic::Euclidean: return static_cast<u64>(euclidean_distance(a, b));
    case heuristic::Manhattan: return static_cast<u64>(manhattan_distance(a, b));
    case heuristic::Chebyshev: return static_cast<u64>(chebyshev_distance(a, b));
    }

    return 0;
}

auto astar_pathfinding::neighbors(size_i gridSize, point_i pos) const -> std::vector<point_i>
{
    static constexpr std::array<point_i, 4> orthogonalDirections {{
        {0, 1},  // Down
        {1, 0},  // Right
        {0, -1}, // Up
        {-1, 0}, // Left
    }};

    static constexpr std::array<point_i, 4> diagonalDirections {{
        {1, 1},   // RightDown
        {-1, -1}, // LeftUp
        {1, -1},  // RightUp
        {-1, 1},  // LeftDown
    }};

    std::vector<point_i> retValue;

    for (auto const& dir : orthogonalDirections) {
        point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
        if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
            retValue.push_back(neighbor);
        }
    }

    if (_allowDiagonal) {
        for (auto const& dir : diagonalDirections) {
            point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
            if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
                retValue.push_back(neighbor);
            }
        }
    }

    return retValue;
}

auto astar_pathfinding::reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) const -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    while (cameFrom.contains(current)) {
        retValue.push_back(current);
        current = cameFrom.at(current);
    }
    std::ranges::reverse(retValue);
    return retValue;
}

auto astar_pathfinding::node::operator>(node const& other) const -> bool
{
    return score > other.score;
}
}

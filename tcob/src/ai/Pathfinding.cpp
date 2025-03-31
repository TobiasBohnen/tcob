// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/Pathfinding.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {

auto astar_pathfinding::heuristic(point_i a, point_i b) const -> u64
{
    return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
}

auto astar_pathfinding::get_neighbors(size_i gridSize, point_i pos) const -> std::vector<point_i>
{
    static constexpr std::array<point_i, 4> directions {{
        {0, 1},  // Down
        {1, 0},  // Right
        {0, -1}, // Up
        {-1, 0}, // Left
    }};

    std::vector<point_i> retValue;

    for (auto const& dir : directions) {
        point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
        if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
            retValue.push_back(neighbor);
        }
    }

    return retValue;
}

auto astar_pathfinding::reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) const -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    while (cameFrom.find(current) != cameFrom.end()) {
        retValue.push_back(current);
        current = cameFrom.at(current);
    }
    std::reverse(retValue.begin(), retValue.end());
    return retValue;
}

auto astar_pathfinding::node::operator>(node const& other) const -> bool
{
    return score > other.score;
}
}

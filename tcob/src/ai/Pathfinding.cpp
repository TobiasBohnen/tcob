// Copyright (c) 2024 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/Pathfinding.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace tcob::ai {

auto astar_pathfinding::find_path(grid<u64> const& testGrid, point_i start, point_i finish) -> std::vector<point_i>
{
    if (testGrid[finish] == IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_set<point_i>                                  used;
    std::unordered_map<point_i, point_i>                         cameFrom;

    grid<u64> gScore {testGrid.extent(), std::numeric_limits<u64>::max()};
    grid<u64> fScore {testGrid.extent(), std::numeric_limits<u64>::max()};

    gScore[start] = 0;
    fScore[start] = heuristic(start, finish);

    openSet.push({start, fScore[start]});

    while (!openSet.empty()) {
        point_i const current {openSet.top().pos};
        openSet.pop();

        if (current == finish) {
            return reconstruct_path(cameFrom, current);
        }

        used.insert(current);

        for (auto const& neighbor : get_neighbors(testGrid, current)) {
            if (testGrid[neighbor] == IMPASSABLE_COST || used.find(neighbor) != used.end()) {
                continue;
            }

            u64 const tentative_gScore {gScore[current] + testGrid[neighbor]};

            if (gScore[neighbor] == std::numeric_limits<u64>::max()) {
                openSet.push({neighbor, tentative_gScore + heuristic(neighbor, finish)});
            } else if (tentative_gScore >= gScore[neighbor]) {
                continue;
            }

            cameFrom[neighbor] = current;
            gScore[neighbor]   = tentative_gScore;
            fScore[neighbor]   = tentative_gScore + heuristic(neighbor, finish);
        }
    }

    return {};
}

auto astar_pathfinding::heuristic(point_i a, point_i b) const -> u64
{
    return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
}

auto astar_pathfinding::get_neighbors(grid<u64> const& grid, point_i pos) const -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    std::array<point_i, 4> static const directions {{
        {0, 1},  // Down
        {1, 0},  // Right
        {0, -1}, // Up
        {-1, 0}, // Left
    }};

    for (auto const& dir : directions) {
        point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
        if (neighbor.X >= 0 && neighbor.X < grid.width() && neighbor.Y >= 0 && neighbor.Y < grid.height()) {
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

// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Pathfinding.hpp"

#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {

auto astar_pathfinding::find_path(AStarGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start) == IMPASSABLE_COST || testGrid.get_cost(finish) == IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;

    grid<u64> gScore {gridExtent, std::numeric_limits<u64>::max()};
    grid<u64> fScore {gridExtent, std::numeric_limits<u64>::max()};

    gScore[start] = 0;
    fScore[start] = distance(start, finish);

    openSet.push({start, fScore[start]});

    while (!openSet.empty()) {
        point_i const current {openSet.top().pos};
        openSet.pop();

        if (current == finish) {
            return reconstruct_path(cameFrom, current);
        }

        // Skip if we already found a better path to this node
        if (gScore[current] == IMPASSABLE_COST) {
            continue;
        }

        for (auto const& neighbor : neighbors(gridExtent, current)) {
            if (testGrid.get_cost(neighbor) == IMPASSABLE_COST) {
                continue;
            }

            u64 const tentative_gScore {gScore[current] + testGrid.get_cost(neighbor)};

            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor]   = tentative_gScore;
                fScore[neighbor]   = tentative_gScore + distance(neighbor, finish);
                openSet.push({neighbor, fScore[neighbor]});
            }
        }
    }

    return {}; // No path found
}
}

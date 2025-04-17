// Copyright (c) 2025 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Pathfinding.hpp"

#include <functional>
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
    if (testGrid.get_cost(start, start) == IMPASSABLE_COST || testGrid.get_cost(finish, finish) == IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;

    grid<u64> gScore {gridExtent, IMPASSABLE_COST};
    grid<u64> fScore {gridExtent, IMPASSABLE_COST};

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
        if (gScore[current] == IMPASSABLE_COST) { continue; }

        for (auto const& neighbor : neighbors(gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == IMPASSABLE_COST) { continue; }

            u64 const tentative_gScore {gScore[current] + cost};

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

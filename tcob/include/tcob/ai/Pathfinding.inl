// Copyright (c) 2026 Tobias Bohnen
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
    gScore[start] = 0;
    openSet.push({.Pos = start, .Dir = {0, 0}, .G = 0, .F = distance(start, finish)});

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        if (top.G > gScore[top.Pos]) { continue; } // stale entry

        point_i const current {top.Pos};

        if (current == finish) {
            return reconstruct_path(cameFrom, current);
        }

        for (auto const& neighbor : neighbors(gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == IMPASSABLE_COST) { continue; }
            if (gScore[current] > IMPASSABLE_COST - cost) { continue; } // overflow guard

            point_i const dir {neighbor.X - current.X, neighbor.Y - current.Y};
            u64 const     turnPenalty {top.Dir != point_i {0, 0} && dir != top.Dir ? _turnPenalty : 0};
            u64 const     tentative {gScore[current] + cost + turnPenalty};

            if (tentative < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor]   = tentative;
                openSet.push({.Pos = neighbor, .Dir = dir, .G = tentative, .F = tentative + distance(neighbor, finish)});
            }
        }
    }

    return {}; // No path found
}

}

// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Pathfinding.hpp"

#include <cstdlib>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {

auto astar_pathfinding::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == pathfinding::IMPASSABLE_COST || testGrid.get_cost(finish, finish) == pathfinding::IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;
    grid<u64>                                                    gScore {gridExtent, pathfinding::IMPASSABLE_COST};

    gScore[start] = 0;
    openSet.push({.Pos = start, .G = 0, .F = pathfinding::distance(_heuristic, start, finish)});

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        if (top.G > gScore[top.Pos]) { continue; }

        point_i const current {top.Pos};

        if (current == finish) {
            return pathfinding::reconstruct_path(cameFrom, current);
        }

        for (auto const& neighbor : pathfinding::neighbors(_allowDiagonal, gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == pathfinding::IMPASSABLE_COST) { continue; }
            if (gScore[current] > pathfinding::IMPASSABLE_COST - cost) { continue; }
            u64 const tentative_g {gScore[current] + cost};

            if (tentative_g < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor]   = tentative_g;
                openSet.push({.Pos = neighbor, .G = tentative_g, .F = tentative_g + pathfinding::distance(_heuristic, neighbor, finish)});
            }
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

auto thetastar_pathfinding::has_line_of_sight(PathGrid auto&& testGrid, size_i gridExtent, point_i a, point_i b) const -> bool
{
    auto blocked {[&](i32 x, i32 y) -> bool {
        if (x < 0 || x >= gridExtent.Width || y < 0 || y >= gridExtent.Height) { return true; }
        return testGrid.get_cost({x, y}, {x, y}) == pathfinding::IMPASSABLE_COST;
    }};

    auto [x0, y0] {a};
    auto [x1, y1] {b};

    i32 const dx {std::abs(x1 - x0)};
    i32 const dy {std::abs(y1 - y0)};
    i32 const sx {x0 < x1 ? 1 : -1};
    i32 const sy {y0 < y1 ? 1 : -1};
    i32       err {dx - dy};

    for (;;) {
        if (blocked(x0, y0)) { return false; }
        if (x0 == x1 && y0 == y1) { return true; }

        i32 const  e2 {2 * err};
        bool const stepX {e2 > -dy};
        bool const stepY {e2 < dx};

        if (stepX && stepY) {
            if (blocked(x0 + sx, y0) || blocked(x0, y0 + sy)) { return false; }
        }

        if (stepX) {
            err -= dy;
            x0 += sx;
        }
        if (stepY) {
            err += dx;
            y0 += sy;
        }
    }
}

auto thetastar_pathfinding::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == pathfinding::IMPASSABLE_COST || testGrid.get_cost(finish, finish) == pathfinding::IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;
    grid<u64>                                                    gScore {gridExtent, pathfinding::IMPASSABLE_COST};

    gScore[start] = 0;
    openSet.push({.Pos = start, .G = 0, .F = pathfinding::distance(pathfinding::heuristic::Euclidean, start, finish)});

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        if (top.G > gScore[top.Pos]) { continue; }

        point_i const current {top.Pos};

        if (current == finish) {
            return pathfinding::reconstruct_path(cameFrom, current);
        }

        for (auto const& neighbor : pathfinding::neighbors(_allowDiagonal, gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == pathfinding::IMPASSABLE_COST) { continue; }
            if (gScore[current] > pathfinding::IMPASSABLE_COST - cost) { continue; }

            point_i const parent {cameFrom.contains(current) ? cameFrom.at(current) : current};
            if (has_line_of_sight(testGrid, gridExtent, parent, neighbor)) {
                u64 const tentative_g {gScore[parent] + pathfinding::distance(pathfinding::heuristic::Euclidean, parent, neighbor)};
                if (tentative_g < gScore[neighbor]) {
                    cameFrom[neighbor] = parent;
                    gScore[neighbor]   = tentative_g;
                    openSet.push({.Pos = neighbor, .G = tentative_g, .F = tentative_g + pathfinding::distance(pathfinding::heuristic::Euclidean, neighbor, finish)});
                }
            } else {
                u64 const tentative_g {gScore[current] + cost};
                if (tentative_g < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor]   = tentative_g;
                    openSet.push({.Pos = neighbor, .G = tentative_g, .F = tentative_g + pathfinding::distance(pathfinding::heuristic::Euclidean, neighbor, finish)});
                }
            }
        }
    }

    return {};
}

}

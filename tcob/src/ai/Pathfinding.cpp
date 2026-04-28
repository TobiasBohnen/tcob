// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/Pathfinding.hpp"

#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai {

auto pathfinding::distance(heuristic h, point_i a, point_i b) -> u64
{
    switch (h) {
    case heuristic::Euclidean: return static_cast<u64>(euclidean_distance(a, b));
    case heuristic::Manhattan: return static_cast<u64>(manhattan_distance(a, b));
    case heuristic::Chebyshev: return static_cast<u64>(chebyshev_distance(a, b));
    }
    return 0;
}

auto pathfinding::neighbors(bool allowDiagonal, size_i gridSize, point_i pos) -> std::vector<point_i>
{
    static constexpr std::array<point_i, 4> orthogonalDirections {{
        {0, 1},
        {1, 0},
        {0, -1},
        {-1, 0},
    }};
    static constexpr std::array<point_i, 4> diagonalDirections {{
        {1, 1},
        {-1, -1},
        {1, -1},
        {-1, 1},
    }};

    std::vector<point_i> retValue;
    for (auto const& dir : orthogonalDirections) {
        point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
        if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
            retValue.push_back(neighbor);
        }
    }
    if (allowDiagonal) {
        for (auto const& dir : diagonalDirections) {
            point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
            if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
                retValue.push_back(neighbor);
            }
        }
    }
    return retValue;
}

auto pathfinding::reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    while (cameFrom.contains(current)) {
        retValue.push_back(current);
        current = cameFrom.at(current);
    }
    std::ranges::reverse(retValue);
    return retValue;
}

auto pathfinding::reconstruct_path(grid<point_i> const& cameFrom, point_i current, point_i sentinel) -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    while (cameFrom[current] != sentinel) {
        retValue.push_back(current);
        current = cameFrom[current];
    }
    std::ranges::reverse(retValue);
    return retValue;
}

////////////////////////////////////////////////////////////

astar_pathfinding::astar_pathfinding(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {allowDiagonal ? pathfinding::heuristic::Chebyshev : pathfinding::heuristic::Manhattan}
{
}

////////////////////////////////////////////////////////////

bidir_astar_pathfinding::bidir_astar_pathfinding(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {allowDiagonal ? pathfinding::heuristic::Chebyshev : pathfinding::heuristic::Manhattan}
{
}

////////////////////////////////////////////////////////////

thetastar_pathfinding::thetastar_pathfinding(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
{
}

}

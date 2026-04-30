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

namespace tcob::ai::pathfinding {

namespace detail {
    auto distance(heuristic h, point_i a, point_i b) -> u64
    {
        switch (h) {
        case heuristic::Euclidean: return static_cast<u64>(euclidean_distance(a, b));
        case heuristic::Manhattan: return static_cast<u64>(manhattan_distance(a, b));
        case heuristic::Chebyshev: return static_cast<u64>(chebyshev_distance(a, b));
        }
        return 0;
    }

    auto neighbors(bool allowDiagonal, size_i gridSize, point_i pos) -> std::vector<point_i>
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

    auto reconstruct_path(std::unordered_map<point_i, point_i> const& cameFrom, point_i current) -> std::vector<point_i>
    {
        std::vector<point_i> retValue;
        while (cameFrom.contains(current)) {
            retValue.push_back(current);
            current = cameFrom.at(current);
        }
        std::ranges::reverse(retValue);
        return retValue;
    }
}

////////////////////////////////////////////////////////////

astar::astar(bool allowDiagonal, pathfinding::heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

////////////////////////////////////////////////////////////

bidir_astar::bidir_astar(bool allowDiagonal, pathfinding::heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto bidir_astar::reconstruct_path(grid<point_i> const& cameFrom, point_i current, point_i sentinel) -> std::vector<point_i>
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

minturns::minturns(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
{
}

////////////////////////////////////////////////////////////

thetastar::thetastar(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
{
}

////////////////////////////////////////////////////////////

lpastar::lpastar(bool allowDiagonal, pathfinding::heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto lpastar::calculate_key(point_i p) const -> key
{
    u64 const gVal {_g[p]};
    u64 const rhsVal {_rhs[p]};
    u64 const minVal {std::min(gVal, rhsVal)};

    u64 const h {pathfinding::detail::distance(_heuristic, p, _finish)};

    u64 const k1 {(minVal >= pathfinding::IMPASSABLE_COST - h)
                      ? pathfinding::IMPASSABLE_COST
                      : minVal + h};

    return {.K1 = k1, .K2 = minVal};
}

void lpastar::rebuild_path()
{
    _path.clear();
    if (_g[_finish] >= pathfinding::IMPASSABLE_COST) { return; }

    point_i cur {_finish};
    while (cur != _start) {
        _path.push_back(cur);
        cur = _parent[cur];
        if (cur == point_i {-1, -1}) {
            _path.clear();
            return;
        }
    }
    std::ranges::reverse(_path);
}

auto lpastar::path() const -> std::vector<point_i> const& { return _path; }

auto lpastar::node::operator<(node const& other) const -> bool
{
    if (Key != other.Key) { return Key < other.Key; }
    if (Pos.X != other.Pos.X) { return Pos.X < other.Pos.X; }
    return Pos.Y < other.Pos.Y;
}

////////////////////////////////////////////////////////////

dstar_lite::dstar_lite(bool allowDiagonal, pathfinding::heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto dstar_lite::calculate_key(point_i p) const -> key
{
    u64 const gVal {_g[p]};
    u64 const rhsVal {_rhs[p]};
    u64 const minVal {std::min(gVal, rhsVal)};
    u64 const h {pathfinding::detail::distance(_heuristic, p, _current)};
    u64 const k1 {(minVal >= pathfinding::IMPASSABLE_COST - h)
                      ? pathfinding::IMPASSABLE_COST
                      : minVal + h + _km};
    return {.K1 = k1, .K2 = minVal};
}

auto dstar_lite::path() const -> std::vector<point_i> const&
{
    return _path;
}

auto dstar_lite::position() const -> point_i
{
    return _current;
}

void dstar_lite::rebuild_path()
{
    _path.clear();
    if (_g[_current] >= pathfinding::IMPASSABLE_COST) { return; }

    point_i cur {_current};
    while (cur != _finish) {
        _path.push_back(cur);
        cur = _parent[cur];
        if (cur == point_i {-1, -1}) {
            _path.clear();
            return;
        }
    }

    if (!_path.empty()) { _path.erase(_path.begin()); }
}

auto dstar_lite::node::operator<(node const& other) const -> bool
{
    if (Key != other.Key) { return Key < other.Key; }
    if (Pos.X != other.Pos.X) { return Pos.X < other.Pos.X; }
    return Pos.Y < other.Pos.Y;
}

}

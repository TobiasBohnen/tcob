// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "tcob/ai/Pathfinding.hpp"

#include <algorithm>
#include <array>
#include <compare>
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

    auto neighbors(bool allowDiagonal, size_i gridSize, point_i pos) -> neighbor_list
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

        neighbor_list retValue;
        for (auto const& dir : orthogonalDirections) {
            point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
            if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
                retValue.Data[retValue.Count++] = neighbor;
            }
        }
        if (allowDiagonal) {
            for (auto const& dir : diagonalDirections) {
                point_i const neighbor {pos.X + dir.X, pos.Y + dir.Y};
                if (neighbor.X >= 0 && neighbor.X < gridSize.Width && neighbor.Y >= 0 && neighbor.Y < gridSize.Height) {
                    retValue.Data[retValue.Count++] = neighbor;
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
    auto neighbor_list::begin() const -> data::const_iterator { return Data.begin(); }
    auto neighbor_list::end() const -> data::const_iterator { return Data.begin() + Count; }
}

////////////////////////////////////////////////////////////

astar::astar(bool allowDiagonal, heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto astar::node::operator<=>(node const& other) const -> std::strong_ordering { return F <=> other.F; }

////////////////////////////////////////////////////////////

bidir_astar::bidir_astar(bool allowDiagonal, heuristic heuristic)
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

auto bidir_astar::node::operator<=>(node const& other) const -> std::strong_ordering { return F <=> other.F; }

////////////////////////////////////////////////////////////

astar_minturns::astar_minturns(bool allowDiagonal, heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto astar_minturns::node::operator<=>(node const& other) const -> std::strong_ordering { return F <=> other.F; }

////////////////////////////////////////////////////////////

thetastar::thetastar(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
{
}

auto thetastar::node::operator<=>(node const& other) const -> std::strong_ordering { return F <=> other.F; }

////////////////////////////////////////////////////////////

lpastar::lpastar(bool allowDiagonal, heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto lpastar::calculate_key(point_i p) const -> key
{
    u64 const gVal {_g[p]};
    u64 const rhsVal {_rhs[p]};
    u64 const minVal {std::min(gVal, rhsVal)};

    u64 const h {detail::distance(_heuristic, p, _finish)};

    u64 const k1 {(minVal >= IMPASSABLE_COST - h)
                      ? IMPASSABLE_COST
                      : minVal + h};

    return {.K1 = k1, .K2 = minVal};
}

void lpastar::rebuild_path()
{
    _path.clear();
    if (_g[_finish] >= IMPASSABLE_COST) { return; }

    point_i cur {_finish};
    while (cur != _start) {
        _path.push_back(cur);
        cur = _parent[cur];
        if (cur == INVALID_POS) {
            _path.clear();
            return;
        }
    }
    std::ranges::reverse(_path);
}

auto lpastar::path() const -> std::vector<point_i> const& { return _path; }

auto lpastar::node::operator<=>(node const& other) const -> std::strong_ordering
{
    if (Key != other.Key) { return Key <=> other.Key; }
    if (Pos.X != other.Pos.X) { return Pos.X <=> other.Pos.X; }
    return Pos.Y <=> other.Pos.Y;
}

////////////////////////////////////////////////////////////

dstar_lite::dstar_lite(bool allowDiagonal, heuristic heuristic)
    : _allowDiagonal {allowDiagonal}
    , _heuristic {heuristic}
{
}

auto dstar_lite::calculate_key(point_i p) const -> key
{
    u64 const gVal {_g[p]};
    u64 const rhsVal {_rhs[p]};
    u64 const minVal {std::min(gVal, rhsVal)};
    u64 const h {detail::distance(_heuristic, p, _current)};
    u64 const k1 {(minVal >= IMPASSABLE_COST - h)
                      ? IMPASSABLE_COST
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
    if (_g[_current] >= IMPASSABLE_COST) { return; }

    point_i   cur {_current};
    i32 const maxSteps {_gridExtent.area()};
    i32       steps {0};

    while (cur != _finish) {
        if (++steps > maxSteps) {
            _path.clear();
            return;
        }
        _path.push_back(cur);
        cur = _parent[cur];
        if (cur == INVALID_POS) {
            _path.clear();
            return;
        }
    }

    if (!_path.empty()) { _path.erase(_path.begin()); }
}

auto dstar_lite::node::operator<=>(node const& other) const -> std::strong_ordering
{
    if (Key != other.Key) { return Key <=> other.Key; }
    if (Pos.X != other.Pos.X) { return Pos.X <=> other.Pos.X; }
    return Pos.Y <=> other.Pos.Y;
}

////////////////////////////////////////////////////////////

flow_field::flow_field(bool allowDiagonal)
    : _allowDiagonal {allowDiagonal}
{
}

auto flow_field::direction(point_i from) const -> point_i
{
    if (!_flow.size().contains(from)) { return INVALID_DIR; }
    return _flow[from];
}

auto flow_field::path(point_i from) const -> std::vector<point_i>
{
    std::vector<point_i> retValue;
    point_i              cur {from};

    while (cur != _finish) {
        point_i const n {cur + direction(cur)};
        if (n == INVALID_DIR || n.X < 0 || n.Y < 0) { return {}; }
        retValue.push_back(n);
        cur = n;
    }

    return retValue;
}

auto flow_field::node::operator<=>(node const& other) const -> std::strong_ordering
{
    return Cost <=> other.Cost;
}

}

// Copyright (c) 2026 Tobias Bohnen
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once
#include "Pathfinding.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>

#include "tcob/core/Grid.hpp"
#include "tcob/core/Point.hpp"
#include "tcob/core/Size.hpp"

namespace tcob::ai::pathfinding {

////////////////////////////////////////////////////////////

auto astar::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == IMPASSABLE_COST || testGrid.get_cost(finish, finish) == IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;
    grid<u64>                                                    gScore {gridExtent, IMPASSABLE_COST};

    gScore[start] = 0;
    openSet.push({.Pos = start, .G = 0, .F = detail::distance(_heuristic, start, finish)});

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        if (top.G > gScore[top.Pos]) { continue; }

        point_i const current {top.Pos};

        if (current == finish) {
            return detail::reconstruct_path(cameFrom, current);
        }

        for (auto const& neighbor : detail::neighbors(_allowDiagonal, gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == IMPASSABLE_COST) { continue; }
            if (gScore[current] > IMPASSABLE_COST - cost) { continue; }
            u64 const g {gScore[current] + cost};

            if (g < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor]   = g;
                openSet.push({.Pos = neighbor, .G = g, .F = g + detail::distance(_heuristic, neighbor, finish)});
            }
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

auto bidir_astar::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == IMPASSABLE_COST || testGrid.get_cost(finish, finish) == IMPASSABLE_COST) { return {}; }

    static constexpr point_i SENTINEL {-1, -1};

    using open_set = std::priority_queue<node, std::vector<node>, std::greater<>>;
    open_set      openF, openB;
    grid<point_i> cameFromF {gridExtent, SENTINEL};
    grid<point_i> cameFromB {gridExtent, SENTINEL};
    grid<u64>     gScoreF {gridExtent, IMPASSABLE_COST};
    grid<u64>     gScoreB {gridExtent, IMPASSABLE_COST};

    gScoreF[start]  = 0;
    gScoreB[finish] = 0;
    openF.push({.Pos = start, .G = 0, .F = detail::distance(_heuristic, start, finish)});
    openB.push({.Pos = finish, .G = 0, .F = detail::distance(_heuristic, finish, start)});

    u64     bestCost {IMPASSABLE_COST};
    point_i meetingPt {SENTINEL};

    auto const expand {[&](open_set& open, grid<u64>& gScore, grid<u64>& gScoreOther, grid<point_i>& cameFrom, point_i target) {
        if (open.empty()) { return; }

        node const top {open.top()};
        open.pop();

        if (top.G > gScore[top.Pos]) { return; }

        point_i const current {top.Pos};

        for (auto const& neighbor : detail::neighbors(_allowDiagonal, gridExtent, current)) {
            auto const cost {testGrid.get_cost(current, neighbor)};
            if (cost == IMPASSABLE_COST) { continue; }
            if (gScore[current] > (IMPASSABLE_COST - cost)) { continue; }

            u64 const g {gScore[current] + cost};
            if (g < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor]   = g;
                u64 const h {detail::distance(_heuristic, neighbor, target)};
                open.push({.Pos = neighbor, .G = g, .F = g + h});

                if (gScoreOther[neighbor] != IMPASSABLE_COST) {
                    u64 const candidate {g + gScoreOther[neighbor]};
                    if (candidate < bestCost) {
                        bestCost  = candidate;
                        meetingPt = neighbor;
                    }
                }
            }
        }
    }};

    for (;;) {
        if (openF.empty() && openB.empty()) { break; }

        u64 const minF {openF.empty() ? IMPASSABLE_COST : openF.top().F};
        u64 const minB {openB.empty() ? IMPASSABLE_COST : openB.top().F};
        u64 const minSum {(minF > (IMPASSABLE_COST - minB)) ? IMPASSABLE_COST : minF + minB};
        if (minSum >= bestCost) { break; }

        if (minF <= minB) {
            expand(openF, gScoreF, gScoreB, cameFromF, finish);
        } else {
            expand(openB, gScoreB, gScoreF, cameFromB, start);
        }
    }

    if (meetingPt == SENTINEL) { return {}; }

    std::vector<point_i> path {reconstruct_path(cameFromF, meetingPt, SENTINEL)};

    point_i cur {meetingPt};
    while (cameFromB[cur] != SENTINEL) {
        cur = cameFromB[cur];
        path.push_back(cur);
    }

    return path;
}

////////////////////////////////////////////////////////////

auto astar_minturns::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == IMPASSABLE_COST || testGrid.get_cost(finish, finish) == IMPASSABLE_COST) { return {}; }

    static constexpr point_i SENTINEL {-1, -1};
    static constexpr i32     DIR_COUNT {8};

    u64 const    area {static_cast<u64>(gridExtent.area()) + 1};
    size_i const stateExtent {gridExtent.Width * DIR_COUNT, gridExtent.Height};

    static auto const idx {[](point_i pos, point_i dir) -> point_i {
        static constexpr std::array<point_i, 8> DIRS {{{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}}};
        for (i32 i {0}; i < 8; ++i) {
            if (DIRS[i] == dir) { return {(pos.X * DIR_COUNT) + i, pos.Y}; }
        }
        return {(pos.X * DIR_COUNT), pos.Y};
    }};

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    grid<u64>                                                    gScore {stateExtent, IMPASSABLE_COST};
    grid<state>                                                  cameFrom {stateExtent, {.Pos = SENTINEL, .Dir = {0, 0}}};

    for (auto const& neighbor : detail::neighbors(_allowDiagonal, gridExtent, start)) {
        auto const cost {testGrid.get_cost(start, neighbor)};
        if (cost == IMPASSABLE_COST) { continue; }
        point_i const dir {neighbor.X - start.X, neighbor.Y - start.Y};
        state const   next {.Pos = neighbor, .Dir = dir};
        point_i const nidx {idx(neighbor, dir)};
        u64 const     tentative {cost}; // no turn on first step
        if (tentative < gScore[nidx]) {
            cameFrom[nidx] = {.Pos = start, .Dir = dir};
            gScore[nidx]   = tentative;
            u64 const h {detail::distance(_heuristic, neighbor, finish)};
            openSet.push({.State = next, .G = tentative, .F = tentative + h});
        }
    }

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        state const   cur {top.State};
        point_i const cidx {idx(cur.Pos, cur.Dir)};

        if (top.G > gScore[cidx]) { continue; }

        if (cur.Pos == finish) {
            std::vector<point_i> path;
            state                s {cur};
            while (s.Pos != start) {
                path.push_back(s.Pos);
                s = cameFrom[idx(s.Pos, s.Dir)];
                if (s.Pos == SENTINEL) { return {}; }
            }
            std::ranges::reverse(path);
            return path;
        }

        for (auto const& neighbor : detail::neighbors(_allowDiagonal, gridExtent, cur.Pos)) {
            auto const cost {testGrid.get_cost(cur.Pos, neighbor)};
            if (cost == IMPASSABLE_COST) { continue; }

            point_i const dir {neighbor.X - cur.Pos.X, neighbor.Y - cur.Pos.Y};
            u64 const     turn {dir != cur.Dir ? 1u : 0u};
            u64 const     dg {(turn * area) + cost};
            if (top.G > IMPASSABLE_COST - dg) { continue; }

            u64 const     g {top.G + dg};
            state const   next {.Pos = neighbor, .Dir = dir};
            point_i const nidx {idx(neighbor, dir)};

            if (g < gScore[nidx]) {
                cameFrom[nidx] = cur;
                gScore[nidx]   = g;
                u64 const h {detail::distance(_heuristic, neighbor, finish)};
                openSet.push({.State = next, .G = g, .F = g + h});
            }
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

auto thetastar::has_line_of_sight(PathGrid auto&& testGrid, size_i gridExtent, point_i a, point_i b) const -> bool
{
    auto blocked {[&](i32 x, i32 y) -> bool {
        if (x < 0 || x >= gridExtent.Width || y < 0 || y >= gridExtent.Height) { return true; }
        return testGrid.get_cost({x, y}, {x, y}) == IMPASSABLE_COST;
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

auto thetastar::find_path(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish) -> std::vector<point_i>
{
    if (start == finish) { return {start}; }
    if (testGrid.get_cost(start, start) == IMPASSABLE_COST || testGrid.get_cost(finish, finish) == IMPASSABLE_COST) { return {}; }

    std::priority_queue<node, std::vector<node>, std::greater<>> openSet;
    std::unordered_map<point_i, point_i>                         cameFrom;
    grid<u64>                                                    gScore {gridExtent, IMPASSABLE_COST};

    gScore[start] = 0;
    openSet.push({.Pos = start, .G = 0, .F = detail::distance(_heuristic, start, finish)});

    while (!openSet.empty()) {
        node const top {openSet.top()};
        openSet.pop();

        if (top.G > gScore[top.Pos]) { continue; }

        point_i const current {top.Pos};

        if (current == finish) {
            return detail::reconstruct_path(cameFrom, current);
        }

        for (auto const& neighbor : detail::neighbors(_allowDiagonal, gridExtent, current)) {
            auto const neighborCost {testGrid.get_cost(current, neighbor)};
            if (neighborCost == IMPASSABLE_COST) { continue; }

            auto          it {cameFrom.find(current)};
            point_i const parent {(it != cameFrom.end()) ? it->second : current};

            if (has_line_of_sight(testGrid, gridExtent, parent, neighbor)) {
                u64 const d {detail::distance(_heuristic, parent, neighbor)};
                if (gScore[parent] > (IMPASSABLE_COST - d)) { continue; }
                u64 const g {gScore[parent] + d};
                if (g < gScore[neighbor]) {
                    cameFrom[neighbor] = parent;
                    gScore[neighbor]   = g;
                    u64 const h {detail::distance(_heuristic, neighbor, finish)};
                    openSet.push({.Pos = neighbor, .G = g, .F = g + h});
                }
            } else {
                u64 const d {detail::distance(_heuristic, current, neighbor)};
                if (gScore[current] > (IMPASSABLE_COST - d)) { continue; }
                u64 const g {gScore[current] + d};
                if (g < gScore[neighbor]) {
                    cameFrom[neighbor] = current;
                    gScore[neighbor]   = g;
                    u64 const h {detail::distance(_heuristic, neighbor, finish)};
                    openSet.push({.Pos = neighbor, .G = g, .F = g + h});
                }
            }
        }
    }

    return {};
}

////////////////////////////////////////////////////////////

inline void lpastar::initialize(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish)
{
    _gridExtent = gridExtent;
    _start      = start;
    _finish     = finish;

    _g       = grid<u64> {gridExtent, IMPASSABLE_COST};
    _rhs     = grid<u64> {gridExtent, IMPASSABLE_COST};
    _parent  = grid<point_i> {gridExtent, {-1, -1}};
    _nodeKey = grid<key> {gridExtent, {.K1 = IMPASSABLE_COST, .K2 = IMPASSABLE_COST}};
    _inOpen  = grid<bool> {gridExtent, false};

    _open.clear();

    _rhs[_start]     = 0;
    _inOpen[_start]  = true;
    _nodeKey[_start] = calculate_key(_start);
    _open.insert({.Pos = _start, .Key = _nodeKey[_start]});

    compute_shortest_path(testGrid);
}

inline void lpastar::update(PathGrid auto&& testGrid, point_i changedTile)
{
    update_vertex(testGrid, changedTile);
    for (auto const& neighbor : detail::neighbors(_allowDiagonal, _gridExtent, changedTile)) {
        update_vertex(testGrid, neighbor);
    }
    compute_shortest_path(testGrid);
}

inline void lpastar::update_vertex(PathGrid auto&& testGrid, point_i p)
{
    if (_inOpen[p]) {
        _open.erase({.Pos = p, .Key = _nodeKey[p]});
        _inOpen[p] = false;
    }

    if (p != _start) {
        u64     minRhs {IMPASSABLE_COST};
        point_i bestPar {-1, -1};
        for (auto const& pred : detail::neighbors(_allowDiagonal, _gridExtent, p)) {
            u64 const cost {testGrid.get_cost(pred, p)};
            if (cost == IMPASSABLE_COST) { continue; }

            u64 const gVal {_g[pred]};
            u64 const tentative {(gVal >= IMPASSABLE_COST - cost)
                                     ? IMPASSABLE_COST
                                     : gVal + cost};

            if (tentative < minRhs) {
                minRhs  = tentative;
                bestPar = pred;
            }
        }
        _rhs[p]    = minRhs;
        _parent[p] = bestPar;
    }

    if (_g[p] != _rhs[p]) {
        _nodeKey[p] = calculate_key(p);
        _open.insert({.Pos = p, .Key = _nodeKey[p]});
        _inOpen[p] = true;
    }
}

inline void lpastar::compute_shortest_path(PathGrid auto&& testGrid)
{
    while (!_open.empty()) {
        auto const& top {*_open.begin()};
        key const   finishKey {calculate_key(_finish)};

        if (top.Key >= finishKey && _rhs[_finish] == _g[_finish]) {
            break;
        }

        point_i const u {top.Pos};
        _open.erase(_open.begin());
        _inOpen[u] = false;

        if (_g[u] > _rhs[u]) {
            _g[u] = _rhs[u];
            for (auto const& s : detail::neighbors(_allowDiagonal, _gridExtent, u)) {
                update_vertex(testGrid, s);
            }
        } else {
            _g[u] = IMPASSABLE_COST;
            update_vertex(testGrid, u);
            for (auto const& s : detail::neighbors(_allowDiagonal, _gridExtent, u)) {
                update_vertex(testGrid, s);
            }
        }
    }
    rebuild_path();
}

////////////////////////////////////////////////////////////

inline void dstar_lite::initialize(PathGrid auto&& testGrid, size_i gridExtent, point_i start, point_i finish)
{
    _gridExtent = gridExtent;
    _start      = start;
    _finish     = finish;
    _current    = start;
    _last       = start;
    _km         = 0;

    _g       = grid<u64> {gridExtent, IMPASSABLE_COST};
    _rhs     = grid<u64> {gridExtent, IMPASSABLE_COST};
    _parent  = grid<point_i> {gridExtent, {-1, -1}};
    _nodeKey = grid<key> {gridExtent, {.K1 = IMPASSABLE_COST, .K2 = IMPASSABLE_COST}};
    _inOpen  = grid<bool> {gridExtent, false};
    _open.clear();

    _rhs[_finish]     = 0;
    _nodeKey[_finish] = calculate_key(_finish);
    _open.insert({.Pos = _finish, .Key = _nodeKey[_finish]});
    _inOpen[_finish] = true;

    compute_shortest_path(testGrid);
}

inline void dstar_lite::update(PathGrid auto&& testGrid, point_i changedTile)
{
    update_vertex(testGrid, changedTile);
    for (auto const& neighbor : detail::neighbors(_allowDiagonal, _gridExtent, changedTile)) {
        update_vertex(testGrid, neighbor);
    }
    compute_shortest_path(testGrid);
}

inline void dstar_lite::move(PathGrid auto&& testGrid)
{
    if (_path.empty() || _current == _finish) { return; }

    point_i const next {_path.front()};

    u64 const h {detail::distance(_heuristic, _last, _current)};
    _km      = (_km >= IMPASSABLE_COST - h) ? IMPASSABLE_COST : _km + h;
    _last    = _current;
    _current = next;

    compute_shortest_path(testGrid);
}

inline void dstar_lite::update_vertex(PathGrid auto&& testGrid, point_i p)
{
    if (_inOpen[p]) {
        _open.erase({.Pos = p, .Key = _nodeKey[p]});
        _inOpen[p] = false;
    }

    if (p != _finish) {
        u64     minRhs {IMPASSABLE_COST};
        point_i bestPar {-1, -1};
        for (auto const& succ : detail::neighbors(_allowDiagonal, _gridExtent, p)) {
            u64 const cost {testGrid.get_cost(p, succ)}; // directed: p -> succ
            if (cost == IMPASSABLE_COST) { continue; }
            u64 const gVal {_g[succ]};
            u64 const tentative {(gVal >= IMPASSABLE_COST - cost)
                                     ? IMPASSABLE_COST
                                     : gVal + cost};
            if (tentative < minRhs) {
                minRhs  = tentative;
                bestPar = succ;
            }
        }
        _rhs[p]    = minRhs;
        _parent[p] = bestPar;
    }

    if (_g[p] != _rhs[p]) {
        _nodeKey[p] = calculate_key(p);
        _open.insert({.Pos = p, .Key = _nodeKey[p]});
        _inOpen[p] = true;
    }
}

inline void dstar_lite::compute_shortest_path(PathGrid auto&& testGrid)
{
    while (!_open.empty()) {
        auto const& top {*_open.begin()};
        key const   startKey {calculate_key(_current)};

        if (top.Key >= startKey && _rhs[_current] == _g[_current]) { break; }

        point_i const u {top.Pos};
        key const     oldKey {top.Key};
        key const     newKey {calculate_key(u)};

        _open.erase(_open.begin());
        _inOpen[u] = false;

        if (oldKey < newKey) {
            _nodeKey[u] = newKey;
            _open.insert({.Pos = u, .Key = newKey});
            _inOpen[u] = true;
        } else if (_g[u] > _rhs[u]) {
            _g[u] = _rhs[u];
            for (auto const& pred : detail::neighbors(_allowDiagonal, _gridExtent, u)) {
                update_vertex(testGrid, pred);
            }
        } else {
            _g[u] = IMPASSABLE_COST;
            update_vertex(testGrid, u);
            for (auto const& pred : detail::neighbors(_allowDiagonal, _gridExtent, u)) {
                update_vertex(testGrid, pred);
            }
        }
    }
    rebuild_path();
}

}
